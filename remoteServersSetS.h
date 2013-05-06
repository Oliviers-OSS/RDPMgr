// -*- C++ -*-
//
// $Id$

// ****  Code generated by the The ACE ORB (TAO) IDL Compiler ****
// TAO and the TAO IDL Compiler have been developed by:
//       Center for Distributed Object Computing
//       Washington University
//       St. Louis, MO
//       USA
//       http://www.cs.wustl.edu/~schmidt/doc-center.html
// and
//       Distributed Object Computing Laboratory
//       University of California at Irvine
//       Irvine, CA
//       USA
//       http://doc.ece.uci.edu/
// and
//       Institute for Software Integrated Systems
//       Vanderbilt University
//       Nashville, TN
//       USA
//       http://www.isis.vanderbilt.edu/
//
// Information about TAO is available at:
//     http://www.cs.wustl.edu/~schmidt/TAO.html

// TAO_IDL - Generated from
// be/be_codegen.cpp:387

#ifndef _TAO_IDL_REMOTESERVERSSETS_H_
#define _TAO_IDL_REMOTESERVERSSETS_H_


#include "remoteServersSetC.h"

#if !defined (ACE_LACKS_PRAGMA_ONCE)
# pragma once
#endif /* ACE_LACKS_PRAGMA_ONCE */

#include "tao/Collocation_Proxy_Broker.h"
#include "tao/PortableServer/PortableServer.h"
#include "tao/PortableServer/Servant_Base.h"

// TAO_IDL - Generated from
// be/be_visitor_module/module_sh.cpp:49

namespace POA_RemoteServersSet
{
  
  
  // TAO_IDL - Generated from
  // be/be_visitor_interface/interface_sh.cpp:87
  
  class Management;
  typedef Management *Management_ptr;
  
  class  Management
    : public virtual PortableServer::ServantBase
  {
  protected:
    Management (void);
  
  public:
    // Useful for template programming.
    typedef ::RemoteServersSet::Management _stub_type;
    typedef ::RemoteServersSet::Management_ptr _stub_ptr_type;
    typedef ::RemoteServersSet::Management_var _stub_var_type;
    
    Management (const Management& rhs);
    virtual ~Management (void);
    
    virtual ::CORBA::Boolean _is_a (
        const char* logical_type_id
        ACE_ENV_ARG_DECL_WITH_DEFAULTS
      );
    
    static void _is_a_skel (
        TAO_ServerRequest & req,
        void * servant_upcall,
        void * servant
        ACE_ENV_ARG_DECL
      );
    
    static void _non_existent_skel (
        TAO_ServerRequest & req,
        void * servant_upcall,
        void * servant
        ACE_ENV_ARG_DECL
      );
    
    static void _interface_skel (
        TAO_ServerRequest & req,
        void * servant_upcall,
        void * servant
        ACE_ENV_ARG_DECL
      );
    
    static void _component_skel (
        TAO_ServerRequest & req,
        void * servant_upcall,
        void * servant
        ACE_ENV_ARG_DECL
      );
    
    static void _repository_id_skel (
        TAO_ServerRequest & req,
        void * servant_upcall,
        void * servant
        ACE_ENV_ARG_DECL
      );
    
    virtual void _dispatch (
        TAO_ServerRequest & req,
        void * servant_upcall
        ACE_ENV_ARG_DECL
      );
    
    ::RemoteServersSet::Management *_this (
        
        ACE_ENV_ARG_DECL_WITH_DEFAULTS
      );
    
    virtual const char* _interface_repository_id (void) const;
    
    // TAO_IDL - Generated from
    // be/be_visitor_operation/operation_sh.cpp:45
    
    virtual char * currentServer (
        
      )
      ACE_THROW_SPEC ((
        ::CORBA::SystemException
      )) = 0;
    
    static void _get_currentServer_skel (
        TAO_ServerRequest & server_request,
        void * servant_upcall,
        void * servant
        ACE_ENV_ARG_DECL
      );
    
    // TAO_IDL - Generated from
    // be/be_visitor_operation/operation_sh.cpp:45
    
    virtual void currentServer (
        const char * currentServer
      )
      ACE_THROW_SPEC ((
        ::CORBA::SystemException
      )) = 0;
    
    static void _set_currentServer_skel (
        TAO_ServerRequest & server_request,
        void * servant_upcall,
        void * servant
        ACE_ENV_ARG_DECL
      );
    
    // TAO_IDL - Generated from
    // be/be_visitor_operation/operation_sh.cpp:45
    
    virtual ::CORBA::Boolean autoMoveMode (
        
      )
      ACE_THROW_SPEC ((
        ::CORBA::SystemException
      )) = 0;
    
    static void _get_autoMoveMode_skel (
        TAO_ServerRequest & server_request,
        void * servant_upcall,
        void * servant
        ACE_ENV_ARG_DECL
      );
    
    // TAO_IDL - Generated from
    // be/be_visitor_operation/operation_sh.cpp:45
    
    virtual void autoMoveMode (
        ::CORBA::Boolean autoMoveMode
      )
      ACE_THROW_SPEC ((
        ::CORBA::SystemException
      )) = 0;
    
    static void _set_autoMoveMode_skel (
        TAO_ServerRequest & server_request,
        void * servant_upcall,
        void * servant
        ACE_ENV_ARG_DECL
      );
    
    // TAO_IDL - Generated from
    // be/be_visitor_operation/operation_sh.cpp:45
    
    virtual ::CORBA::Long currentCommStatus (
        
      )
      ACE_THROW_SPEC ((
        ::CORBA::SystemException
      )) = 0;
    
    static void _get_currentCommStatus_skel (
        TAO_ServerRequest & server_request,
        void * servant_upcall,
        void * servant
        ACE_ENV_ARG_DECL
      );
    
    // TAO_IDL - Generated from
    // be/be_visitor_operation/operation_sh.cpp:45
    
    virtual ::CORBA::Long start (
        
      )
      ACE_THROW_SPEC ((
        ::CORBA::SystemException
      )) = 0;
    
    static void start_skel (
        TAO_ServerRequest & server_request,
        void * servant_upcall,
        void * servant
        ACE_ENV_ARG_DECL
      );
  };
  
