/* 
 * File:   Singleton.cpp
 * Author: oc
 * 
 * Created on January 15, 2011, 1:25 PM
 */
#include "globals.h"
#include "singleton.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <signal.h>
#include <unistd.h>

#define MODULE_FLAG FLAG_SINGLETON
#define LOCK_FILE_NAME  "/tmp/" identity ".pid"

static inline int unsetLock() {
    int error = EXIT_SUCCESS;
    if (unlink(LOCK_FILE_NAME) == -1) {
        error = errno;
        ERROR_MSG("unlink " LOCK_FILE_NAME " error %d (%m)", error);
    } else {
        DEBUG_MSG("lock file " LOCK_FILE_NAME " deleted");
    }
    return error;
}

static inline int getFullProcessName(const pid_t pid, char *processName, size_t processNameAllocatedSize) {
    int error = EXIT_SUCCESS;
    char procEntryName[PATH_MAX];
    ssize_t n = 0;

    sprintf(procEntryName, "/proc/%u/exe", pid);
    n = readlink(procEntryName, processName, processNameAllocatedSize);
    if (n != -1) {
        processName[n] = '\0'; // because readlink does not append a  NULL  character to  buf 
        DEBUG_VAR(processName, "%s");
    } else {
        error = errno;
        if (error != EACCES) {
            INFO_MSG("readlink %s error %d (%m)", procEntryName, error); //not a real error, just notice it
        }
    }

    return error;
}

static inline int getCurrentFullProcessName(char *processName, size_t processNameAllocatedSize) {
    const pid_t pid = getpid();
    return getFullProcessName(pid, processName, processNameAllocatedSize);
}

static inline int sameProcessAlreadyRunning(const pid_t previousPID) {
    int error = EXIT_SUCCESS;
    char runningProcessName[PATH_MAX];
    error = getFullProcessName(previousPID, runningProcessName, sizeof (runningProcessName));
    if (EXIT_SUCCESS == error) {
        /* this is a PID of a running process: check its name against the current */
        char currentProcessName[PATH_MAX];
        error = getCurrentFullProcessName(currentProcessName, sizeof (currentProcessName));
        if (EXIT_SUCCESS == error) {
            DEBUG_VAR(currentProcessName, "%s");
            DEBUG_VAR(runningProcessName, "%s");
            if (strcmp(currentProcessName, runningProcessName) == 0) {
                /* same process */
                error = EEXIST;
                ERROR_MSG("another instance of %s is still running (%u)", runningProcessName, previousPID);
            } else {
                /* not the same name */
                DEBUG_MSG("the PID %u is now %s and no more %s", previousPID, runningProcessName, currentProcessName);
                error = EXIT_SUCCESS;
            }
        } else {
            /* getCurrentFullProcessName error already logged */
            error = EXIT_SUCCESS;
        }
    } else {
        /* error: this PID is no more a running process ? (error details already logged) */
        error = EXIT_SUCCESS;
    }
    return error;
}

static inline pid_t stringPIDToPID(const char *string) {
    const char *cursor = string;
    pid_t pid = 0;
    while ((*cursor >= '0') && (*cursor <= '9')) {
        pid = pid * 10 + (*cursor) - '0';
        cursor++;
    }
    return pid;
}

static bool isNumber(const char *string) {
    bool test(true);
    while ((*string != '\0') && (test)) {
        test = isdigit(*string);
        string++;
    }
    return test;
}

static inline int StopProcess(pid_t pid) {
   int error = EXIT_SUCCESS;

   DEBUG_MSG("stopping %u",pid);
   if (kill(pid,SIGTERM)) {
      unsigned int delay = theConfiguration.getKillDelay();
      usleep(delay);
   } else {
      error = errno;
      ERROR_MSG("kill %u error %d (%m)",pid,error);
   }

   if (!kill(pid,SIGKILL)) {
      error = errno;
      ERROR_MSG("kill %u error %d (%m)",pid,error);
   }
   return error;
}

