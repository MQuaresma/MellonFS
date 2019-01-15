/**
 * 
 *
 */


#include "mellon.h"
/**
 * Initial configurations for file system:
 *      - disable caching of various inode related info
 */
static void *mellon_init(struct fuse_conn_info *conn, struct fuse_config *cfg){
    cfg->use_ino = 1;
    cfg->entry_timeout = 0;
	cfg->attr_timeout = 0;
	cfg->negative_timeout = 0;
    return NULL;
}

static int mellon_statfs(const char *path, struct statvfs *stfs){
    if(statvfs(path, stfs)==-1)
        return -errno;
    else return 0;
}

/**
 * Check for access permissions (set with umask(0) call in main)
 */
static int mellon_access(const char *path, int mask){
    if(access(path, mask)==-1) 
        return -errno;
    else return 0;
}

/**
 * Get file attributes for access control
 */
static int mellon_getattr(const char *path, struct stat *st, struct fuse_file_info *fi){
    //Check if file/dir is open
    if((fi && fstat(fi->fh, st)!=-1) || lstat(path, st)!=-1) 
        return 0;
    else return -errno;
}

/**
 * Change owner of dir/file
 */
static int mellon_chown(const char *path, uid_t uid, gid_t gid, struct fuse_file_info *fi){
    if((fi && fchown(fi->fh, uid, gid)!=-1) || lchown(path, uid, gid)==-1) 
        return 0;
    else return -errno;
}

/**
 * Change permissions of dir/file
 */
static int mellon_chmod(const char *path, mode_t mode, struct fuse_file_info *fi){
    if((fi && fchmod(fi->fh, mode)!=-1) || chmod(path, mode)==-1) 
        return 0;
    else return -errno;
}

/**
 * Make subdirectory
 */
static int mellon_mkdir(const char *dir, mode_t mode){
    if(mkdir(dir, mode)==-1) 
        return -errno;
    else return 0;
}

/**
 * Remove directory
 */
static int mellon_rmdir(const char *dir){
    if(rmdir(dir)==-1) 
        return -errno;
    else return 0;
}

/**
 * Change file/directory name from old to new
 */
static int mellon_rename(const char *old, const char *new, unsigned int flgs){
    if(flgs)
        return -EINVAL;
    else if(!rename(old, new))
        return -errno;
    else return 0;
}

/**
 * List files in directory
 * FIX: list actual files
 */
static int mellon_readdir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi, enum fuse_readdir_flags flags){
    if(!strcmp(path, "/")){
        filler(buffer, ".", NULL, 0, 0);
        filler(buffer, "..", NULL, 0, 0);
    }
    return 0;
}


/**
 * Create/touch files
 */
static int mellon_create(const char *file_name, mode_t mode, struct fuse_file_info *fi){
    int fd = open(file_name, fi->flags, mode);

    if(fd==-1)
        return -errno;

    fi->fh = fd;
    return 0;
}

/**
 * Generate random number between 0000 and 9999
 * returns -1 if error occurs
 */
int gen2FACode(){
	return 1;
	/*bytes fa_code_byte[sizeof(int)];
	int fa_code;

	if(RAND_bytes(fa_code_byte, sizeof(fa_code)) != 1){
		fprintf(stderr, "Couldn't generated 2FA code, exiting...\n");
		return -1;
	}*/
}

/*
 * Called when opening a file for reading/writing/appending
 */
static int mellon_open(const char *file_name, struct fuse_file_info *fi){
    int fh;
    char fa_code[5];

    puts("Enter access code: ");
    fgets(fa_code, sizeof(fa_code), stdin);

    if(!strcmp(fa_code, "1\n")){
        fh = open(file_name, fi->flags);
        if(fh!=-1){
            fi->fh = fh;        //update current file handler
            return 0;
        }
    }

    return -errno;
}

/*
 * Called for reading a file e.g: cat <file_name>
 */
static int mellon_read(const char *file_name, char *buf, size_t size, off_t offset, struct fuse_file_info *fi){
    int bytes_r = pread(fi->fh, buf, size, offset);

    return (bytes_r == -1 ? -errno : bytes_r);
}

/**
 * Callback for writing files e.g: echo teste > <file_name>
 */
static int mellon_write(const char *file_name, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi){
    int bytes_w = pwrite(fi->fh, buf, size, offset);

    return (bytes_w == -1 ? -errno : bytes_w);
}


int main(int argc, char *argv[]){
    umask(0); //remove all restrictions
    fuse_main(argc, argv, &mellon_ops, NULL);
}
