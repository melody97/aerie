#ifndef __STAMNOS_IPC_RPC_NUMBERS_H
#define __STAMNOS_IPC_RPC_NUMBERS_H

#define ALL_RPC_NUMBER(module, protocol, handle)                               \
    module##_##protocol##_##handle,

#define RPC_NUMBER(module, protocol, handle)                                   \
    handle = StamnosGlobalRpcNumbers::module##_##protocol##_##handle,

#define DEFINE_RPC_NUMBER(ACTION)                                              \
    ACTION(RPC_NUMBER)


// Module/protocol RPC numbers

#define BCS_IPC_PROTOCOL(ACTION)                                               \
	ACTION(bcs, IpcProtocol, kRpcServerIsAlive)                                \
	ACTION(bcs, IpcProtocol, kRpcSubscribe)

#define BCS_SHARED_BUFFER_PROTOCOL(ACTION)                                     \
	ACTION(bcs, SharedBufferProtocol, kConsume)                                

#define SSA_LOCK_PROTOCOL(ACTION)                                              \
	ACTION(ssa, lock_protocol, acquire)                                        \
	ACTION(ssa, lock_protocol, acquirev)                                       \
	ACTION(ssa, lock_protocol, release)                                        \
	ACTION(ssa, lock_protocol, convert)                                        \
	ACTION(ssa, lock_protocol, stat)

#define SSA_RLOCK_PROTOCOL(ACTION)                                             \
	ACTION(ssa, rlock_protocol, revoke)                                        \
	ACTION(ssa, rlock_protocol, retry)

#define SSA_STORAGE_PROTOCOL(ACTION)                                           \
	ACTION(ssa, StorageProtocol, kAllocateContainerVector)                     \
	ACTION(ssa, StorageProtocol, kAllocateContainer)

#define SSA_REGISTRY_PROTOCOL(ACTION)                                          \
	ACTION(ssa, RegistryProtocol, kLookup)                                     \
	ACTION(ssa, RegistryProtocol, kAdd)                                        \
	ACTION(ssa, RegistryProtocol, kRemove)

#define SSA_PUBLISHER_PROTOCOL(ACTION)                                         \
	ACTION(ssa, PublisherProtocol, kPublish)                                    

#define PXFS_FILESYSTEM_PROTOCOL(ACTION)                                       \
	ACTION(pxfs, FileSystemProtocol, kMount)


class StamnosGlobalRpcNumbers {
public:

	enum {
		null_rpc = 0x4000,
		BCS_IPC_PROTOCOL(ALL_RPC_NUMBER)
		BCS_SHARED_BUFFER_PROTOCOL(ALL_RPC_NUMBER)
		SSA_LOCK_PROTOCOL(ALL_RPC_NUMBER)
		SSA_RLOCK_PROTOCOL(ALL_RPC_NUMBER)
		SSA_STORAGE_PROTOCOL(ALL_RPC_NUMBER)
		SSA_REGISTRY_PROTOCOL(ALL_RPC_NUMBER)
		SSA_PUBLISHER_PROTOCOL(ALL_RPC_NUMBER)
		PXFS_FILESYSTEM_PROTOCOL(ALL_RPC_NUMBER)
	};

};


#endif // __STAMNOS_IPC_RPC_NUMBERS_H