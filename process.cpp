/* 
 * File:   process.cpp
 * Author: oc
 * 
 * Created on January 15, 2011, 2:56 PM
 */

#include "globals.h"
#include "process.h"
#include "messagesMgr.h"
#include "mutexMgr.h"

#ifdef ENABLE_X11_MANAGEMENT
#include "xwindowMngt.h"
#endif  //ENABLE_X11_MANAGEMENT

#include <pthread.h>
#include <signal.h>
#include <sys/wait.h>
#include <sstream>

#include "singleton.h" // for its stopAllRunningProcess function

#define MODULE_FLAG FLAG_MONITOR

inline int Process::stopExec() {
    int error(EXIT_SUCCESS);

    if (childProcess != 0) {
         killedByMgr = true;
        if (state != killing) {
            state = killing;
            if (kill(childProcess,SIGTERM) != 0) {
                error = errno;
                ERROR_MSG("kill %d SIGTERM error %d (%m)",childProcess,error);
            }
        } else {
            if (kill(childProcess,SIGKILL) != 0) {
                error = errno;
                ERROR_MSG("kill %d SIGTERM error %d (%m)",childProcess,error);
            }
        }
    } else {
        DEBUG_MSG("pid childProcess is 0");
    }

    return error;
}

Process::~Process() {
    // disabled SIGCHLD processing, else the signal will use a freed process object !
    sigset_t sigSet;    
    sigemptyset(&sigSet);
    sigaddset(&sigSet,SIGCHLD);
    if (sigprocmask(SIG_BLOCK,&sigSet,NULL) != 0) {
        const int error = errno;
        ERROR_MSG("sigprocmask error %d (%m)",error);
    }
    
    // now when can stop it..
    stopExec();
    pthread_mutex_destroy(&runMutex);
}

inline int Process::toggleExec() {
    int error(EXIT_SUCCESS);
    //DEBUG_VAR(childProcess,"%u");
    DUMP();
	const std::string program = theConfiguration.getProgram();
	bool isRunning(false);
	error = processIsStillRunning(program,childProcess,isRunning);
    if (EXIT_SUCCESS == error) {	
		if (childProcess != 0) {
			error = stopExec();
		} else {
			error = run();
		}
	} else {
	  stopAllRunningProcess(program);
	  error = run();
	}	
    return error;
}

#define EXIT_MSG  "the child terminated normally"
#define TERM_MSG  "the child process was terminated by signal"

inline bool Process::autoRestart(const int status) {
    bool reStart(false);

    if (!killedByMgr) {
        //TODO: manage communicationStatus
        reStart = theConfiguration.getAutoRestartMode();
#ifdef ENABLE_X11_MANAGEMENT
        reStart &= (!XWindowIsMinimized);
#endif //ENABLE_X11_MANAGEMENT
        if (reStart) {
            if ((state != starting) && (state != restarting)) {
                if (WIFEXITED(status)) {
                    // the child terminated normally.
                    NOTICE_MSG(EXIT_MSG "(status %d)",WEXITSTATUS(status));
                    reStart = false;
                } else if (WIFSIGNALED(status)) {
                    // the child process was terminated by a signal.
                    #ifdef WCOREDUMP
                    if WCOREDUMP(status) {
                        ERROR_MSG(TERM_MSG " %d" " (core dumped)",WTERMSIG(status));
                    } else {
                        ERROR_MSG(TERM_MSG " %d",WTERMSIG(status));
                    }
                    #else
                        ERROR_MSG(TERM_MSG " %d",WTERMSIG(status));
                    #endif
                    if (SIGUSR1 == WTERMSIG(status)) {
                       // login canceled
                    }
                }
#ifdef ENABLE_X11_MANAGEMENT 
                //DEBUG_VAR_BOOL(XWindowIsMinimized);
                else if (XWindowIsMinimized) {
			   reStart = false;
			}
#endif //ENABLE_X11_MANAGEMENT
            } else {
                // restart already in progress
                reStart = false;
            }
        } else {
            DEBUG_MSG("autorestart mode disabled");
        }
    } else {
        DEBUG_MSG("external program stopped by its manager");
    }
    DEBUG_VAR_BOOL(reStart);
    return reStart;
}

