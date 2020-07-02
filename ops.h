#include "common.h"

#include "squashfs_fs.h"


#ifndef SQFS_OPS_H
#define SQFS_OPS_H

extern void hello_world ( );

extern static void *sqfs_hl_op_init(struct fuse_conn_info *conn
#if FUSE_USE_VERSION >= 30
                             ,struct fuse_config *cfg
#endif
                             ) ;
#endif
