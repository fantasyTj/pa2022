#define SDL_malloc  malloc
#define SDL_free    free
#define SDL_realloc realloc

#define SDL_STBIMAGE_IMPLEMENTATION
#include "SDL_stbimage.h"

SDL_Surface* IMG_Load_RW(SDL_RWops *src, int freesrc) {
  assert(src->type == RW_TYPE_MEM);
  assert(freesrc == 0);
  return NULL;
}

SDL_Surface* IMG_Load(const char *filename) {
  FILE* fp = fopen(filename, "r+");
  assert(fp);
  size_t size;
  fseek(fp, 0, SEEK_END);
  size = ftell(fp);
  // printf("size is %u\n", size);
  fseek(fp, 0, SEEK_SET);
  void *buf = malloc(size);
  fread(buf, size, 1, fp);
  SDL_Surface *ret_pt = malloc(sizeof(SDL_Surface));
  ret_pt = STBIMG_LoadFromMemory(buf, size);
  free(buf);
  fclose(fp);
  return ret_pt;
}

int IMG_isPNG(SDL_RWops *src) {
  return 0;
}

SDL_Surface* IMG_LoadJPG_RW(SDL_RWops *src) {
  return IMG_Load_RW(src, 0);
}

char *IMG_GetError() {
  return "Navy does not support IMG_GetError()";
}
