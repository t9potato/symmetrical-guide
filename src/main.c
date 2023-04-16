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

#define WIDTH 2400
#define HEIGHT 1360
#define MAP_SIZE 10
#define RAYCAST_PRECISION 64
#define RENDER_DISTANCE RAYCAST_PRECISION * 7

#define SENSITIVITY 0.001
#define MAX_ROT_SPEED 0.01
#define ACCELERATION 0.001
#define MAX_SPEED 0.01

#define set_pixel(pixels, x, y, color) (*(pixels + x + y * WIDTH/5) = color << 8)
#define vertical_line(pixels, x, top, length, color) for (int _i = top; _i < top + length; _i++) set_pixel(pixels, x, _i, color)
#define horisontal_line(pixels, y, left, length, color) for (int _i = left; _i < left + length; _i++) set_pixel(pixels, y, _i, color)
#define rad(value) (f32) value * M_PI /180
#define square(value) (value) * (value)

typedef enum {
    POS,
    NEUTRAL,
    NEG,
}input;

typedef struct {
    f64 direction;
    f32 x;
    f32 y;
    f32 f_vel;
    f64 r_vel;
    u8  fov;
    input mov;
    input rot;
} player;

void raycast_render(player character, u32 pixels[WIDTH/5 * HEIGHT/5], u8 map[MAP_SIZE * MAP_SIZE]);
void update_player(player *character, u8 map[square(MAP_SIZE)]);

#define WALL_1 0xffffff
#define WALL_2 0xff0000
#define WALL_3 0x00ff00
#define WALL_4 0x0000ff
#define WALL_5 0xff00ff

static u8 MAP[MAP_SIZE * MAP_SIZE] = {
        1,1,1,1,1,1,1,1,1,1,
        1,0,0,0,0,0,0,0,0,1,
        1,0,0,0,0,0,0,0,0,1,
        1,0,0,1,1,0,1,0,0,1,
        1,0,0,1,0,0,1,0,0,1,
        1,0,0,1,0,0,1,0,0,1,
        1,0,0,1,0,1,1,0,0,1,
        1,0,0,0,0,0,0,0,0,1,
        1,0,0,0,0,0,0,0,0,1,
        1,1,1,1,1,1,1,1,1,1,
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
    SDL_Texture *texture = SDL_CreateTexture(render, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, WIDTH/5, HEIGHT/5);
    u32 pixels[WIDTH/5 * HEIGHT/5];

    player character = {
        0.0, 1.0, 2.0, 0.0, 0.0, 60, 1, 1
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
                            break;
                        }
                        case SDLK_w: {
                            character.mov = POS;
                            break;
                        }
                        case SDLK_s: {
                            character.mov = NEG;
                            break;
                        }
                        case SDLK_a: {
                            character.rot = POS;
                            break;
                        }
                        case SDLK_d: {
                            character.rot = NEG;
                            break;
                        }
                    }
                    break;
                }
                case SDL_KEYUP: {
                    switch (event.key.keysym.sym) {
                        case SDLK_w: {
                            if (character.mov == POS)
                                character.mov = NEUTRAL;
                            break;
                        }
                        case SDLK_s: {
                            if (character.mov == NEG)
                                character.mov = NEUTRAL;
                            break;
                        }
                        case SDLK_a: {
                            if (character.rot == POS)
                                character.rot = NEUTRAL;
                            break;
                        }
                        case SDLK_d: {
                            if (character.rot == NEG)
                                character.rot = NEUTRAL;
                            break;
                        }
                    }
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
        update_player(&character, MAP);
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

    for (u16 ray_count = 0;  ray_count < WIDTH/5; ray_count++) {
        f64 ray_sin = sin(angle)/RAYCAST_PRECISION;
        f64 ray_cos = cos(angle)/RAYCAST_PRECISION;
        f32 ray_x = character.x;
        f32 ray_y = character.y;
        for (u16 i = 0; i < RENDER_DISTANCE; i++) {
            if(map[(int)ray_x + (int)ray_y * MAP_SIZE]) {
                f32 dist = sqrt(square(character.x - ray_x) + square(character.y - ray_y));
                dist *= cos(angle - character.direction);
                u32 height = (int)floor((HEIGHT/5.0)/(dist >=1?dist:1));
                height = height >> 1 << 1;
                u32 color;
                switch (map[(int)ray_x + (int)ray_y*MAP_SIZE]) {
                    case 1:
                        color = WALL_1;
                        break;
                    case 2:
                        color = WALL_2;
                        break;
                    case 3:
                        color = WALL_3;
                        break;
                    case 4:
                        color = WALL_4;
                        break;
                    case 5:
                        color = WALL_5;
                        break;
                }
                vertical_line(pixels, ray_count, (HEIGHT/5 - height)/2, height, color);
                break;
            }
            ray_x += ray_cos;
            ray_y += ray_sin;
        }

        angle += rad(character.fov) * 5/WIDTH;
    }
}

void update_player(player *character, u8 map[square(MAP_SIZE)]) {
    if (character->mov == POS && character->f_vel < MAX_SPEED)
        character->f_vel += ACCELERATION;
    else if (character->mov == NEG && character->f_vel > -MAX_SPEED)
        character->f_vel -= ACCELERATION;
    else if (character->mov == NEUTRAL) {
        if (character->f_vel >= ACCELERATION)
            character->f_vel -= ACCELERATION;
        else if (character->f_vel <= -ACCELERATION)
            character->f_vel += ACCELERATION;
        else
            character->f_vel = 0;
    }
    if (character->rot == POS && character->r_vel > -MAX_ROT_SPEED)
        character->r_vel -= SENSITIVITY;
    else if (character->rot == NEG && character->r_vel < MAX_ROT_SPEED)
        character->r_vel += SENSITIVITY;
    else if (character->rot == NEUTRAL) {
        if (character->r_vel >= SENSITIVITY)
            character->r_vel -= SENSITIVITY;
        else if (character->r_vel <= -SENSITIVITY)
            character->r_vel += SENSITIVITY;
        else
            character->r_vel = 0;
    }
    character->direction += character->r_vel;
    character->x += cos(character->direction) * character->f_vel;
    character->y += sin(character->direction) * character->f_vel;
//  if (map[(int)(round(character->x) + round(character->y)*MAP_SIZE)]) {
//      character->x = 4;
//      character->y = 4;
//  }
}
