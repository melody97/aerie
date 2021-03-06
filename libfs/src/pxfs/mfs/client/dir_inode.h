#ifndef __STAMNOS_MFS_CLIENT_DIRECTORY_INODE_H
#define __STAMNOS_MFS_CLIENT_DIRECTORY_INODE_H

#include <stdint.h>
#include <google/dense_hash_map>
#include "pxfs/common/types.h"
#include "pxfs/common/const.h"
#include "pxfs/client/inode.h"
#include "osd/containers/map/hashtable.h"
#include "osd/containers/name/container.h"
#include "osd/main/common/obj.h"
#include <stdio.h>

namespace client {
	class Session; // forward declaration
}

namespace mfs {
namespace client {

class FileInode;

class DirInode: public ::client::Inode
{
friend class FileInode;
public:
	DirInode(osd::common::ObjectProxyReference* ref)
		: parent_(NULL)
	{ 
		//printf("\n Creating DirInode.");
		ref_ = ref;
		fs_type_ = kMFS;
		type_ = kDirInode;
	}

	int Read(::client::Session* session, char* dst, uint64_t off, uint64_t n) { return 0; }
	int Write(::client::Session* session, char* src, uint64_t off, uint64_t n) { return 0; }
	int Unlink(::client::Session* session, const char* name);
	int Lookup(::client::Session* session, const char* name, int flags, ::client::Inode** ipp);
	int xLookup(::client::Session* session, const char* name, int flags, ::client::Inode** ipp);
	int Link(::client::Session* session, const char* name, ::client::Inode* ip, bool overwrite);
	int Link(::client::Session* session, const char* name, uint64_t ino, bool overwrite) { assert(0); }
	int Sync(::client::Session* session);

	int nlink();
	int set_nlink(int nlink);

	int Lock(::client::Session* session, Inode* parent_inode, lock_protocol::Mode mode); 
	int Lock(::client::Session* session, lock_protocol::Mode mode); 
	int Unlock(::client::Session* session);
	int xOpenRO(::client::Session* session); 

	int ioctl(::client::Session* session, int request, void* info);
        int return_dentry(::client::Session*, void *);

	struct dentry {
                char key[128];
                uint64_t val;
                struct dentry *next_dentry;
        };


	struct list_item {
                void *data;
                struct list_item *next;
        };



private:
	osd::containers::client::NameContainer::Reference* rw_ref() {
		//printf("\nInside osd::containers::client::NameContainer::Reference::rw_ref().");
		return static_cast<osd::containers::client::NameContainer::Reference*>(ref_);
	}
	int              nlink_;
	::client::Inode* parent_; // special case; see comment under link
};


} // namespace client
} // namespace mfs

#endif // __STAMNOS_MFS_CLIENT_DIRECTORY_INODE_H
