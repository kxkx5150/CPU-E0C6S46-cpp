#include <cstddef>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include "tamago.h"

extern Tamago *tamgo;


void *Tamago::hal_malloc(u32_t size)
{
    return SDL_malloc(size);
}
void Tamago::hal_free(void *ptr)
{
    SDL_free(ptr);
}
void Tamago::hal_halt(void)
{
    exit(EXIT_SUCCESS);
}
timestamp_t Tamago::hal_get_timestamp(void)
{
    struct timespec time;
    clock_gettime(CLOCK_REALTIME, &time);
    return (time.tv_sec * 1000000 + time.tv_nsec / 1000);
}
void Tamago::hal_sleep_until(timestamp_t ts)
{
    struct timespec t;
    int32_t         remaining = (int32_t)(ts - hal_get_timestamp());
    if (remaining > 0) {
        t.tv_sec  = remaining / 1000000;
        t.tv_nsec = (remaining % 1000000) * 1000;
        nanosleep(&t, NULL);
    }
}
void Tamago::hal_update_screen(void)
{
    unsigned int i, j;
    SDL_Rect     r, src_icon_r, dest_icon_r;
    SDL_RenderCopy(renderer, bg, NULL, &bg_rect);
    for (j = 0; j < LCD_HEIGHT; j++) {
        for (i = 0; i < LCD_WIDTH; i++) {
            r.w = PIXEL_SIZE;
            r.h = PIXEL_SIZE;
            r.x = i * DEFAULT_PIXEL_STRIDE + LCD_OFFSET_X;
            r.y = j * DEFAULT_PIXEL_STRIDE + LCD_OFFSET_Y;
            if (matrix_buffer[j][i]) {
                SDL_SetRenderDrawColor(renderer, 0, 0, 128, DEFAULT_LCD_ALPHA_ON);
            } else {
                SDL_SetRenderDrawColor(renderer, 0, 0, 128, PIXEL_ALPHA_OFF);
            }
            SDL_RenderFillRect(renderer, &r);
        }
    }
    for (i = 0; i < ICON_NUM; i++) {
        src_icon_r.w = ICON_SRC_SIZE;
        src_icon_r.h = ICON_SRC_SIZE;
        src_icon_r.x = (i % 4) * ICON_SRC_SIZE;
        src_icon_r.y = (i / 4) * ICON_SRC_SIZE;

        dest_icon_r.w = ICON_DEST_SIZE;
        dest_icon_r.h = ICON_DEST_SIZE;

        dest_icon_r.x = (i % 4) * ICON_STRIDE_X + ICON_OFFSET_X;
        dest_icon_r.y = (i / 4) * ICON_STRIDE_Y + ICON_OFFSET_Y;

        SDL_SetTextureColorMod(icons, 0, 0, 128);
        if (icon_buffer[i]) {
            SDL_SetTextureAlphaMod(icons, DEFAULT_LCD_ALPHA_ON);
        } else {
            SDL_SetTextureAlphaMod(icons, PIXEL_ALPHA_OFF);
        }
        SDL_RenderCopy(renderer, icons, &src_icon_r, &dest_icon_r);
    }

    SDL_RenderPresent(renderer);
}
void Tamago::hal_play_frequency(bool_t en)
{
    if (is_audio_playing != en) {
        is_audio_playing = en;
    }
}
int Tamago::handle_sdl_events(SDL_Event *event)
{
    char save_path[256];
    switch (event->type) {
        case SDL_QUIT:
            return 1;
        case SDL_WINDOWEVENT:
            switch (event->window.event) {
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                    break;
            }
            break;
        case SDL_KEYDOWN:
            switch (event->key.keysym.sym) {
                case SDLK_AC_BACK:
                case SDLK_ESCAPE:
                case SDLK_q:
                    return 1;
                case SDLK_f:
                    switch (speed) {
                        case SPEED_1X:
                            speed = SPEED_10X;
                            break;
                        case SPEED_10X:
                            speed = SPEED_UNLIMITED;
                            break;
                        case SPEED_UNLIMITED:
                            speed = SPEED_1X;
                            break;
                    }
                    TAMALIB_SET_SPEED((u8_t)speed);
                    break;
                case SDLK_LEFT:
                    TAMALIB_SET_BUTTON(BTN_LEFT, BTN_STATE_PRESSED);
                    break;
                case SDLK_DOWN:
                    TAMALIB_SET_BUTTON(BTN_MIDDLE, BTN_STATE_PRESSED);
                    break;
                case SDLK_RIGHT:
                    TAMALIB_SET_BUTTON(BTN_RIGHT, BTN_STATE_PRESSED);
                    break;
            }
            break;
        case SDL_KEYUP:
            switch (event->key.keysym.sym) {
                case SDLK_LEFT:
                    TAMALIB_SET_BUTTON(BTN_LEFT, BTN_STATE_RELEASED);
                    break;
                case SDLK_DOWN:
                    TAMALIB_SET_BUTTON(BTN_MIDDLE, BTN_STATE_RELEASED);
                    break;
                case SDLK_RIGHT:
                    TAMALIB_SET_BUTTON(BTN_RIGHT, BTN_STATE_RELEASED);
                    break;
            }
            break;
    }
    return 0;
}
int Tamago::hal_handler(void)
{
    SDL_Event   event;
    timestamp_t ts;
    while (SDL_PollEvent(&event)) {
        if (handle_sdl_events(&event)) {
            return 1;
        }
    }
    return 0;
}
void Tamago::audio_callback(void *userdata, Uint8 *stream, int len)
{
    unsigned int i;
    int          samples = len / sizeof(float);
    if (is_audio_playing) {
        for (i = 0; i < samples; i++) {
            ((float *)stream)[i] = AUDIO_VOLUME * SDL_sinf(2 * M_PI * (i + sin_pos) * 23406 / (AUDIO_FREQUENCY * 10));
        }
        sin_pos = (sin_pos + samples) % (AUDIO_FREQUENCY * 10);
    } else {
        SDL_memset(stream, 0, len);
        sin_pos = 0;
    }
}
void audio_cb(void *userdata, Uint8 *stream, int len)
{
    tamgo->audio_callback(userdata, stream, len);
}
void Tamago::sdl_release(void)
{
    SDL_DestroyTexture(icons);
    SDL_DestroyTexture(bg);
    IMG_Quit();
    SDL_DestroyWindow(window);
    SDL_Quit();
}
bool_t Tamago::sdl_init(void)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO) != 0) {
        return 1;
    }
    if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) {
        SDL_Quit();
        return 1;
    }
    window   = SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, BG_SIZE, BG_SIZE, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    bg = IMG_LoadTexture(renderer, BACKGROUND_PATH);
    if (!bg) {
        sdl_release();
        return 1;
    }

    icons = IMG_LoadTexture(renderer, ICONS_PATH);
    if (!icons) {
        sdl_release();
        return 1;
    }

    bg_rect.x = 0;
    bg_rect.y = 0;
    bg_rect.w = BG_SIZE;
    bg_rect.h = BG_SIZE;

    SDL_AudioSpec audio_spec;
    SDL_memset(&audio_spec, 0, sizeof(audio_spec));

    audio_spec.freq     = AUDIO_FREQUENCY;
    audio_spec.format   = AUDIO_F32SYS;
    audio_spec.channels = 1;
    audio_spec.samples  = AUDIO_SAMPLES;
    audio_spec.callback = &audio_cb;

    SDL_AudioDeviceID audio_dev =
        SDL_OpenAudioDevice(NULL, 0, &audio_spec, &audio_spec, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);

    if (!audio_dev) {
        sdl_release();
        return 1;
    }
    SDL_PauseAudioDevice(audio_dev, SDL_FALSE);
    return 0;
}
void Tamago::tamalib_step(void)
{
    if (exec_mode == EXEC_MODE_PAUSE) {
        return;
    }
    if (g_cpu->cpu_step()) {
        exec_mode  = EXEC_MODE_PAUSE;
        step_depth = g_cpu->cpu_get_depth();
    } else {
        switch (exec_mode) {
            case EXEC_MODE_PAUSE:
            case EXEC_MODE_RUN:
                break;
            case EXEC_MODE_STEP:
                exec_mode = EXEC_MODE_PAUSE;
                break;
            case EXEC_MODE_NEXT:
                if (g_cpu->cpu_get_depth() <= step_depth) {
                    exec_mode  = EXEC_MODE_PAUSE;
                    step_depth = g_cpu->cpu_get_depth();
                }
                break;
            case EXEC_MODE_TO_CALL:
                if (g_cpu->cpu_get_depth() > step_depth) {
                    exec_mode  = EXEC_MODE_PAUSE;
                    step_depth = g_cpu->cpu_get_depth();
                }
                break;
            case EXEC_MODE_TO_RET:
                if (g_cpu->cpu_get_depth() < step_depth) {
                    exec_mode  = EXEC_MODE_PAUSE;
                    step_depth = g_cpu->cpu_get_depth();
                }
                break;
        }
    }
}
void Tamago::tamalib_mainloop(void)
{
    timestamp_t ts;
    while (!hal_handler()) {
        tamalib_step();
        ts = hal_get_timestamp();
        if (ts - screen_ts >= g_ts_freq / DEFAULT_FRAMERATE) {
            screen_ts = ts;
            hal_update_screen();
        }
    }
}
void Tamago::hw_set_lcd_pin(u8_t seg, u8_t com, u8_t val)
{
    if (seg_pos[seg] < LCD_WIDTH) {
        matrix_buffer[com][seg_pos[seg]] = val;
    } else {
        if (seg == 8 && com < 4) {
            icon_buffer[com] = val;
        } else if (seg == 28 && com >= 12) {
            icon_buffer[com - 8] = val;
        }
    }
}
void Tamago::hw_set_button(button_t btn, btn_state_t state)
{
    pin_state_t pin_state = (state == BTN_STATE_PRESSED) ? PIN_STATE_LOW : PIN_STATE_HIGH;
    switch (btn) {
        case BTN_LEFT:
            g_cpu->cpu_set_input_pin(PIN_K02, pin_state);
            break;
        case BTN_MIDDLE:
            g_cpu->cpu_set_input_pin(PIN_K01, pin_state);
            break;
        case BTN_RIGHT:
            g_cpu->cpu_set_input_pin(PIN_K00, pin_state);
            break;
    }
}
bool_t Tamago::hw_init(void)
{
    g_cpu->cpu_set_input_pin(PIN_K00, PIN_STATE_HIGH);
    g_cpu->cpu_set_input_pin(PIN_K01, PIN_STATE_HIGH);
    g_cpu->cpu_set_input_pin(PIN_K02, PIN_STATE_HIGH);
    return 0;
}
int Tamago::init(int argc, char **argv)
{
    sdl_init();

    bool_t   res  = 0;
    uint64_t freq = 1000000;
    res |= g_cpu->cpu_init(g_program, NULL, freq);
    res |= hw_init();
    g_ts_freq = freq;

    tamalib_mainloop();

    sdl_release();
    return 0;
}