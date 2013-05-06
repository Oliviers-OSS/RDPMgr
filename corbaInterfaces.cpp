#include "globals.h"
#include "remoteServersSetS.h"
#include "remoteServersSetManagementImpl.h"
#include "remoteServersSetEventSubscriberImpl.h"
#include <orbsvcs/CosNamingC.h>
#include <unistd.h>

#define MODULE_FLAG FLAG_CORBA

static inline void registerServants(CosNaming::NamingContext_var initialNamingContext, ::RemoteServersSet::Management_var managementServant, ::RemoteServersSet::EventSubscriber_var eventSubscriber) {
    CosNaming::Name name;
    char hostname[PATH_MAX];
    int error = gethostname(hostname, sizeof (hostname));
    if (0 == error) {
        // create the RDPMgr naming context
        name.length(1);
        name[0].id = CORBA::string_dup(PACKAGE);
        name[0].kind = CORBA::string_dup("");
        CosNaming::NamingContext_var RDPMgrRootNamingContext = initialNamingContext->new_context();
        try {
            //initialNamingContext->rebind_context(name, RDPMgrRootNamingContext);
            initialNamingContext->bind_context(name, RDPMgrRootNamingContext);
        }  catch (CosNaming::NamingContext::AlreadyBound &exception) {
            INFO_MSG("CosNaming::NamingContext::AlreadyBound exception fired for " PACKAGE);
        }

        name.length(2);        
        name[1].id = CORBA::string_dup(hostname);
        name[1].kind = CORBA::string_dup("");
        CosNaming::NamingContext_var RDPMgrHostNamingContext = initialNamingContext->new_context();
        //try {
            //initialNamingContext->bind_context(name, RDPMgrHostNamingContext);
            initialNamingContext->rebind_context(name, RDPMgrHostNamingContext);
        //}  catch (CosNaming::NamingContext::AlreadyBound &exception) {
        //    INFO_MSG("CosNaming::NamingContext::AlreadyBound exception fired for " PACKAGE);
        //}

        // register the server in their naming context.
        name.length(3);
        name[2].id = CORBA::string_dup("management");
        name[2].kind = CORBA::string_dup(CORBA_NS_KIND);
        initialNamingContext->rebind(name, managementServant);

        name[2].id = CORBA::string_dup("eventSubscriber");
        name[2].kind = CORBA::string_dup(CORBA_NS_KIND);
        initialNamingContext->rebind(name, eventSubscriber);
    } else {
        error = errno;
        ERROR_MSG("gethostname error %d (%m)", error);
    }
}

static inline void unregisterServants(CosNaming::NamingContext_var initialNamingContext) {
    CosNaming::Name name;
    char hostname[PATH_MAX];
    int error = gethostname(hostname,sizeof(hostname));
    if (0 == error) {
        name.length(2);
        name[0].id = CORBA::string_dup(PACKAGE);
        name[0].kind = CORBA::string_dup("");
        name[1].id = CORBA::string_dup("management");
        name[1].kind = CORBA::string_dup(CORBA_NS_KIND);
        initialNamingContext->unbind(name);

        name.length(2);
        name[1].id = CORBA::string_dup("eventSubscriber");
        name[1].kind = CORBA::string_dup(CORBA_NS_KIND);
        initialNamingContext->unbind(name);
    } else {
        error = errno;
        ERROR_MSG("gethostname error %d (%m)",error);
    }
}

int startRemoteServersSetInterface(int argc, char **argv) {
    int error(EXIT_SUCCESS);
    try {
        // init ORB.
        CORBA::ORB_var orb = CORBA::ORB_init(argc, argv);

        // activate root POA.
        CORBA::Object_var poa_object = orb->resolve_initial_references("RootPOA");
        PortableServer::POA_var root_poa = PortableServer::POA::_narrow(poa_object);
        PortableServer::POAManager_var poa_manager = root_poa->the_POAManager();
        poa_manager->activate();

        // create corba servants objects.
        remoteServersSetManagementImpl managementImpl;
        ::RemoteServersSet::Management_var managementServant = managementImpl._this();

        remoteServersSetEventSubscriberImpl eventSubscriberImpl;
        ::RemoteServersSet::EventSubscriber_var eventSubscriber = eventSubscriberImpl._this();
        try { //else the process will crash in case of NameService failure because of the ORB hasn't been cleanup
            // get reference to naming service.
            CORBA::Object_var initialNamingContextObject = orb->resolve_initial_references("NameService");
            if (!CORBA::is_nil(initialNamingContextObject)) {
                CosNaming::NamingContext_var initialNamingContext = CosNaming::NamingContext::_narrow(initialNamingContextObject);
                if (!CORBA::is_nil(initialNamingContext)) {
                    registerServants(initialNamingContext,managementServant,eventSubscriber);

                    // run the ORB & wait for clients
                    orb->run();

                    unregisterServants(initialNamingContext);
                } else {
                    ERROR_MSG("naming_context NameService is nil !");
                    error = EPROTO;
                }
            } else {
                ERROR_MSG("resolve_initial_references NameService is nil !");
                error = EPROTO;
            }
        } //try
        catch (CORBA::Exception & ex) {
            ERROR_STREAM << "CORBA::Exception " << ex << std::endl;
            error = EIO; //TODO: set better (real ?) error code number
        }
        catch(...) {
            ERROR_MSG("catch all: exception caught !");
            error = EFAULT;
        }
        // clean up.
        root_poa->destroy(1, 1);
        orb->destroy();

    }//try
    catch (CORBA::Exception & ex) {
        ERROR_STREAM << "CORBA::Exception " << ex << std::endl;
        error = EIO; //TODO: set better (real ?) error code number
    }

    DEBUG_VAR(error, "%d");
    return error;
}
