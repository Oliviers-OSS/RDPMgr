/*
 * File:   configuration.h
 * Author: oc
 *
 * Created on January 15, 2011, 8:44 AM
 */

#ifndef _CONFIGURATION_H
#define	_CONFIGURATION_H

#include "config.h"

#include <string>
#include <vector>
#include <sys/time.h>
#include <time.h>
#include <cstdlib>
#include "locks.h"

#define MODULE_FLAG FLAG_CONFIG

#define DISPLAY_MODE                           (1<<0)
#define CHANGE_SERVER_AUTO_MODE            (1<<1)
#define AUTORESTART_CHILD                    (1<<2)
#define XWINDOWS_MANAGEMENT                 (1<<3)
#define X11_CLASS                               (1<<4)
#define X11_TITLE                               (1<<5)
#define X11_SUBTITLE                           (1<<6)
#define X11_PID                                 (1<<7)
#define KILL_ALL_ON_STARTUP                   (1<<8)
#define STOP_MODE                             (1<<9)
#define MINIMIZE_WINDOW_MODE                 (1<<10)
#define MAXIMIZE_WINDOW_MODE                 (1<<11)

#define CONFIGURATION_FILENAME              "/" PACKAGE ".conf"
#define KILL_DELAY                          (250*1000)     /* in ms */

template <class T> unsigned int findNReplace(const T &pattern,const T &value,T &workArea) {
    const typename T::size_type patternSize = pattern.size();
    const typename T::size_type valueSize = value.size();
    unsigned int n(0);
    for(typename T::size_type pos = 0 ; (pos = workArea.find(pattern,pos)) != T::npos; pos += valueSize) {
        ++n;
        workArea.replace(pos,patternSize,value);
    }
    return n;
}

class CmdLineParameters;

class Configuration {

    std::string program;
    std::string program_parameters;
    std::string filename;
    std::vector<std::string> servers;
    std::string username;
    std::string password;
    unsigned int flags;
    unsigned int moveServerDelay;
    unsigned int currentServerPosition;
    struct timeval currentServerStartOfUse;
    rwLock lock;
    unsigned int killDelay;

#ifdef ENABLE_X11_MANAGEMENT
    std::string xwin_parameter;
#endif //ENABLE_X11_MANAGEMENT

    Configuration(const Configuration& orig) {
        //copy not allowed
    }
    friend class CmdLineParameters;
    inline void apply(CmdLineParameters &parameters);

    void setConfigurationFile(const char *f) {
        filename = f;
    }
    
    inline int readFile(void);

    void setXWindowsManagementMode() {
        flags |= XWINDOWS_MANAGEMENT;
    }

    void setProcessManagementMode() {
        flags &= ~XWINDOWS_MANAGEMENT;
    }

    void setDisplayMode() {
        flags |= DISPLAY_MODE;
    }

    void setStopMode() {
       flags |= STOP_MODE;
    }
	
	void setKillAllOnStartupMode() {
		flags |= KILL_ALL_ON_STARTUP;
	}

    inline void removeORBParameters(int argc, char *argv[],std::vector<char*> &RDPMgrParameters);

    inline bool isSet(const unsigned int option) {
        return (option == (flags & option));
    }
	
	void setMinimizeWindowMode() {
		flags |= MINIMIZE_WINDOW_MODE;
	}

	void setMaximizeWindowMode() {
		flags |= MAXIMIZE_WINDOW_MODE;
	}
	
public:

    Configuration()        
        :flags(0)
        , moveServerDelay(250)
        , currentServerPosition(0) {
        currentServerStartOfUse.tv_sec = 0;
        currentServerStartOfUse.tv_usec = 0;
        filename = TO_STRING(CONFIGDIR) CONFIGURATION_FILENAME;        
    }

    virtual ~Configuration() {        
    }

    int init(int argc, char *argv[]);

    const std::string &getCurrentServer() {
        readLockMgr readLock(lock);
        return servers[currentServerPosition];
    }

    const char* getCurrentServer(struct timeval *currentServerStrtOfUse) {
        readLockMgr readLock(lock);
        if (currentServerStrtOfUse != NULL) {
            currentServerStrtOfUse->tv_sec = currentServerStartOfUse.tv_sec;
            currentServerStrtOfUse->tv_usec = currentServerStartOfUse.tv_usec;
        }
        return servers[currentServerPosition].c_str();
    }

    int changeCurrentServer(const struct timeval *previousServerStartOfUse) {
        writeLockMgr writeLock(lock);
        int error(EXIT_SUCCESS);
        /* currentServer already changed (since last getCurrentServer call) ? */
        if ((currentServerStartOfUse.tv_sec == previousServerStartOfUse->tv_sec)
                && (currentServerStartOfUse.tv_usec == previousServerStartOfUse->tv_usec)) {
            if (gettimeofday(&currentServerStartOfUse, NULL) == 0) {
                currentServerPosition++;
                if (currentServerPosition >= servers.size()) {
                    currentServerPosition = 0;
                }
                DEBUG_VAR(currentServerPosition, "%d");
            } else {
                error = errno;
                ERROR_MSG("gettimeofday error %d (%m)", error);
            }
        } else {
            DEBUG_MSG("current server has already been changed ({%d,%d} > {%d,%d})", currentServerStartOfUse.tv_sec, currentServerStartOfUse.tv_usec, previousServerStartOfUse->tv_sec, previousServerStartOfUse->tv_usec);
        }
        return error;
    }

