#include "squashfuse.h"
#include "fuseprivate.h"
#include "stat.h"

#include "nonstd.h"

#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "ops.h"

void hello_world ( )
{
	printf("Hello World \n");

}

void *sqfs_hl_op_init(struct fuse_conn_info *conn
#if FUSE_USE_VERSION >= 30
			     ,struct fuse_config *cfg
#endif
			     ) {
	return fuse_get_context()->private_data;
}

