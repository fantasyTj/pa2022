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
static int events_fd;

uint32_t NDL_GetTicks() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_usec;
}

int NDL_PollEvent(char *buf, int len) {
  if(!events_fd) events_fd = open("/dev/events", "r+");
  // events_fd = open("/dev/events", "r+");
  assert(events_fd);
  size_t flag = 0;
  if(read(events_fd, buf, len)) flag = 1;
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
  screen_w = *w; 
  screen_h = *h;
  
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
  assert(W>=w && H>=h);
  // printf("W is %d, H is %d, w is %d, h is %d\n", W, H, w, h);
  int left_w = (W-screen_w)/2, right_w = W-left_w-screen_w;
  int on_h = (H-screen_h)/2, below_h = H-on_h-screen_h;

  int fb_fd = open("/dev/fb", 0, 0);
  for(int i = 0; i < on_h+y; i++) write(fb_fd, NULL, W);
  for(int i = 0; i < h; i++){
    write(fb_fd, NULL, left_w+x);
    write(fb_fd, pixels+(i*w), w);
    write(fb_fd, NULL, right_w+(screen_w-x-w));
  }
  for(int i = 0; i < below_h+(screen_h-y-h); i++) write(fb_fd, NULL, W);
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
