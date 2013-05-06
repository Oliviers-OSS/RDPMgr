/*
 * File:   configuration.cpp
 * Author: oc
 *
 * Created on January 15, 2011, 8:44 AM
 */

#include "globals.h"
#include "configuration.h"
#include "iniparser.h"

#define MODULE_FLAG FLAG_CONFIG

#include <getopt.h>
#include <sys/time.h>
#include <time.h>
#include <cstdlib>
#include <cstdio>


Configuration theConfiguration;

static __inline void printVersion(void) {
    printf(PACKAGE_NAME " v" PACKAGE_VERSION "\n");
}

static const struct option longopts[] = {
    {"display", no_argument, NULL, 'd'},
    {"log-level", required_argument, NULL, 'l'},
    {"log-mask", required_argument, NULL, 'm'},
    {"config-file", required_argument, NULL, 'f'},
    { "stop", no_argument, NULL, 's'},
	{ "kill-all", no_argument, NULL, 'k'},
	{ "iconize", no_argument, NULL, 'i'},
	{ "maximize", no_argument, NULL, 'a'},
    { "help", no_argument, NULL, 'h'},
    { "version", no_argument, NULL, 'v'},
    { NULL, 0, NULL, 0}
};

static __inline void printHelp(const char *errorMsg) {
#define USAGE  "Usage: " PACKAGE_NAME " [OPTIONS] \n" \
    "  -d, --display                     \n"\
    "  -l, --log-level=<level>           \n"\
    "  -m, --log-mask=<mask>             \n"\
    "  -f, --config-file=<file's name>   \n"\
    "  -s, --stop                        \n"\
	"  -k, --kill-all                    \n"\
	"  -i, --iconize                     \n"\
	"  -m, --maximize                    \n"\
    "  -h, --help                        \n"\
    "  -v, --version                     \n"\
    "  [CORBA parameters]                \n"

    if (errorMsg != NULL) {
        fprintf(stderr, "Error: %s" USAGE, errorMsg);
    } else {
        fprintf(stdout, USAGE);
    }

#undef USAGE
}

inline int parseSyslogLevel(const char *param) {
        int error = EXIT_SUCCESS;
        int newSyslogLevel = 0;

        if (isdigit(param[0])) /* 0x... (hexa value) ) 0.. (octal value or zero) 1 2 3 4 5 6 7 8 9 (decimal value)*/ {
            char *endptr = NULL;
            newSyslogLevel = strtoul(param, &endptr, 0);
            if (endptr != param) {
                if (newSyslogLevel > LOG_UPTO(LOG_DEBUG)) {
                    error = EINVAL;
                    ERROR_MSG("bad syslog value %d", newSyslogLevel);
                }
                DEBUG_VAR(newSyslogLevel, "%d");
            } else {
                error = EINVAL;
            }
        } else if (strcasecmp(param, "debug") == 0) {
            newSyslogLevel = LOG_UPTO(LOG_DEBUG);
            DEBUG_VAR(newSyslogLevel, "%d");
        } else if (strcasecmp(param, "info") == 0) {
            newSyslogLevel = LOG_UPTO(LOG_INFO);
            DEBUG_VAR(newSyslogLevel, "%d");
        } else if (strcasecmp(param, "notice") == 0) {
            newSyslogLevel = LOG_UPTO(LOG_NOTICE);
            DEBUG_VAR(newSyslogLevel, "%d");
        } else if (strcasecmp(param, "warning") == 0) {
            newSyslogLevel = LOG_UPTO(LOG_WARNING);
            DEBUG_VAR(newSyslogLevel, "%d");
        } else if (strcasecmp(param, "error") == 0) {
            newSyslogLevel = LOG_UPTO(LOG_ERR);
            DEBUG_VAR(newSyslogLevel, "%d");
        } else if (strcasecmp(param, "critical") == 0) {
            newSyslogLevel = LOG_UPTO(LOG_CRIT);
            DEBUG_VAR(newSyslogLevel, "%d");
        } else if (strcasecmp(param, "alert") == 0) {
            newSyslogLevel = LOG_UPTO(LOG_ALERT);
            DEBUG_VAR(newSyslogLevel, "%d");
        } else if (strcasecmp(param, "not set") == 0) {
            newSyslogLevel = INITIAL_FLAGS_VALUE;
            DEBUG_VAR(newSyslogLevel, "%d");
        }/*else if (strcasecmp(param,"emergency") == 0) emergency message are not maskable
    {
    newSyslogLevel = LOG_EMERG;
    DEBUG_VAR(newSyslogLevel,"%d");
    }*/
    else {
#ifdef _DEBUG_
        if (isalnum(param[0])) {
            DEBUG_VAR(param, "%s");
        } else {
            DEBUG_VAR(param[0], "%d");
        }
#endif /* _DEBUG_ */
        error = EINVAL;
    }

    SET_LOG_MASK(newSyslogLevel);

    return error;
}