    int getServerPosition(const char *newServer) {
        readLockMgr readLock(lock);
        int position(0);
        const unsigned int end(servers.size());

        while ((position < end) && (servers[position].compare(newServer) != 0)) {
            position++;
        }
        return position;
    }

    int setCurrentServer(const char *newServer) {
        int error = EXIT_SUCCESS;
	const std::string currentServer = getCurrentServer(); 
        DEBUG_CPP_VAR(currentServer);
        if (currentServer.compare(newServer) != 0) {
		const int serverPosition = getServerPosition(newServer);
                DEBUG_VAR(serverPosition,"%d");
		if (serverPosition < servers.size()) {
			writeLockMgr writeLock(lock);
			struct timeval currentServerStartOfUse;
			if (gettimeofday(&currentServerStartOfUse, NULL) == 0) {                
				currentServerPosition = serverPosition;
				DEBUG_VAR(currentServerPosition, "%d");
			} else {
				error = errno;
				ERROR_MSG("gettimeofday error %d (%m)", error);
			}
		} else { /* !(serverPosition < NUMBER_OF_SERVERS_MAX) */
		   ERROR_MSG("server %s not found", newServer);
		   error = ENOENT;
		}
        } else {
          NOTICE_MSG("same server (%s)",newServer);
          error = EEXIST;
        }
        return error;
    }

    bool isChangeServerAutoModeSet() {
        readLockMgr readLock(lock);
        return (isSet(CHANGE_SERVER_AUTO_MODE));
    }

    void setChangeServerAutoMode() {
        writeLockMgr writeLock(lock);
        flags |= CHANGE_SERVER_AUTO_MODE;
    }

    void unsetChangeServerAutoMode() {
        writeLockMgr writeLock(lock);
        flags &= ~CHANGE_SERVER_AUTO_MODE;
    }
    
    const std::string &getProgram() {
        readLockMgr readLock(lock);
        return program;
    }

    const void getProgramParameters(std::string &parameters) {
        readLockMgr readLock(lock);
        //set last parameters for running the external program
        parameters = program_parameters;
        findNReplace<std::string>("%S",getCurrentServer(),parameters);
        DEBUG_VAR(parameters.c_str(),"%s");
    }

    const std::string &getUsername() {
        readLockMgr readLock(lock);
        return username;
    }

    const std::string &getPassword() {
        readLockMgr readLock(lock);
        return password;
    }

    const size_t maxParameterSize() {
        return program_parameters.length() + username.length() + password.length() + 20;
    }
    
    bool displayMode() {
        readLockMgr readLock(lock);
        return (isSet(DISPLAY_MODE));
    }

    bool getAutoRestartMode() {
        readLockMgr readLock(lock);        
        return (isSet(AUTORESTART_CHILD));
    }

    bool windowManagementMode() {
        readLockMgr readLock(lock);        
        return (isSet(XWINDOWS_MANAGEMENT));
    }

#ifdef ENABLE_X11_MANAGEMENT
    bool isStrategySet(const unsigned int strategy) {
        readLockMgr readLock(lock);
        return isSet(strategy);               
    }

    const std::string &getXWinParameter() {
        readLockMgr readLock(lock);
        return xwin_parameter;
    }
#endif //ENABLE_X11_MANAGEMENT

    bool killAllOnStartup() {
       readLockMgr readLock(lock);
       return (isSet(KILL_ALL_ON_STARTUP));
    }

    bool stopMode() {
       readLockMgr readLock(lock);
	   const int n = isSet(STOP_MODE);
	   DEBUG_VAR_BOOL(n);
	   DEBUG_VAR(flags,"0X%X");
       return (isSet(STOP_MODE));
    }
	
	unsigned int getKillDelay() {
	   readLockMgr readLock(lock);
	   return killDelay;
	}
	
	bool minimizeWindowMode() {
	    readLockMgr readLock(lock);
		const int n = isSet(MINIMIZE_WINDOW_MODE);
	    DEBUG_VAR_BOOL(n);
	    DEBUG_VAR(flags,"0X%X");
        return (isSet(MINIMIZE_WINDOW_MODE));		
	}

	bool maximizeWindowMode() {
	    readLockMgr readLock(lock);
		const int n = isSet(MAXIMIZE_WINDOW_MODE);
	    DEBUG_VAR_BOOL(n);
	    DEBUG_VAR(flags,"0X%X");
        return (isSet(MAXIMIZE_WINDOW_MODE));		
	}
};

#undef MODULE_FLAG

extern Configuration theConfiguration;
#endif	/* _CONFIGURATION_H */

