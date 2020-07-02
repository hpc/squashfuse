#include "squashfuse.h"
#include "fuseprivate.h"
#include "stat.h"

#include "nonstd.h"

#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fs.h"
#include "ops.h"

void hello_world ( )
{
	printf("Hello World \n");

}

static sqfs_err sqfs_hl_lookup(sqfs **fs, sqfs_inode *inode,
                const char *path) {
        bool found;

        sqfs_hl *hl = fuse_get_context()->private_data;
        *fs = &hl->fs;
        if (inode)
                *inode = hl->root; /* copy */

        if (path) {
                sqfs_err err = sqfs_lookup_path(*fs, inode, path, &found);
                if (err)
                        return err;
                if (!found)
                        return SQFS_ERR;
        }
        return SQFS_OK;
}



void *sqfs_hl_op_init(struct fuse_conn_info *conn
#if FUSE_USE_VERSION >= 30
			     ,struct fuse_config *cfg
#endif
			     ) {
	return fuse_get_context()->private_data;
}

void sqfs_hl_op_destroy(void *user_data) {
	sqfs_hl *hl = (sqfs_hl*)user_data;
	sqfs_destroy(&hl->fs);
	free(hl);
}

int sqfs_hl_op_getxattr(const char *path, const char *name,
                char *value, size_t size
#ifdef FUSE_XATTR_POSITION
                , uint32_t position
#endif
                ) {
        sqfs *fs;
        sqfs_inode inode;
        size_t real = size;

#ifdef FUSE_XATTR_POSITION
        if (position != 0) /* We don't support resource forks */
                return -EINVAL;
#endif

        if (sqfs_hl_lookup(&fs, &inode, path))
                return -ENOENT;

        if ((sqfs_xattr_lookup(fs, &inode, name, value, &real)))
                return -EIO;
        if (real == 0)
                return -sqfs_enoattr();
        if (size != 0 && size < real)
                return -ERANGE;
        return real;
}