inline int Process::waitForChildExitStatus(pid_t child /*= -1*/,int option /*= 0*/) {
    int error(EXIT_SUCCESS);
    pid_t childId = -1;    

    if (0 == option) {
        // wait for the end of the child
        do {
             childId = waitpid(child,&childExitStatus,0);
             DEBUG_VAR(childId,"%d");
             error = errno;
             if ((error != EINTR) && (error != ECHILD)) {                 
		ERROR_MSG("waitpid %d error %d (%m)",childId,error);
             } else if (ECHILD == error) {
                 ERROR_MSG("waitpid %d ECHILD error");
                 childId = 0;
             }
        } while(childId < 0);
        DUMP();        

        //
        if (childId != 0) {
            DEBUG_MSG("%d exit status %d (%d)",childId,childExitStatus,error);            
        }
        setStopped();
        
    } else {
        childId = waitpid(child,&childExitStatus,option);
        if (childId > 0) {
            DEBUG_MSG("child %d exited with status %d",childId,childExitStatus);
            DUMP();            
            setStopped();
        } else if (childId < 0) {
            error = errno;
            ERROR_MSG("waitpid %d error %d (%m)",child,error);
        } else if (0 == childId) {
            DEBUG_MSG("no child(ren) specified (%d) has yet changed state",child);
        }
    }

	
    // should the manager restart it ?        
    /* done later !!!
	if (autoRestart(childExitStatus)) {
        state = restarting;
        DEBUG_MSG("auto restarting...");
        error = run();
    }*/
        
    return error;
}

inline int Process::doXWindowAction(void *toDo) {
   int error(EXIT_SUCCESS);   
#ifdef ENABLE_X11_MANAGEMENT  
    XWindowAction *action = (XWindowAction *)toDo;  
    DEBUG_MSG("Windows management started");
    if (theConfiguration.isStrategySet(X11_CLASS)) {
        XWindowOfClass wndClass(theConfiguration.getXWinParameter());
        error = findWindowAndDoAction(&wndClass,action);
    } else if (theConfiguration.isStrategySet(X11_TITLE)) {
        XWindowNamed wdnName(theConfiguration.getXWinParameter());
        error = findWindowAndDoAction(&wdnName,action);
    } else if (theConfiguration.isStrategySet(X11_SUBTITLE)) {
        XWindowWhichNameContains wdnName(theConfiguration.getXWinParameter());
        error = findWindowAndDoAction(&wdnName,action);
    } else if (theConfiguration.isStrategySet(X11_PID)) {
        WindowOf wndOf(childProcess);
        error = findWindowAndDoAction(&wndOf,action);
    }
#endif  //ENABLE_X11_MANAGEMENT
    return error;   
}

int Process::updateXWindowState() {
   int error(EXIT_SUCCESS);
#ifdef ENABLE_X11_MANAGEMENT      	
    XWindowState state; 
	if (theConfiguration.isStrategySet(X11_CLASS)) {
        XWindowOfClass wndClass(theConfiguration.getXWinParameter());
        error = getWindowState(&wndClass,state);
    } else if (theConfiguration.isStrategySet(X11_TITLE)) {
        XWindowNamed wdnName(theConfiguration.getXWinParameter());
        error = getWindowState(&wdnName,state);
    } else if (theConfiguration.isStrategySet(X11_SUBTITLE)) {
        XWindowWhichNameContains wdnName(theConfiguration.getXWinParameter());
        error = getWindowState(&wdnName,state);
    } else if (theConfiguration.isStrategySet(X11_PID)) {
        WindowOf wndOf(childProcess);
        error = getWindowState(&wndOf,state);
    }
	if (EXIT_SUCCESS == error) {
	   XWindowIsMinimized = (Iconic == state);
	   DEBUG_VAR_BOOL(XWindowIsMinimized);
	}
#endif  //ENABLE_X11_MANAGEMENT
    return error;
}

