#include <sys/types.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <iostream>
#include "rpc/rpc.h"
#include "rpc/jsl_log.h"
#include "server/api.h"
#include "v6fs/v6fs.h"
#include "client/libfs.h"
#include "client/inode.h"
#include "client/namespace.h"
#include "mfs/pstruct.h"
#include "mfs/sb.h"


rpcc* rpc_client;  // client rpc object
struct sockaddr_in dst; //server's ip address
pthread_attr_t attr;
unsigned int principal_id;

void demo()
{
	int      ret;

	libfs_mount("/superblock/A", "/home/hvolos", "mfs", 0);
	//libfs_open("/etc/hvolos/test", 0);
	//libfs_open("/home/hvolos/test", 0);
	libfs_mkdir("/home/hvolos/dir", 0);
	libfs_mkdir("/home/hvolos/dir/test", 0);
	//libfs_open("/home/hvolos/dir/test", O_CREATE);
	//libfs_open("/home/hvolos/file", O_CREATE);
}


int
main(int argc, char *argv[])
{
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);
    int   port;
	int   debug_level = 0;
	uid_t principal_id;
	char  operation[16];
	char ch = 0;

	principal_id = getuid();
	srandom(getpid());
	port = 20000 + (getpid() % 10000);

	while ((ch = getopt(argc, argv, "d:p:li:o:s:"))!=-1) {
		switch (ch) {
			case 'd':
				debug_level = atoi(optarg);
				break;
			case 'p':
				port = atoi(optarg);
				break;
			case 'l':
				assert(setenv("RPC_LOSSY", "5", 1) == 0);
				break;
			case 'i':
				principal_id = atoi(optarg);
				break;
			case 'o':
				strcpy(operation, optarg); 
				break;
			default:
				break;
		}
	}

	if (debug_level > 0) {
		jsl_set_debug(debug_level);
		jsl_log(JSL_DBG_1, "DEBUG LEVEL: %d\n", debug_level);
	}

	pthread_attr_init(&attr);
	// set stack size to 32K, so we don't run out of memory
	pthread_attr_setstacksize(&attr, 32*1024);
	
	// server's address.
	memset(&dst, 0, sizeof(dst));
	dst.sin_family = AF_INET;
	dst.sin_addr.s_addr = inet_addr("127.0.0.1");
	dst.sin_port = htons(port);

	// start the client.  bind it to the server.
	// starts a thread to listen for replies and hand them to
	// the correct waiting caller thread. there should probably
	// be only one rpcc per process. you probably need one
	// rpcc per server.
	rpc_client = new rpcc(dst);
	assert (rpc_client->bind() == 0);

	dbg_set_level(5);

	libfs_init(rpc_client, principal_id);

	if (strcmp(operation, "mkfs") == 0) {
		libfs_mkfs("/superblock/A", "mfs", 0);
	} else if (strcmp(operation, "demo") == 0) {
		demo();	
	} else {
		// unknown
	}

	return 0;
}
