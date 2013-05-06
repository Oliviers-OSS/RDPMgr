#include "globals.h"
#include "configuration.h"
#include "singleton.h"
#include "messagesMgr.h"
#include "corbaInterfaces.h"
#include "logConfigurationMgr.h"

#define MODULE_FLAG FLAG_RUNTIME

Process *currentProcessMgr = NULL;

int main(int argc, char *argv[]) {
  int error(EXIT_SUCCESS);
  NOTICE_MSG(PACKAGE_NAME " (v" PACKAGE_VERSION ") build on the " __DATE__ " " __TIME__ " started");
  LogConfigurationMgr logConfigurationMgr;
  
  error = theConfiguration.init(argc,argv);  
  if (error == EXIT_SUCCESS) {
      Singleton singleton;
      
      if (singleton.isValid()) {
          // only one instance is running...
          Process monitoredProcess;

          if (theConfiguration.killAllOnStartup()) {
             const std::string program = theConfiguration.getProgram();
             stopAllRunningProcess(program);
          }

          if (!theConfiguration.stopMode()) {
          currentProcessMgr = &monitoredProcess;
          error = monitoredProcess.run(theConfiguration.displayMode());
          if (error == EXIT_SUCCESS) {
              // start the CORBA interface
              error = startRemoteServersSetInterface(argc,argv);
              if (error != EXIT_SUCCESS) {
                 ERROR_MSG("startRemoteServersSetInterface error %d",error);
              }
          } // error already printed
          currentProcessMgr = NULL;
          } //(!theConfiguration.stopMode())
      } else { // !singleton.isvalid()
          WriteMessagesMgr messagesMgr;
          Message msg;
		  
		  if (theConfiguration.displayMode()) {
		    msg.setContent(DISPLAY_MSG);
			DEBUG_MSG("sending DISPLAY_MSG");
		  } else if (theConfiguration.stopMode()) {
			msg.setContent(STOP_MSG);
			DEBUG_MSG("sending STOP_MSG");
		  } else if (theConfiguration.minimizeWindowMode()) {
		    msg.setContent(ICONIZE_MSG);
			DEBUG_MSG("sending ICONIZE_MSG"); 
		  } else if (theConfiguration.maximizeWindowMode()) {
		    msg.setContent(MAXIMIZE_MSG);
			DEBUG_MSG("sending MAXIMIZE_MSG"); 
		  } else {
		    msg.setContent(DISPLAY_MSG);
			DEBUG_MSG("sending (default) DISPLAY_MSG");
		  }

          error = messagesMgr.send(msg);
          if (error != EXIT_SUCCESS) {
              ERROR_MSG("error sending message %d",error);
          } else {
              DEBUG_MSG("message send");
          }
      }
    } // error already printed
    NOTICE_MSG(PACKAGE_NAME " (v" PACKAGE_VERSION ") build on the " __DATE__ " " __TIME__ " ended with status %d", error);

   return error;
}
