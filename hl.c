/*
 * Copyright (c) 2014 Dave Vasilevsky <dave@vasilevsky.ca>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR(S) ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR(S) BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "nonstd.h"

#include <stddef.h>
#include <string.h>
#include "ops.h"

int main(int argc, char *argv[]) {
	struct fuse_args args;
	sqfs_opts opts;
	sqfs_hl *hl;
	int ret;
	
	struct fuse_opt fuse_opts[] = {
		{"offset=%zu", offsetof(sqfs_opts, offset), 0},
		FUSE_OPT_END
	};

	struct fuse_operations sqfs_hl_ops;
	get_fuse_ops(&sqfs_hl_ops);
	args.argc = argc;
	args.argv = argv;
	args.allocated = 0;
	
	opts.progname = argv[0];
	opts.image = NULL;
	opts.mountpoint = 0;
	opts.offset = 0;
	if (fuse_opt_parse(&args, &opts, fuse_opts, sqfs_opt_proc) == -1)
		sqfs_usage(argv[0], true);
	if (!opts.image)
		sqfs_usage(argv[0], true);
	
	hl = sqfs_hl_open(opts.image, opts.offset);
	if (!hl)
		return -1;
	
	fuse_opt_add_arg(&args, "-s"); /* single threaded */
	ret = fuse_main(args.argc, args.argv, &sqfs_hl_ops, hl);
	fuse_opt_free_args(&args);
	return ret;
}