static inline int lookForProcessAndStop(const char *processName, const char *stopProcessName,pid_t &pid) {
   int error = EXIT_SUCCESS;
   pid = 0;
   DIR *directory = opendir("/proc");
   if (directory != NULL) {
      struct dirent *directoryEntry  = readdir(directory);
      const pid_t currentPID = getpid();
      while ((directoryEntry != NULL) && (0 == pid)) {
            const char *dirName = directoryEntry->d_name;
            if (isNumber(dirName)) {
               pid_t runningProcessPID = stringPIDToPID(dirName);
               if (currentPID != runningProcessPID) { //skip myself !
                  char runningProcessName[PATH_MAX];
                  error = getFullProcessName(runningProcessPID, runningProcessName, sizeof(runningProcessName));
                  if (EXIT_SUCCESS == error) {
                        if (strcmp(runningProcessName,processName) == 0) {
                           // found one !
                           pid = runningProcessPID;
                           DEBUG_MSG("found a running instance of %s: %u",processName,pid);
                        }

                        if (strcmp(runningProcessName,stopProcessName) == 0) {
						   DEBUG_MSG("%u %s found to stop",runningProcessPID,runningProcessName); 
                           StopProcess(runningProcessPID);  //error already printed
                        }
                  } //error already printed
               } //(currentPID != runningProcessPID)
            } //(isNumber(directory))
            directoryEntry = readdir(directory); //move to next one...
      } //while ((directoryEntry != NULL) && (0 == pid))

      // check the exit loop reason
      error = errno;
      if (error != EXIT_SUCCESS) {
            ERROR_MSG("readdir /proc error %d (%m)",error);
      }

      if (closedir(directory) != 0) {
            error = errno;
            ERROR_MSG("closedir /proc error %d (%m)",error);
      }
   } else {
      error = errno;
      ERROR_MSG("opendir /proc error %d (%m)",error);
   }

   return error;
}

static inline int lookForProcess(const char *processName, pid_t &pid) {
    int error = EXIT_SUCCESS;
    pid = 0;
    DIR *directory = opendir("/proc");
    if (directory != NULL) {
        struct dirent *directoryEntry  = readdir(directory);
        const pid_t currentPID = getpid();
        while ((directoryEntry != NULL) && (0 == pid)) {
            const char *dirName = directoryEntry->d_name;
            if (isNumber(dirName)) {
                pid_t runningProcessPID = stringPIDToPID(dirName);
                if (currentPID != runningProcessPID) { //skip myself !
                    char runningProcessName[PATH_MAX];
                    error = getFullProcessName(runningProcessPID, runningProcessName, sizeof(runningProcessName));
                    if (EXIT_SUCCESS == error) {
                        if (strcmp(runningProcessName,processName) == 0) {                            
                            // found one !
                            pid = runningProcessPID;
                            DEBUG_MSG("found a running instance of %s: %u",processName,pid);
                        }
                    } //error already printed                      
                } //(currentPID != runningProcessPID)
            } //(isNumber(directory))
            directoryEntry = readdir(directory); //move to next one...
        } //while ((directoryEntry != NULL) && (0 == pid))

        // check the exit loop reason
        error = errno;
        if (error != EXIT_SUCCESS) {
            ERROR_MSG("readdir /proc error %d (%m)",error);
        }
        
        if (closedir(directory) != 0) {
            error = errno;
            ERROR_MSG("closedir /proc error %d (%m)",error);
        }
    } else {
        error = errno;
        ERROR_MSG("opendir /proc error %d (%m)",error);
    }
    
    return error;
}

