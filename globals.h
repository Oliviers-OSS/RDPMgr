#ifndef _GLOBALS_H_
#define	_GLOBALS_H_

#define identity  "RDPMgr"

#include "config.h"

#include <errno.h>
#include <pthread.h>
#include <cstdio>
#include <cstdlib>
#include "debug.h"
#include "configuration.h"

#ifndef PACKAGE_NAME
#define PACKAGE_NAME identity
#endif /* PACKAGE_NAME */

#ifndef PACKAGE_VERSION
#define PACKAGE_VERSION "2.0"
#endif /* PACKAGE_VERSION */

#ifdef __cplusplus
extern "C"
{
#endif

#if defined(_SYS_SYSLOG_EX_H)
    #define SET_LOG_MASK(x) setlogmaskex(x)
#else
    #define SET_LOG_MASK(x) setlogmask(x)
#endif

#ifdef _DEBUGFLAGS_H_
#define REGISTER_DBGFLAGS(x)    registerDebugFlags(&x)
#define UNREGISTER_DBGFLAGS(x)  unregisterDebugFlags(&x)
#else
#define REGISTER_DBGFLAGS(x)
#define UNREGISTER_DBGFLAGS(x)
#endif /*_DEBUGFLAGS_H_*/

#ifdef DISABLE_CORBA_NS_KIND
#define CORBA_NS_KIND ""
#else /* DISABLE_CORBA_NS_KIND */
#define CORBA_NS_KIND "remoteServersSet"
#endif /* DISABLE_CORBA_NS_KIND */

#ifdef __cplusplus
}
#endif

#include "process.h"
extern Process *currentProcessMgr;

#endif	/* _GLOBALS_H_ */

