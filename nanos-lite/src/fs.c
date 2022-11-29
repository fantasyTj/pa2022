#include <fs.h>

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

size_t ramdisk_read(void *buf, size_t offset, size_t len);
size_t ramdisk_write(const void *buf, size_t offset, size_t len);

typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  ReadFn read;
  WriteFn write;
  size_t open_offset;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB};

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  [FD_STDIN]  = {"stdin", 0, 0, invalid_read, invalid_write},
  [FD_STDOUT] = {"stdout", 0, 0, invalid_read, invalid_write},
  [FD_STDERR] = {"stderr", 0, 0, invalid_read, invalid_write},
#include "files.h"
  {"end", 0, 0},
};

int fs_open(const char *pathname, int flags, int mode){
  int i = 3;
  while(true){
    putch(i+'0');
    if(strcmp(file_table[i].name, "end") == 0) assert(0);
    if(strcmp(file_table[i].name, pathname) == 0) return i;
    i++;
  }
}

int fs_close(int fd){
  return 0;
}

size_t fs_read(int fd, void *buf, size_t count){
  size_t size = file_table[fd].size, disk_offset = file_table[fd].disk_offset, open_offset = file_table[fd].open_offset;
  if(disk_offset+count > size){
    ramdisk_read(buf, disk_offset+open_offset, size-open_offset);
    file_table[fd].open_offset = 0;
    return size-open_offset;
  }else{
    ramdisk_read(buf, disk_offset+open_offset, count);
    file_table[fd].open_offset = (open_offset + count);
    return count;
  }
}

size_t fs_write(int fd, const void *buf, size_t count){
  size_t size = file_table[fd].size, disk_offset = file_table[fd].disk_offset, open_offset = file_table[fd].open_offset;
  assert(open_offset+count <= size);
  ramdisk_write(buf, disk_offset+open_offset, count);
  file_table[fd].open_offset = open_offset+count;
  return count;
}

size_t fs_lseek(int fd, size_t offset, int whence){ // modified size_t with off_t?
  switch(whence){
    case SEEK_SET: file_table[fd].open_offset = offset; break;
    case SEEK_CUR: file_table[fd].open_offset += offset; break;
    case SEEK_END: file_table[fd].open_offset = file_table[fd].size + offset; break;
    default: return -1;
  }
  return file_table[fd].open_offset;
}


void init_fs() {
  // TODO: initialize the size of /dev/fb
}