inline int Process::manageXWindow() {
    int error(EXIT_SUCCESS);
    SwapState action;
	error = doXWindowAction(&action);   
#ifdef ENABLE_X11_MANAGEMENT    
    DEBUG_VAR(error,"%d");
	if (EXIT_SUCCESS == error) {
	   // because we don't know the current XWindow's state 
	   usleep(250000);
       error = updateXWindowState();
	}
#endif  //ENABLE_X11_MANAGEMENT
    return error;
}

inline int Process::iconizeXWindow() {
    int error(EXIT_SUCCESS);
    Iconify action;
	error = doXWindowAction(&action); 
	if (EXIT_SUCCESS == error) {
		XWindowIsMinimized = true;
		DEBUG_VAR_BOOL(XWindowIsMinimized);
	}
	return error;
}

inline int Process::maximizeXWindow() {
    int error(EXIT_SUCCESS);
    UnIconify action;
	error = doXWindowAction(&action);
    if (EXIT_SUCCESS == error) {
		XWindowIsMinimized = false;
		DEBUG_VAR_BOOL(XWindowIsMinimized);
	}	
	return error;
}

void* waitForMessages(void *param) {
    int error(EXIT_SUCCESS);
    Message message;
    ReadMessagesMgr messagesMgr;
    Process *currentProcessMgr = (Process *)param;
    bool continueLoop(true);

    do
    {
        error = messagesMgr.get(message);
        if (EXIT_SUCCESS == error) {
            const unsigned int content(message.getContent());
            switch(content) {
                case DISPLAY_MSG:
				    //DEBUG_MSG("DISPLAY_MSG received");
                    if ((theConfiguration.windowManagementMode()) && (currentProcessMgr->isRunning())) {
                        error = currentProcessMgr->manageXWindow();
                    } else {
                        error = currentProcessMgr->toggleExec();
                    }
                    DEBUG_VAR(error,"%d");
                    break;
                case SIGCHILD_MSG: {
                        DEBUG_MSG("SIGCHILD_MSG received");
                        //error = currentProcessMgr->waitForChildExitStatus(-1,WNOHANG);
                    }
                    break;
                case CHILD_EXEC_ERROR:
                    currentProcessMgr->setStateExecError();
                    break;
               case STOP_MSG: {
					//DEBUG_MSG("STOP_MSG received");
					const std::string program = theConfiguration.getProgram();
					stopAllRunningProcess(program);
					DEBUG_MSG("exiting...");
					exit(EXIT_SUCCESS);
                  }
                  break;
				case ICONIZE_MSG:
				  if ((theConfiguration.windowManagementMode()) && (currentProcessMgr->isRunning())) {
				     error = currentProcessMgr->iconizeXWindow();
				  } else {
				    error = currentProcessMgr->toggleExec();
					usleep(250000); //to let time to the external program to open its root window
					currentProcessMgr->iconizeXWindow();
                  }
                  DEBUG_VAR(error,"%d");				  
				  break;
                case MAXIMIZE_MSG:		
                  if ((theConfiguration.windowManagementMode()) && (currentProcessMgr->isRunning())) {
				     error = currentProcessMgr->maximizeXWindow();
				  } else {
				    error = currentProcessMgr->toggleExec();
					//currentProcessMgr->maximizeXWindow();
                  }
                  DEBUG_VAR(error,"%d");				
				  break;
                default:
                    ERROR_MSG("unknown Message content (%d)",content);
            } //switch(content)
        }
    } while(continueLoop);

    currentProcessMgr->messagesThread = 0;
    return NULL;
}