#define FLAGS_SET           (1<<0)
#define LOG_LEVEL_SET      (1<<1)
#define LOG_MASK_SET       (1<<2)

class CmdLineParameters {
    unsigned int parametersSet;
    unsigned int flags;
    int logLevel;
    unsigned int logMask;

    inline void setFlags(const int f) {
        flags |= f;
        parametersSet |= FLAGS_SET;
        DEBUG_VAR(flags, "0x%X");
    }

    inline void setLogLevel(const int level) {
        logLevel = level;
        parametersSet |= LOG_LEVEL_SET;
        DEBUG_VAR(logLevel, "%d");
    }

    inline void setLogMask(const unsigned int mask) {
        logMask = mask;
        parametersSet |= LOG_MASK_SET;
        DEBUG_VAR(logMask, "0x%X");
    }

public:

    CmdLineParameters()
        :parametersSet(0)
        ,flags(0)
        ,logLevel(0)
        ,logMask(0) {
    }

    ~CmdLineParameters() {
    }    

    inline int parse(int argc, char *argv[], Configuration *configuration) {
        int error(EXIT_SUCCESS);
        int optc;
        
		flags = 0x0;
		parametersSet = 0x0;
		
        while (((optc = getopt_long(argc, argv, "dl:m:f:skiahv", longopts, NULL)) != -1) && (EXIT_SUCCESS == error)) {
            char errorMsg[100];
            DEBUG_VAR(optc,"%d");
            switch (optc) {                
                case 'l':
                    error = parseSyslogLevel(optarg);
                    if (EXIT_SUCCESS == error) {
                        setLogLevel(SET_LOG_MASK(0));
                    }
                    break;
                case 'm':
                    setLogMask(strtol(optarg, NULL, 0));
                    break;
                case 'f':
                    configuration->setConfigurationFile(optarg);
                    DEBUG_VAR(optarg, "(configuration filename)%s");
                    break;
                case 'd':
                    setFlags(DISPLAY_MODE);
                    configuration->setDisplayMode();                    
                    DEBUG_MSG("DISPLAY_MODE set");
                    break;
                case 's':
				    setFlags(STOP_MODE);
                    configuration->setStopMode();
					DEBUG_MSG("STOP_MODE set");
                    break;
				case 'i':
				    setFlags(MINIMIZE_WINDOW_MODE);
                    configuration->setMinimizeWindowMode();
					DEBUG_MSG("MINIMIZE_WINDOW_MODE set");
                    break;
				case 'a':
				    setFlags(MAXIMIZE_WINDOW_MODE);
                    configuration->setMaximizeWindowMode();
					DEBUG_MSG("MAXIMIZE_WINDOW_MODE set");
                    break;	
				case 'k':  
				    setFlags(KILL_ALL_ON_STARTUP);
				    configuration->setKillAllOnStartupMode();
					DEBUG_MSG("KILL ALL  ON STARTUP MODE set");
					break;
				case 'h':
                    printHelp(NULL);
                    exit(EXIT_SUCCESS);
                    break;	
                case 'v':
                    printVersion();
                    exit(EXIT_SUCCESS);
                    break;
                case '?':
                    /* getopt_long} already printed an error message.  */
                    error = EINVAL;
                    printHelp("");
                    break;
                    // ORB's optionals parameters
                default:
                    DEBUG_VAR(optc,"%d");
                    error = EINVAL;
                    printHelp("invalid option value");
                    break;
            } /* switch (optc) */
        } /* while ((optc = getopt_long (argc, argv, " */

        return error;
    }
    inline unsigned int getParametersSet() {
        return parametersSet;
    }
    inline unsigned int getFlags() {
        return flags;
    }
    inline int getLogLevel() {
        return logLevel;
    }
    inline unsigned int getLogMask() {
        return logMask;
    }
};

