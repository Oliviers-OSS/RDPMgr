/* 
 * File:   debug.h
 * Author: oc
 *
 * Created on July 10, 2010, 7:23 PM
 */

#ifndef _DEBUG_H_
#define	_DEBUG_H_

#include "config.h"

#if defined(DEBUG) && !defined(_DEBUG_)
#define _DEBUG_
#endif

#include <dbgflags/loggers.h>

#ifdef HAVE_DBGFLAGS_DBGFLAGS_H
#include <dbgflags-1/dbgflags.h>
    
extern DebugFlags debugFlags;

#endif

#include <dbgflags-1/syslogex.h>

//#define LOGGER consoleLogger
//#define LOG_OPTS 0

#ifndef DEBUG_LOG_HEADER
#define DEBUG_LOG_HEADER ""
#endif /*DEBUG_LOG_HEADER */
#define DEBUG_EOL "\n"
#include <dbgflags-1/debug_macros.h>

#define FLAG_INIT   FLAG_0
#define FLAG_SINGLETON    FLAG_1
#define FLAG_RUNTIME     FLAG_2
#define FLAG_MONITOR  FLAG_3
#define FLAG_CORBA     FLAG_4
#define FLAG_CONFIG    FLAG_5
#define FLAG_NOTIFICATION   FLAG_6
#define FLAG_DEADLOCK   FLAG_7
#define FLAG_XWINDOWS    FLAG_8

#ifdef _DEBUG_
#define INITIAL_FLAGS_VALUE  FLAG_INIT|FLAG_SINGLETON|FLAG_RUNTIME|FLAG_MONITOR|FLAG_CORBA|FLAG_CONFIG|FLAG_NOTIFICATION // ((unsigned int)-1)
#define INITIAL_LOG_LEVEL    "debug"
#define DUMP() dump()
#else
#define INITIAL_FLAGS_VALUE  FLAG_RUNTIME|FLAG_MONITOR
#define INITIAL_LOG_LEVEL    "warning"
#define DUMP()
#endif

#endif	/* _DEBUG_H_ */

