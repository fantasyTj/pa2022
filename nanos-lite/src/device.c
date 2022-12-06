#include <common.h>

extern uint8_t ramdisk_start;
extern uint8_t ramdisk_end;
#define RAMDISK_SIZE ((&ramdisk_end) - (&ramdisk_start))

#if defined(MULTIPROGRAM) && !defined(TIME_SHARING)
# define MULTIPROGRAM_YIELD() yield()
#else
# define MULTIPROGRAM_YIELD()
#endif

#define NAME(key) \
  [AM_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [AM_KEY_NONE] = "NONE",
  AM_KEYS(NAME)
};

size_t serial_write(const void *buf, size_t offset, size_t len) {
  const char *p = buf;
  for(size_t i = 0; i < len; i++){
    putch(*(p+i));
  }
  return len;
}

size_t events_read(void *buf, size_t offset, size_t len) {
  AM_INPUT_KEYBRD_T ev = io_read(AM_INPUT_KEYBRD);
  char fmt1[3];
  char fmt2[20];
  if(ev.keycode == AM_KEY_NONE){
    return 0;
  }else{
    strcpy(fmt1, (ev.keydown)?("kd "):("ku ")); // initialnize msg
    strcpy(fmt2, keyname[ev.keycode]);
    snprintf(buf, len, "%s %s\n", fmt1, fmt2);
    return len;
  }
}

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  return 0;
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
  return 0;
}

size_t unsuported_r(void *buf, size_t offset, size_t len){
  panic("This device doesn't support read\n");
  return -1;
}

size_t unsuported_w(const void *buf, size_t offset, size_t len){
  panic("This device doesn't support write\n");
  return -1;
}

void init_device() {
  Log("Initializing devices...");
  ioe_init();
}
