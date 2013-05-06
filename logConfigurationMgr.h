/* 
 * File:   logConfigurationMgr.h
 * Author: oc
 *
 * Created on January 22, 2011, 5:31 PM
 */

#ifndef _LOG_CONFIGURATION_MGR_H_
#define	_LOG_CONFIGURATION_MGR_H_

#include "config.h"
#include "debug.h"

#ifdef _DEBUG_
#define DEFAULT_LOG_OPTION LOG_PERROR|LOG_PID|LOG_TID
#else
#define DEFAULT_LOG_OPTION LOG_PID|LOG_TID
#endif //_DEBUG_

class LogConfigurationMgr {

public:
    LogConfigurationMgr(const char *ident = NULL,const int option = DEFAULT_LOG_OPTION,const int facility = LOG_USER) {
        openlogex(ident,option,facility);
        registerDebugFlags(&debugFlags);
    }

    ~LogConfigurationMgr() {
        closelogex();
    }
};

#endif	/* _LOG_CONFIGURATION_MGR_H_ */

