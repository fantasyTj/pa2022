#include <NDL.h>
#include <SDL.h>

#define keyname(k) #k,

static const char *keyname[] = {
  "NONE",
  _KEYS(keyname)
};

static int NR_KEYSTATE = sizeof(keyname)/sizeof(char *);
static uint8_t keystate[sizeof(keyname)/sizeof(char *)];

int SDL_PushEvent(SDL_Event *ev) {
  return 0;
}

int SDL_PollEvent(SDL_Event *ev) {
  char buf[64];
  int res;
  if(!NDL_PollEvent(buf, 64)) return 0;
  else{
    // printf("keydown\n");
    int keycode;
    if(buf[1] == 'd'){
      ev->key.type = SDL_KEYDOWN;
      sscanf(buf, "kd (%d)", &keycode);
      keystate[keycode] = 1;
    }else{
      ev->key.type = SDL_KEYUP;
      sscanf(buf, "ku (%d)", &keycode);
      keystate[keycode] = 0;
    }
    ev->key.keysym.sym = (uint8_t)keycode;
    return 1;
  }
}

int SDL_WaitEvent(SDL_Event *event) {
  char buf[64];
  while(!NDL_PollEvent(buf, 64)) ;
  int keycode;
  if(buf[1] == 'd'){ // kd
    event->key.type = SDL_KEYDOWN;
    sscanf(buf, "kd (%d)", &keycode);
    keystate[keycode] = 1;
  }else{
    event->key.type = SDL_KEYUP;
    sscanf(buf, "ku (%d)", &keycode);
    keystate[keycode] = 0;
  }
  event->key.keysym.sym = (uint8_t)keycode;
  return 1;
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
  return 0;
}

uint8_t* SDL_GetKeyState(int *numkeys) {
  return keystate;
}