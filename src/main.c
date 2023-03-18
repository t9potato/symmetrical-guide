#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>
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

typedef struct {
    double direction;
    float x;
    float y;
    float f_vel;
    double r_vel;
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
        SDL_RenderPresent(render);
        SDL_Delay(1);
    }
}
