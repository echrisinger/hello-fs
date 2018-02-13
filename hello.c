/*
  FUSE: Filesystem in Userspace
  Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>

  This program can be distributed under the terms of the GNU GPL.
  See the file COPYING.

  gcc -Wall `pkg-config fuse --cflags --libs` hello.c -o hello
*/

#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>




static char hello_str[14] = "Hello World!\n";
static const char *hello_path = "/hello";
// TODO: make it a reference to hello_path
static const char *valid_paths[4] = {"/", "..", ".", "/hello"};


static int hello_getattr(const char *path, struct stat *stbuf)
{
	int res = 0;
	memset(stbuf, 0, sizeof(struct stat));
	if (!strcmp(path, "/") || !strcmp(path, ".")) {
		stbuf->st_mode = S_IFDIR | 0777;		
		stbuf->st_nlink = 2;
	} else if (!strcmp(path, hello_path)) {
		stbuf->st_mode = S_IFREG | 0777;		
		stbuf->st_nlink = 1;
		stbuf->st_size = get_str_size(hello_str);
	} else
		res = -ENOENT;

	return 0;
}

int get_str_size(char *str) {
	int i = 0;
	while (str[i] != '\n') {
		i++;
	}
	return i+1;
}

static int hello_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			 off_t offset, struct fuse_file_info *fi)
{
	(void) offset;
	(void) fi;

	if (strcmp(path, "/") != 0)
		return -ENOENT;

	filler(buf, hello_path + 1, NULL, 0);
	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);

	return 0;
}

// TODO: Lambda
int is_valid_path(const char *path) {
	for (int i=0; i < sizeof(valid_paths) / sizeof(char *); i++) {
		if (strcmp(path, valid_paths[i])) {
			return 1;
		}
	}
	return 0;
}

static int hello_access(const char* path, int mask) {
	int perm = 0;
	if (!is_valid_path(path)) {
		return -ENOENT;
	}
	return perm;
}

static int hello_open(const char *path, struct fuse_file_info *fi)
{
	if (strcmp(path, hello_path) != 0)
		return -ENOENT;
	return 0;
}

static int hello_read(const char *path, char *buf, size_t size, off_t offset,
		      struct fuse_file_info *fi)
{
	size_t len;
	(void) fi;
	if(strcmp(path, hello_path) != 0)
		return -ENOENT;

	len = sizeof hello_str - 1;
	if (offset < len) {
		if (offset + size > len) {
			size = len - offset;
		}
		memcpy(buf, hello_str + offset, len);
	} else
		size = 0;
	return size;
}

static int hello_write(const char* path, const char *buf, size_t size, off_t offset,
			struct fuse_file_info* fi)
{	
	size_t len;
	(void) fi;
	if (strcmp(path, hello_path) != 0)
		return -ENOENT;
	len = sizeof hello_str - 1;
	if (offset < len) {
		if (size > len - offset)
			size = len - offset;
		//write to hello_str instead of read
		memcpy(hello_str, buf, size);
	} else
		size = 0;

	return size;
}

static int hello_truncate(const char* path, off_t size) {
	return 0;
}

static int hello_getxattr(const char* path, const char* name, 
	char* value, size_t size) {
		return 0;
}

static int hello_flush(const char* path, struct fuse_file_info* fi) {
	return 0;
}
static struct fuse_operations hello_oper = {
	.getattr	= hello_getattr,
	.readdir	= hello_readdir,
	.open		= hello_open,
	.read		= hello_read,
	.access     = hello_access,
	.write      = hello_write,
	.truncate   = hello_truncate,
	.getxattr   = hello_getxattr,
	.flush      = hello_flush,
#ifdef HAVE_SETXATTR
    .getxattr    = hello_getxattr,
#endif
};

int main(int argc, char *argv[])
{
	umask(0);
	return fuse_main(argc, argv, &hello_oper, NULL);
}