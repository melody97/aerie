#define  __CACHE_GUARD__

#include "osd/containers/byte/container.h"
#include <stdint.h>
#include <inttypes.h>
#include <vector>
#include "common/errno.h"
#include "osd/main/client/session.h"
#include "osd/main/client/salloc.h"
#include "osd/main/common/const.h"
#include "osd/main/common/publisher.h"
#include "common/interval_tree.h"
#include "scm/const.h"
#include <stdio.h>
namespace osd {
namespace containers {
namespace client {

#define min(a,b) ((a) < (b)? (a) : (b))
#define max(a,b) ((a) > (b)? (a) : (b))

/////////////////////////////////////////////////////////////////////////////
// 
// Helper Class: ByteInterval
//
/////////////////////////////////////////////////////////////////////////////


const int INTERVAL_MIN_SIZE = 64;

class ByteInterval: public Interval {
public:
	ByteInterval(const int low, const int high)
		: low_(low), 
		  high_(high)
	{ }

	ByteInterval(OsdSession* session, ByteContainer::Object* obj, ByteContainer::Slot& slot, const int low, const int size)
		: object_(obj),
		  low_(low), 
		  high_(low+size-1), 
		  slot_(slot)
	{ 
		assert(slot.slot_base_);
		if (size <= INTERVAL_MIN_SIZE) {
			// Adjust the slot offset to be multiple of interval size
			slot_.slot_offset_ &= ~(INTERVAL_MIN_SIZE-1);
			block_array_ = new char *[INTERVAL_MIN_SIZE];
			for (int i=0; i<INTERVAL_MIN_SIZE; i++) {
				block_array_[i] = 0;
			}
			region_ = NULL;
		} else {
			block_array_ = NULL;
			region_ = new ByteContainer::Region(session, slot); // FIXME: pass session
		}	
	}
	  
	inline int GetLowPoint() const { return low_;}
	inline int GetHighPoint() const { return high_;}
	int Write(OsdSession* session, char*, uint64_t, uint64_t);
	int Read(OsdSession* session, char*, uint64_t, uint64_t);

protected:
	ByteContainer::Object*         object_;
	uint64_t                       low_;
	uint64_t                       high_;
	ByteContainer::Region*         region_;     
	ByteContainer::Slot            slot_;       // needed when region_ is NULL
	char**                         block_array_;

