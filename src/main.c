#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>
//#include <cstring>
#include <stdlib.h>

typedef float    f32;
typedef double   f64;
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;
typedef size_t   usize;
typedef ssize_t  isize;

void draw_pixel(SDL_Texture *screen, usize x, usize y, u8 R, u8 G, u8 B);


typedef struct {
    f64 direction;
    f32 x;
    f32 y;
    f32 f_vel;
    f64 r_vel;
} player;

#define WIDTH 1200
#define HEIGHT 675

#define MAP_SIZE 8
static u8 MAP[MAP_SIZE * MAP_SIZE] = {
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
};

int main(int argc, char *argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    atexit(SDL_Quit);

    SDL_Window *window = SDL_CreateWindow("Symmetrical Guide",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        WIDTH, HEIGHT,
        0
    );
    u32 window_id = SDL_GetWindowID(window);
    SDL_Renderer *render = SDL_CreateRenderer(window,
            -1, SDL_RENDERER_SOFTWARE);
    SDL_Texture *texture = SDL_CreateTexture(render, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, WIDTH / 5, HEIGHT / 5);
    u32 pixels[WIDTH/5 * HEIGHT/5];

    player character = {
        0.0, 4.0, 4.0, 0.0, 0.0
    };

    SDL_SetRelativeMouseMode(SDL_TRUE);
    for (;;) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_WINDOWEVENT: {
                    if (event.window.windowID == window_id) {
                        switch (event.window.event) {
                            case SDL_WINDOWEVENT_CLOSE: {
                                event.type = SDL_QUIT;
                                SDL_PushEvent(&event);
                                break;
                            }
                        }
                    }
                    break;
                }
                case SDL_KEYDOWN: {
                    switch (event.key.keysym.sym) {
                        case SDLK_ESCAPE: {
                            SDL_SetRelativeMouseMode(SDL_FALSE);
                        }
                    }
                    break;
                }
                case SDL_MOUSEBUTTONDOWN: {
                    SDL_SetRelativeMouseMode(SDL_TRUE);
                    break;
                }
                case SDL_QUIT: {
                    return 0;
                }
            }
        }
        SDL_UpdateTexture(texture, NULL, pixels, WIDTH/5 * 4);
        SDL_RenderCopy(render, texture, NULL, NULL);
        SDL_RenderPresent(render);
        SDL_memset4(pixels, 0x0000ff00, sizeof(pixels)/4);
        SDL_Delay(1);
    }
}

void draw_pixel(SDL_Texture *screen, usize x, usize y, u8 R, u8 G, u8 B) {

}