  // TAO_IDL - Generated from
  // be/be_visitor_interface/interface_sh.cpp:87
  
  class EventListener;
  typedef EventListener *EventListener_ptr;
  
  class  EventListener
    : public virtual PortableServer::ServantBase
  {
  protected:
    EventListener (void);
  
  public:
    // Useful for template programming.
    typedef ::RemoteServersSet::EventListener _stub_type;
    typedef ::RemoteServersSet::EventListener_ptr _stub_ptr_type;
    typedef ::RemoteServersSet::EventListener_var _stub_var_type;
    
    EventListener (const EventListener& rhs);
    virtual ~EventListener (void);
    
    virtual ::CORBA::Boolean _is_a (
        const char* logical_type_id
        ACE_ENV_ARG_DECL_WITH_DEFAULTS
      );
    
    static void _is_a_skel (
        TAO_ServerRequest & req,
        void * servant_upcall,
        void * servant
        ACE_ENV_ARG_DECL
      );
    
    static void _non_existent_skel (
        TAO_ServerRequest & req,
        void * servant_upcall,
        void * servant
        ACE_ENV_ARG_DECL
      );
    
    static void _interface_skel (
        TAO_ServerRequest & req,
        void * servant_upcall,
        void * servant
        ACE_ENV_ARG_DECL
      );
    
    static void _component_skel (
        TAO_ServerRequest & req,
        void * servant_upcall,
        void * servant
        ACE_ENV_ARG_DECL
      );
    
    static void _repository_id_skel (
        TAO_ServerRequest & req,
        void * servant_upcall,
        void * servant
        ACE_ENV_ARG_DECL
      );
    
    virtual void _dispatch (
        TAO_ServerRequest & req,
        void * servant_upcall
        ACE_ENV_ARG_DECL
      );
    
    ::RemoteServersSet::EventListener *_this (
        
        ACE_ENV_ARG_DECL_WITH_DEFAULTS
      );
    
    virtual const char* _interface_repository_id (void) const;
    
    // TAO_IDL - Generated from
    // be/be_visitor_operation/operation_sh.cpp:45
    
    virtual void notifyEvent (
        const ::RemoteServersSet::Event & newEvent
      )
      ACE_THROW_SPEC ((
        ::CORBA::SystemException
      )) = 0;
    
    static void notifyEvent_skel (
        TAO_ServerRequest & server_request,
        void * servant_upcall,
        void * servant
        ACE_ENV_ARG_DECL
      );
  };
  
