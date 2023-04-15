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
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

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

#define WIDTH 1200
#define HEIGHT 675
#define MAP_SIZE 8
#define RAYCAST_PRECISION 64
#define RENDER_DISTANCE 255

#define set_pixel(pixels, x, y, color) (*(pixels + x + y * WIDTH/5) = color << 8)
#define vertical_line(pixels, x, top, length, color) for (int _i = top; _i < top + length; _i++) set_pixel(pixels, x, _i, color)
#define horisontal_line(pixels, y, left, length, color) for (int _i = left; _i < left + length; _i++) set_pixel(pixels, y, _i, color)
#define rad(value) (f32) value * M_PI /180
#define square(value) (value) * (value)

typedef struct {
    f64 direction;
    f32 x;
    f32 y;
    f32 f_vel;
    f64 r_vel;
    u8  fov;
} player;

void raycast_render(player character, u32 pixels[WIDTH/5 * HEIGHT/5], u8 map[MAP_SIZE * MAP_SIZE]);

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
        0.0, 4.0, 4.0, 0.0, 0.0, 60
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
                        case SDLK_w: {
                            character.f_vel += 0.1;
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
        raycast_render(character, pixels, MAP);
        SDL_UpdateTexture(texture, NULL, pixels, WIDTH/5 * 4);
        SDL_RenderCopy(render, texture, NULL, NULL);
        SDL_RenderPresent(render);
        SDL_memset4(pixels, 0x00000000, sizeof(pixels)/4);
        SDL_Delay(1);
    }
}

void raycast_render(player character, u32 pixels[WIDTH/5 * HEIGHT/5], u8 map[MAP_SIZE * MAP_SIZE]) {
    f64 angle = character.direction - rad(character.fov) / 2;

    for (u8 ray_count = 0;  ray_count < WIDTH/5; ray_count++) {
        f64 ray_sin = sin(angle)/RAYCAST_PRECISION;
        f64 ray_cos = cos(angle)/RAYCAST_PRECISION;
        f32 ray_x = character.x + ray_sin;
        f32 ray_y = character.y + ray_cos;
        for (u8 i = 0; i < RENDER_DISTANCE; i++) {
            if(map[(int)ray_x + (int)ray_y * MAP_SIZE]) {
                u32 height = (int) floor((HEIGHT/5.0)/sqrt(square(character.x - ray_x) + square(character.y - ray_y)));
                vertical_line(pixels, ray_count, (HEIGHT/5 - height)/2, height/2 * 2, 0xffffff);
                break;
            }
            ray_x += ray_sin;
            ray_y += ray_cos;
        }

        angle += rad(character.fov) * 5.0 /WIDTH;
    }
}