static inline int setLock(pid_t &previousPID) {
    int error = EXIT_SUCCESS;
    int fd = open(LOCK_FILE_NAME, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    previousPID = 0;
    if (fd > 0) {
        /* check if another instance is running */
        /* first get an exclusiv access to this file (during this function) */
        int n = lockf(fd, F_TLOCK, 0);
        if (n != -1) {
            char fileContent[15]; /* because (unsigned int)-1 needs 11 characters to be printed */
            n = read(fd, &fileContent, sizeof (fileContent));
            if (n > 0) {
                /* still running ? */
                previousPID = stringPIDToPID(fileContent);
                if (previousPID != 0) {
                    DEBUG_VAR(previousPID,"%u");
                    error = sameProcessAlreadyRunning(previousPID);
                   if (EXIT_SUCCESS == error) {
                       previousPID = 0;
                   }
                } else {
                    DEBUG_MSG("the content of the file " LOCK_FILE_NAME " is not valid (%s)", fileContent);
                    previousPID = 0;
                }
            } else { /* read(fd,&fileContent,sizeof(fileContent)) <= 0 */
                if (-1 == n) {
                    /* not an error */
                    DEBUG_MSG("read " LOCK_FILE_NAME " error (%d)", errno);
                } else {
                    DEBUG_MSG("read " LOCK_FILE_NAME " return only %d bytes", n);
                }
            }

            if (error != EEXIST) {
                // last chance to find one...
                char currentProcessName[PATH_MAX];
                error = getCurrentFullProcessName(currentProcessName, sizeof (currentProcessName));
                if (EXIT_SUCCESS == error) {
                    lookForProcess(currentProcessName,previousPID);
                  /* BAD IDEA: only one loop BUT if set and we just want to send a display message...
                  if (!theConfiguration.killAllOnStartup()) {
                     lookForProcess(currentProcessName,previousPID);
                  } else {
                     const std::string program = theConfiguration.getProgram();
                     lookForProcessAndStop(currentProcessName,program.c_str(),previousPID);
                  }*/
                } //error already printed

                if (0 == previousPID) { //still not found: go on
                    const pid_t pid = getpid();
                    ssize_t written = 0;
                    if (ftruncate(fd, 0) == -1) {
                        error = errno;
                        WARNING_MSG("ftruncate to 0 file " LOCK_FILE_NAME " error %d (%m)", error);
                    }
                    n = sprintf(fileContent, "%u", pid);
                    written = write(fd, fileContent, n);
                    if (written != n) {
                        if (-1 == written) {
                            error = errno;
                            ERROR_MSG("write into " LOCK_FILE_NAME " error %d (%m)", error);
                        } else {
                            error = EIO;
                            ERROR_MSG("only %d bytes have been written into " LOCK_FILE_NAME, written);
                        }
                    } /* (written != n) */
                } else {
                    error = EEXIST;
                }
            } /* (error != EEXIST) error already logged */

            /* unlock this file */
            n = lockf(fd, F_ULOCK, 0);
            if (-1 == n) {
                if (EXIT_SUCCESS == error) { /* don't overwrite a previous error */
                    error = errno;
                }
                ERROR_MSG("flock F_ULOCK " LOCK_FILE_NAME " error %d (%m)", errno);
            }
        } else {
            /* some one else may have lock it: another process is running this function ? */
            error = errno;
            ERROR_MSG("flock F_TLOCK " LOCK_FILE_NAME " error %d (%m)", error);
        }

        if (close(fd) != 0) {
            error = errno;
            ERROR_MSG("close " LOCK_FILE_NAME " error %d (%m)", error);
        }
    } else {
        error = errno;
        ERROR_MSG("open " LOCK_FILE_NAME " error %d (%m)", error);		
    }
    return error;
}

Singleton::Singleton()
    :previouslyRunningPID(0) {
    const int error = setLock(previouslyRunningPID);
    /*if (error != EEXIST) {
        previouslyRunningPID = 0;
    }*/
}

Singleton::~Singleton() {
    if (isValid()) {
        unsetLock();
    }
}

int stopAllRunningProcess(const std::string &processName) {
   int error = EXIT_SUCCESS;
   DIR *directory = opendir("/proc");
   if (directory != NULL) {
      struct dirent *directoryEntry  = readdir(directory);
      const pid_t currentPID = getpid();
      while (directoryEntry != NULL) {
         const char *dirName = directoryEntry->d_name;
         if (isNumber(dirName)) {
            pid_t runningProcessPID = stringPIDToPID(dirName);
            if (currentPID != runningProcessPID) { //skip myself !
               char runningProcessName[PATH_MAX];
               error = getFullProcessName(runningProcessPID, runningProcessName, sizeof(runningProcessName));
               if (EXIT_SUCCESS == error) {
                  if (processName.compare(runningProcessName) == 0) {
                     // found one !
                     StopProcess(runningProcessPID);  //error already printed
                  }
               } //error already printed
            } //(currentPID != runningProcessPID)
         } //(isNumber(directory))
         directoryEntry = readdir(directory); //move to next one...
      } //while ((directoryEntry != NULL) && (0 == pid))

      // check the exit loop reason
      error = errno;
      if (error != EXIT_SUCCESS) {
         ERROR_MSG("readdir /proc error %d (%m)",error);
      }

      if (closedir(directory) != 0) {
         error = errno;
         ERROR_MSG("closedir /proc error %d (%m)",error);
      }
   } else {
      error = errno;
      ERROR_MSG("opendir /proc error %d (%m)",error);
   }

   return error;
}

int processIsStillRunning(const std::string &processName,const pid_t pid, bool &isRunning) {  
  int error = EXIT_SUCCESS;
  isRunning = false;
  if (pid != 0) {	  
	    // check if it's our process
		char runningProcessName[PATH_MAX];
        error = getFullProcessName(pid, runningProcessName, sizeof(runningProcessName));
        if (EXIT_SUCCESS == error) {
		   isRunning = (processName.compare(runningProcessName) == 0);
		} ////error already printed		  			  
   }
   return error;
}
