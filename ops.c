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

int sqfs_hl_op_getattr(const char *path, struct stat *st
#if FUSE_USE_VERSION >= 30
                              , struct fuse_file_info *fi
#endif
                              ) {
        sqfs *fs;
        sqfs_inode inode;
        if (sqfs_hl_lookup(&fs, &inode, path))
                return -ENOENT;

        if (sqfs_stat(fs, &inode, st))
                return -ENOENT;

        return 0;
}
 int sqfs_hl_op_opendir(const char *path, struct fuse_file_info *fi) {
	sqfs *fs;
	sqfs_inode *inode;
	
	inode = malloc(sizeof(*inode));
	if (!inode)
		return -ENOMEM;
	
	if (sqfs_hl_lookup(&fs, inode, path)) {
		free(inode);
		return -ENOENT;
	}
		
	if (!S_ISDIR(inode->base.mode)) {
		free(inode);
		return -ENOTDIR;
	}
	
	fi->fh = (intptr_t)inode;
	return 0;
}

int sqfs_hl_op_releasedir(const char *path,
                struct fuse_file_info *fi) {
        free((sqfs_inode*)(intptr_t)fi->fh);
        fi->fh = 0;
        return 0;
}

int sqfs_hl_op_readdir(const char *path, void *buf,
                fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi
#if FUSE_USE_VERSION >= 30
        ,enum fuse_readdir_flags flags
#endif
        ) {
        sqfs_err err;
        sqfs *fs;
        sqfs_inode *inode;
        sqfs_dir dir;
        sqfs_name namebuf;
        sqfs_dir_entry entry;
        struct stat st;

        sqfs_hl_lookup(&fs, NULL, NULL);
        inode = (sqfs_inode*)(intptr_t)fi->fh;

        if (sqfs_dir_open(fs, inode, &dir, offset))
                return -EINVAL;

        memset(&st, 0, sizeof(st));
        sqfs_dentry_init(&entry, namebuf);
        while (sqfs_dir_next(fs, &dir, &entry, &err)) {
                sqfs_off_t doff = sqfs_dentry_next_offset(&entry);
                st.st_mode = sqfs_dentry_mode(&entry);
                if (filler(buf, sqfs_dentry_name(&entry), &st, doff
#if FUSE_USE_VERSION >= 30
                           , 0
#endif
                     )) {
                        return 0;
                }
        }
        if (err)
                return -EIO;
        return 0;
}

int sqfs_hl_op_open(const char *path, struct fuse_file_info *fi) {
	sqfs *fs;
	sqfs_inode *inode;
	
	if (fi->flags & (O_WRONLY | O_RDWR))
		return -EROFS;
	
	inode = malloc(sizeof(*inode));
	if (!inode)
		return -ENOMEM;
	
	if (sqfs_hl_lookup(&fs, inode, path)) {
		free(inode);
		return -ENOENT;
	}
	
	if (!S_ISREG(inode->base.mode)) {
		free(inode);
		return -EISDIR;
	}
	
	fi->fh = (intptr_t)inode;
	fi->keep_cache = 1;
	return 0;
}

int sqfs_hl_op_create(const char* unused_path, mode_t unused_mode,
		struct fuse_file_info *unused_fi) {
	return -EROFS;
}

int sqfs_hl_op_release(const char *path, struct fuse_file_info *fi) {
        free((sqfs_inode*)(intptr_t)fi->fh);
        fi->fh = 0;
        return 0;
}

int sqfs_hl_op_read(const char *path, char *buf, size_t size,
                off_t off, struct fuse_file_info *fi) {
        sqfs *fs;
        sqfs_hl_lookup(&fs, NULL, NULL);
        sqfs_inode *inode = (sqfs_inode*)(intptr_t)fi->fh;

        off_t osize = size;
        if (sqfs_read_range(fs, inode, off, &osize, buf))
                return -EIO;
        return osize;
}

 int sqfs_hl_op_readlink(const char *path, char *buf, size_t size) {
	sqfs *fs;
	sqfs_inode inode;
	if (sqfs_hl_lookup(&fs, &inode, path))
		return -ENOENT;
	
	if (!S_ISLNK(inode.base.mode)) {
		return -EINVAL;
	} else if (sqfs_readlink(fs, &inode, buf, &size)) {
		return -EIO;
	}	
	return 0;
}

sqfs_hl *sqfs_hl_open(const char *path, size_t offset) {
        sqfs_hl *hl;

        hl = malloc(sizeof(*hl));
        if (!hl) {
                perror("Can't allocate memory");
        } else {
                memset(hl, 0, sizeof(*hl));
                if (sqfs_open_image(&hl->fs, path, offset) == SQFS_OK) {
                        if (sqfs_inode_get(&hl->fs, &hl->root, sqfs_inode_root(&hl->fs)))
                                fprintf(stderr, "Can't find the root of this filesystem!\n");
                        else
                                return hl;
                        sqfs_destroy(&hl->fs);
                }

                free(hl);
        }
        return NULL;
}

int sqfs_hl_op_listxattr(const char *path, char *buf, size_t size) {
	sqfs *fs;
	sqfs_inode inode;
	int ferr;
	
	if (sqfs_hl_lookup(&fs, &inode, path))
		return -ENOENT;

	ferr = sqfs_listxattr(fs, &inode, buf, &size);
	if (ferr)
		return -ferr;
	return size;
}

int sqfs_hl_op_statfs(const char *path, struct statvfs *st) {
        sqfs_hl *hl = fuse_get_context()->private_data;
        return sqfs_statfs(&hl->fs, st);
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
        if (position != 0) 
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

fuse_operations *get_fuse_ops(){
       struct fuse_operations sqfs_hl_ops;
        memset(&sqfs_hl_ops, 0, sizeof(sqfs_hl_ops));
        sqfs_hl_ops.init                        = sqfs_hl_op_init;
        sqfs_hl_ops.destroy             = sqfs_hl_op_destroy;
        sqfs_hl_ops.getattr             = sqfs_hl_op_getattr;
        sqfs_hl_ops.opendir             = sqfs_hl_op_opendir;
        sqfs_hl_ops.releasedir  = sqfs_hl_op_releasedir;
        sqfs_hl_ops.readdir             = sqfs_hl_op_readdir;
        sqfs_hl_ops.open                = sqfs_hl_op_open;
        sqfs_hl_ops.create              = sqfs_hl_op_create;
        sqfs_hl_ops.release             = sqfs_hl_op_release;
        sqfs_hl_ops.read                = sqfs_hl_op_read;
        sqfs_hl_ops.readlink    = sqfs_hl_op_readlink;
        sqfs_hl_ops.listxattr   = sqfs_hl_op_listxattr;
        sqfs_hl_ops.getxattr    = sqfs_hl_op_getxattr;
        sqfs_hl_ops.statfs    = sqfs_hl_op_statfs;
        return &sqfs_hl_ops;
}
