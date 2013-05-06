/* 
 * File:   singleton.h
 * Author: oc
 *
 * Created on January 15, 2011, 1:25 PM
 */

#ifndef _SINGLETON_H
#define	_SINGLETON_H

#include <sys/types.h>

#define MODULE_FLAG FLAG_SINGLETON
class Singleton {
    
    Singleton(const Singleton& orig) {
        //not allowed
    }
    pid_t previouslyRunningPID;
public:
    Singleton();    
    virtual ~Singleton();

    bool isValid() {
	    const bool valid = (0 == previouslyRunningPID);
		DEBUG_VAR_BOOL(valid);
        return valid;
    }

};

int stopAllRunningProcess(const std::string &processName);
int processIsStillRunning(const std::string &processName,const pid_t pid, bool &isRunning);

#undef  MODULE_FLAG
#endif	/* _SINGLETON_H */

