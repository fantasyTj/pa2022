#include <fs.h>

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

size_t ramdisk_read(void *buf, size_t offset, size_t len);
size_t ramdisk_write(const void *buf, size_t offset, size_t len);
size_t serial_write(const void *buf, size_t offset, size_t len);
size_t events_read(void *buf, size_t offset, size_t len);
size_t dispinfo_read(void *buf, size_t offset, size_t len);
size_t fb_write(const void *buf, size_t offset, size_t len);
size_t unsuported_r(void *buf, size_t offset, size_t len);
size_t unsuported_w(const void *buf, size_t offset, size_t len);

typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  ReadFn read;
  WriteFn write;
  size_t open_offset;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_EVENTS, FD_DISPINFO, FD_FB};

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
  [FD_STDIN]  = {"stdin", 0, 0, unsuported_r, unsuported_w},
  [FD_STDOUT] = {"stdout", 0, 0, unsuported_r, serial_write},
  [FD_STDERR] = {"stderr", 0, 0, unsuported_r, serial_write},
  [FD_EVENTS] = {"/dev/events", 0, 0, events_read, unsuported_w},
  [FD_DISPINFO] = {"/proc/dispinfo", 0, 0, dispinfo_read, unsuported_w},
  [FD_FB] = {"/dev/fb", 0, 0, unsuported_r, fb_write},
#include "files.h"
  {"end", 0, 0},
};

// static void bind(){
//   int i = 3;
//   while(true){
//     if(strcmp(file_table[i].name, "end") == 0) break;
//     if(!strcmp(file_table[i].name, "/dev/events")){
//       file_table[i].read = events_read;
//       file_table[i].write = unsuported_w;
//     }else{
//       file_table[i].read = ramdisk_read;
//       file_table[i].write = ramdisk_write;
//     }
//     i++;
//   }
// }

int fs_open(const char *pathname, int flags, int mode){
  int i = 3; // ???
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
  if(open_offset+count > size){
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
    case SEEK_END: file_table[fd].open_offset = (file_table[fd].size) + offset; break; // refer to "man 2 lseek"
    default: return -1;
  }
  return file_table[fd].open_offset;
}

const char *fd2name(int fd){
  return file_table[fd].name;
}

size_t vfs_read(int fd, void *buf, size_t count){
  size_t size = file_table[fd].size, disk_offset = file_table[fd].disk_offset, open_offset = file_table[fd].open_offset;
  if(file_table[fd].read == NULL){
    if(open_offset+count > size){
      ramdisk_read(buf, disk_offset+open_offset, size-open_offset);
      file_table[fd].open_offset = 0;
      printf("offset is %u\n", file_table[fd].open_offset);
      return size-open_offset;
    }else{
      ramdisk_read(buf, disk_offset+open_offset, count);
      file_table[fd].open_offset = (open_offset + count);
      printf("offset is %u\n", file_table[fd].open_offset);
      return count;
    }
  }else{
    return file_table[fd].read(buf, disk_offset, count);
  }
  // ReadFn func = file_table[fd].read;
  
  // if(open_offset+count > size){
  //   func(buf, disk_offset+open_offset, size-open_offset);
  //   file_table[fd].open_offset = 0;
  //   printf("offset is %u\n", file_table[fd].open_offset);
  //   return size-open_offset;
  // }else{
  //   func(buf, disk_offset+open_offset, count);
  //   file_table[fd].open_offset = (open_offset + count);
  //   printf("offset is %u\n", file_table[fd].open_offset);
  //   return count;
  // }
}

size_t vfs_write(int fd, const void *buf, size_t count){
  size_t size = file_table[fd].size, disk_offset = file_table[fd].disk_offset, open_offset = file_table[fd].open_offset;
  if(file_table[fd].write == NULL){
    if(open_offset+count>size){
      ramdisk_write(buf, disk_offset+open_offset, size-open_offset);
      file_table[fd].open_offset = 0;
      return size-open_offset;
    }else{
      ramdisk_write(buf, disk_offset+open_offset, count);
      file_table[fd].open_offset += count;
      return count;
    }
  }else{
    return file_table[fd].write(buf, file_table[fd].disk_offset, count);
  }
  // WriteFn func = file_table[fd].write;
  // // if(size) assert(open_offset+count <= size); // assume read is always legal
  // // if()
  // func(buf, disk_offset+open_offset, count);
  // file_table[fd].open_offset = open_offset+count;
  // return count;
}

void init_fs() {
  // TODO: initialize the size of /dev/fb
  AM_GPU_CONFIG_T cfg = io_read(AM_GPU_CONFIG);
  int w = cfg.width, h = cfg.height;
  file_table[fs_open("/proc/dispinfo", 0, 0)].size = w*h*4;
}
