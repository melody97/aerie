/// \file hlckmgr.h
///
/// \brief Server hierarchical lock manager

#ifndef __STAMNOS_OSD_SERVER_HIERARCHICAL_LOCK_MANAGER_H
#define __STAMNOS_OSD_SERVER_HIERARCHICAL_LOCK_MANAGER_H

#include <pthread.h>
#include "osd/main/server/lckmgr.h"
#include "bcs/bcs.h"


namespace osd {
namespace cc {
namespace server {


class HLockManager {
public:
	HLockManager(::server::Ipc* ipc);
	~HLockManager();

	int Init();

	lock_protocol::status Stat(lock_protocol::LockId, int&);
	lock_protocol::status Acquire(int clt, int seq, unsigned long long lid_u64, int mode_set, int flags, unsigned long long arg, int& mode_granted);
	lock_protocol::status Convert(int clt, int seq, unsigned long long lid_u64, int mode, int flags, int& unused);
	lock_protocol::status Release(int clt, int seq, unsigned long long lid_u64, int flags, int& unused);
	
	lock_protocol::Mode LockMode(int clt, lock_protocol::LockId lid) {
		return lm_->LockMode(clt, lid);
	}

private:
	::server::Ipc*  ipc_;
	LockManager*    lm_;
	pthread_mutex_t mutex_;
};


} // namespace server
} // namespace cc
} // namespace osd

#endif // __STAMNOS_OSD_SERVER_HIERARCHICAL_LOCK_MANAGER_H
