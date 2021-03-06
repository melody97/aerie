/**
 * \file rwproxy.h
 *
 * \brief Read-write proxy to a read-only persistent object
 * 
 */

#ifndef __STAMNOS_OSD_CLIENT_RW_PROXY_H
#define __STAMNOS_OSD_CLIENT_RW_PROXY_H

#include "osd/main/client/proxy.h"
#include <assert.h>
#include "osd/main/client/omgr.h"
#include "osd/main/client/session.h"


namespace osd {
namespace client {
namespace rw {

typedef ::osd::client::OsdSession OsdSession;

template<class Subject, class VersionManager> class ObjectProxy;

/**
 * \brief Implementation of type specific persistent object manager
 */
template<class Subject, class VersionManager>
class ObjectManager: public osd::client::ObjectManagerOfType {
public:
	osd::client::ObjectProxy* Load(OsdSession* session, ObjectId oid) {
		// insight : DO ME : Verify that here is where new object proxy is created
//		printf("\nVerifying 1. Inside ObjectManager::Load...");
		// insight : oid gets used here 
		osd::client::ObjectProxy* proxy = new ObjectProxy<Subject, VersionManager>(session, oid);
		// insight : For ObjectProxy, see below
		return proxy;
	}
	
	void Close(OsdSession* session, ObjectId oid, bool update) {
		ObjectProxy<Subject, VersionManager>* obj_proxy;
		osd::client::ObjectProxy*             obj2_proxy;
		assert(oid2obj_map_.Lookup(oid, &obj2_proxy) == E_SUCCESS);
		obj_proxy = static_cast<ObjectProxy<Subject, VersionManager>* >(obj2_proxy);
		assert(obj_proxy->vClose(session, update) == E_SUCCESS);
	}
	
	void CloseAll(OsdSession* session, bool update, bool flush = false) 
	{
		ObjectProxy<Subject, VersionManager>* obj_proxy;
		osd::client::ObjectProxy*             obj2_proxy;
		ObjectMap::iterator                   itr;

		for (itr = oid2obj_map_.begin(); itr != oid2obj_map_.end(); itr++) {
			obj2_proxy = itr->second;
			DBG_LOG(DBG_DEBUG, DBG_MODULE(client_omgr), "Close object: %lx\n", 
			        obj2_proxy->object()->oid().u64());
			obj_proxy = static_cast<ObjectProxy<Subject, VersionManager>* >(obj2_proxy);
			assert(obj_proxy->vClose(session, update, flush) == E_SUCCESS);
		}
	}
};


template<class Subject, class VersionManager>
// insight : osd::client::rw 
class ObjectProxyReference: public osd::common::ObjectProxyReference {
public:
	ObjectProxyReference(void* owner = NULL)
		: osd::common::ObjectProxyReference(owner)
	{ }

	ObjectProxy<Subject, VersionManager>* proxy() { 
//		printf("\nInside osd::client::rw::ObjectProxyReference::proxy().");
		return static_cast<ObjectProxy<Subject, VersionManager>*>(proxy_);
	}
//	void printme() { printf("\n* Hailing from ObjectProxyReference."); }
};



/**
 *
 *
 *
 */
template<class Subject, class VersionManager>
class ObjectProxy: public osd::vm::client::ObjectProxy< ObjectProxy<Subject, VersionManager>, Subject, VersionManager> {
// insight : Due to its inheritance, this class gets vm_ member
// insight : Checkout osd/main/client/proxy.h
// insight : namspace : osd::client:rw
public:
	ObjectProxy(OsdSession* session, ObjectId oid)
		: osd::vm::client::ObjectProxy<ObjectProxy<Subject, VersionManager>, Subject, VersionManager>(session, oid),
	      session_(session)
	{
		// insight : DO ME : verify if this is the ObjectProxy constructor getting called when creating a new object.
//		printf("\nVerifying 2 : Inside ObjectProxy()...");
	}

	int Lock(OsdSession* session, lock_protocol::Mode mode) {
		int ret;
		
		// TODO: check if object is private or public. if public then lock. 

		if ((ret = osd::cc::client::ObjectProxy::Lock(session, mode)) != lock_protocol::OK) {
			return ret;
		}
		assert((osd::vm::client::ObjectProxy<ObjectProxy<Subject, VersionManager>, Subject, VersionManager>::vOpen() == 0));
		return lock_protocol::OK;
	}

	int Lock(OsdSession* session, osd::cc::client::ObjectProxy* parent, lock_protocol::Mode mode, int flags = 0) {
		int ret;
		
		// TODO: check if object is private or public. if public then lock. 
		
		if ((ret = osd::cc::client::ObjectProxy::Lock(session, parent, mode, flags)) != lock_protocol::OK) {
			return ret;
		}
		assert((osd::vm::client::ObjectProxy<ObjectProxy<Subject, VersionManager>, Subject, VersionManager>::vOpen() == 0));
		return lock_protocol::OK;

	}

	int Unlock(OsdSession* session) {
		// TODO: check if object is private or public. if public then unlock. 
		return osd::cc::client::ObjectProxy::Unlock(session);
	}
// ~ObjectProxy() { printf("\n + Destroying ObjectProxy 004. in src/osd/main/client/rwproxy.h");}

private:
	OsdSession* session_;
};


} // namespace rw
} // namespace client
} // namespace osd

#endif // __STAMNOS_OSD_CLIENT_RWPROXY_H
