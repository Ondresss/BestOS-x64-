#define FUSE_USE_VERSION 31
 
#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <assert.h>
#include <stdlib.h>

# include "diskAccess.h"


static void set_context_by_path(const char *path) {
        char parentPath[256] = {0};
        getParentPath(path, parentPath);
        changeDirAbsolute_(parentPath);
}


static int unlinkCallback(const char *path) {
        set_context_by_path(path);
        char FILENAME[256] = {0};
        stringParseFilename(FILENAME,path);
        delete_(FILENAME);
        return 0;
}

static int rmDirCallback(const char *path) {
        changeDirAbsolute_(path);
        char DIRNAME[256] = {0};
        stringParseFilename(DIRNAME,path);
        rmCurrentDir_(DIRNAME);
        return 0;
}
static int createCallback(const char *path, mode_t mode, struct fuse_file_info *fi) {
        (void) mode; (void) fi;

        set_context_by_path(path);
        char FILENAME[256] = {0};
        stringParseFilename(FILENAME,path);
        create_(FILENAME);

        return 0;
}

static int writeCallback(const char *path, const char *buf, size_t size,
                      off_t offset, struct fuse_file_info *fi) {
        (void) fi;
        set_context_by_path(path);
        char FILENAME[256] = {0};
        stringParseFilename(FILENAME,path);
        write_(FILENAME,buf,size);
        return size;
}

static void *hello_init(struct fuse_conn_info *conn,
                        struct fuse_config *cfg)
{
        (void) conn;
        cfg->kernel_cache = 1;
        return NULL;
}


static int utimensCallback(const char *path, const struct timespec tv[2],
                         struct fuse_file_info *fi) {
        (void) path; (void) tv; (void) fi;

        return 0;
}




static int hello_getattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi)
{
        (void) fi;
        memset(stbuf, 0, sizeof(struct stat));

        if (strcmp(path, "/") == 0) {
                stbuf->st_mode = S_IFDIR | 0755;
                stbuf->st_nlink = 2;
                return 0;
        }

        set_context_by_path(path);
        char filenameOnly[256] = {0};
        stringParseFilename(filenameOnly, path);
        Fat16Entry entry = findEntryInCurrentDir(filenameOnly);

        if (entry.filename[0] == 0x00 || (unsigned char)entry.filename[0] == 0xE5) {
                return -ENOENT;
        }

        if (entry.attributes & 0x10) {
                stbuf->st_mode = S_IFDIR | 0755;
                stbuf->st_nlink = 2;
        } else {
                stbuf->st_mode = S_IFREG | 0444;
                stbuf->st_nlink = 1;
                stbuf->st_size = entry.file_size;
        }
        return 0;
}
//DAVAT UMOUNT na adresari
static int hello_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                         off_t offset, struct fuse_file_info *fi,
                         enum fuse_readdir_flags flags)
{
        (void) offset;
        (void) fi;
        (void) flags;
        filler(buf, ".", NULL, 0, 0);
        filler(buf, "..", NULL, 0, 0);

        changeDirAbsolute_(path);
        Fat16Entry entries[32] = {0};
        int noEntries = getCurrentEntries(entries);
        for (int i = 0; i < noEntries; ++i) {
                char filename[32] = {0};
                stringFat16Format(filename,entries[i].filename,entries[i].ext);
                filler(buf,filename,NULL,0,FUSE_FILL_DIR_PLUS);
        }
        return 0;
}
 
static int hello_open(const char *path, struct fuse_file_info *fi) {
        (void) fi;
        set_context_by_path(path);
        char filenameOnly[256] = {0};
        stringParseFilename(filenameOnly, path);
        Fat16Entry entry = findEntryInCurrentDir(filenameOnly);

        if (entry.filename[0] == 0x00 || (unsigned char)entry.filename[0] == 0xE5) {
                return -ENOENT;
        }
        return 0;
}
 
static int hello_read(const char *path, char *buf, size_t size, off_t offset,
                      struct fuse_file_info *fi) {
        (void) fi;
        set_context_by_path(path);
        char filenameOnly[256] = {0};
        stringParseFilename(filenameOnly, path);

        Fat16Entry entry = findEntryInCurrentDir(filenameOnly);

        if (entry.filename[0] == 0x00 || (uint8_t)entry.filename[0] == 0xE5) {
                return -ENOENT;
        }
        if (offset >= entry.file_size) return 0;
        if (offset + size > entry.file_size) size = entry.file_size - offset;

        char* tempBuf = (char*) malloc(entry.file_size * sizeof(char));
        char filenameFAT[13] = {0};
        stringFat16Format(filenameFAT, entry.filename, entry.ext);

        read_(filenameFAT, tempBuf);
        memcpy(buf, tempBuf + offset, size);
        free(tempBuf);
        return size;
}
 
static const struct fuse_operations hello_oper = {
        .init           = hello_init,
        .getattr        = hello_getattr,
        .readdir        = hello_readdir,
        .open           = hello_open,
        .read           = hello_read,
        .write          = writeCallback,
        .create         = createCallback,
        .utimens        = utimensCallback,
        .unlink         = unlinkCallback,
        .rmdir          = rmDirCallback
};

int main(int argc, char *argv[]) {
        int ret;
        printf("%d\n",currentDir.dirLBA);
        struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
        initFileSystem("./sd.img");
        ret = fuse_main(args.argc, args.argv, &hello_oper, NULL);
        fuse_opt_free_args(&args);
        return ret;
}