inline void Configuration::apply(CmdLineParameters &parameters) {
    const unsigned int parametersSet = parameters.getParametersSet();
    
    if (parametersSet & LOG_LEVEL_SET) {
        SET_LOG_MASK(parameters.getLogLevel());
    }

    if (parametersSet & LOG_MASK_SET) {
        debugFlags.mask = parameters.getLogMask();
    }

    if (parametersSet & FLAGS_SET) {
        flags |= parameters.getFlags();
    }
}

inline int Configuration::readFile(void) {
    int error(EXIT_SUCCESS);

    dictionary *dict = iniparser_load(filename.c_str());
    if (dict) {        
        unsigned int iter = 0;
        char *key = NULL;
        char *value = NULL;
        const char *logLevel = NULL;

        /* read the log level first to be able to activate the following traces
         using the configuration file */
        logLevel = iniparser_getstring(dict, "debug:level", INITIAL_LOG_LEVEL);
        error = parseSyslogLevel(logLevel);
        NOTICE_MSG("log level = %s", logLevel);

        #ifdef HAVE_DBGFLAGS_DBGFLAGS_H
        debugFlags.mask = iniparser_getuint(dict, "debug:mask", INITIAL_FLAGS_VALUE);
        NOTICE_MSG("debug flags mask = 0x%X", debugFlags.mask);
        #endif //HAVE_DBGFLAGS_DBGFLAGS_H

        //flags = 0;

        if (iniparser_getboolean(dict, "connexion:auto_change_server", 1)) {
            flags |= CHANGE_SERVER_AUTO_MODE;
            NOTICE_MSG("auto change server set");
        } else {
            NOTICE_MSG("auto change server disabled");
        }

        moveServerDelay = iniparser_getuint(dict, "connexion:auto_change_delay", INITIAL_FLAGS_VALUE);
        NOTICE_MSG("auto_change_delay = %u ms", moveServerDelay);        

        program = iniparser_getstring(dict, "program:program", "");
        NOTICE_MSG("program = %s",program.c_str());
        program_parameters = iniparser_getstring(dict, "program:parameters", "");
        NOTICE_MSG("program's parameters = %s",program_parameters.c_str());

        if (iniparser_getboolean(dict, "connexion:kill_all_on_startup", 1)) {
           flags |= KILL_ALL_ON_STARTUP;
           NOTICE_MSG("kill all running instance of %s on startup",program.c_str());
        } else {
           NOTICE_MSG("kill all running instance of %s on startup disabled",program.c_str());
        }

        if (iniparser_getboolean(dict, "program:autorestart",0)) {
            flags |= AUTORESTART_CHILD;
            NOTICE_MSG("auto restart program set");
        } else {
            NOTICE_MSG("auto restart program disabled");
        }

        killDelay = iniparser_getuint(dict, "program:kill_delay",KILL_DELAY) * 1000; /* ms to usec */
        NOTICE_MSG("kill_delay = %u ms", moveServerDelay);

#ifdef ENABLE_X11_MANAGEMENT
        const char *mode = iniparser_getstring(dict, "program:mode","");
        if (strcasecmp(mode,"window") == 0) {
            setXWindowsManagementMode();
            NOTICE_MSG("X11 Windows Management enabled");
        }

        const char *X11Mode = iniparser_getstring(dict, "X11:mode","");
        if (strcasecmp(X11Mode,"title") == 0) {
            flags |= X11_TITLE;
            NOTICE_MSG("X11 Windows mode = title");
        } else if (strcasecmp(X11Mode,"subtitle") == 0) {
            flags |= X11_SUBTITLE;
            NOTICE_MSG("X11 Windows mode = subtitle");
        } else if (strcasecmp(X11Mode,"class") == 0) {
            flags |= X11_CLASS;
            NOTICE_MSG("X11 Windows mode = class");
        } else if (strcasecmp(X11Mode,"pid") == 0) {
            flags |= X11_PID;
            NOTICE_MSG("X11 Windows mode = pid");
        } else {
            flags |= X11_CLASS;
            NOTICE_MSG("X11 Windows mode (default) = class (%s)",X11Mode);
        }
        
        xwin_parameter = iniparser_getstring(dict, "X11:parameter","");
        NOTICE_MSG("XWindow parameter = %s",xwin_parameter.c_str());
#endif //ENABLE_X11_MANAGEMENT

        iter=0;
        error = iniparser_findfirst(dict, "servers", &key, &value, &iter);
        if (EXIT_SUCCESS == error) {
            unsigned int n = 0;
            while (EXIT_SUCCESS == error) {
                servers.push_back(value);
                NOTICE_MSG("(%d) %s = %s ", n, key, servers[n].c_str());
                n++;
                error = iniparser_findnext(dict, "servers", &key, &value, &iter);
            }
            error = EXIT_SUCCESS;
            if (gettimeofday(&currentServerStartOfUse, NULL) != 0) {
                error = errno;
                ERROR_MSG("gettimeofday error %d (%m)", error);
            }
            currentServerPosition = 0;
        } else {
            CRIT_MSG("no server defined in the servers section of the configuration file %s", filename.c_str());
        }

        username = iniparser_getstring(dict, "login:username", "");
	NOTICE_MSG("username = %s",username.c_str());
        password = iniparser_getstring(dict, "login:password", "");
	NOTICE_MSG("password = %s",password.c_str());

        findNReplace<std::string>("%U",username,program_parameters);
        findNReplace<std::string>("%P",password,program_parameters);

        iniparser_freedict(dict);
        dict = NULL;
    } else {
        error = errno; /* error already logged */
    }    

    if (ENOENT == error) { /* configuration file not found */
        static unsigned int nbRetries(0);
        if (0 == nbRetries) {
            ++nbRetries;
            INFO_MSG("configuration file %s not found, trying to read /etc" CONFIGURATION_FILENAME " configuration file instead", filename.c_str());
            filename = "/etc" CONFIGURATION_FILENAME;
            error = readFile();
        }        
    }

    return error;
}

inline void Configuration::removeORBParameters(int argc, char *argv[],std::vector<char*> &RDPMgrParameters) {    
    int i = 0;

    while(i<argc) {
        if (strncasecmp(argv[i],"-ORB",4) != 0) {
            RDPMgrParameters.push_back(argv[i]);
            ++i;
        } else {
            // ORB's parameter, check if the next one is its argument
            ++i;
            if (argv[i][0] != '-') {
                ++i;
            }
        }
    }  //while(i<argc)
}

int Configuration::init(int argc, char *argv[]) {
    int error(EXIT_SUCCESS);
    CmdLineParameters parameters;
    //rwLock readWriteLock(lock);
    writeLockMgr writeLock(lock);
    std::vector<char*> RDPMgrParameters;

	// initialization
    removeORBParameters(argc,argv,RDPMgrParameters);
	flags = 0x0;
	
    // first parse cmd line parameters (because the configuration file parameter could be defined there)
    error = parameters.parse(RDPMgrParameters.size(), (char**)RDPMgrParameters.data(),this);
    if (EXIT_SUCCESS == error) {
        // then read the configuration file
        error = readFile();
        // then override the parameters from the configuration file with the ones set from cmdLine
        apply(parameters);
    }
    return error;
}

