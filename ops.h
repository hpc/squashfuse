#include "common.h"

#include "squashfs_fs.h"
#include "fuseprivate.h"
#include "squashfuse.h"

#include "stat.h"
#include "nonstd.h"


#ifndef SQFS_OPS_H
#define SQFS_OPS_H
typedef struct fuse_operations fuse_operations;
typedef struct sqfs_hl sqfs_hl;
struct sqfs_hl {
	sqfs fs;
	sqfs_inode root;
};

extern void get_fuse_ops(fuse_operations *sqfs_hl_ops);

extern bool sqfs_hl_check(const char *path, size_t offset);
extern sqfs_hl *sqfs_hl_open(const char *path, size_t offset);
#endif
