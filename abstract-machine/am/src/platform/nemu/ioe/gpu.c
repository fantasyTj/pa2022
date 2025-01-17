#include <am.h>
#include <nemu.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)

void __am_gpu_init() {
  // int i;
  // int w = inl(VGACTL_ADDR) >> 16;  // TODO: get the correct width
  // int h = (uint16_t)inl(VGACTL_ADDR);  // TODO: get the correct height
  // uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  // for (i = 0; i < w * h; i ++) fb[i] = i;
  // outl(SYNC_ADDR, 1);
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  uint32_t w_h = inl(VGACTL_ADDR);
  uint16_t w = w_h >> 16;
  uint16_t h = w_h;
  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = w, .height = h,
    .vmemsz = (int)w * (int)h * sizeof(uint32_t)
  };
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  if (ctl->sync) {
    outl(SYNC_ADDR, 1);
  }
  int width = inl(VGACTL_ADDR) >> 16;
  int idx = 0;
  uint32_t *pixel = (uint32_t *)ctl->pixels;
  uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  for(int h_idx = ctl->y; h_idx < ctl->y+ctl->h; h_idx++){
    for(int w_idx = ctl->x; w_idx < ctl->x+ctl->w; w_idx++){
      fb[h_idx*width + w_idx] = pixel[idx++];
    }
  }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
