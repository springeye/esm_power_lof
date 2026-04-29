#pragma once

#include <cstdint>

extern "C" {

typedef struct SDL_Window { int unused; } SDL_Window;
typedef struct SDL_Renderer { int unused; } SDL_Renderer;
typedef struct SDL_Texture { int unused; } SDL_Texture;
typedef struct SDL_Event {
    uint32_t type;
    struct { struct { int sym; } keysym; } key;
} SDL_Event;

static constexpr uint32_t SDL_INIT_VIDEO = 0x00000020u;
static constexpr uint32_t SDL_WINDOW_SHOWN = 0x00000004u;
static constexpr uint32_t SDL_RENDERER_ACCELERATED = 0x00000002u;
static constexpr uint32_t SDL_RENDERER_PRESENTVSYNC = 0x00000004u;
static constexpr uint32_t SDL_PIXELFORMAT_RGB888 = 0;
static constexpr uint32_t SDL_TEXTUREACCESS_STREAMING = 0;
static constexpr uint32_t SDL_QUIT = 0x100u;
static constexpr uint32_t SDL_KEYDOWN = 0x300u;
static constexpr int SDLK_ESCAPE = 27;
static constexpr int SDL_WINDOWPOS_CENTERED = 0;

static inline int SDL_Init(uint32_t) { return 0; }
static inline const char* SDL_GetError(void) { return "SDL stub"; }
static inline SDL_Window* SDL_CreateWindow(const char*, int, int, int, int, uint32_t) { return reinterpret_cast<SDL_Window*>(1); }
static inline SDL_Renderer* SDL_CreateRenderer(SDL_Window*, int, uint32_t) { return reinterpret_cast<SDL_Renderer*>(1); }
static inline SDL_Texture* SDL_CreateTexture(SDL_Renderer*, uint32_t, int, int, int) { return reinterpret_cast<SDL_Texture*>(1); }
static inline void SDL_SetRenderDrawColor(SDL_Renderer*, uint8_t, uint8_t, uint8_t, uint8_t) {}
static inline void SDL_RenderClear(SDL_Renderer*) {}
static inline void SDL_RenderPresent(SDL_Renderer*) {}
static inline int SDL_LockTexture(SDL_Texture*, const void*, void** pixels, int* pitch) { static uint32_t dummy[240 * 280] = {}; if (pixels) *pixels = dummy; if (pitch) *pitch = 240 * 4; return 0; }
static inline void SDL_UnlockTexture(SDL_Texture*) {}
static inline int SDL_RenderCopy(SDL_Renderer*, SDL_Texture*, const void*, const void*) { return 0; }
static inline int SDL_PollEvent(SDL_Event*) { return 0; }
static inline uint32_t SDL_GetTicks(void) { static uint32_t t = 0; return ++t; }
static inline void SDL_DestroyTexture(SDL_Texture*) {}
static inline void SDL_DestroyRenderer(SDL_Renderer*) {}
static inline void SDL_DestroyWindow(SDL_Window*) {}
static inline void SDL_Quit(void) {}

}