void signals_handler(int signalNumber, siginfo_t *signalInfo, void *context) {    
    if (SIGCHLD == signalNumber) {
//        Message msg(SIGCHILD_MSG);
//        WriteMessagesMgr writeMsgMgr;
//        const int error = writeMsgMgr.send(msg);
//        if (error != EXIT_SUCCESS) {
//              ERROR_MSG("error sending SIGCHILD_MSG %d",error);
//        }
    } else {
        WARNING_MSG("signals_handler has received signal %d",signalNumber);
    }   
}
#if 0
// tokenizer to build the argv parameter
inline void Process::tokenizer(const std::string &parameters,std::vector<const char*> &argv) {
    
}
#endif

#ifdef _DEBUG_
#define DUMP_ARGV(v)   dumpArgv(v)
inline void dumpArgv(const std::vector<const char*> &argv) {
    std::vector<const char*>::const_iterator p =  argv.begin();
    while(p < argv.end()) {
        DEBUG_VAR(*p,"%s");
        p++;
    }
}
#else /* _DEBUG_ */
#define DUMP_ARGV(v)
#endif /* _DEBUG_ */

inline int Process::execChild() {
    int error(EXIT_SUCCESS);
    
    try {
        const char *program = theConfiguration.getProgram().c_str();
        std::string parameters;
        theConfiguration.getProgramParameters(parameters);        
        const size_t n(parameters.length());                
        std::vector<const char*> argv; // to store ptr to token in the tokenized string
        std::auto_ptr<char> tokens(new char[n]); // to store the tokenized string
        
        //memset(cursor,0,n);
        argv.push_back(program);
        if (n > 0) {            
            std::string::const_iterator i = parameters.begin();
            std::string::const_iterator end = parameters.end();
            char *cursor = tokens.get();
            argv.push_back(cursor);
            while (i < end) {
                if ((*i) != ' ') {
                    *cursor = *i;
                } else {
                    *cursor = '\0';
                    argv.push_back(cursor + 1);
                }

                i++;
                cursor++;
            }
            *cursor = '\0';
        }

        argv.push_back(NULL);
        DUMP_ARGV(argv);

        // exec the child process
        if (execvp(program,(char**)argv.data()) == -1) {
            error = errno;
            ERROR_MSG("execlp %s %s error %d (%m)",program,parameters.c_str(),error);

            Message msg(CHILD_EXEC_ERROR);
            WriteMessagesMgr writeMsgMgr;
            const int sendError = writeMsgMgr.send(msg);
            if (sendError != EXIT_SUCCESS) {
                ERROR_MSG("error sending CHILD_EXEC_ERROR %d",sendError);
            } 
        }        
    } //try
    catch(std::bad_alloc exception) {
        ERROR_MSG("std::bad_alloc exception caught %s",exception.what());
        error = ENOMEM;
    }
    catch(...) {
        ERROR_MSG("exception caught !");
        error = EFAULT;
    }
    return error;
}

#if 0
inline int Process::exec() {
    int error(EXIT_SUCCESS);

    DUMP();
    childProcess = fork();
    if (0 == childProcess ) {
        // child:
        error = execChild();
        exit(-error);
    } else if (childProcess > 0) {
        // parent
        DEBUG_VAR(childProcess,"%u");
        setStateRunning();
        error = waitForChildExitStatus(childProcess,0);        
        DEBUG_MSG("%d exit status %d",childProcess,childExitStatus);
    } else {
        //error
        error = errno;
        ERROR_MSG("fork error %d (%m)",error);
    }
    return error;
}

