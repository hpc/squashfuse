#include <stdio.h>
#include "ops.h"

void hello_world ( )
{
	printf("Hello World \n");

}

static void *sqfs_hl_op_init(struct fuse_conn_info *conn
#if FUSE_USE_VERSION >= 30
			     ,struct fuse_config *cfg
#endif
			     ) {
	return fuse_get_context()->private_data;
}