  // TAO_IDL - Generated from
  // be/be_visitor_interface/interface_sh.cpp:87
  
  class EventSubscriber;
  typedef EventSubscriber *EventSubscriber_ptr;
  
  class  EventSubscriber
    : public virtual PortableServer::ServantBase
  {
  protected:
    EventSubscriber (void);
  
  public:
    // Useful for template programming.
    typedef ::RemoteServersSet::EventSubscriber _stub_type;
    typedef ::RemoteServersSet::EventSubscriber_ptr _stub_ptr_type;
    typedef ::RemoteServersSet::EventSubscriber_var _stub_var_type;
    
    EventSubscriber (const EventSubscriber& rhs);
    virtual ~EventSubscriber (void);
    
    virtual ::CORBA::Boolean _is_a (
        const char* logical_type_id
        ACE_ENV_ARG_DECL_WITH_DEFAULTS
      );
    
    static void _is_a_skel (
        TAO_ServerRequest & req,
        void * servant_upcall,
        void * servant
        ACE_ENV_ARG_DECL
      );
    
    static void _non_existent_skel (
        TAO_ServerRequest & req,
        void * servant_upcall,
        void * servant
        ACE_ENV_ARG_DECL
      );
    
    static void _interface_skel (
        TAO_ServerRequest & req,
        void * servant_upcall,
        void * servant
        ACE_ENV_ARG_DECL
      );
    
    static void _component_skel (
        TAO_ServerRequest & req,
        void * servant_upcall,
        void * servant
        ACE_ENV_ARG_DECL
      );
    
    static void _repository_id_skel (
        TAO_ServerRequest & req,
        void * servant_upcall,
        void * servant
        ACE_ENV_ARG_DECL
      );
    
    virtual void _dispatch (
        TAO_ServerRequest & req,
        void * servant_upcall
        ACE_ENV_ARG_DECL
      );
    
    ::RemoteServersSet::EventSubscriber *_this (
        
        ACE_ENV_ARG_DECL_WITH_DEFAULTS
      );
    
    virtual const char* _interface_repository_id (void) const;
    
    // TAO_IDL - Generated from
    // be/be_visitor_operation/operation_sh.cpp:45
    
    virtual ::CORBA::ULong subscribe (
        ::RemoteServersSet::EventListener_ptr clientsInterface,
        ::CORBA::ULong filter
      )
      ACE_THROW_SPEC ((
        ::CORBA::SystemException
      )) = 0;
    
    static void subscribe_skel (
        TAO_ServerRequest & server_request,
        void * servant_upcall,
        void * servant
        ACE_ENV_ARG_DECL
      );
    
    // TAO_IDL - Generated from
    // be/be_visitor_operation/operation_sh.cpp:45
    
    virtual ::CORBA::ULong unsubscribe (
        ::RemoteServersSet::EventListener_ptr clientsInterface
      )
      ACE_THROW_SPEC ((
        ::CORBA::SystemException
      )) = 0;
    
    static void unsubscribe_skel (
        TAO_ServerRequest & server_request,
        void * servant_upcall,
        void * servant
        ACE_ENV_ARG_DECL
      );
  };

// TAO_IDL - Generated from
// be/be_visitor_module/module_sh.cpp:80

} // module RemoteServersSet

// TAO_IDL - Generated from 
// be/be_codegen.cpp:1148

#include "remoteServersSetS_T.h"

#if defined (__ACE_INLINE__)
#include "remoteServersSetS.inl"
#endif /* defined INLINE */

#endif /* ifndef */
