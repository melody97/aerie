#ifndef __STAMNOS_TEST_CLIENT_FIXTURE_H
#define __STAMNOS_TEST_CLIENT_FIXTURE_H

#include "client/session.h"

namespace dpo {
namespace client {

#define __STAMNOS_DPO_CLIENT_STORAGE_MANAGER_H // ugly trick to prevent smgr.h from redefining 

// StorageManager
class StorageManager {
public:
	int Alloc(::client::Session* session, size_t nbytes, std::type_info const& typid, void** ptr)
	{
		*ptr = malloc(nbytes);
		return E_SUCCESS;
	}
};

} // namespace client
} // namespace dpo

struct ClientFixture 
{
	ClientFixture() 
		: session(NULL)
	{ 
		dpo::client::StorageManager* smgr = new dpo::client::StorageManager();
		session = new client::Session(NULL, NULL, smgr, NULL);
	}

	~ClientFixture() 
	{
		delete session->smgr_;
		delete session;
	}
	
	client::Session* session;
};

#endif // __STAMNOS_TEST_CLIENT_FIXTURE_H