void* runProgram(void *param) {    
    struct sigaction signalAction;
    Process *process = (Process *)param;

    signalAction.sa_handler = NULL;
    signalAction.sa_sigaction = signals_handler;
    signalAction.sa_flags = SA_SIGINFO|SA_NOCLDSTOP|SA_RESTART;
    sigemptyset(&signalAction.sa_mask);
    sigaddset(&signalAction.sa_mask,SIGCHLD);

    if (sigaction(SIGCHLD,&signalAction,NULL) == 0) {
        // can't use the system function because it blocks SIGCHLD !
        process->exec();
    } else {
        const int error = errno;
        ERROR_MSG("sigaction SIGCHLD error %d (%m)",error);
    }

    process->execThread = 0;
    return NULL;
}
#endif

inline int Process::system(const char *command) {
  int error = -1; // because of man 3 system
  struct sigaction signalAction;
  signalAction.sa_handler = SIG_IGN;
  signalAction.sa_flags = 0;
  sigemptyset(&signalAction.sa_mask);
  if (sigaction(SIGCHLD,&signalAction,NULL) == 0) {
     childProcess = fork();
     if (0 == childProcess ) {
        // child:
        error = execChild();
        exit(-error);
     } else if (childProcess > 0) {
        // parent
        DEBUG_VAR(childProcess,"%u");
        setStateRunning();
        error = waitForChildExitStatus(childProcess,0);
        DEBUG_MSG("%d exit status %d",childProcess,childExitStatus);
     } else {
        //error
        error = errno;
        ERROR_MSG("fork error %d (%m)",error);
     }
  } else {
     const int error = errno;
     ERROR_MSG("sigaction SIGCHLD error %d (%m)",error);
  }
  return error;
}

inline int Process::exec() {
    const char *program = theConfiguration.getProgram().c_str();
    std::string parameters;
    theConfiguration.getProgramParameters(parameters);
    std::ostringstream cmdLine;

    cmdLine << program << " " << parameters;
    //setStateRunning();
    const int childExitStatus = system((cmdLine.str()).c_str());
    DEBUG_VAR(childExitStatus,"%d");   

    // should the manager restart it ?        
    if (autoRestart(childExitStatus)) {
        state = restarting;
        DEBUG_MSG("auto restarting...");
        int error = run();
    }
	setStopped();
    return childExitStatus;
}

void* runProgram(void *param) {
    Process *process = (Process *)param;
    process->exec();
    process->execThread = 0;
    return NULL;
}


int Process::run(bool startProgram /*= true*/) {
    int error(EXIT_SUCCESS);
    MutexMgr mutexMgr(runMutex);
    pthread_attr_t attributs;
    
    pthread_attr_init(&attributs);
    pthread_attr_setdetachstate(&attributs, PTHREAD_CREATE_DETACHED);

    if (0 == messagesThread) {
        error = pthread_create(&messagesThread,&attributs,waitForMessages,this);
    } else {
        DEBUG_MSG("messagesThread is already running");
    }
    pthread_attr_destroy(&attributs);
    if (0 == error) {
        // a thread attribute object must not be reused until it is reinitialized.
        if (startProgram) {
            pthread_attr_init(&attributs);
            pthread_attr_setdetachstate(&attributs, PTHREAD_CREATE_DETACHED);
            if ((0 == execThread) || (restarting == state)) {
                state = starting;
                error  = pthread_create(&execThread,&attributs,runProgram,this);
                if (error != 0) {
                    state = execError;
                    ERROR_MSG("pthread_create execThread error %d",error);                    
                }
            } else {
                DEBUG_MSG("execThread is already running");
            }
            pthread_attr_destroy(&attributs);            
        } else {
            DEBUG_MSG("start program disabled");
        }
    } else {
        ERROR_MSG("pthread_create messagesThread error %d",error);
    }
    
    return error;
}

int Process::restart() {
    int error(EXIT_SUCCESS);
    MutexMgr mutexMgr(runMutex);
    
    if (running == state) {        
        stopExec(); //error already printed, anyway go on
        state = restarting;
        error = run(); //error already printed
    } else {
        WARNING_MSG("no restart (state = %d)",state);
        error = ECHILD;
    }    
    return error;
}
