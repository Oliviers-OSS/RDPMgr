/* 
 * File:   process.h
 * Author: oc
 *
 * Created on January 15, 2011, 2:56 PM
 */

#ifndef _PROCESS_H
#define	_PROCESS_H

#include "globals.h"

#define MODULE_FLAG FLAG_MONITOR

class Process {
    pthread_t messagesThread;
    pthread_t execThread;
    pid_t childProcess;
    int childExitStatus;
    enum {
        running = 0
       ,restarting
       ,starting
       ,stopped
       ,killing
       ,execError
       ,windowMinimized
    } state;
    pthread_mutex_t runMutex;
    long communicationStatus;
    bool killedByMgr;
	bool XWindowIsMinimized;

#ifdef _DEBUG_
    void dump() {
        DEBUG_VAR(messagesThread,"0x%X");
        DEBUG_VAR(execThread,"0x%X");
        DEBUG_VAR(childProcess,"%u");
        DEBUG_VAR(childExitStatus,"%d");
        const char *stateValue = "unknown";
        switch(state) {
            case running:
                stateValue = "running";
                break;
            case restarting:
                stateValue = "restarting";
                break;
            case starting:
                stateValue = "starting";
                break;
            case stopped:
                stateValue = "stopped";
                break;
            case killing:
                stateValue = "killing";
                break;
            case execError:
                stateValue = "execError";
                break;
            case windowMinimized:
                stateValue = "windowMinimized";
                break;
        } /* switch(state) */
        DEBUG_VAR(stateValue,"%s");
        DEBUG_VAR(communicationStatus,"%d");
        DEBUG_VAR_BOOL(killedByMgr);
    }
#endif /* _DEBUG_ */

    void setStateRunning() {
        state = running;
        killedByMgr = false;
#ifdef ENABLE_X11_MANAGEMENT
        usleep(250000);
		updateXWindowState();
#endif  //ENABLE_X11_MANAGEMENT		
    }

    void setStateExecError() {
        state = execError;
        CRIT_MSG("child exec fatal error, aborting...");
        exit(-1);
    }    

    void setStopped() {
        childProcess = 0;
        state = stopped;
#ifdef ENABLE_X11_MANAGEMENT
        //XWindowIsMinimized = false;
		//DEBUG_VAR_BOOL(XWindowIsMinimized);
#endif  //ENABLE_X11_MANAGEMENT	
    }

    friend void* runProgram(void *param);
    friend void* waitForMessages(void *param);
    inline int stopExec();
    inline int toggleExec();
    inline void tokenizer(const std::string &parameters,std::vector<const char*> &argv);
    inline int execChild();
    inline int exec();
    inline int waitForChildExitStatus(pid_t child = -1,int option = 0);
    inline void buildParametersArray(const char *program,const char *parameters);
    inline bool autoRestart(const int status);
	inline int doXWindowAction(void *toDo); //because compiler refuses forward declaration of XWindowAction
    inline int manageXWindow();
	inline int iconizeXWindow();
    inline int maximizeXWindow();
	int updateXWindowState();

    Process(const Process &orig) {
        //copy not allowed
    }

    int system(const char *command);

public:

    Process()
        :messagesThread(0)
        ,execThread(0)
        ,childProcess(0)
        ,childExitStatus(-1)
        ,state(stopped)
        ,communicationStatus(EXIT_SUCCESS)
        ,killedByMgr(false)
        ,XWindowIsMinimized(false) {
            pthread_mutexattr_t mutexAttribut;
            pthread_mutexattr_init(&mutexAttribut);
            pthread_mutexattr_settype(&mutexAttribut,PTHREAD_MUTEX_RECURSIVE_NP);
            pthread_mutex_init(&runMutex,&mutexAttribut);
            pthread_mutexattr_destroy(&mutexAttribut);
    }
    
    ~Process();
    
    int run(bool startProgram = true);

    long getCommunicationStatus() {
        return communicationStatus;
    }
    
    int getChildExitStatus() {
        return childExitStatus;
    }

    pid_t getChildProcessID() {
        return childProcess;
    }

    int restart();

    inline bool isRunning() {
        return ((running == state) || (windowMinimized == state));
    }
};
#undef MODULE_FLAG

#endif	/* _PROCESS_H */

