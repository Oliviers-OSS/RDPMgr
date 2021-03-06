#include "globals.h"

emergencyLogger logEmergency;
alertLogger logAlert;
criticalLogger logCritical;
errorLogger logError;
warningLogger logWarning;
noticeLogger logNotice;
infoLogger logInfo;
debugLogger logDebug;
contextLogger logContext;

#ifdef HAVE_DBGFLAGS_DBGFLAGS_H

#include "configuration.h"

#define MODULE_FLAG FLAG_RUNTIME

static void helpCommands(FILE *stream);
static int dbgCommandsHandler(int argc, char *argv[],FILE *stream);

DebugFlags debugFlags =
{
  identity,
  {
     "Init"
    ,"Singleton"
    ,"Runtime"
    ,"Monitor"
    ,"Corba"
    ,"Configuration"
    ,"Notification"
    ,"Deadlock"
    ,"XWindows"
    ,""
    ,""
    ,""
    ,""
    ,""
    ,""
    ,""
    ,""
    ,""
    ,""
    ,""
    ,""
    ,""
    ,""
    ,""
    ,""
    ,""
    ,""
    ,""
    ,""
    ,""
    ,""
    ,""}
  ,INITIAL_FLAGS_VALUE
  ,{helpCommands,dbgCommandsHandler}
};

static void helpCommands(FILE *stream) {
    fprintf(stream, "debug help commands:\n\tcurrentServer\n");
}

static int dbgCommandsHandler(int argc, char *argv[],FILE *stream) {
    int status = EINVAL;
    if (argc > 1) {
        if (strcmp(argv[0], "currentServer") == 0) {
            struct timeval currentServerStartOfUse;
            const char *currentServer = theConfiguration.getCurrentServer(&currentServerStartOfUse);
            fprintf(stream,"current server is %s since %d seconds\n",currentServer,currentServerStartOfUse.tv_sec);
        } else { /* unknow/invalid cmd */
            status = EINVAL;
            ERROR_MSG("unknow or invalid cmd received");
        }
    } else { /*! argc > 1 */
        ERROR_MSG("bad debug cmd received");
        status = EINVAL;
    }

    DEBUG_VAR(status, "%d");
    return status;
}

#endif /* HAVE_DBGFLAGS_DBGFLAGS_H */