	int WriteBlockNoRegion(OsdSession* session, char*, uint64_t, int, int);
	int WriteNoRegion(OsdSession* session, char*, uint64_t, uint64_t);
	int ReadBlockNoRegion(OsdSession* session, char*, uint64_t, int, int);
	int ReadNoRegion(OsdSession* session, char*, uint64_t, uint64_t);
};


int
ByteInterval::WriteBlockNoRegion(OsdSession* session, char* src, uint64_t bn, int off, int n)
{
	int   ret;
	char* bp;
	void* ptr;

	assert(low_ <= bn && bn <= high_);

	if (!(bp = block_array_[bn - low_])) {
		if ((ret = session->salloc()->AllocateExtent(session, kBlockSize, 
		                                             kData, &ptr)) < 0)
		{ 
			return ret;
		}
		// The allocator journaled the allocation. We just journal the data block link.
		session->journal() << osd::Publisher::Message::ContainerOperation::LinkBlock(object_->oid(), bn, ptr);
		bp = block_array_[bn - low_] = (char*) ptr;
		// Zero the part of the newly allocated block that is not written to
		// ensure we later read zeros and not garbage.
		if (off>0) {
			memset(bp, 0, off);
		}	
		memset(&bp[off+n], 0, kBlockSize-n); 
	}

	//memmove(&bp[off], src, n);
	ScmMemCopy(&bp[off], src, n);
	return n;
}


int
ByteInterval::WriteNoRegion(OsdSession* session, char* src, uint64_t off, uint64_t n)
{
	uint64_t tot;
	uint64_t m;
	uint64_t bn;
	uint64_t f;
	int      ret;

	for(tot=0; tot<n; tot+=m, off+=m) {
		bn = off / kBlockSize;
		f = off % kBlockSize;
		m = min(n - tot, kBlockSize - f);
		ret = WriteBlockNoRegion(session, &src[tot], bn, f, m);
		if (ret < 0) {
			return ((ret < 0) ? ( (tot>0)? tot: ret)  
			                  : tot + ret);
		}
	}

	return tot;
}


int
ByteInterval::Write(OsdSession* session, char* src, uint64_t off, uint64_t n)
{
	if (region_) {
		return region_->Write(session, src, off, n);
	} else {
		return WriteNoRegion(session, src, off, n);
	}
}


int
ByteInterval::ReadBlockNoRegion(OsdSession* session, char* dst, uint64_t bn, int off, int n)
{
	char* bp;

	assert(low_ <= bn && bn <= high_);

	if (!(bp = block_array_[bn - low_])) {
		memset(dst, 0, n);
		return n;
	}

	memmove(dst, &bp[off], n);
	return n;
}


int
ByteInterval::ReadNoRegion(OsdSession* session, char* dst, uint64_t off, uint64_t n)
{
	uint64_t tot;
	uint64_t m;
	uint64_t bn;
	uint64_t f;
	int      ret;

	for(tot=0; tot<n; tot+=m, off+=m) {
		bn = off / kBlockSize;
		f = off % kBlockSize;
		m = min(n - tot, kBlockSize - f);
		ret = ReadBlockNoRegion(session, &dst[tot], bn, f, m);
		if (ret < 0) {
			return ((ret < 0) ? ( (tot>0)? tot: ret)  
			                  : tot + ret);
		}
	}

	return tot;
}


int
ByteInterval::Read(OsdSession* session, char* dst, uint64_t off, uint64_t n)
{
	if (region_) {
		return region_->Read(session, dst, off, n);
	} else {
		return ReadNoRegion(session, dst, off, n);
	}
}


/////////////////////////////////////////////////////////////////////////////
// 
// ByteContainer Verion Manager: Logical Copy-On-Write 
//
/////////////////////////////////////////////////////////////////////////////



int 
ByteContainer::VersionManager::vOpen()
{
	// FIXME: check if object is private or public. If private
	// then mark it as directly mutable
	
	osd::vm::client::VersionManager<ByteContainer::Object>::vOpen();
	
	if (0 /* private */) {
		mutable_ = true;
		intervaltree_ = NULL;
	} else {
		mutable_ = false;
		intervaltree_ = new IntervalTree();
	}
	region_ = NULL;
	size_ = object()->Size();

	return E_SUCCESS;
}



// FIXME: Currently we publish by simply doing the updates in-place. 
// Normally this must be done via the trusted server using the journal 
int 
ByteContainer::VersionManager::vUpdate(OsdSession* session)
{
	osd::vm::client::VersionManager<ByteContainer::Object>::vUpdate(session);

	// TODO
	
	return 0;
}


int 
ByteContainer::VersionManager::ReadImmutable(OsdSession* session, 
                                             char* dst, 
                                             uint64_t off, 
                                             uint64_t n)
{
	uint64_t                tot;
	uint64_t                m;
	uint64_t                fbn; // first block number
	uint64_t                bn;
	uint64_t                base_bn;
	ByteContainer::Iterator start;
	ByteContainer::Iterator iter;
	int                     ret;
	int                     f;
	uint64_t                bcount;
	uint64_t                size;
	char*                   ptr;
	ByteInterval*           interval;

	dbg_log (DBG_DEBUG, "Immutable range = [%" PRIu64 ", %" PRIu64 ") n=%" PRIu64 ", size=%" PRIu64 "\n", off, off+n, n, size_);

	// find out how much is really there to read
	if ((off + n) > size_) {
		if (off > size_ ) { // insight : size_ is set by ByteContainer::Object() in osd/containers/name/common.h, initially set to 0
			return 0;
		} else {
			n = min(size_ - off, n);
		}
		dbg_log (DBG_DEBUG, "Immutable range pruned = [%" PRIu64 ", %" PRIu64 ") n=%" PRIu64 ", size=%" PRIu64 "\n", off, off+n, n, size_);
	} 

	fbn = off/kBlockSize; // insight : kBlockSize = 1,000,000,000,000 bytes
	start.Init(session, object(), fbn);
	iter = start;
	bcount = 1 << (((*iter).slot_height_ - 1)*RADIX_TREE_MAP_SHIFT);
	size = bcount * kBlockSize;
	f = off % size;


	for (tot=0, bn=fbn; 
	     !iter.terminate() && tot < n; // insight : n is the number of bytes to be read
	     iter++, tot+=m, off+=m, bn++) // insight : m is the number of bytes read at a time. I want to see how this maps to the SCM ???
	{
		base_bn = (*iter).get_base_bn();
		bcount = 1 << (((*iter).slot_height_ - 1)*RADIX_TREE_MAP_SHIFT);
		size = bcount * kBlockSize;
		m = min(n - tot, size - f); // insight : Either you read 'size' bytes or whatever is left of 'n' bytes.
						// insight : "tot" is the number of bytes you read in each iteration
						// 

		ptr = (char*) (*iter).slot_base_[(*iter).slot_offset_];

		//printf("bn=%" PRIu64 " , base_bn = %" PRIu64 " , block=%p R[%d, %" PRIu64 "] A[%" PRIu64 " , %" PRIu64 " ] size=%" PRIu64 "  (%" PRIu64 "  blocks)\n", 
		//       bn, base_bn, ptr, f, f+m-1, off, off+m-1, size, bcount);

		if (!ptr) {
			// Downcasting via a static cast is generally dangerous, but we know 
			// that Interval is always of type ByteInterval so it should be safe

			//TODO: Optimization: we should check whether the block falls in the last 
			//interval to save a lookup.
			interval = static_cast<ByteInterval*>(intervaltree_->LeftmostOverlap(bn, bn));
			if (!interval) {
				// return zeros
				memset(&dst[tot], 0, m);
			} else {
				if ((ret = interval->Read(session, &dst[tot], off, m)) < m) {
					return ((ret < 0) ? ( (tot>0)? tot: ret)  
					                  : tot + ret);
				}
			}
		} else {
			// pinode already points to a block, therefore we do an in-place read
			assert(bcount == 1);
			memmove(&dst[tot], &ptr[f], m);
		}

		f = 0; // after the first block is read, each other block is read 
		       // starting at its first byte
	}

	return tot;
}


int 
ByteContainer::VersionManager::ReadMutable(OsdSession* session, char* dst, 
                                           uint64_t off, uint64_t n)
{
	int vn;

	//printf ("ReadMutable: range = [%" PRIu64 " , %" PRIu64 " ]\n", off, off+n-1);

	dbg_log (DBG_DEBUG, "Mutable range = [%" PRIu64 " , %" PRIu64 " ]\n", off, off+n-1);

	if (off > size_) {
		return 0;
	}
	vn = min(size_ - off, n);

	if (mutable_) {
		assert(region_ == NULL);
		return object()->Read(session, dst, off, vn);
	} else if (region_) {
		return	region_->Read(session, dst, off, vn);
	}

	return 0;
}


int 
ByteContainer::VersionManager::Read(OsdSession* session, char* dst, 
                                    uint64_t off, uint64_t n)
{
	uint64_t  immmaxsize; // immutable range max size
	uint64_t  mn;
	int       ret1 = 0;
	int       ret2 = 0;
	int       r;

	//printf("\n Yippee ! I am inside Read.");	
	immmaxsize = (!mutable_) ? object()->get_maxsize(): 0;

	dbg_log (DBG_DEBUG, "Read range = [%" PRIu64 ", %" PRIu64 "] n=%" PRIu64 " (size=%" PRIu64 ", immmaxsize=%" PRIu64 ")\n", off, off+n-1, n, size_, immmaxsize);

	//printf ("Read: range = [%" PRIu64 " , %" PRIu64 " ]\n", off, off+n-1);
	//printf("Read: immmaxsize=%lu\n", immmaxsize);
	//printf("Read: get_maxsize=%lu\n", object()->get_maxsize());
	//printf("Read: get_size=%lu\n", object()->Size());


	// insight : n is the number of bytes you are going to read
	// insight : off : is the file offset
	// insight : for all reads to the left of immaxsize, use ReadImmutable
	// insight : for all reads to the right of immaxsize, use ReadMutable
	if (off + n < immmaxsize) 
	{
	//	printf("\n Yip ! Read Path 1.");
		ret1 = ReadImmutable(session, dst, off, n);
	} else if ( off > immmaxsize - 1) {
//		printf("\n Yip ! Read Path 2.");
		ret2 = ReadMutable(session, dst, off, n);
	} else {
		mn = off + n - immmaxsize; 
		ret1 = ReadImmutable(session, dst, off, n - mn);
		// If ReadImmutable read less than what we asked for 
		// then we should short circuit and return because POSIX
		// semantics require us to return the number of contiguous
		// bytes read. Is this true?
		if (ret1 < n - mn) {
			return ret1;
		}
		ret2 = ReadMutable(session, &dst[n-mn], immmaxsize, mn);
		if (ret2 < 0) {
			ret2 = 0;
		}
	}

	if (ret1 < 0 || ret2 < 0) {
		return -1;
	}
	r = ret1 + ret2;

	return r;
}


int 
ByteContainer::VersionManager::WriteMutable(OsdSession* session, 
                                            char* src, 
                                            uint64_t off, 
                                            uint64_t n)
{
	uint64_t bn;

        s_log("[%ld] ByteContainer::VersionManager::%s",s_tid, __func__);
	dbg_log (DBG_DEBUG, "Mutable range = [%" PRIu64 " , %" PRIu64 " ]\n", off, off+n-1);

	if (mutable_) {
		assert(region_ == NULL);
        s_log("[%ld] ByteContainer::VersionManager::%s case 1",s_tid, __func__);
		return object()->Write(session, src, off, n);
	} else if (!region_) {
		bn = off/kBlockSize;
        s_log("[%ld] ByteContainer::VersionManager::%s case 2",s_tid, __func__);
		region_ = new ByteContainer::Region(session, object(), bn);
	}
        s_log("[%ld] ByteContainer::VersionManager::%s case 3",s_tid, __func__);
	return region_->Write(session, src, off, n);
}


int 
ByteContainer::VersionManager::WriteImmutable(OsdSession* session, 
                                              char* src, 
                                              uint64_t off, 
                                              uint64_t n)
{
        s_log("[%ld] ByteContainer::VersionManager::%s",s_tid, __func__);
	uint64_t                tot;
	uint64_t                m;
	uint64_t                fbn; // first block number
	uint64_t                bn;
	uint64_t                base_bn;
	ByteContainer::Iterator start;
	ByteContainer::Iterator iter;
	int                     ret;
	int                     f;
	uint64_t                bcount;
	uint64_t                size;
	char*                   ptr;
	uint64_t                interval_size;
	uint64_t                interval_low;
	ByteInterval*           interval;

	dbg_log (DBG_DEBUG, "Immutable range = [%" PRIu64 " , %" PRIu64 " ] n=%" PRIu64 " \n", off, off+n-1, n);

	fbn = off/kBlockSize;
	start.Init(session, object(), fbn);
	// object() is the object that holds the radix tree
	iter = start;
	bcount = 1 << (((*iter).slot_height_ - 1)*RADIX_TREE_MAP_SHIFT);
	size = bcount * kBlockSize;
	f = off % size;


	for (tot=0, bn=fbn; 
	     !iter.terminate() && tot < n; 
	     iter++, tot+=m, off+=m, bn++) 
	{
		base_bn = (*iter).get_base_bn();
		bcount = 1 << (((*iter).slot_height_ - 1)*RADIX_TREE_MAP_SHIFT);
		size = bcount * kBlockSize;
		m = min(n - tot, size - f);

		ptr = (char*) ((*iter).slot_base_[(*iter).slot_offset_]);

		//printf("bn=%" PRIu64 " , base_bn = %" PRIu64 " , block=%p R[%d, %" PRIu64 "] A[%" PRIu64 " , %" PRIu64 " ] size=%" PRIu64 "  (%" PRIu64 "  blocks)\n", 
		//       bn, base_bn, ptr, f, f+m-1, off, off+m-1, size, bcount);

		if (!ptr) {
			// Downcasting via a static cast is generally dangerous, but we know 
			// that Interval is always of type ByteInterval so it should be safe

			//TODO: Optimization: we should check whether the block falls in the last 
			//interval to save a lookup.
			interval = static_cast<ByteInterval*>(intervaltree_->LeftmostOverlap(bn, bn));
			if (!interval) {
				// create new interval
				if (bn < ::osd::containers::common::ByteContainer::N_DIRECT) {
					interval_size = 8;
					interval_low = 0;
				} else {
					interval_size = max(bcount, INTERVAL_MIN_SIZE);
					interval_low = ::osd::containers::common::ByteContainer::N_DIRECT + 
					               ((base_bn-::osd::containers::common::ByteContainer::N_DIRECT) & ~(interval_size - 1));
				}
				interval = new ByteInterval(session, object(), (*iter), interval_low, interval_size);
				intervaltree_->Insert(interval);
			}
			ret = interval->Write(session, &src[tot], off, m);
			if (ret < 0 || ((uint64_t) ret) < m) {
				return ((ret < 0) ? ( (tot>0)? tot: ret)  
				                  : tot + ret);
			}
		} else {
			// pinode already points to a block, therefore we do an in-place write
			assert(bcount == 1);
			//memmove(&ptr[f], &src[tot], m);
			ScmMemCopy(&ptr[f], &src[tot], m);
		}

		f = 0; // after the first block is written, each other block is written 
		       // starting at its first byte
	}

	return tot;
}


int 
ByteContainer::VersionManager::Write(OsdSession* session, 
                                     char* src, 
                                     uint64_t off, 
                                     uint64_t n)
{
	uint64_t  immmaxsize; // immutable range max size
	uint64_t  mn;
	int       ret1 = 0;
	int       ret2 = 0;
	int       w;

        s_log("[%ld] ByteContainer::VersionManager::%s",s_tid, __func__);

	
	immmaxsize = (!mutable_) ? object()->get_maxsize() : 0;
	// insight : i think get_maxsize returns the current size of the file
	// direct + indirect blocks !
	// immaxsize is the current size of the file and cannot be zero
	// because of the number of direct blocks avlb with the each file.

	dbg_log (DBG_DEBUG, "Write range = [%" PRIu64 " , %" PRIu64 " ] n=%" PRIu64 ", immmaxsize=%" PRIu64 "\n", off, off+n-1, n, immmaxsize);

/*
 * insight
 * visualize : 
 * | = immmaxsize
 * (---) = off + n
 *
 */
	if (off + n < immmaxsize) 
	// insight : case 1 (---) |
	{
        s_log("[%ld] ByteContainer::VersionManager::%s case 1 (off + n) = %ld immmaxsize = %ld",s_tid, __func__, off + n, immmaxsize);
		ret1 = WriteImmutable(session, src, off, n);
	} else if ( off >= immmaxsize) {
	// insight : case 2 | (---)
        s_log("[%ld] ByteContainer::VersionManager::%s case 2 (off + n) = %ld immmaxsize = %ld",s_tid, __func__, off + n, immmaxsize);
		ret2 = WriteMutable(session, src, off, n);
	} else {
	// insight : case 3 (--|-)
        s_log("[%ld] ByteContainer::VersionManager::%s case 3 (off + n) = %ld immmaxsize = %ld",s_tid, __func__, off + n, immmaxsize);
		mn = off + n - immmaxsize; 
		ret1 = WriteImmutable(session, src, off, n - mn);
		// If WriteImmutable wrote less than what we asked for 
		// then we should short circuit and return because POSIX
		// semantics require us to return the number of contiguous
		// bytes written. Is this true?
		if (ret1 < n - mn) {
			return ret1;
		}
		ret2 = WriteMutable(session, &src[n-mn], immmaxsize, mn);
		if (ret2 < 0) {
			ret2 = 0;
		}
	}

	if (ret1 < 0 || ret2 < 0) {
		return -1;
	}
	w = ret1 + ret2;

	if ( (off + w) > size_ ) {
		size_ = off + w;
	}

#ifdef DURABLE_DATA
	// wait for the writes to be performed to SCM
        s_log("[%ld] ByteContainer::VersionManager::%s ScmFence",s_tid, __func__);
	ScmFence();
#endif
	return w;
}


int
ByteContainer::VersionManager::Size(OsdSession* session)
{
	return size_;
}


int
ByteContainer::VersionManager::Size()
{
	return size_;
}


} // namespace osd
} // namespace containers
} // namespace client
