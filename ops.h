#include "common.h"

#include "squashfs_fs.h"
#include "fuseprivate.h"
#include "squashfuse.h"

#include "stat.h"
#include "nonstd.h"
#include "fuse.h"

#ifndef SQFS_OPS_H
#define SQFS_OPS_H

extern void hello_world ( );

extern void *sqfs_hl_op_init(struct fuse_conn_info *conn
#if FUSE_USE_VERSION >= 30
                             ,struct fuse_config *cfg
#endif
                             ) ;
typedef struct sqfs_hl sqfs_hl;
struct sqfs_hl {
	sqfs fs;
	sqfs_inode root;
};
extern void sqfs_hl_op_destroy(void *user_data);

int sqfs_hl_op_getattr(const char *path, struct stat *st
#if FUSE_USE_VERSION >= 30
                              , struct fuse_file_info *fi
#endif
                              );
int sqfs_hl_op_opendir(const char *path, struct fuse_file_info *fi);

extern int sqfs_hl_op_releasedir(const char *path,
                struct fuse_file_info *fi);

extern int sqfs_hl_op_readdir(const char *path, void *buf,
                fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi
#if FUSE_USE_VERSION >= 30
        ,enum fuse_readdir_flags flags
#endif
        );

int sqfs_hl_op_open(const char *path, struct fuse_file_info *fi);
int sqfs_hl_op_create(const char* unused_path, mode_t unused_mode,
		struct fuse_file_info *unused_fi);

extern int sqfs_hl_op_release(const char *path, struct fuse_file_info *fi);

extern int sqfs_hl_op_read(const char *path, char *buf, size_t size,
                off_t off, struct fuse_file_info *fi);

extern int sqfs_hl_op_readlink(const char *path, char *buf, size_t size);
extern int sqfs_hl_op_listxattr(const char *path, char *buf, size_t size);

#endif
