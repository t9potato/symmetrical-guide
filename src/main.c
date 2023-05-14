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
#include <string.h>

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
#define RAYCAST_PRECISION 64
#define RENDER_DISTANCE RAYCAST_PRECISION * 10
#define CEIL_HEIGHT HEIGHT/10 - 7*HEIGHT/680
#define MAX_MAP_WIDTH 2*25

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

typedef struct {
    u8 map_width;
    u8 map_height;
    u8 *map;
} map_info;

void raycast_render(player character, u32 pixels[WIDTH/5 * HEIGHT/5], map_info map);
void update_player(player *character, map_info map);
map_info load_map(char *level);
void drop_map(map_info map);

#define WALL_1 0xffffff
#define WALL_2 0x7acbf5
#define WALL_3 0xeaacb8
#define WALL_4 0x0000ff
#define WALL_5 0xff00ff

#define FLOOR 0x000000
#define CEIL 0x222222



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
        0.0, 2.0, 2.0, 0.0, 0.0, 60, 1, 1
    };
    map_info map = load_map("levels/level1");
    if (map.map == NULL) {
        exit(1);
    }

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
        update_player(&character, map);
        raycast_render(character, pixels, map);
        SDL_UpdateTexture(texture, NULL, pixels, WIDTH/5 * 4);
        SDL_RenderCopy(render, texture, NULL, NULL);
        SDL_RenderPresent(render);
        SDL_memset4(pixels, 0x00000000, sizeof(pixels)/4);
        SDL_Delay(1);
    }
}

void raycast_render(player character, u32 pixels[WIDTH/5 * HEIGHT/5], map_info map) {
    f64 angle = character.direction - rad(character.fov) / 2;

    for (u16 ray_count = 0;  ray_count < WIDTH/5; ray_count++) {
        f64 ray_sin = sin(angle)/RAYCAST_PRECISION;
        f64 ray_cos = cos(angle)/RAYCAST_PRECISION;
        f32 ray_x = character.x;
        f32 ray_y = character.y;
        vertical_line(pixels, ray_count, 0, CEIL_HEIGHT, CEIL);
        vertical_line(pixels, ray_count, (HEIGHT/10), HEIGHT/10, FLOOR);
        for (u16 i = 0; i < RENDER_DISTANCE; i++) {
            if(map.map[(int)ray_x + (int)ray_y * map.map_width]) {
                f32 dist = sqrt(square(character.x - ray_x) + square(character.y - ray_y));
                dist *= cos(angle - character.direction);
                u32 height = (int)floor((HEIGHT/5.0)/(dist >=1?dist:1));
                height = height >> 1 << 1;
                u32 color;
                switch (map.map[(int)ray_x + (int)ray_y*map.map_width]) {
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

void update_player(player *character, map_info map) {
    char *e = "testing";
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
    if (character->direction > 2 * M_PI)
        character->direction -= 2*M_PI;
    f32 x_vel = cos(character->direction) * character->f_vel;
    f32 y_vel = sin(character->direction) * character->f_vel;
    character->x += x_vel;
    if (map.map[(int)floor(character->x) + (int)floor(character->y) * map.map_width]) {
        character->x -= x_vel;
    }
    character->y += y_vel;
    if (map.map[(int)floor(character->x) + (int)floor(character->y) * map.map_width]) {
        character->y -= y_vel;
    }
}


map_info load_map(char *level) {
    FILE *map_source = fopen(level, "r");
    if (map_source == NULL) {
        fclose(map_source);
        map_info map = { -1, -1, NULL };
        return map;
    }
    const char delim[2] = ",";
    char contents[MAX_MAP_WIDTH];
    fgets(contents, MAX_MAP_WIDTH, map_source);
    int map_width, map_height;
    sscanf(contents, "%i,%i", &map_width, &map_height);
    map_info map = { map_width, map_height, malloc(map_width * map_height * sizeof(u8)) };
    i32 i = 0;
    while (fgets(contents, MAX_MAP_WIDTH, map_source) != NULL) {
        for (u16 k = 0; k < map.map_width; k++) {
            sscanf((contents + 2*k), "%s,", (map.map + i));
            map.map[i] -= 48;
            i++;
        }
    }
    return map;
}

void drop_map(map_info map) {
   // free(map.map);
}
