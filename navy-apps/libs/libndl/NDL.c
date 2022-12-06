#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <assert.h>

static int evtdev = -1;
static int fbdev = -1;
static int screen_w = 0, screen_h = 0;

// static FILE* events_fp = NULL;

uint32_t NDL_GetTicks() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_usec;
}

int NDL_PollEvent(char *buf, int len) {
  // if(events_fp) printf("here\n");
  // if(!events_fp) {
  //   printf("here\n");
    
  //   events_fp = fopen("/dev/events", "r+");
  // }
  int events_fd = open("/dev/events", "r+");
  // if(events_fp) printf("h1\n");
  // else printf("h2\n");
  assert(events_fd);
  // printf("succ\n");
  size_t flag = 0;
  if(read(events_fd, buf, len)) flag = 1;
  // return fread(buf, 1, len, events_fp);
  return flag;
}

void NDL_OpenCanvas(int *w, int *h) {
  if(*w==0 && *h==0){
    int disp_fd = open("/proc/dispinfo", 0, 0);
    assert(disp_fd);
    char buf[64];
    read(disp_fd, buf, 64);
    sscanf(buf, "WIDTH:%d HEIGHT:%d", w, h);
  }
  
  if (getenv("NWM_APP")) {
    int fbctl = 4;
    fbdev = 5;
    screen_w = *w; screen_h = *h;
    char buf[64];
    int len = sprintf(buf, "%d %d", screen_w, screen_h);
    // let NWM resize the window and create the frame buffer
    write(fbctl, buf, len);
    while (1) {
      // 3 = evtdev
      int nread = read(3, buf, sizeof(buf) - 1);
      if (nread <= 0) continue;
      buf[nread] = '\0';
      if (strcmp(buf, "mmap ok") == 0) break;
    }
    close(fbctl);
  }
}

void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h) {
  int W, H;
  int disp_fd = open("/proc/dispinfo", 0, 0);
  assert(disp_fd);
  char buf[64];
  read(disp_fd, buf, 64);
  sscanf(buf, "WIDTH:%d HEIGHT:%d", &W, &H);

  int left_w = (W-w)/2, right_w = W-left_w-w;
  int on_h = (H-h)/2, below_h = H-on_h-h;

  int fb_fd = open("/dev/fb", 0, 0);
  for(int i = 0; i < on_h; i++) write(fb_fd, NULL, W);
  for(int i = 0; i < h; i++){
    write(fb_fd, NULL, left_w);
    write(fb_fd, pixels+(i*w), w);
    write(fb_fd, NULL, right_w);
  }
  for(int i = 0; i < below_h; i++) write(fb_fd, NULL, W);
}

void NDL_OpenAudio(int freq, int channels, int samples) {
}

void NDL_CloseAudio() {
}

int NDL_PlayAudio(void *buf, int len) {
  return 0;
}

int NDL_QueryAudio() {
  return 0;
}

int NDL_Init(uint32_t flags) {
  if (getenv("NWM_APP")) {
    evtdev = 3;
  }
  return 0;
}

void NDL_Quit() {
}
