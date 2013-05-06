/* 
 * File:   remoteServersSetImpl.cpp
 * Author: oc
 * 
 * Created on November 12, 2010, 9:07 AM
 */

#include "remoteServersSetManagementImpl.h"
#include "globals.h"
#include "configuration.h"

#define MODULE_FLAG FLAG_CORBA

remoteServersSetManagementImpl::remoteServersSetManagementImpl() ACE_THROW_SPEC ((CORBA::SystemException)) {
}

remoteServersSetManagementImpl::remoteServersSetManagementImpl(const remoteServersSetManagementImpl& orig) ACE_THROW_SPEC ((CORBA::SystemException)){
}

remoteServersSetManagementImpl::~remoteServersSetManagementImpl() {
}

char* remoteServersSetManagementImpl::currentServer ()  ACE_THROW_SPEC ((CORBA::SystemException)) {
    const char *currentServer = theConfiguration.getCurrentServer(NULL);
    DEBUG_VAR(currentServer,"%s");
    return CORBA::string_dup(const_cast<char*>(currentServer));
}

void remoteServersSetManagementImpl::currentServer (const char * currentServer)  ACE_THROW_SPEC ((CORBA::SystemException)) {
   int error(theConfiguration.setCurrentServer(currentServer));
   DEBUG_VAR(error,"%d");
   DEBUG_VAR(currentServer,"%s");
   try {
       if (EXIT_SUCCESS == error) {
           if (currentProcessMgr != NULL) {
                if (currentProcessMgr->isRunning()) {
                   error = currentProcessMgr->restart(); //error already printed
                } else {
                  INFO_MSG("no external process is currently running");
                }
           } else {
               error = EFAULT;
               ERROR_MSG("currentProcessMgr ptr is NULL");
           }
       }
   } //try
   catch(...){
       error = EFAULT;
       ERROR_MSG("bad currentProcessMgr ptr");
   }
   if ((error != EXIT_SUCCESS) && (error != EEXIST)) {
       throw RemoteServersSet::error(error);
   }
}

CORBA::Boolean remoteServersSetManagementImpl::autoMoveMode ()  ACE_THROW_SPEC ((CORBA::SystemException)) {
    CORBA::Boolean currentMode(theConfiguration.isChangeServerAutoModeSet());
    DEBUG_VAR_BOOL(currentMode);
    return currentMode;
}

void remoteServersSetManagementImpl::autoMoveMode (::CORBA::Boolean autoMoveMode)  ACE_THROW_SPEC ((CORBA::SystemException)) {
    DEBUG_VAR_BOOL(autoMoveMode);
    if (autoMoveMode) {
        theConfiguration.setChangeServerAutoMode();
    } else {
        theConfiguration.unsetChangeServerAutoMode();
    }
}

CORBA::Long remoteServersSetManagementImpl::currentCommStatus () ACE_THROW_SPEC ((CORBA::SystemException)) {
    CORBA::Long currentStatus(EXIT_SUCCESS);
    DEBUG_VAR(currentStatus,"%d");
    return currentStatus;
}

CORBA::Long remoteServersSetManagementImpl::start () ACE_THROW_SPEC ((CORBA::SystemException)) {
   CORBA::Long  error(ENOSYS);
   return error;
}
