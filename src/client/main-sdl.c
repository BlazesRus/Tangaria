/*
 * File: main-sdl.c
 * Purpose: Angband SDL port
 *
 * Copyright (c) 2007 Ben Harrison, Gregory Velichansky, Eric Stevens,
 * Leon Marrick, Iain McFall, and others
 * Copyright (c) 2022 MAngband and PWMAngband Developers
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband licence":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */
#define USE_SDL2
#if defined(USE_SDL)||defined(USE_SDL2)

#include "c-angband.h"

#ifdef WINDOWS
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"
#else
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_image.h>
#endif

#ifdef USE_SDL2

#define MAX_SUBWINDOWS \
    ANGBAND_TERM_MAX
/* that should be plenty... */
#define MAX_WINDOWS 4
#define MAX_FONTS 128
#define MAX_BUTTONS 32

#define INIT_SDL_FLAGS \
    (SDL_INIT_VIDEO)
#define INIT_IMG_FLAGS \
    (IMG_INIT_PNG)

/* this is the main term screen, where all the action takes place */
#define MAIN_SUBWINDOW 0
/* for symmetry with main subwindow */
#define MAIN_WINDOW 0
/* size of the keypress queue (term->key_queue) */
#define SUBWINDOW_KEYQ_SIZE(subwindow_ptr) \
    ((subwindow_ptr)->index == MAIN_SUBWINDOW ? 1024 : 32)

#define DEFAULT_ATTR_BLANK \
    COLOUR_WHITE
#define DEFAULT_CHAR_BLANK ' '

#define DEFAULT_DISPLAY 0

#define DEFAULT_CONFIG_FILE "sdl2init.txt"
#define DEFAULT_CONFIG_FILE_DIR \
    ANGBAND_DIR_USER

#define DEFAULT_ALPHA_FULL 0xFF
#define ALPHA_PERCENT(p) \
    (DEFAULT_ALPHA_FULL * (p) / 100)
#define DEFAULT_ALPHA_LOW \
    ALPHA_PERCENT(80)
/* for "Alpha" button; in percents */
#define DEFAULT_ALPHA_STEP 10
#define DEFAULT_ALPHA_LOWEST 0

#define DEFAULT_WALLPAPER "att-128.png"
#define DEFAULT_WALLPAPER_DIR \
    ANGBAND_DIR_ICONS
#define DEFAULT_WINDOW_ICON "att-32.png"
#define DEFAULT_WINDOW_ICON_DIR \
    ANGBAND_DIR_ICONS
#define DEFAULT_ABOUT_ICON "att-128.png"
#define DEFAULT_ABOUT_ICON_DIR \
    ANGBAND_DIR_ICONS

#define DEFAULT_FONT_HINTING \
    TTF_HINTING_LIGHT
/* border of subwindows, in pixels */
#define DEFAULT_BORDER 8
#define DEFAULT_XTRA_BORDER \
    (DEFAULT_BORDER * 2)
#define DEFAULT_VISIBLE_BORDER 2

#define DEFAULT_FONT_SIZE 0
/* XXX hack: the widest character present in a font
 * for determining font advance (width) */
#define GLYPH_FOR_ADVANCE 'W'
#define DEFAULT_VECTOR_FONT_SIZE 12

#define DEFAULT_FONT "10x20x.fon"
#define DEFAULT_FONT_W 10
#define DEFAULT_FONT_H 20

#define DEFAULT_STATUS_BAR_FONT "8x13x.fon"
#define DEFAULT_STATUS_BAR_FONT_W 8
#define DEFAULT_STATUS_BAR_FONT_H 13

#define MAX_VECTOR_FONT_SIZE 36
#define MIN_VECTOR_FONT_SIZE 4

#define DEFAULT_BUTTON_BORDER 8
#define DEFAULT_LINE_HEIGHT(h) ((h) * 150 / 100)
#define DEFAULT_MENU_LINE_HEIGHT(h) ((h) * 200 / 100)
#define DEFAULT_MENU_LINE_WIDTH(w) \
    ((w) + DEFAULT_BUTTON_BORDER + DEFAULT_XTRA_BORDER)
/* update period in window delays (160 milliseconds, assuming 60 fps) */
#define DEFAULT_IDLE_UPDATE_PERIOD 10

#define DEFAULT_WINDOW_BG_COLOR \
    COLOUR_L_DARK
#define DEFAULT_SUBWINDOW_BG_COLOR \
    COLOUR_DARK
#define DEFAULT_SUBWINDOW_CURSOR_COLOR \
    COLOUR_YELLOW
#define DEFAULT_STATUS_BAR_BG_COLOR \
    COLOUR_DARK
#define DEFAULT_SHADE_COLOR \
    COLOUR_SHADE
#define DEFAULT_SUBWINDOW_BORDER_COLOR \
    COLOUR_SHADE
#define DEFAULT_STATUS_BAR_BUTTON_ACTIVE_COLOR \
    COLOUR_WHITE
#define DEFAULT_STATUS_BAR_BUTTON_INACTIVE_COLOR \
    COLOUR_L_DARK

#define DEFAULT_MENU_FG_ACTIVE_COLOR \
    COLOUR_WHITE
#define DEFAULT_MENU_FG_INACTIVE_COLOR \
    COLOUR_WHITE
#define DEFAULT_MENU_BG_ACTIVE_COLOR \
    COLOUR_SHADE
#define DEFAULT_MENU_BG_INACTIVE_COLOR \
    COLOUR_DARK

#define DEFAULT_MENU_TOGGLE_FG_ACTIVE_COLOR \
    COLOUR_WHITE
#define DEFAULT_MENU_TOGGLE_FG_INACTIVE_COLOR \
    COLOUR_L_DARK

#define DEFAULT_MENU_PANEL_OUTLINE_COLOR \
    COLOUR_SHADE

#define DEFAULT_ERROR_COLOR \
    COLOUR_RED

#define DEFAULT_ABOUT_BG_COLOR \
    COLOUR_SHADE
#define DEFAULT_ABOUT_BORDER_OUTER_COLOR \
    COLOUR_L_DARK
#define DEFAULT_ABOUT_BORDER_INNER_COLOR \
    COLOUR_WHITE
#define DEFAULT_ABOUT_TEXT_COLOR \
    COLOUR_WHITE

/* shockbolt's tiles are 64x64; dungeon is 198 tiles long;
 * 64 * 198 is 12672 which is bigger than any possible texture! */
#define REASONABLE_MAP_TILE_WIDTH 16
#define REASONABLE_MAP_TILE_HEIGHT 16

/* angband needs at least 80x24 main term, else severe bugs happen */
#define MIN_COLS_MAIN 80
#define MIN_ROWS_MAIN 24
/* some reasonable values - we dont want the player to resize
 * the term into nothing! */
#define MIN_COLS_OTHER 12
#define MIN_ROWS_OTHER 3

#define MIN_TILE_WIDTH 1
#define MAX_TILE_WIDTH 9
#define MIN_TILE_HEIGHT 1
#define MAX_TILE_HEIGHT 9

/* some random numbers */
#define DEFAULT_WINDOW_MINIMUM_W 198
#define DEFAULT_WINDOW_MINIMUM_H 66

#define DEFAULT_SNAP_RANGE \
    DEFAULT_FONT_W

#define CHECK_BUTTON_DATA_TYPE(button, data_type) \
    assert((button)->data.type == (data_type))
#else

#define MIN_SCREEN_WIDTH    640
#define MIN_SCREEN_HEIGHT   480

/* SDL flags used for the main window surface */
static Uint32 vflags = SDL_ANYFORMAT;

/* Current screen dimensions */
static int screen_w = 800;
static int screen_h = 600;

/* Fullscreen dimensions */
static int full_w;
static int full_h;

/* Want fullscreen? */
static bool fullscreen = false;

/* Want nice graphics? */
static bool nicegfx = false;

static int overdraw = 0;
static int overdraw_max = 0;

/* Default color */
static int d_color_r = 160;
static int d_color_g = 160;
static int d_color_b = 60;

/*
 * Status bar color:
 *   0 = default color
 *   COLOUR_WHITE to COLOUR_DEEP_L_BLUE = colored status bar
 */
static int statusbar_color = 0;

static char *sdl_settings_file;

/*
 * Used as 'system' font
 */
static const char *DEFAULT_FONT_FILE = "6x10x.fon";

#define MAX_FONTS 60
char *FontList[MAX_FONTS];
static int num_fonts = 0;

/* Default font size */
#define DEFAULT_FONT_SIZE    12
#endif

#ifdef USE_SDL2

enum wallpaper_mode {
    /* so that we won't forget to actually set wallpaper */
    WALLPAPER_INVALID = 0,
    WALLPAPER_DONT_SHOW,
    WALLPAPER_TILED,
    WALLPAPER_CENTERED,
    WALLPAPER_SCALED
};

enum button_data_type {
    BUTTON_DATA_INVALID = 0,
    BUTTON_DATA_NONE,
    BUTTON_DATA_INT,
    BUTTON_DATA_UNSIGNED,
    BUTTON_DATA_SUBWINDOW,
    BUTTON_DATA_FONT,
    BUTTON_DATA_TERM_FLAG,
    BUTTON_DATA_ALPHA
};

enum button_movesize {
    BUTTON_MOVESIZE_INVALID = 0,
    BUTTON_MOVESIZE_MOVING,
    BUTTON_MOVESIZE_SIZING
};

enum button_tile_scale {
    BUTTON_TILE_SIZE_INVALID = 0,
    BUTTON_TILE_SIZE_WIDTH,
    BUTTON_TILE_SIZE_HEIGHT
};

enum button_volume {
    BUTTON_VOLUME_INVALID = 0,
    BUTTON_VOLUME_SOUND,
    BUTTON_VOLUME_MUSIC
};

enum button_caption_position {
    CAPTION_POSITION_INVALID = 0,
    CAPTION_POSITION_CENTER,
    CAPTION_POSITION_LEFT,
    CAPTION_POSITION_RIGHT
};

enum font_type {
    FONT_TYPE_INVALID = 0,
    FONT_TYPE_RASTER,
    FONT_TYPE_VECTOR
};

struct ttf {
    TTF_Font *handle;
    struct { int w; int h; } glyph;
};

/* the array of ascii chars was generated by this perl script:
 * my $ascii = join '', map /\p{Print}/ ? $_ : ' ', map chr, 0 .. 127; 
 * for (my $i = 0; $i < length($ascii); $i += 32) {
 *     my $s = substr $ascii, $i, 32;
 *     $s =~ s#\\#\\\\#g;
 *     $s =~ s#"#\\"#g;
 *     print qq(\t"$s"\n);
 * }
 * and then manually hacked to substitute the escape codes for the non-null
 * control characters (0x01 to 0x1f) so the graphics characters at those
 * indices in Angband's packaged .fon files remain available, if the user
 * wants to access them (those characters are likely the ones specified by
 * code page 437, https://en.wikipedia.org/wiki/Code_page_437 ).
 */
static const char g_ascii_codepoints_for_cache[] =
    " \x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f"
    "\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f"
    " !\"#$%&'()*+,-./0123456789:;<=>?"
    "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
    "`abcdefghijklmnopqrstuvwxyz{|}~ ";
/* Simple font cache. Only for ascii (which is like 99.99% (?) of what the game
 * displays, anyway) */
#define ASCII_CACHE_SIZE \
        (N_ELEMENTS(g_ascii_codepoints_for_cache) - 1)
struct font_cache {
    SDL_Texture *texture;
    /* it wastes some space... so what? */
    SDL_Rect rects[ASCII_CACHE_SIZE];
};
/* 0 is also a valid codepoint, of course... that's just for finding bugs */
#define IS_CACHED_ASCII_CODEPOINT(c) \
        ((c) > 0 && (c) < ASCII_CACHE_SIZE)

struct font {
    struct ttf ttf;
    char *name;
    char *path;
    int size;
    /* index of font in g_font_info array */
    size_t index;

    struct font_cache cache;
};

struct subwindow_border {
    bool visible;
    bool error;
    SDL_Color color;
    int width;
};

struct subwindow_config {
    char *font_name;
    int font_size;
};

struct window_config {
    int renderer_flags;
    int renderer_index;
    int window_flags;

    char *wallpaper_path;
    char *font_name;
    int font_size;
};

/* struct subwindow is representation of angband's term */
struct subwindow {
    bool inited;
    bool loaded;
    bool linked;
    bool visible;

    struct subwindow_config *config;

    /* top in z-order */
    bool top;
    bool always_top;

    unsigned index;

    int rows;
    int cols;

    /* struct ttf also has this information; these members are
     * just for convinience */
    int font_width;
    int font_height;

    /* coordinates of full rect are relative to coordinates of window
     * (basically, full rect is texture) */
    SDL_Rect full_rect;
    /* coordinates of inner rect are relative to that of full rect */
    SDL_Rect inner_rect;
    /* for use when resizing term */
    SDL_Rect sizing_rect;
    /* a one pixel texture, mostly for displaying something when
     * the player is resizing term */
    SDL_Texture *aux_texture;

    /* background color */
    SDL_Color color;

    struct subwindow_border borders;

    SDL_Texture *texture;

    struct font *font;

    struct window *window;
    struct _term *term;
};

struct button;

struct button_bank {
    struct button *buttons;
    size_t size;
    size_t number;
};

struct menu_panel {
    SDL_Rect rect;
    struct button_bank button_bank;
    struct menu_panel *next;
};

typedef void (*button_render)(const struct window *window,
        struct button *button);
typedef bool (*button_event)(struct window *window,
        struct button *button, const SDL_Event *event);
typedef void (*button_menu)(struct window *window,
        struct button *button, const SDL_Event *event,
        struct menu_panel *menu_panel);

struct font_value {
    struct subwindow *subwindow;
    /* index of font in g_font_info array */
    size_t index;
    bool size_ok;
};

struct term_flag_value {
    struct subwindow *subwindow;
    uint32_t flag;
};

struct alpha_value {
    struct subwindow *subwindow;
    int real_value;
    int show_value;
};

struct button_data {
    enum button_data_type type;
    union {
        int int_value;
        unsigned unsigned_value;
        struct subwindow *subwindow_value;
        struct font_value font_value;
        struct term_flag_value term_flag_value;
        struct alpha_value alpha_value;
    } value;
};

struct menu_elem {
    const char *caption;
    struct button_data data;
    button_render on_render;
    button_menu on_menu;
};

struct button_callbacks {
    /* this function should render the button;
     * otherwise, the button will be invisible */
    button_render on_render;
    /* event handler for status bar buttons */
    button_event on_event;
    /* event handler for buttons in "Menu" */
    button_menu on_menu;
};

struct button {
    /* selected means the user pointed at button and
     * pressed mouse button (but not released yet) */
    bool selected;
    /* highlighted means the user pointed
     * at button but not clicking yet */
    bool highlighted;
    /* At least, that was the theory; buttons in "Menu" work a bit differently,
     * some are selected when the user points at button, all are highlighted
     * regardless of what event happened on them (click or mouse motion)... */

    char *caption;

    SDL_Rect full_rect;
    SDL_Rect inner_rect;

    struct button_data data;
    struct button_callbacks callbacks;
};

struct status_bar {
    struct font *font;

    struct button_bank button_bank;
    struct menu_panel *menu_panel;

    struct window *window;

    SDL_Rect full_rect;
    SDL_Rect inner_rect;
    SDL_Color color;
    SDL_Texture *texture;

    bool in_menu;
};

struct graphics {
    SDL_Texture *texture;
    int id;
    int tile_pixel_w;
    int tile_pixel_h;

    int overdraw_row;
    int overdraw_max;
};

/* thats for dragging terms */
struct move_state {
    bool active;
    bool moving;

    int originx;
    int originy;

    struct subwindow *subwindow;
};

/* thats for resizing terms */
struct size_state {
    bool active;
    bool sizing;

    int originx;
    int originy;

    bool left;
    bool top;

    struct subwindow *subwindow;
};

struct wallpaper {
    int w;
    int h;
    SDL_Texture *texture;
    enum wallpaper_mode mode;
};

/* struct window is a real window on screen, it has one or more
 * subwindows (terms) in it */
struct window {
    bool inited;
    bool loaded;

    /* id is SDL's id, for use with events */
    Uint32 id;
    /* and this is our id, mostly for debugging */
    unsigned index;

    struct window_config *config;

    /* does window have mouse focus? */
    bool focus;
    /* window has changed and must be redrawn */
    bool dirty;

    /* limiter for frames */
    Uint32 next_redraw;
    /* from display mode */
    int delay;

    /* as reported by SDL_GetWindowFlags() */
    Uint32 flags;

    /* position and size of window as it is on display */
    SDL_Rect full_rect;
    /* size of window without status bar, basically */
    SDL_Rect inner_rect;

    SDL_Color color;
    /* for making terms transparent while moving or sizing them */
    Uint8 alpha;

    SDL_Window *window;
    SDL_Renderer *renderer;

    int pixelformat;

    struct wallpaper wallpaper;
    struct move_state move_state;
    struct size_state size_state;
    struct status_bar status_bar;
    struct graphics graphics;

    struct subwindow *subwindows[MAX_SUBWINDOWS];
};

struct font_info {
    char *name;
    char *path;
    int size;
    size_t index;
    enum font_type type;
    bool loaded;
};

const char help_sdl2[] = "SDL2 frontend";
static SDL_Color g_colors[MAX_COLORS];
static struct font_info g_font_info[MAX_FONTS];
/* these arrays contain windows and terms that the ui operates on */
static struct subwindow g_subwindows[MAX_SUBWINDOWS];
static struct window g_windows[MAX_WINDOWS];
/* True if KC_MOD_KEYPAD will be sent for numeric keypad keys at the expense
 * of not handling some keyboard layouts properly. */
static int g_kp_as_mod = 1;

/* term_view_map_hook */
static int view_map_hook_mod = 0;

/* handle_menu_font_names_page */
static bool font_page = false;

/* Forward declarations */

static void init_globals(void);
static void free_globals(void);
static bool read_config_file(void);
static void dump_config_file(void);
static void init_colors(void);
static void start_windows(void);
static void start_window(struct window *window);
static void load_font(struct font *font);
static bool reload_font(struct subwindow *subwindow,
        const struct font_info *info);
static void free_font(struct font *font);
static const struct font_info *find_font_info(const char *name);
static void get_string_metrics(struct font *font, const char *str, int *w, int *h);
static struct window *get_new_window(unsigned index);
static void wipe_window(struct window *window, int display);
/* create default config for spawning a window via gui */
static void wipe_window_aux_config(struct window *window);
static void adjust_window_geometry(struct window *window);
static void free_window(struct window *window);
static struct window *get_window_by_id(Uint32 id);
static struct window *get_window_direct(unsigned index);
static bool has_visible_subwindow(const struct window *window, unsigned index);
static void resize_window(struct window *window, int w, int h);
static struct subwindow *get_new_subwindow(unsigned index);
static void load_subwindow(struct window *window, struct subwindow *subwindow);
static bool is_subwindow_loaded(unsigned index);
static struct subwindow *transfer_subwindow(struct window *window, unsigned index);
static struct subwindow *get_subwindow_by_xy(const struct window *window, int x, int y);
static struct subwindow *get_subwindow_by_index(const struct window *window,
        unsigned index, bool visible);
static struct subwindow *get_subwindow_direct(unsigned index);
/* this function loads new subwindow if it's not already loaded */
static struct subwindow *make_subwindow(struct window *window, unsigned index);
static void sort_to_top(struct window *window);
static void bring_to_top(struct window *window, struct subwindow *subwindow);
static void render_borders(struct subwindow *subwindow);
static SDL_Texture *load_image(const struct window *window, const char *path);
static void reload_all_graphics(graphics_mode *mode);
static void free_graphics(struct graphics *graphics);
static void load_terms(void);
static void load_term(struct subwindow *subwindow);
static void clear_pw_flag(struct subwindow *subwindow);
static bool adjust_subwindow_geometry(const struct window *window,
        struct subwindow *subwindow);
static bool is_ok_col_row(const struct subwindow *subwindow,
        const SDL_Rect *rect, int cell_w, int cell_h);
static void resize_rect(SDL_Rect *rect,
        int left, int top, int right, int bottom);
static void crop_rects(SDL_Rect *src, SDL_Rect *dst);
static bool is_point_in_rect(int x, int y, const SDL_Rect *rect);
static bool is_close_to(int a, int b, unsigned range);
static bool is_over_status_bar(const struct status_bar *status_bar, int x, int y);
static void make_button_bank(struct button_bank *bank);
static void free_button_bank(struct button_bank *button_bank);
static void free_menu_panel(struct menu_panel *menu_panel);
static struct menu_panel *get_menu_panel_by_xy(struct menu_panel *menu_panel,
        int x, int y);
static void refresh_angband_terms(void);
static void handle_quit(void);
static void wait_anykey(void);

/* Functions */

static void render_clear(const struct window *window,
        SDL_Texture *texture, const SDL_Color *color)
{
    SDL_SetRenderTarget(window->renderer, texture);
    SDL_SetRenderDrawColor(window->renderer,
            color->r, color->g, color->b, color->a);
    SDL_RenderClear(window->renderer);
}

static void render_wallpaper_tiled(const struct window *window)
{
    SDL_SetRenderTarget(window->renderer, NULL);

    SDL_Rect rect = {0, 0, window->wallpaper.w, window->wallpaper.h};
    for (rect.y = window->inner_rect.y;
            rect.y < window->inner_rect.h;
            rect.y += rect.h)
    {
        for (rect.x = window->inner_rect.x;
                rect.x < window->inner_rect.w;
                rect.x += rect.w)
        {
            SDL_RenderCopy(window->renderer, window->wallpaper.texture, NULL, &rect);
        }
    }
}

static void render_wallpaper_scaled(const struct window *window)
{
    SDL_SetRenderTarget(window->renderer, NULL);
    SDL_RenderCopy(window->renderer, window->wallpaper.texture, NULL, NULL);
}

static void render_wallpaper_centered(const struct window *window)
{
    SDL_Rect rect;

    rect.w = window->wallpaper.w;
    rect.h = window->wallpaper.h;
    rect.x = window->inner_rect.x + (window->inner_rect.w - rect.w) / 2;
    rect.y = window->inner_rect.y + (window->inner_rect.h - rect.h) / 2;

    SDL_SetRenderTarget(window->renderer, NULL);
    SDL_RenderCopy(window->renderer, window->wallpaper.texture, NULL, &rect);
}

static void render_background(const struct window *window)
{
    render_clear(window, NULL, &window->color);

    switch (window->wallpaper.mode) {
        case WALLPAPER_DONT_SHOW:
            return;
        case WALLPAPER_TILED:
            render_wallpaper_tiled(window);
            return;
        case WALLPAPER_CENTERED:
            render_wallpaper_centered(window);
            return;
        case WALLPAPER_SCALED:
            render_wallpaper_scaled(window);
            return;
        default:
            quit_fmt("bad wallpaper mode %d in window %u",
                    window->wallpaper.mode, window->index);
            break;
    }
}

static void render_all(const struct window *window)
{
    render_background(window);

    SDL_RenderCopy(window->renderer,
            window->status_bar.texture, NULL, &window->status_bar.full_rect);

    for (size_t i = 0; i < N_ELEMENTS(window->subwindows); i++) {
        struct subwindow *subwindow = window->subwindows[i];
        if (subwindow != NULL && subwindow->visible) {
            SDL_RenderCopy(window->renderer,
                    subwindow->texture,
                    NULL, &subwindow->full_rect);
        }
    }
}

static void render_status_bar(const struct window *window)
{
    render_clear(window, window->status_bar.texture, &window->status_bar.color);

    for (size_t i = 0; i < window->status_bar.button_bank.number; i++) {
        struct button *button = &window->status_bar.button_bank.buttons[i];
        if (button->callbacks.on_render != NULL) {
            button->callbacks.on_render(window, button);
        }
    }
}

static void render_outline_rect(const struct window *window,
        SDL_Texture *texture, const SDL_Rect *rect, const SDL_Color *color)
{
    SDL_SetRenderTarget(window->renderer, texture);
    SDL_SetRenderDrawColor(window->renderer,
            color->r, color->g, color->b, color->a);
    SDL_RenderDrawRect(window->renderer, rect);
}

static void render_outline_rect_width(const struct window *window,
        SDL_Texture *texture, const SDL_Rect *rect, const SDL_Color *color, int width)
{
    SDL_Rect dst = *rect;

    for (int i = 0; i < width; i++) {
        render_outline_rect(window, texture, &dst, color);
        resize_rect(&dst, 1, 1, -1, -1);
    }
}

static void render_fill_rect(const struct window *window,
        SDL_Texture *texture, const SDL_Rect *rect, const SDL_Color *color)
{
    SDL_SetRenderTarget(window->renderer, texture);
    SDL_SetRenderDrawColor(window->renderer,
            color->r, color->g, color->b, color->a);
    SDL_RenderFillRect(window->renderer, rect);
}

static void render_window_in_menu(const struct window *window)
{
    render_background(window);

    SDL_SetRenderTarget(window->renderer, NULL);

    for (size_t i = 0; i < N_ELEMENTS(window->subwindows); i++) {
        struct subwindow *subwindow = window->subwindows[i];
        if (subwindow != NULL && subwindow->visible) {
            if (subwindow->sizing_rect.w > 0 && subwindow->sizing_rect.h > 0) {
                SDL_SetRenderTarget(window->renderer, subwindow->aux_texture);
                /* in case subwindow's color changed */
                render_fill_rect(window,
                        subwindow->aux_texture, NULL, &subwindow->color);

                SDL_SetRenderTarget(window->renderer, NULL);
                SDL_RenderCopy(window->renderer,
                        subwindow->aux_texture, NULL, &subwindow->sizing_rect);
            }

            SDL_RenderCopy(window->renderer,
                    subwindow->texture,
                    NULL, &subwindow->full_rect);
        }
    }

    /* render it last to allow the menu to draw over subwindows */
    render_status_bar(window);
    SDL_SetRenderTarget(window->renderer, NULL);
    SDL_RenderCopy(window->renderer,
            window->status_bar.texture, NULL, &window->status_bar.full_rect);
}

static void set_subwindow_alpha(struct subwindow *subwindow, int alpha)
{
    SDL_SetTextureAlphaMod(subwindow->texture, alpha);
    SDL_SetTextureAlphaMod(subwindow->aux_texture, alpha);
}

static void set_subwindows_alpha(const struct window *window, int alpha)
{
    for (size_t i = 0; i < N_ELEMENTS(window->subwindows); i++) {
        struct subwindow *subwindow = window->subwindows[i];
        if (subwindow != NULL) {
            set_subwindow_alpha(subwindow, alpha);
        }
    }
}

/* this function allows to perform special things that are not
 * needed while playing the game, like moving terms */
static void redraw_window_in_menu(struct window *window)
{
    set_subwindows_alpha(window, window->alpha);
    render_window_in_menu(window);
    SDL_RenderPresent(window->renderer);
    window->next_redraw = SDL_GetTicks() + window->delay;
}

/* this function is mostly used while normally playing the game */
static void redraw_window(struct window *window)
{
    if (window->status_bar.in_menu) {
        /* we called (perhaps via refresh_angband_terms()) Term_fresh() in menu */
        redraw_window_in_menu(window);
        return;
    }

    /* XXX XXX dont forget to prerender status bar in loader! */
    render_all(window);
    SDL_RenderPresent(window->renderer);
    window->next_redraw = SDL_GetTicks() + window->delay;
}

static void try_redraw_window(struct window *window)
{
    if (window->next_redraw < SDL_GetTicks()) {
        redraw_window(window);
    }
}

static void redraw_all_windows(bool dirty)
{
    for (unsigned i = 0; i < MAX_WINDOWS; i++) {
        struct window *window = get_window_direct(i);
        if (window != NULL && (dirty ? window->dirty : true)) {
            render_status_bar(window);
            redraw_window(window);
            window->dirty = false;
        }
    }
}

static void render_utf8_string(const struct window *window,
        const struct font *font, SDL_Texture *dst_texture,
        SDL_Color fg, SDL_Rect rect, const char *utf8_string)
{
    SDL_Surface *surface = TTF_RenderUTF8_Blended(font->ttf.handle, utf8_string, fg);
    SDL_Texture *src_texture = SDL_CreateTextureFromSurface(window->renderer, surface);
    SDL_FreeSurface(surface);

    SDL_SetRenderTarget(window->renderer, dst_texture);
    SDL_RenderCopy(window->renderer, src_texture, NULL, &rect);

    SDL_DestroyTexture(src_texture);
}

/* this function is typically called in a loop, so for efficiency it doesnt
 * SetRenderTarget; caller must do it (but it does SetTextureColorMod) */
static void render_glyph_mono(const struct window *window,
        const struct font *font, SDL_Texture *dst_texture,
        int x, int y, const SDL_Color *fg, uint32_t codepoint)
{
    if (codepoint == DEFAULT_CHAR_BLANK) {
        return;
    }

    SDL_Rect dst = {x, y, font->ttf.glyph.w, font->ttf.glyph.h};

    if (IS_CACHED_ASCII_CODEPOINT(codepoint)) {
        SDL_Rect src = font->cache.rects[codepoint];

        crop_rects(&src, &dst);

        SDL_SetTextureColorMod(font->cache.texture, fg->r, fg->g, fg->b);

        SDL_RenderCopy(window->renderer,
                font->cache.texture, &font->cache.rects[codepoint], &dst);
    } else {
        SDL_Surface *surface = TTF_RenderGlyph_Blended(font->ttf.handle,
                (Uint16) codepoint, *fg);
        if (surface == NULL) {
            return;
        }

        SDL_Texture *texture = SDL_CreateTextureFromSurface(window->renderer, surface);
        assert(texture != NULL);

        SDL_Rect src = {0, 0, surface->w, surface->h};

        crop_rects(&src, &dst);

        SDL_RenderCopy(window->renderer, texture, &src, &dst);

        SDL_FreeSurface(surface);
        SDL_DestroyTexture(texture);
    }
}

static void render_cursor(struct subwindow *subwindow, 
        int col, int row, bool big)
{
    SDL_Color color = g_colors[DEFAULT_SUBWINDOW_CURSOR_COLOR];
    SDL_Rect rect = {
        subwindow->inner_rect.x + subwindow->font_width * col,
        subwindow->inner_rect.y + subwindow->font_height * row,
        subwindow->font_width * (big ? tile_width : 1),
        subwindow->font_height * (big ? tile_height : 1)
    };

    render_outline_rect(subwindow->window, subwindow->texture,
            &rect, &color);
}

static void render_grid_cell_text(const struct subwindow *subwindow,
        SDL_Texture *texture, int x, int y)
{
//    struct grid_data grid_data;
    int a;
    int ta;
    wchar_t c;
    wchar_t tc;

    /* map_info(); */
    a = player->scr_info[y][x].a;
    c = player->scr_info[y][x].c;
    ta = player->trn_info[y][x].a;
    tc = player->trn_info[y][x].c;

//    map_info(loc(x, y), &grid_data);
//    grid_data_as_text(&grid_data, &a, &c, &ta, &tc);
    /* apparently either the same as a or obscured by a */
    (void) tc;

    SDL_Color fg = g_colors[a % MAX_COLORS];
    SDL_Color bg;

    switch (ta / MAX_COLORS) {
        case BG_BLACK:
            bg = subwindow->color;
            break;
        case BG_SAME:
            bg = fg;
            break;
        case BG_DARK:
            bg = g_colors[DEFAULT_SHADE_COLOR];
            break;
        default:
            /* debugging */
            bg = g_colors[DEFAULT_ERROR_COLOR];
    }

    SDL_Rect rect = {
        x * subwindow->font_width,
        y * subwindow->font_height,
        subwindow->font_width,
        subwindow->font_height
    };

    render_fill_rect(subwindow->window, texture, &rect, &bg);
    render_glyph_mono(subwindow->window,
            subwindow->font, texture, rect.x, rect.y, &fg, (uint32_t) c);
}

/* does not SetRenderTarget */
static void render_tile_rect_scaled(const struct subwindow *subwindow,
        int col, int row, SDL_Rect dst, int a, int c)
{
    struct graphics *graphics = &subwindow->window->graphics;

    SDL_Rect src = {0, 0, graphics->tile_pixel_w, graphics->tile_pixel_h};

    int src_row = a & 0x7f;
    int src_col = c & 0x7f;

    src.x = src_col * src.w;
    src.y = src_row * src.h;

    if (graphics->overdraw_row != 0
            && src_row >= graphics->overdraw_row
            && src_row <= graphics->overdraw_max)
    {
        src.y -= src.h;
        dst.y -= dst.h;
        dst.h *= 2;
        src.h *= 2;
    }

    SDL_RenderCopy(subwindow->window->renderer,
            graphics->texture, &src, &dst);
}

static void render_tile_font_scaled(const struct subwindow *subwindow,
        int col, int row, int a, int c, bool fill)
{
    struct graphics *graphics = &subwindow->window->graphics;

    SDL_Rect dst = {
        subwindow->inner_rect.x + col * subwindow->font_width,
        subwindow->inner_rect.y + row * subwindow->font_height,
        subwindow->font_width * (!Term->minimap_active ? tile_width : 1),
        subwindow->font_height * (!Term->minimap_active ? tile_height : 1)
    };

    if (fill) {
        render_fill_rect(subwindow->window, subwindow->texture, &dst, &subwindow->color);
    }

    SDL_Rect src = {0, 0, graphics->tile_pixel_w, graphics->tile_pixel_h};

    SDL_SetRenderTarget(subwindow->window->renderer, subwindow->texture);

    int src_row = a & 0x7f;
    int src_col = c & 0x7f;

    src.x = src_col * src.w;
    src.y = src_row * src.h;

    if (graphics->overdraw_row != 0
            && row > 2
            && src_row >= graphics->overdraw_row
            && src_row <= graphics->overdraw_max)
    {
        src.y -= src.h;
        dst.y -= dst.h;
        dst.h *= 2;
        src.h *= 2;

        SDL_RenderCopy(subwindow->window->renderer,
                graphics->texture, &src, &dst);

        Term_mark(col, row - tile_height);
        Term_mark(col, row);
    } else {
        SDL_RenderCopy(subwindow->window->renderer,
                graphics->texture, &src, &dst);
    }
}

static void render_grid_cell_tile(const struct subwindow *subwindow,
        SDL_Texture *texture, SDL_Rect tile, int x, int y)
{
//    struct grid_data grid_data;
    int a;
    int ta;
    wchar_t c;
    wchar_t tc;

    /* map_info(); */
    a = player->scr_info[y][x].a;
    c = player->scr_info[y][x].c;
    ta = player->trn_info[y][x].a;
    tc = player->trn_info[y][x].c;

//    map_info(loc(x, y), &grid_data);
//    grid_data_as_text(&grid_data, &a, &c, &ta, &tc);

    SDL_SetRenderTarget(subwindow->window->renderer, texture);

    render_tile_rect_scaled(subwindow, x, y, tile, ta, tc);

    if (a == ta && c == tc) {
        return;
    }

    render_tile_rect_scaled(subwindow, x, y, tile, a, c);
}

static void clear_all_borders(struct window *window)
{
    for (size_t i = 0; i < N_ELEMENTS(window->subwindows); i++) {
        struct subwindow *subwindow = window->subwindows[i];
        if (subwindow != NULL) {
            subwindow->borders.error = false;
            render_borders(subwindow);
        }
    }
}

static void render_borders(struct subwindow *subwindow)
{
    SDL_Rect rect = {0};
    SDL_QueryTexture(subwindow->texture, NULL, NULL, &rect.w, &rect.h);

    SDL_Color *color;
    if (!subwindow->borders.error) {
        if (subwindow->borders.visible) {
            color = &subwindow->borders.color;
        } else {
            color = &subwindow->color;
        }
    } else {
        color = &g_colors[DEFAULT_ERROR_COLOR];
    }

    render_outline_rect_width(subwindow->window,
            subwindow->texture, &rect, color,
            subwindow->borders.width);
}

static SDL_Texture *make_subwindow_texture(const struct window *window, int w, int h)
{
    SDL_Texture *texture = SDL_CreateTexture(window->renderer,
            window->pixelformat, SDL_TEXTUREACCESS_TARGET, w, h);
    if (texture == NULL) {
        quit_fmt("cannot create texture for subwindow in window %u: %s",
                window->index, SDL_GetError());
    }

    if (SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND) != 0) {
        SDL_DestroyTexture(texture);
        quit_fmt("cannot set blend mode for texture in window %u: %s",
                window->index, SDL_GetError());
    }

    return texture;
}

static void render_menu_panel(const struct window *window, struct menu_panel *menu_panel)
{
    if (menu_panel == NULL) {
        return;
    }

    for (size_t i = 0; i < menu_panel->button_bank.number; i++) {
        struct button *button = &menu_panel->button_bank.buttons[i];

        assert(button->callbacks.on_render != NULL);
        button->callbacks.on_render(window, button);
    }
    render_outline_rect(window,
            NULL, &menu_panel->rect, &g_colors[DEFAULT_MENU_PANEL_OUTLINE_COLOR]);

    /* recurse */
    render_menu_panel(window, menu_panel->next);
}

static SDL_Rect get_button_caption_rect(const struct button *button)
{
    SDL_Rect rect = {
        button->full_rect.x + button->inner_rect.x,
        button->full_rect.y + button->inner_rect.y,
        button->inner_rect.w,
        button->inner_rect.h
    };

    return rect;
}

static void render_button_menu(const struct window *window,
        struct button *button, const SDL_Color *fg, const SDL_Color *bg)
{
    SDL_Rect rect = get_button_caption_rect(button);

    render_fill_rect(window,
            NULL, &button->full_rect, bg);
    render_utf8_string(window, window->status_bar.font, NULL, 
            *fg, rect, button->caption);
}

static void render_button_menu_toggle(const struct window *window,
        struct button *button, bool active)
{
    SDL_Color *bg;
    SDL_Color *fg;

    if (active) {
        fg = &g_colors[DEFAULT_MENU_TOGGLE_FG_ACTIVE_COLOR];
    } else {
        fg = &g_colors[DEFAULT_MENU_TOGGLE_FG_INACTIVE_COLOR];
    }
    if (button->highlighted) {
        bg = &g_colors[DEFAULT_MENU_BG_ACTIVE_COLOR];
    } else {
        bg = &g_colors[DEFAULT_MENU_BG_INACTIVE_COLOR];
    }

    render_button_menu(window, button, fg, bg);
}

static void render_button_menu_simple(const struct window *window, struct button *button)
{
    SDL_Color *fg;
    SDL_Color *bg;

    if (button->highlighted) {
        fg = &g_colors[DEFAULT_MENU_FG_ACTIVE_COLOR];
        bg = &g_colors[DEFAULT_MENU_BG_ACTIVE_COLOR];
    } else {
        fg = &g_colors[DEFAULT_MENU_FG_INACTIVE_COLOR];
        bg = &g_colors[DEFAULT_MENU_BG_INACTIVE_COLOR];
    }

    render_button_menu(window, button, fg, bg);
}

static void render_button_menu_pw(const struct window *window, struct button *button)
{
    CHECK_BUTTON_DATA_TYPE(button, BUTTON_DATA_TERM_FLAG);

    struct subwindow *subwindow = button->data.value.term_flag_value.subwindow;
    uint32_t flag = button->data.value.term_flag_value.flag;

    assert(subwindow->index != MAIN_SUBWINDOW);
    assert(subwindow->index < N_ELEMENTS(window_flag));

    render_button_menu_toggle(window,
            button, window_flag[subwindow->index] & flag);
}

static void render_button_menu_terms(const struct window *window, struct button *button)
{
    CHECK_BUTTON_DATA_TYPE(button, BUTTON_DATA_SUBWINDOW);

    if (button->highlighted) {
        /* draw a border around subwindow, so that it would be easy to see
         * which subwindow corresponds to that button */
        struct subwindow *subwindow = button->data.value.subwindow_value;
        int outline_width = (subwindow->full_rect.w - subwindow->inner_rect.w) / 2
                - subwindow->borders.width;
        SDL_Rect outline_rect = subwindow->full_rect;
        resize_rect(&outline_rect,
                subwindow->borders.width, subwindow->borders.width,
                -subwindow->borders.width, -subwindow->borders.width);
        render_outline_rect_width(window,
                NULL,
                &outline_rect,
                &g_colors[DEFAULT_SUBWINDOW_BORDER_COLOR],
                outline_width);
    }

    render_button_menu_simple(window, button);
}

static void render_button_menu_borders(const struct window *window, struct button *button)
{
    CHECK_BUTTON_DATA_TYPE(button, BUTTON_DATA_SUBWINDOW);

    struct subwindow *subwindow = button->data.value.subwindow_value;

    render_button_menu_toggle(window, button, subwindow->borders.visible);
}

static void render_button_menu_alpha(const struct window *window, struct button *button)
{
    CHECK_BUTTON_DATA_TYPE(button, BUTTON_DATA_ALPHA);

    struct subwindow *subwindow = button->data.value.alpha_value.subwindow;
    int alpha = button->data.value.alpha_value.real_value;

    SDL_Color fg;
    SDL_Color *bg;

    if (is_close_to(alpha, subwindow->color.a, DEFAULT_ALPHA_STEP / 2)) {
        fg = g_colors[DEFAULT_MENU_TOGGLE_FG_ACTIVE_COLOR];
    } else {
        fg = g_colors[DEFAULT_MENU_TOGGLE_FG_INACTIVE_COLOR];
    }
    if (button->highlighted) {
        bg = &g_colors[DEFAULT_MENU_BG_ACTIVE_COLOR];
    } else {
        bg = &g_colors[DEFAULT_MENU_BG_INACTIVE_COLOR];
    }

    SDL_Rect rect = get_button_caption_rect(button);

    render_fill_rect(window,
            NULL, &button->full_rect, bg);
    render_utf8_string(window, window->status_bar.font, NULL, 
            fg, rect, format(button->caption, button->data.value.alpha_value.show_value));
}

static void render_button_menu_top(const struct window *window, struct button *button)
{
    CHECK_BUTTON_DATA_TYPE(button, BUTTON_DATA_SUBWINDOW);

    struct subwindow *subwindow = button->data.value.subwindow_value;

    render_button_menu_toggle(window, button, subwindow->always_top);
}

static void render_button_menu_tile_size(const struct window *window,
        struct button *button)
{
    CHECK_BUTTON_DATA_TYPE(button, BUTTON_DATA_INT);

    assert(button->data.value.int_value == BUTTON_TILE_SIZE_WIDTH
            || button->data.value.int_value == BUTTON_TILE_SIZE_HEIGHT);

    SDL_Color fg;
    SDL_Color *bg;

    if (window->graphics.id != GRAPHICS_NONE) {
        fg = g_colors[DEFAULT_MENU_TOGGLE_FG_ACTIVE_COLOR];
    } else {
        fg = g_colors[DEFAULT_MENU_TOGGLE_FG_INACTIVE_COLOR];
    }
    if (button->highlighted) {
        bg = &g_colors[DEFAULT_MENU_BG_ACTIVE_COLOR];
    } else {
        bg = &g_colors[DEFAULT_MENU_BG_INACTIVE_COLOR];
    }

    SDL_Rect rect = get_button_caption_rect(button);

    int scale = 0;
    if (button->data.value.int_value == BUTTON_TILE_SIZE_WIDTH) {
        scale = tile_width;
    } else if (button->data.value.int_value == BUTTON_TILE_SIZE_HEIGHT) {
        scale = tile_height;
    }

    render_fill_rect(window,
            NULL, &button->full_rect, bg);
    render_utf8_string(window, window->status_bar.font, NULL, 
            fg, rect, format(button->caption, scale));
}

static void render_button_menu_tile_set(const struct window *window, struct button *button)
{
    CHECK_BUTTON_DATA_TYPE(button, BUTTON_DATA_INT);

    render_button_menu_toggle(window,
            button, button->data.value.int_value == current_graphics_mode->grafID);
}

static void render_button_menu_tile_sets(const struct window *window, struct button *button)
{
    CHECK_BUTTON_DATA_TYPE(button, BUTTON_DATA_SUBWINDOW);

    render_button_menu_toggle(window, button, !Setup.initialized);
}

static void render_button_menu_font_size(const struct window *window,
        struct button *button)
{
    CHECK_BUTTON_DATA_TYPE(button, BUTTON_DATA_FONT);

    SDL_Color fg;
    SDL_Color *bg;

    struct font_value font_value = button->data.value.font_value;

    if (!font_value.size_ok) {
        fg = g_colors[DEFAULT_ERROR_COLOR];
    } else if (g_font_info[font_value.index].type == FONT_TYPE_VECTOR) {
        fg = g_colors[DEFAULT_MENU_TOGGLE_FG_ACTIVE_COLOR];
    } else {
        fg = g_colors[DEFAULT_MENU_TOGGLE_FG_INACTIVE_COLOR];
    }
    if (button->highlighted) {
        bg = &g_colors[DEFAULT_MENU_BG_ACTIVE_COLOR];
    } else {
        bg = &g_colors[DEFAULT_MENU_BG_INACTIVE_COLOR];
    }

    SDL_Rect rect = get_button_caption_rect(button);

    render_fill_rect(window,
            NULL, &button->full_rect, bg);
    render_utf8_string(window, window->status_bar.font, NULL, 
            fg, rect, format(button->caption, font_value.subwindow->font->size));
}

static void render_button_menu_font_name(const struct window *window, struct button *button)
{
    CHECK_BUTTON_DATA_TYPE(button, BUTTON_DATA_FONT);

    SDL_Color fg;
    SDL_Color *bg;

    struct subwindow *subwindow = button->data.value.font_value.subwindow;
    size_t index = button->data.value.font_value.index;

    if (!button->data.value.font_value.size_ok) {
        fg = g_colors[DEFAULT_ERROR_COLOR];
    } else if (subwindow->font->index == index) {
        fg = g_colors[DEFAULT_MENU_TOGGLE_FG_ACTIVE_COLOR];
    } else {
        fg = g_colors[DEFAULT_MENU_TOGGLE_FG_INACTIVE_COLOR];
    }
    if (button->highlighted) {
        bg = &g_colors[DEFAULT_MENU_BG_ACTIVE_COLOR];
    } else {
        bg = &g_colors[DEFAULT_MENU_BG_INACTIVE_COLOR];
    }

    SDL_Rect rect = get_button_caption_rect(button);

    render_fill_rect(window,
            NULL, &button->full_rect, bg);
    render_utf8_string(window, window->status_bar.font, NULL, 
            fg, rect, button->caption);
}

static void render_button_menu_sound_volume(const struct window *window,
        struct button *button)
{
    CHECK_BUTTON_DATA_TYPE(button, BUTTON_DATA_INT);

    assert(button->data.value.int_value == BUTTON_VOLUME_SOUND
            || button->data.value.int_value == BUTTON_VOLUME_MUSIC);

    SDL_Color fg;
    SDL_Color *bg;

    fg = g_colors[DEFAULT_MENU_TOGGLE_FG_ACTIVE_COLOR];

    if (button->highlighted) {
        bg = &g_colors[DEFAULT_MENU_BG_ACTIVE_COLOR];
    } else {
        bg = &g_colors[DEFAULT_MENU_BG_INACTIVE_COLOR];
    }

    SDL_Rect rect = get_button_caption_rect(button);

    int scale = 0;
    if (button->data.value.int_value == BUTTON_VOLUME_SOUND) {
        scale = sound_volume;
    } else if (button->data.value.int_value == BUTTON_VOLUME_MUSIC) {
        scale = music_volume;
    }

    render_fill_rect(window,
            NULL, &button->full_rect, bg);
    render_utf8_string(window, window->status_bar.font, NULL, 
            fg, rect, format(button->caption, scale));
}

static void render_button_menu_window(const struct window *window,
        struct button *button)
{
    CHECK_BUTTON_DATA_TYPE(button, BUTTON_DATA_UNSIGNED);

    struct window *w = get_window_direct(button->data.value.unsigned_value);

    SDL_Color fg;
    SDL_Color *bg;

    if (w != NULL) {
        fg = g_colors[DEFAULT_MENU_TOGGLE_FG_ACTIVE_COLOR];
    } else {
        fg = g_colors[DEFAULT_MENU_TOGGLE_FG_INACTIVE_COLOR];
    }
    if (button->highlighted) {
        bg = &g_colors[DEFAULT_MENU_BG_ACTIVE_COLOR];
    } else {
        bg = &g_colors[DEFAULT_MENU_BG_INACTIVE_COLOR];
    }

    SDL_Rect rect = get_button_caption_rect(button);

    render_fill_rect(window, NULL, &button->full_rect, bg);
    render_utf8_string(window, window->status_bar.font, NULL, 
            fg, rect, format(button->caption, button->data.value.unsigned_value));
}

static void render_button_menu_fullscreen(const struct window *window,
        struct button *button)
{
    CHECK_BUTTON_DATA_TYPE(button, BUTTON_DATA_NONE);

    render_button_menu_toggle(window, button,
            window->flags & SDL_WINDOW_FULLSCREEN_DESKTOP);
}

static void render_button_menu_kp_mod(const struct window *window,
        struct button *button)
{
    CHECK_BUTTON_DATA_TYPE(button, BUTTON_DATA_NONE);

    render_button_menu_toggle(window, button, g_kp_as_mod);
}

/* the menu proper is rendered via this callback, used by the "Menu" button */
static void render_menu_button(const struct window *window, struct button *button)
{
    CHECK_BUTTON_DATA_TYPE(button, BUTTON_DATA_NONE);

    SDL_Color color;
    if (button->highlighted) {
        color = g_colors[DEFAULT_STATUS_BAR_BUTTON_ACTIVE_COLOR];
    } else {
        color = g_colors[DEFAULT_STATUS_BAR_BUTTON_INACTIVE_COLOR];
    }

    SDL_Rect rect = get_button_caption_rect(button);

    render_utf8_string(window, window->status_bar.font, window->status_bar.texture, 
            color, rect, button->caption);

    if (button->highlighted) {
        render_menu_panel(window, window->status_bar.menu_panel);
    }
}

static void render_button_subwindows(const struct window *window, struct button *button)
{
    CHECK_BUTTON_DATA_TYPE(button, BUTTON_DATA_UNSIGNED);

    SDL_Color color;
    if (has_visible_subwindow(window, button->data.value.unsigned_value)
            || button->highlighted) {
        color = g_colors[DEFAULT_STATUS_BAR_BUTTON_ACTIVE_COLOR];
    } else {
        color = g_colors[DEFAULT_STATUS_BAR_BUTTON_INACTIVE_COLOR];
    }

    SDL_Rect rect = get_button_caption_rect(button);

    render_utf8_string(window, window->status_bar.font, window->status_bar.texture, 
            color, rect, button->caption);
}

static void render_button_movesize(const struct window *window, struct button *button)
{
    CHECK_BUTTON_DATA_TYPE(button, BUTTON_DATA_INT);

    bool active = false;
    switch (button->data.value.int_value) {
        case BUTTON_MOVESIZE_MOVING:
            active = window->move_state.active;
            break;
        case BUTTON_MOVESIZE_SIZING:
            active = window->size_state.active;
            break;
        default:
            quit_fmt("button '%s' has wrong int_value %d",
                    button->caption, button->data.value.int_value);
            break;
    }

    SDL_Color color;
    if (active || button->highlighted) {
        color = g_colors[DEFAULT_STATUS_BAR_BUTTON_ACTIVE_COLOR];
    } else {
        color = g_colors[DEFAULT_STATUS_BAR_BUTTON_INACTIVE_COLOR];
    }

    SDL_Rect rect = get_button_caption_rect(button);

    render_utf8_string(window, window->status_bar.font, window->status_bar.texture, 
            color, rect, button->caption);
}

static void show_about(const struct window *window)
{
    const char *about_text[] = {
        version_build(VERSION_NAME, true),
        "See https://tangaria.com",
        "Visit our Discord at https://discord.gg/zBNG369"
    };

    struct { SDL_Rect rect; const char *text; } elems[N_ELEMENTS(about_text)];
    for (size_t i = 0; i < N_ELEMENTS(elems); i++) {
        elems[i].text = about_text[i];
    }

    char path[4096];
    path_build(path, sizeof(path), DEFAULT_ABOUT_ICON_DIR, DEFAULT_ABOUT_ICON);

    SDL_Texture *texture = load_image(window, path);
    SDL_Rect texture_rect = {0};
    SDL_QueryTexture(texture, NULL, NULL, &texture_rect.w, &texture_rect.h);

    SDL_Rect total = {
        0, 0,
        2 * DEFAULT_XTRA_BORDER + texture_rect.w,
        /* the default icon just looks better without bottom border */
        DEFAULT_XTRA_BORDER + texture_rect.h
    };

    for (size_t i = 0; i < N_ELEMENTS(elems); i++) {
        int w;
        int h;
        get_string_metrics(window->status_bar.font,
                elems[i].text, &w, &h);

        elems[i].rect.h = h;
        elems[i].rect.w = w;
        elems[i].rect.y = total.h + (DEFAULT_LINE_HEIGHT(h) - h) / 2;

        total.w = MAX(w + 2 * DEFAULT_XTRA_BORDER, total.w);
        total.h += DEFAULT_LINE_HEIGHT(h);
    }
    total.h += DEFAULT_XTRA_BORDER;

    total.x = window->full_rect.w / 2 - total.w / 2;
    total.y = window->full_rect.h / 2 - total.h / 2;

    render_window_in_menu(window);

    render_fill_rect(window, NULL, &total, &g_colors[DEFAULT_ABOUT_BG_COLOR]);
    render_outline_rect_width(window, NULL, &total,
            &g_colors[DEFAULT_ABOUT_BORDER_OUTER_COLOR], DEFAULT_VISIBLE_BORDER);
    resize_rect(&total,
            DEFAULT_VISIBLE_BORDER, DEFAULT_VISIBLE_BORDER,
            -DEFAULT_VISIBLE_BORDER, -DEFAULT_VISIBLE_BORDER);
    render_outline_rect_width(window, NULL, &total,
            &g_colors[DEFAULT_ABOUT_BORDER_INNER_COLOR], DEFAULT_VISIBLE_BORDER);

    for (size_t i = 0; i < N_ELEMENTS(elems); i++) {
        /* center the string in total rect */
        elems[i].rect.x = total.x + (total.w - elems[i].rect.w) / 2;
        /* make the y coord of string absolute (was relative to total rect) */
        elems[i].rect.y += total.y;

        render_utf8_string(window, window->status_bar.font,
            NULL, g_colors[DEFAULT_ABOUT_TEXT_COLOR],
            elems[i].rect, elems[i].text);
    }

    texture_rect.x = total.x + (total.w - texture_rect.w) / 2;
    texture_rect.y = total.y + DEFAULT_XTRA_BORDER;

    SDL_SetRenderTarget(window->renderer, NULL);
    SDL_RenderCopy(window->renderer, texture, NULL, &texture_rect);
    SDL_RenderPresent(window->renderer);

    wait_anykey();

    SDL_DestroyTexture(texture);
}

static void signal_move_state(struct window *window)
{
    assert(!window->size_state.active);

    bool was_active = window->move_state.active;

    if (was_active) {
        window->move_state.active = false;
        window->move_state.moving = false;
        window->move_state.subwindow = NULL;
    } else {
        window->move_state.active = true;
    }

    SDL_SetWindowGrab(window->window,
            was_active ? SDL_FALSE : SDL_TRUE);
    window->alpha = was_active ? DEFAULT_ALPHA_FULL : DEFAULT_ALPHA_LOW;
}

static void signal_size_state(struct window *window)
{
    assert(!window->move_state.active);

    bool was_active = window->size_state.active;

    if (was_active) {
        window->size_state.active = false;
        window->size_state.sizing = false;
        if (window->size_state.subwindow != NULL) {
            memset(&window->size_state.subwindow->sizing_rect,
                    0, sizeof(window->size_state.subwindow->sizing_rect));
            window->size_state.subwindow = NULL;
        }
    } else {
        window->size_state.active = true;
    }

    SDL_SetWindowGrab(window->window,
            was_active ? SDL_FALSE : SDL_TRUE);
    window->alpha = was_active ? DEFAULT_ALPHA_FULL : DEFAULT_ALPHA_LOW;
}

static bool ignore_status_bar_button(struct window *window,
        struct button *button, const SDL_Event *event)
{
    (void) window;
    (void) button;
    (void) event;

    return false;
}

static bool click_status_bar_button(struct window *window,
        struct button *button, const SDL_Event *event)
{
    switch (event->type) {
        case SDL_MOUSEBUTTONDOWN:
            if (is_point_in_rect(event->button.x, event->button.y, &button->full_rect)) {
                button->selected = true;
                return false;
            }
            break;
        case SDL_MOUSEBUTTONUP:
            if (is_point_in_rect(event->button.x, event->button.y, &button->full_rect)
                    && button->selected)
            {
                button->selected = false;
                return true;
            }
            break;
        case SDL_MOUSEMOTION:
            if (is_point_in_rect(event->motion.x, event->motion.y, &button->full_rect)) {
                button->highlighted = true;
                return false;
            }
            break;
    }

    button->highlighted = false;
    button->selected = false;

    return false;
}

static bool handle_button_movesize(struct window *window,
        struct button *button, const SDL_Event *event)
{
    CHECK_BUTTON_DATA_TYPE(button, BUTTON_DATA_INT);

    if (!click_status_bar_button(window, button, event)) {
        return false;
    }

    switch (button->data.value.int_value) {
        case BUTTON_MOVESIZE_MOVING:
            if (window->size_state.active) {
                /* toggle size button */
                signal_size_state(window);
            }
            signal_move_state(window);
            break;
        case BUTTON_MOVESIZE_SIZING:
            if (window->move_state.active) {
                /* toggle move button */
                signal_move_state(window);
            }
            signal_size_state(window);
            break;
    }

    return true;
}

static void push_button(struct button_bank *bank, struct font *font,
        const char *caption, struct button_data data, struct button_callbacks callbacks,
        const SDL_Rect *rect, enum button_caption_position position)
{
    assert(bank->number < bank->size);

    struct button *button = &bank->buttons[bank->number];

    int w;
    int h;
    get_string_metrics(font, caption, &w, &h);

    int x = 0;
    switch (position) {
        case CAPTION_POSITION_CENTER:
            x = (rect->w - w) / 2;
            break;
        case CAPTION_POSITION_LEFT:
            x = DEFAULT_BUTTON_BORDER;
            break;
        case CAPTION_POSITION_RIGHT:
            x = rect->w - DEFAULT_BUTTON_BORDER - w;
            break;
        default:
            quit_fmt("bad caption position %d in button '%s'",
                    position, button->caption);
            break;
    }

    button->inner_rect.x = x;
    button->inner_rect.y = (rect->h - h) / 2;
    button->inner_rect.w = w;
    button->inner_rect.h = h;

    button->full_rect = *rect;

    assert(button->full_rect.w >= button->inner_rect.w
            && button->full_rect.h >= button->inner_rect.h);

    button->caption = string_make(caption);

    button->callbacks = callbacks;
    button->data = data;
    button->highlighted = false;
    button->selected = false;

    bank->number++;
}

static struct menu_panel *new_menu_panel(void)
{
    struct menu_panel *menu = mem_zalloc(sizeof(*menu));

    make_button_bank(&menu->button_bank);
    menu->next = NULL;

    return menu;
}

/* if caption of some button is NULL, the button is not included in menu (skipped) */
static struct menu_panel *make_menu_panel(const struct button *origin,
        struct font *font, size_t n_elems, struct menu_elem *elems)
{
    int maxlen = 0;

    for (size_t i = 0; i < n_elems; i++) {
        if (elems[i].caption == NULL) {
            continue;
        }
        int w;
        get_string_metrics(font, elems[i].caption, &w, NULL);
        maxlen = MAX(maxlen, w);
    }

    struct menu_panel *menu_panel = new_menu_panel();
    if (menu_panel == NULL) {
        return NULL;
    }

    SDL_Rect rect = {
        origin->full_rect.x + origin->full_rect.w,
        origin->full_rect.y,
        DEFAULT_MENU_LINE_WIDTH(maxlen),
        DEFAULT_MENU_LINE_HEIGHT(font->ttf.glyph.h)
    };

    menu_panel->rect = rect;
    menu_panel->rect.h = 0;

    for (size_t i = 0; i < n_elems; i++) {
        if (elems[i].caption == NULL) {
            continue;
        }

        struct button_callbacks callbacks = {
            elems[i].on_render, NULL, elems[i].on_menu
        };
        push_button(&menu_panel->button_bank,
                font,
                elems[i].caption,
                elems[i].data,
                callbacks,
                &rect,
                CAPTION_POSITION_LEFT);

        rect.y += rect.h;
        menu_panel->rect.h += rect.h;
    }

    return menu_panel;
}

static void load_next_menu_panel(const struct window *window,
        struct menu_panel *menu_panel, const struct button *origin,
        size_t n_elems, struct menu_elem *elems)
{
    assert(menu_panel->next == NULL);
    menu_panel->next = make_menu_panel(origin,
            window->status_bar.font, n_elems, elems);
}

static void do_menu_cleanup(struct button *button,
        struct menu_panel *menu_panel, const SDL_Event *event)
{
    switch (event->type) {
        case SDL_MOUSEMOTION:     /* fallthru */
        case SDL_MOUSEBUTTONDOWN: /* fallthru */
        case SDL_MOUSEBUTTONUP:
            if (menu_panel->next != NULL) {
                free_menu_panel(menu_panel->next);
                menu_panel->next = NULL;
            }
            break;
        default:
            quit_fmt("non mouse event %d for button '%s'",
                    event->type, button->caption);
            break;
    }
}

static bool select_menu_button(struct button *button,
        struct menu_panel *menu_panel, const SDL_Event *event)
{
    if (button->selected) {
        return false;
    } else {
        do_menu_cleanup(button, menu_panel, event);
        button->selected = true;

        return true;
    }
}

static bool click_menu_button(struct button *button,
        struct menu_panel *menu_panel, const SDL_Event *event)
{
    /* any event on that button removes the existing panel submenus
     * (and clickable buttons should not have their own submenus) */
    do_menu_cleanup(button, menu_panel, event);

    switch (event->type) {
        case SDL_MOUSEBUTTONDOWN:
            button->selected = true;
            return false;
        case SDL_MOUSEBUTTONUP:
            if (button->selected) {
                button->selected = false;
                /* click the button */
                return true;
            } else {
                return false;
            }
        default:
            return false;
    }
}

static void handle_menu_sound_volume(struct window *window,
        struct button *button, const SDL_Event *event,
        struct menu_panel *menu_panel)
{
    CHECK_BUTTON_DATA_TYPE(button, BUTTON_DATA_INT);

    if (!click_menu_button(button, menu_panel, event)) {
        return;
    }

    int increment =
        (event->button.x - button->full_rect.x < button->full_rect.w / 2) ? -5 : +5;

    if (button->data.value.int_value == BUTTON_VOLUME_SOUND) {
        sound_volume += increment;
        if (sound_volume < 0) {
            sound_volume = 0;
        } else if (sound_volume > 100) {
            sound_volume = 100;
        }
    } else if (button->data.value.int_value == BUTTON_VOLUME_MUSIC) {
        music_volume += increment;
        if (music_volume < 0) {
            music_volume = 0;
        } else if (music_volume > 100) {
            music_volume = 100;
        }
    } else {
        quit_fmt("bad int_value in button '%s'", button->caption);
    }
}

static void handle_menu_sound(struct window *window,
        struct button *button, const SDL_Event *event,
        struct menu_panel *menu_panel)
{
    CHECK_BUTTON_DATA_TYPE(button, BUTTON_DATA_NONE);

    if (!select_menu_button(button, menu_panel, event)) {
        return;
    }

    struct menu_elem elems[] = {
        {
            "< Sound Volume: %d >",
            {
                BUTTON_DATA_INT,
                {.int_value = BUTTON_VOLUME_SOUND},
            },
            render_button_menu_sound_volume,
            handle_menu_sound_volume
        },
        {
            "< Music Volume: %d >",
            {
                BUTTON_DATA_INT,
                {.int_value = BUTTON_VOLUME_MUSIC},
            },
            render_button_menu_sound_volume,
            handle_menu_sound_volume
        }
    };

    load_next_menu_panel(window, menu_panel, button, N_ELEMENTS(elems), elems);
}

static void handle_menu_window(struct window *window,
        struct button *button, const SDL_Event *event,
        struct menu_panel *menu_panel)
{
    CHECK_BUTTON_DATA_TYPE(button, BUTTON_DATA_UNSIGNED);

    if (!click_menu_button(button, menu_panel, event)) {
        return;
    }

    struct window *other = get_window_direct(button->data.value.unsigned_value);
    if (other == NULL) {
        other = get_new_window(button->data.value.unsigned_value);
        assert(other != NULL);
        wipe_window_aux_config(other);
        start_window(other);
    }
}

static void handle_menu_windows(struct window *window,
        struct button *button, const SDL_Event *event,
        struct menu_panel *menu_panel)
{
    CHECK_BUTTON_DATA_TYPE(button, BUTTON_DATA_NONE);

    if (!select_menu_button(button, menu_panel, event)) {
        return;
    }

    struct menu_elem elems[MAX_WINDOWS];

    for (unsigned i = 0; i < MAX_WINDOWS; i++) {
        elems[i].caption = "Window-%u";
        elems[i].data.type = BUTTON_DATA_UNSIGNED;
        elems[i].data.value.unsigned_value = i;
        elems[i].on_render = render_button_menu_window;
        elems[i].on_menu = handle_menu_window;
    }

    load_next_menu_panel(window, menu_panel, button, N_ELEMENTS(elems), elems);
}

static void handle_menu_fullscreen(struct window *window,
        struct button *button, const SDL_Event *event,
        struct menu_panel *menu_panel)
{
    CHECK_BUTTON_DATA_TYPE(button, BUTTON_DATA_NONE);

    if (!click_menu_button(button, menu_panel, event)) {
        return;
    }

    if (window->flags & SDL_WINDOW_FULLSCREEN_DESKTOP) {
        SDL_SetWindowFullscreen(window->window, 0);
        SDL_SetWindowMinimumSize(window->window,
                DEFAULT_WINDOW_MINIMUM_W, DEFAULT_WINDOW_MINIMUM_H);
    } else {
        SDL_SetWindowFullscreen(window->window, SDL_WINDOW_FULLSCREEN_DESKTOP);
    }

    window->flags = SDL_GetWindowFlags(window->window);
}

static void handle_menu_kp_mod(struct window *window,
        struct button *button, const SDL_Event *event,
        struct menu_panel *menu_panel)
{
    CHECK_BUTTON_DATA_TYPE(button, BUTTON_DATA_NONE);

    if (!click_menu_button(button, menu_panel, event)) {
        return;
    }

    g_kp_as_mod = !g_kp_as_mod;
}

static void handle_menu_about(struct window *window,
        struct button *button, const SDL_Event *event,
        struct menu_panel *menu_panel)
{
    CHECK_BUTTON_DATA_TYPE(button, BUTTON_DATA_NONE);

    if (!click_menu_button(button, menu_panel, event)) {
        return;
    }

    show_about(window);
}

static void handle_menu_quit(struct window *window,
        struct button *button, const SDL_Event *event,
        struct menu_panel *menu_panel)
{
    CHECK_BUTTON_DATA_TYPE(button, BUTTON_DATA_NONE);

    if (!click_menu_button(button, menu_panel, event)) {
        return;
    }

    handle_quit();
}

static void handle_menu_tile_set(struct window *window,
        struct button *button, const SDL_Event *event,
        struct menu_panel *menu_panel)
{
    CHECK_BUTTON_DATA_TYPE(button, BUTTON_DATA_INT);

    if (!click_menu_button(button, menu_panel, event)) {
        return;
    }
    
    graphics_mode *mode = get_graphics_mode(button->data.value.int_value, true);
    assert(mode != NULL);

    reload_all_graphics(mode);

    refresh_angband_terms();
}

static void handle_menu_tile_size(struct window *window,
        struct button *button, const SDL_Event *event,
        struct menu_panel *menu_panel)
{
    CHECK_BUTTON_DATA_TYPE(button, BUTTON_DATA_INT);

    if (!click_menu_button(button, menu_panel, event)) {
        return;
    }
    if (window->graphics.id == GRAPHICS_NONE) {
        return;
    }

    int increment =
        (event->button.x - button->full_rect.x < button->full_rect.w / 2) ? -1 : +1;

    if (button->data.value.int_value == BUTTON_TILE_SIZE_WIDTH) {
        tile_width += increment;
        if (tile_width < MIN_TILE_WIDTH) {
            tile_width = MAX_TILE_WIDTH;
        } else if (tile_width > MAX_TILE_WIDTH) {
            tile_width = MIN_TILE_WIDTH;
        }
    } else if (button->data.value.int_value == BUTTON_TILE_SIZE_HEIGHT) {
        tile_height += increment;
        if (tile_height < MIN_TILE_HEIGHT) {
            tile_height = MAX_TILE_HEIGHT;
        } else if (tile_height > MAX_TILE_HEIGHT) {
            tile_height = MIN_TILE_HEIGHT;
        }
    } else {
        quit_fmt("bad int_value in button '%s'", button->caption);
    }

    refresh_angband_terms();
}

static void handle_menu_tile_sizes(struct window *window,
        struct button *button, const SDL_Event *event,
        struct menu_panel *menu_panel)
{
    CHECK_BUTTON_DATA_TYPE(button, BUTTON_DATA_SUBWINDOW);

    if (!select_menu_button(button, menu_panel, event)) {
        return;
    }

    struct menu_elem elems[] = {
        {
            "< Tile width  %d >",
            {
                BUTTON_DATA_INT,
                {.int_value = BUTTON_TILE_SIZE_WIDTH},
            },
            render_button_menu_tile_size,
            handle_menu_tile_size
        },
        {
            "< Tile height %d >",
            {
                BUTTON_DATA_INT,
                {.int_value = BUTTON_TILE_SIZE_HEIGHT},
            },
            render_button_menu_tile_size,
            handle_menu_tile_size
        }
    };

    load_next_menu_panel(window, menu_panel, button, N_ELEMENTS(elems), elems);
}

static void handle_menu_tile_sets(struct window *window,
        struct button *button, const SDL_Event *event,
        struct menu_panel *menu_panel)
{
    CHECK_BUTTON_DATA_TYPE(button, BUTTON_DATA_SUBWINDOW);

    if (!select_menu_button(button, menu_panel, event)) {
        return;
    }
    if (Setup.initialized) {
        return;
    }

    size_t num_elems = 0;
#ifdef BUILDINGWithVS
    struct menu_elem *elems;
#endif

    graphics_mode *mode = graphics_modes;
    while (mode != NULL) {
        ++num_elems;
        mode = mode->pNext;
    }

#ifndef BUILDINGWithVS
    struct menu_elem elems[num_elems];
#else
    elems = mem_alloc(num_elems * sizeof(*elems));
#endif

    mode = graphics_modes;
    for (size_t i = 0; i < num_elems; ++i) {
        elems[i].caption = mode->menuname;
        elems[i].data.type = BUTTON_DATA_INT;
        elems[i].data.value.int_value = mode->grafID;
        elems[i].on_render = render_button_menu_tile_set;
        elems[i].on_menu = handle_menu_tile_set;

        mode = mode->pNext;
    }

    load_next_menu_panel(window, menu_panel, button, num_elems, elems);

#ifdef BUILDINGWithVS
    mem_free(elems);
#endif
}

static void handle_menu_tiles(struct window *window,
        struct button *button, const SDL_Event *event,
        struct menu_panel *menu_panel)
{
    CHECK_BUTTON_DATA_TYPE(button, BUTTON_DATA_SUBWINDOW);

    if (!select_menu_button(button, menu_panel, event)) {
        return;
    }

    struct button_data data = {
        BUTTON_DATA_SUBWINDOW, {.subwindow_value = button->data.value.subwindow_value}
    };

    struct menu_elem elems[] = {
        {"Set", data, render_button_menu_tile_sets, handle_menu_tile_sets},
        {"Size", data, render_button_menu_simple, handle_menu_tile_sizes}
    };

    load_next_menu_panel(window, menu_panel, button, N_ELEMENTS(elems), elems);
}

static void handle_menu_pw(struct window *window,
        struct button *button, const SDL_Event *event,
        struct menu_panel *menu_panel)
{
    CHECK_BUTTON_DATA_TYPE(button, BUTTON_DATA_TERM_FLAG);

    if (!click_menu_button(button, menu_panel, event)) {
        return;
    }

    uint32_t new_flags[N_ELEMENTS(window_flag)];
    assert(sizeof(new_flags) == sizeof(window_flag));
    memcpy(new_flags, window_flag, sizeof(new_flags));

    struct subwindow *subwindow = button->data.value.term_flag_value.subwindow;

    assert(subwindow->index != MAIN_SUBWINDOW);
    assert(subwindow->index < N_ELEMENTS(window_flag));

    uint32_t flag = button->data.value.term_flag_value.flag;

    new_flags[subwindow->index] = flag;
    subwindows_set_flags(new_flags, N_ELEMENTS(new_flags));

    refresh_angband_terms();
}

static void handle_menu_font_name(struct window *window,
        struct button *button, const SDL_Event *event,
        struct menu_panel *menu_panel)
{
    CHECK_BUTTON_DATA_TYPE(button, BUTTON_DATA_FONT);

    if (!click_menu_button(button, menu_panel, event)) {
        return;
    }

    assert(button->data.value.font_value.index < N_ELEMENTS(g_font_info));

    unsigned index = button->data.value.font_value.index;
    struct subwindow *subwindow = button->data.value.font_value.subwindow;

    const struct font_info *font_info = &g_font_info[index];
    assert(font_info->loaded);

    if (subwindow->font->index == index) {
        /* already loaded */
        return;
    } 

    if (reload_font(subwindow, font_info)) {
        button->data.value.font_value.size_ok = true;
    } else {
        button->data.value.font_value.size_ok = false;
    }
}

static void handle_menu_font_size(struct window *window,
        struct button *button, const SDL_Event *event,
        struct menu_panel *menu_panel)
{
    CHECK_BUTTON_DATA_TYPE(button, BUTTON_DATA_FONT);

    if (!click_menu_button(button, menu_panel, event)) {
        return;
    }
    if (!button->data.value.font_value.size_ok) {
        return;
    }

    unsigned index = button->data.value.font_value.index;
    assert(index < N_ELEMENTS(g_font_info));

    struct font_info *info = &g_font_info[index];

    if (info->type == FONT_TYPE_RASTER) {
        return;
    }

    struct subwindow *subwindow = button->data.value.font_value.subwindow; 

    int size = subwindow->font->size;

    int increment =
        (event->button.x - button->full_rect.x < button->full_rect.w / 2) ? -1 : +1;

    for (size_t i = 0; i < MAX_VECTOR_FONT_SIZE - MIN_VECTOR_FONT_SIZE; i++) {
        size += increment;
        if (size > MAX_VECTOR_FONT_SIZE) {
            size = MIN_VECTOR_FONT_SIZE;
        } else if (size < MIN_VECTOR_FONT_SIZE) {
            size = MAX_VECTOR_FONT_SIZE;
        }
        info->size = size;
        if (reload_font(subwindow, info)) {
            return;
        }
    }

    button->data.value.font_value.size_ok = false;
}

static void handle_menu_font_sizes(struct window *window,
        struct button *button, const SDL_Event *event,
        struct menu_panel *menu_panel)
{
    CHECK_BUTTON_DATA_TYPE(button, BUTTON_DATA_SUBWINDOW);

    if (!select_menu_button(button, menu_panel, event)) {
        return;
    }

    struct subwindow *subwindow = button->data.value.subwindow_value;

    struct button_data data = {
        BUTTON_DATA_FONT,
        {.font_value =
            {.subwindow = subwindow, .index = subwindow->font->index, .size_ok = true}},
    };
    struct menu_elem elems[] = {
        {"< %2d points >", data, render_button_menu_font_size, handle_menu_font_size}
    };

    load_next_menu_panel(window, menu_panel, button, N_ELEMENTS(elems), elems);
}

static void handle_menu_font_names(struct window *window,
        struct button *button, const SDL_Event *event,
        struct menu_panel *menu_panel)
{
    CHECK_BUTTON_DATA_TYPE(button, BUTTON_DATA_SUBWINDOW);

    if (!select_menu_button(button, menu_panel, event)) {
        return;
    }

    struct menu_elem elems[N_ELEMENTS(g_font_info)];

    size_t num_elems = 0;
    for (size_t i = 0; i < MAX_BUTTONS; i++) {
        if (g_font_info[i].loaded) {
            elems[num_elems].caption = g_font_info[i].name;
            elems[num_elems].data.type = BUTTON_DATA_FONT;
            elems[num_elems].data.value.font_value.subwindow = button->data.value.subwindow_value;
            elems[num_elems].data.value.font_value.size_ok = true;
            elems[num_elems].data.value.font_value.index = i;
            elems[num_elems].on_render = render_button_menu_font_name;
            elems[num_elems].on_menu = handle_menu_font_name;
            num_elems++;
        }
    }

    load_next_menu_panel(window, menu_panel, button, num_elems, elems);
}

static void handle_menu_font_names_page(struct window *window,
        struct button *button, const SDL_Event *event,
        struct menu_panel *menu_panel)
{
    CHECK_BUTTON_DATA_TYPE(button, BUTTON_DATA_SUBWINDOW);

    if (!select_menu_button(button, menu_panel, event)) {
        return;
    }

    struct menu_elem elems[N_ELEMENTS(g_font_info)];

    size_t num_elems = 0;
    for (size_t i = MAX_BUTTONS; i < N_ELEMENTS(g_font_info); i++) {
        if (g_font_info[i].loaded) {
            elems[num_elems].caption = g_font_info[i].name;
            elems[num_elems].data.type = BUTTON_DATA_FONT;
            elems[num_elems].data.value.font_value.subwindow = button->data.value.subwindow_value;
            elems[num_elems].data.value.font_value.size_ok = true;
            elems[num_elems].data.value.font_value.index = i;
            elems[num_elems].on_render = render_button_menu_font_name;
            elems[num_elems].on_menu = handle_menu_font_name;
            num_elems++;
        }
    }

    load_next_menu_panel(window, menu_panel, button, num_elems, elems);
}

static void handle_menu_purpose(struct window *window,
        struct button *button, const SDL_Event *event,
        struct menu_panel *menu_panel)
{
    CHECK_BUTTON_DATA_TYPE(button, BUTTON_DATA_SUBWINDOW);

    struct subwindow *subwindow = button->data.value.subwindow_value;
    assert(subwindow->index != MAIN_SUBWINDOW);

    if (!select_menu_button(button, menu_panel, event)) {
        return;
    }

    struct menu_elem elems[N_ELEMENTS(window_flag_desc)];

    size_t num_elems = 0;
    while (num_elems < N_ELEMENTS(elems)
            && window_flag_desc[num_elems] != NULL)
    {
        elems[num_elems].caption = window_flag_desc[num_elems];
        elems[num_elems].data.value.term_flag_value.subwindow = subwindow;
        elems[num_elems].data.value.term_flag_value.flag = ((uint32_t)1) << num_elems;
        elems[num_elems].data.type = BUTTON_DATA_TERM_FLAG;
        elems[num_elems].on_render = render_button_menu_pw;
        elems[num_elems].on_menu = handle_menu_pw;
        num_elems++;
    }

    load_next_menu_panel(window, menu_panel, button, num_elems, elems);
}

static void handle_menu_font(struct window *window,
        struct button *button, const SDL_Event *event,
        struct menu_panel *menu_panel)
{
    CHECK_BUTTON_DATA_TYPE(button, BUTTON_DATA_SUBWINDOW);

    if (!select_menu_button(button, menu_panel, event)) {
        return;
    }

    struct button_data data = {
        BUTTON_DATA_SUBWINDOW, {.subwindow_value = button->data.value.subwindow_value}
    };

    if (!font_page)
    {
        struct menu_elem elems[] = {
            {"Name", data, render_button_menu_simple, handle_menu_font_names},
            {"Size", data, render_button_menu_simple, handle_menu_font_sizes}
        };

        load_next_menu_panel(window, menu_panel, button, N_ELEMENTS(elems), elems);
    }
    else
    {
        struct menu_elem elems[] = {
            {"Name", data, render_button_menu_simple, handle_menu_font_names},
            {" >>>", data, render_button_menu_simple, handle_menu_font_names_page},
            {"Size", data, render_button_menu_simple, handle_menu_font_sizes}
        };

        load_next_menu_panel(window, menu_panel, button, N_ELEMENTS(elems), elems);
    }
}

static void handle_menu_borders(struct window *window,
        struct button *button, const SDL_Event *event,
        struct menu_panel *menu_panel)
{
    CHECK_BUTTON_DATA_TYPE(button, BUTTON_DATA_SUBWINDOW);

    if (!click_menu_button(button, menu_panel, event)) {
        return;
    }

    struct subwindow *subwindow = button->data.value.subwindow_value;

    subwindow->borders.visible = !subwindow->borders.visible;
    render_borders(subwindow);
}

static void handle_menu_subwindow_alpha(struct window *window,
        struct button *button, const SDL_Event *event,
        struct menu_panel *menu_panel)
{
    CHECK_BUTTON_DATA_TYPE(button, BUTTON_DATA_ALPHA);

    if (!click_menu_button(button, menu_panel, event)) {
        return;
    }

    struct subwindow *subwindow = button->data.value.alpha_value.subwindow;
    subwindow->color.a = button->data.value.alpha_value.real_value;

    render_clear(subwindow->window, subwindow->texture, &subwindow->color);
    render_borders(subwindow);

    refresh_angband_terms();
}

static void handle_menu_alpha(struct window *window,
        struct button *button, const SDL_Event *event,
        struct menu_panel *menu_panel)
{
    CHECK_BUTTON_DATA_TYPE(button, BUTTON_DATA_SUBWINDOW);

    if (!select_menu_button(button, menu_panel, event)) {
        return;
    }

    struct subwindow *subwindow = button->data.value.subwindow_value;

    struct menu_elem elems[(100 - DEFAULT_ALPHA_LOWEST) / DEFAULT_ALPHA_STEP +
        1 + ((100 - DEFAULT_ALPHA_LOWEST) % DEFAULT_ALPHA_STEP == 0 ? 0 : 1)];

    for (size_t i = 0; i < N_ELEMENTS(elems); i++) {
        int alpha = ALPHA_PERCENT(DEFAULT_ALPHA_LOWEST + i * DEFAULT_ALPHA_STEP);

        elems[i].caption = " %3d%% ";
        elems[i].data.type = BUTTON_DATA_ALPHA;
        elems[i].data.value.alpha_value.subwindow = subwindow;
        elems[i].data.value.alpha_value.real_value = alpha;
        elems[i].data.value.alpha_value.show_value = i * DEFAULT_ALPHA_STEP;
        elems[i].on_render = render_button_menu_alpha;
        elems[i].on_menu = handle_menu_subwindow_alpha;
    }
    elems[N_ELEMENTS(elems) - 1].data.value.alpha_value.real_value =
        DEFAULT_ALPHA_FULL;

    load_next_menu_panel(window, menu_panel, button, N_ELEMENTS(elems), elems);
}

static void handle_menu_top(struct window *window,
        struct button *button, const SDL_Event *event,
        struct menu_panel *menu_panel)
{
    CHECK_BUTTON_DATA_TYPE(button, BUTTON_DATA_SUBWINDOW);
    
    if (!click_menu_button(button, menu_panel, event)) {
        return;
    }

    struct subwindow *subwindow = button->data.value.subwindow_value;

    subwindow->always_top = !subwindow->always_top;

    sort_to_top(subwindow->window);
}

static void handle_menu_terms(struct window *window,
        struct button *button, const SDL_Event *event,
        struct menu_panel *menu_panel)
{
    CHECK_BUTTON_DATA_TYPE(button, BUTTON_DATA_SUBWINDOW);

    if (!select_menu_button(button, menu_panel, event)) {
        return;
    }

    struct subwindow *subwindow = button->data.value.subwindow_value;

    struct button_data data = {
        BUTTON_DATA_SUBWINDOW, {.subwindow_value = subwindow}
    };

    struct menu_elem elems[] = {
        {
            "Font", data, render_button_menu_simple, handle_menu_font
        },
        {
            subwindow->index == MAIN_SUBWINDOW ? "Tiles" : NULL,
            data, render_button_menu_simple, handle_menu_tiles
        },
        {
            subwindow->index == MAIN_SUBWINDOW ? NULL: "Purpose",
            data, render_button_menu_simple, handle_menu_purpose
        },
        {
            "Alpha", data, render_button_menu_simple, handle_menu_alpha
        },
        {
            "Borders", data, render_button_menu_borders, handle_menu_borders
        },
        {
            "Top", data, render_button_menu_top, handle_menu_top
        }
    };

    load_next_menu_panel(window, menu_panel, button, N_ELEMENTS(elems), elems);
}

static void load_main_menu_panel(struct status_bar *status_bar)
{
    assert(N_ELEMENTS(angband_term_name) == MAX_SUBWINDOWS);

    struct menu_elem term_elems[N_ELEMENTS(angband_term_name)];
    size_t n_terms = 0;

    for (size_t i = 0; i < N_ELEMENTS(angband_term_name); i++) {
        struct subwindow *subwindow =
            get_subwindow_by_index(status_bar->window, i, true);
        if (subwindow == NULL) {
            continue;
        }

        term_elems[n_terms].caption = angband_term_name[i];
        term_elems[n_terms].data.type = BUTTON_DATA_SUBWINDOW;
        term_elems[n_terms].data.value.subwindow_value = subwindow;
        term_elems[n_terms].on_render = render_button_menu_terms;
        term_elems[n_terms].on_menu = handle_menu_terms;
        n_terms++;
    }

    struct button_data data = {BUTTON_DATA_NONE, {0}};
    struct menu_elem other_elems[] = {
        {
            "Fullscreen",
            data, render_button_menu_fullscreen, handle_menu_fullscreen
        },
        {
            status_bar->window->index == MAIN_WINDOW ? "Send Keypad Modifier" : NULL,
            data, render_button_menu_kp_mod, handle_menu_kp_mod
        },
        {
            status_bar->window->index == MAIN_WINDOW ? "Sound" : NULL,
            data, render_button_menu_simple, handle_menu_sound
        },
        {
            status_bar->window->index == MAIN_WINDOW ? "Windows" : NULL,
            data, render_button_menu_simple, handle_menu_windows
        },
        {
            "About",
            data, render_button_menu_simple, handle_menu_about
        },
        {
            "Quit", 
            data, render_button_menu_simple, handle_menu_quit
        }
    };

    struct menu_elem elems[N_ELEMENTS(term_elems) + N_ELEMENTS(other_elems)];

    memcpy(elems, term_elems, n_terms * sizeof(term_elems[0]));
    memcpy(elems + n_terms, other_elems, sizeof(other_elems));

    struct button dummy = {0};
    dummy.full_rect.x = status_bar->full_rect.x;
    dummy.full_rect.y = status_bar->full_rect.y + status_bar->full_rect.h;

    status_bar->menu_panel = make_menu_panel(&dummy, status_bar->font,
            n_terms + N_ELEMENTS(other_elems), elems);
}

static void unselect_menu_buttons(struct menu_panel *menu_panel)
{
    if (menu_panel == NULL) {
        return;
    }

    for (size_t i = 0; i < menu_panel->button_bank.number; i++) {
        menu_panel->button_bank.buttons[i].selected = false;
        menu_panel->button_bank.buttons[i].highlighted = false;
    }

    unselect_menu_buttons(menu_panel->next);
}

static bool handle_menu_event(struct window *window, const SDL_Event *event)
{
    int x = -1;
    int y = -1;

    switch (event->type) {
        case SDL_MOUSEMOTION:
            x = event->motion.x;
            y = event->motion.y;
            break;

        case SDL_MOUSEBUTTONUP: /* fallthru */
        case SDL_MOUSEBUTTONDOWN:
            x = event->button.x;
            y = event->button.y;
            break;

        default:
            return false;
    }

    assert(x >= 0);
    assert(y >= 0);

    struct menu_panel *menu_panel =
        get_menu_panel_by_xy(window->status_bar.menu_panel, x, y);

    if (menu_panel == NULL) {
        return false;
    }

    bool handled = false;

    for (size_t i = 0; i < menu_panel->button_bank.number; i++) {
        struct button *button = &menu_panel->button_bank.buttons[i];
        if (is_point_in_rect(x, y, &button->full_rect)) {
            /* note that the buttons themselves set "selected" */
            button->highlighted = true;

            assert(button->callbacks.on_menu != NULL);
            button->callbacks.on_menu(window, button, event, menu_panel);

            handled = true;

        } else {
            button->highlighted = false;
            /* but we do unset selected */
            button->selected = false;
        }
    }

    unselect_menu_buttons(menu_panel->next);

    return handled;
}

static bool is_menu_button_mouse_click(const struct button *button,
        const SDL_Event *event)
{
    if ((event->type == SDL_MOUSEBUTTONDOWN || event->type == SDL_MOUSEBUTTONUP)
            && is_point_in_rect(event->button.x, event->button.y, &button->full_rect))
    {
        return true;
    }

    return false;
}

static bool handle_menu_button(struct window *window,
        struct button *button, const SDL_Event *event)
{
    CHECK_BUTTON_DATA_TYPE(button, BUTTON_DATA_NONE);

    switch (event->type) {
        case SDL_MOUSEMOTION:
            if (is_point_in_rect(event->motion.x, event->motion.y, &button->full_rect)) {
                /* create menu on mouseover */
                if (window->status_bar.menu_panel == NULL) {
                    load_main_menu_panel(&window->status_bar);
                }
                button->highlighted = true;
                return true;
            } else if (handle_menu_event(window, event)) {
                return true;
            } else if (button->highlighted) {
                /* the menu sticks around so that it is not horrible to use */
                return true;
            }
            return false;
        default:
            if (handle_menu_event(window, event)) {
                return true;
            } else if (is_menu_button_mouse_click(button, event)) {
                /* menu button just eats mouse clicks */
                return true;
            }

            if (window->status_bar.menu_panel != NULL) {
                free_menu_panel(window->status_bar.menu_panel);
                window->status_bar.menu_panel = NULL;
            }
            button->highlighted = false;
            return false;
    }
}

static bool is_close_to(int a, int b, unsigned range)
{
    if (a > 0 && b > 0) {
        return (unsigned) ABS(a - b) < range;
    } else if (a < 0 && b < 0) {
        return (unsigned) ABS(ABS(a) - ABS(b)) < range;
    } else {
        return (unsigned) (ABS(a) + ABS(b)) < range;
    }
}

static bool is_point_in_rect(int x, int y, const SDL_Rect *rect)
{
    return x >= rect->x && x < rect->x + rect->w
        && y >= rect->y && y < rect->y + rect->h;
}

//Renaming with postfix Rec to prevent conflict with typedef from rpcndr.h
static bool is_rect_in_rect(const SDL_Rect *smallRec, const SDL_Rect *bigRec)
{
    return smallRec->x >= bigRec->x
        && smallRec->x + smallRec->w <= bigRec->x + bigRec->w
        && smallRec->y >= bigRec->y
        && smallRec->y + smallRec->h <= bigRec->y + bigRec->h;
}

static void fit_rect_in_rect_by_hw(SDL_Rect *smallRec, const SDL_Rect *bigRec)
{
    if (smallRec->x < bigRec->x) {
        smallRec->w -= bigRec->x - smallRec->x;
        smallRec->x = bigRec->x;
    }
    if (smallRec->x + smallRec->w > bigRec->x + bigRec->w) {
        smallRec->w = bigRec->x + bigRec->w - smallRec->x;
    }
    if (smallRec->y < bigRec->y) {
        smallRec->h -= bigRec->y - smallRec->y;
        smallRec->y = bigRec->y;
    }
    if (smallRec->y + smallRec->h > bigRec->y + bigRec->h) {
        smallRec->h = bigRec->y + bigRec->h - smallRec->y;
    }
}

static void fit_rect_in_rect_by_xy(SDL_Rect *smallRec, const SDL_Rect *bigRec)
{
    if (smallRec->x < bigRec->x) {
        smallRec->x = bigRec->x;
    }
    if (smallRec->y < bigRec->y) {
        smallRec->y = bigRec->y;
    }
    if (smallRec->x + smallRec->w > bigRec->x + bigRec->w) {
        smallRec->x = MAX(bigRec->x, bigRec->x + bigRec->w - smallRec->w);
    }
    if (smallRec->y + smallRec->h > bigRec->y + bigRec->h) {
        smallRec->y = MAX(bigRec->y, bigRec->y + bigRec->h - smallRec->h);
    }
}

static void fit_rect_in_rect_proportional(SDL_Rect *smallRec, const SDL_Rect *bigRec)
{
    if (smallRec->x < bigRec->x) {
        smallRec->x = bigRec->x;
    }
    if (smallRec->y < bigRec->y) {
        smallRec->y = bigRec->y;
    }
    if (smallRec->w > bigRec->w) {
        smallRec->h = smallRec->h * bigRec->w / smallRec->w;
        smallRec->w = bigRec->w;
    }
    if (smallRec->h > bigRec->h) {
        smallRec->w = smallRec->w * bigRec->h / smallRec->h;
        smallRec->h = bigRec->h;
    }
}

static void resize_rect(SDL_Rect *rect,
        int left, int top, int right, int bottom)
{
    if (rect->w - left + right <= 0
            || rect->h - top + bottom <= 0)
    {
        return;
    }
    rect->x += left;
    rect->w -= left;

    rect->y += top;
    rect->h -= top;

    rect->w += right;
    rect->h += bottom;
}

static void crop_rects(SDL_Rect *src, SDL_Rect *dst)
{
    if (src->w == dst->w) {
        /* most common case - do nothing */
    } else if (src->w > dst->w) {
        /* second most common case - font glyph is too large */
        src->x += (src->w - dst->w) / 2;
        src->w = dst->w;
    } else {
        /* uncommon case - font glyph is too small */ 
        dst->x += (dst->w - src->w) / 2;
        dst->w = src->w;
    }

    if (src->h == dst->h) {
        ;
    } else if (src->h > dst->h) {
        src->y += (src->h - dst->h) / 2;
        src->h = dst->h;
    } else  {
        dst->y += (dst->h - src->h) / 2;
        dst->h = src->h;
    }
}

/* tries to snap to other term in such a way so that their
 * (visible) borders overlap */
static void try_snap(struct window *window,
        struct subwindow *subwindow, SDL_Rect *rect)
{
    for (size_t i = N_ELEMENTS(window->subwindows); i > 0; --i) {
        struct subwindow *other = window->subwindows[i - 1];
        if (other == NULL
                || !other->visible
                || other->index == subwindow->index)
        {
            continue;
        }

        int ox = other->full_rect.x;
        int oy = other->full_rect.y;
        int ow = other->full_rect.w;
        int oh = other->full_rect.h;

        if (oy < rect->y + rect->h && rect->y < oy + oh) {
            if (is_close_to(rect->x, ox + ow, DEFAULT_SNAP_RANGE)) {
                rect->x = ox + ow - DEFAULT_VISIBLE_BORDER;
            }
            if (is_close_to(rect->x + rect->w, ox, DEFAULT_SNAP_RANGE)) {
                rect->x = ox - rect->w + DEFAULT_VISIBLE_BORDER;
            }
        }
        if (ox < rect->x + rect->w && rect->x < ox + ow) {
            if (is_close_to(rect->y, oy + oh, DEFAULT_SNAP_RANGE)) {
                rect->y = oy + oh - DEFAULT_VISIBLE_BORDER;
            }
            if (is_close_to(rect->y + rect->h, oy, DEFAULT_SNAP_RANGE)) {
                rect->y = oy - rect->h + DEFAULT_VISIBLE_BORDER;
            }
        }
    }
}

static void start_moving(struct window *window,
        struct subwindow *subwindow, const SDL_MouseButtonEvent *mouse)
{
    assert(!window->size_state.active);

    bring_to_top(window, subwindow);

    window->move_state.originx = mouse->x;
    window->move_state.originy = mouse->y;

    window->move_state.subwindow = subwindow;
    window->move_state.moving = true;
}

static void start_sizing(struct window *window,
        struct subwindow *subwindow, const SDL_MouseButtonEvent *mouse)
{
    assert(!window->move_state.active);

    bring_to_top(window, subwindow);

    subwindow->sizing_rect = subwindow->full_rect;

    int x = mouse->x - (subwindow->full_rect.x + subwindow->full_rect.w / 2);
    int y = mouse->y - (subwindow->full_rect.y + subwindow->full_rect.h / 2);

    window->size_state.left = x < 0 ? true : false;
    window->size_state.top  = y < 0 ? true : false;

    window->size_state.originx = mouse->x;
    window->size_state.originy = mouse->y;

    window->size_state.subwindow = subwindow;
    window->size_state.sizing = true;
}

static bool handle_menu_mousebuttondown(struct window *window,
        const SDL_MouseButtonEvent *mouse)
{
    if (window->move_state.active || window->size_state.active) {
        struct subwindow *subwindow = get_subwindow_by_xy(window, mouse->x, mouse->y);
        if (subwindow != NULL
                && is_rect_in_rect(&subwindow->full_rect, &window->inner_rect))
        {
            if (window->move_state.active && !window->move_state.moving) {
                start_moving(window, subwindow, mouse);
            } else if (window->size_state.active && !window->size_state.sizing) {
                start_sizing(window, subwindow, mouse);
            }
        }
        return true;
    } else if (is_over_status_bar(&window->status_bar, mouse->x, mouse->y)) {
        return true;
    } else {
        return false;
    }
}

static void handle_window_closed(const SDL_WindowEvent *event)
{
    struct window *window = get_window_by_id(event->windowID);
    assert(window != NULL);

    if (window->index == MAIN_WINDOW) {
        handle_quit();
    } else {
        for (size_t i = 0; i < N_ELEMENTS(window->subwindows); i++) {
            struct subwindow *subwindow = window->subwindows[i];
            if (subwindow != NULL) {
                clear_pw_flag(subwindow);
            }
        }
        free_window(window);
    }
}

static void handle_window_focus(const SDL_WindowEvent *event)
{
    assert(event->event == SDL_WINDOWEVENT_FOCUS_GAINED
            || event->event == SDL_WINDOWEVENT_FOCUS_LOST);

    struct window *window = get_window_by_id(event->windowID);
    if (window == NULL) {
        /* when window is closed, it sends FOCUS_LOST event */
        assert(event->event == SDL_WINDOWEVENT_FOCUS_LOST);
        return;
    }
    
    switch (event->event) {
        case SDL_WINDOWEVENT_FOCUS_GAINED:
            window->focus = true;
            break;
        case SDL_WINDOWEVENT_FOCUS_LOST:
            window->focus = false;
            break;
    }
}

static void handle_last_resize_event(int num_events, const SDL_Event *events)
{
    assert(num_events > 0);

    for (int i = num_events - 1; i >= 0; i--) {
        if (events[i].window.event == SDL_WINDOWEVENT_RESIZED) {
            const SDL_WindowEvent event = events[i].window;

            struct window *window = get_window_by_id(event.windowID);
            assert(window != NULL);
            resize_window(window, event.data1, event.data2);

            return;
        }
    }
}

static void handle_windowevent(const SDL_WindowEvent *event)
{
    SDL_Event events[128];
    events[0].window = *event;

    int num_events = 1 + SDL_PeepEvents(events + 1, (int) N_ELEMENTS(events) - 1,
            SDL_GETEVENT, SDL_WINDOWEVENT, SDL_WINDOWEVENT);

    bool resize = false;

    for (int i = 0; i < num_events; i++) {
        switch (events[i].window.event) {
            case SDL_WINDOWEVENT_RESIZED:
                /* just for efficiency */
                resize = true;
                break;
            case SDL_WINDOWEVENT_CLOSE:
                handle_window_closed(&events[i].window);
                break;
            case SDL_WINDOWEVENT_FOCUS_GAINED: /* fallthru */
            case SDL_WINDOWEVENT_FOCUS_LOST:
                handle_window_focus(&events[i].window);
                break;
        }
    }

    if (resize) {
        handle_last_resize_event(num_events, events);
    }

    redraw_all_windows(false);
}

static void resize_subwindow(struct subwindow *subwindow)
{
    SDL_DestroyTexture(subwindow->texture);

    subwindow->full_rect = subwindow->sizing_rect;
    if (!adjust_subwindow_geometry(subwindow->window, subwindow)) {
        quit_fmt("bad_geometry of subwindow %u in window %u",
                subwindow->index, subwindow->window->index);
    }

    subwindow->texture = make_subwindow_texture(subwindow->window,
            subwindow->full_rect.w, subwindow->full_rect.h);

    render_clear(subwindow->window, subwindow->texture, &subwindow->color);
    render_borders(subwindow);

    term *old = Term;
    Term_activate(subwindow->term);
    Term_resize(subwindow->cols, subwindow->rows, subwindow->rows);
    /* XXX if we don't redraw the term, resizing in birth screen is buggy */
    Term_redraw();
    Term_activate(old);

    /* Dungeon size */
    if (subwindow->index == MAIN_SUBWINDOW)
        net_term_resize(subwindow->cols, subwindow->rows, subwindow->rows);

    refresh_angband_terms();
}

static void do_sizing(struct window *window, int x, int y)
{
    struct size_state *size_state = &window->size_state;

    assert(size_state->subwindow != NULL);

    SDL_Rect rect = size_state->subwindow->sizing_rect;

    int newx = x - size_state->originx;
    int newy = y - size_state->originy;

    int left   = size_state->left ? newx : 0;
    int top    = size_state->top  ? newy : 0;
    int right  = size_state->left ? 0 : newx;
    int bottom = size_state->top  ? 0 : newy;

    resize_rect(&rect, left, top, right, bottom);
    fit_rect_in_rect_by_hw(&rect, &window->inner_rect);

    if (is_ok_col_row(size_state->subwindow,
                &rect,
                size_state->subwindow->font_width,
                size_state->subwindow->font_height))
    {
        size_state->subwindow->sizing_rect = rect;
    }

    size_state->originx = x;
    size_state->originy = y;
}

static void do_moving(struct window *window, int x, int y)
{
    struct move_state *move_state = &window->move_state;

    assert(move_state->subwindow != NULL);

    SDL_Rect *rect = &move_state->subwindow->full_rect;

    rect->x += x - move_state->originx;
    rect->y += y - move_state->originy;

    try_snap(window, move_state->subwindow, rect);
    fit_rect_in_rect_by_xy(rect, &window->inner_rect);

    move_state->originx = x;
    move_state->originy = y;
}

static bool handle_menu_mousebuttonup(struct window *window,
        const SDL_MouseButtonEvent *mouse)
{
    if (window->move_state.active && window->move_state.moving) {
        window->move_state.moving = false;
    } else if (window->size_state.active && window->size_state.sizing) {
        window->size_state.sizing = false;

        if (window->size_state.subwindow != NULL) {
            resize_subwindow(window->size_state.subwindow);
        }
    } 
    
    if (window->move_state.active
            || window->size_state.active
            || is_over_status_bar(&window->status_bar, mouse->x, mouse->y))
    {
        return true;
    } else {
        return false;
    }
}

static bool handle_menu_mousemotion(struct window *window,
        const SDL_MouseMotionEvent *mouse)
{
    if (window->move_state.moving) {
        do_moving(window, mouse->x, mouse->y);
        return true;
    } else if (window->size_state.sizing) {
        do_sizing(window, mouse->x, mouse->y);
        return true;
    } else if (window->move_state.active || window->size_state.active) {
        return true;
    } else if (is_over_status_bar(&window->status_bar, mouse->x, mouse->y)) {
        return true;
    }

    return false;
}

static bool handle_menu_keyboard(struct window *window, const SDL_Event *event)
{
    if (window->move_state.active || window->size_state.active) {
        return true;
    }

    SDL_Event key = *event;
    /* the user pressed a key; probably wants to play? */
    SDL_PushEvent(&key);

    return false;
}

static bool handle_status_bar_buttons(struct window *window,
        const SDL_Event *event)
{
    bool handled = false;

    for (size_t i = 0; i < window->status_bar.button_bank.number; i++) {
        struct button *button = &window->status_bar.button_bank.buttons[i];
        assert(button->callbacks.on_event != NULL);
        handled |= button->callbacks.on_event(window, button, event);
    }

    return handled;
}

static void redraw_status_bar_buttons(struct window *window)
{
    SDL_Event shutdown = {.type = SDL_USEREVENT};
    (void) handle_status_bar_buttons(window, &shutdown);
}

static bool handle_menu_windowevent(struct window *window,
        const SDL_WindowEvent *event)
{
    if (window->move_state.active) {
        signal_move_state(window);
    } else if (window->size_state.active) {
        signal_size_state(window);
    }

    redraw_status_bar_buttons(window);

    handle_windowevent(event);

    return false;
}

static bool is_event_windowid_ok(const struct window *window, const SDL_Event *event)
{
    switch (event->type) {
        case SDL_KEYDOWN: /* fallthru */
        case SDL_KEYUP:
            return event->key.windowID == window->id;
        case SDL_TEXTINPUT:
            return event->text.windowID == window->id;
        case SDL_MOUSEMOTION:
            return event->motion.windowID == window->id;
        case SDL_MOUSEBUTTONDOWN: /* fallthru */
        case SDL_MOUSEBUTTONUP:
            return event->button.windowID == window->id;
        default:
            return true;
    }
}

/* returns true for events that should be processed by buttons */
static bool is_ok_button_event(const struct window *window, const SDL_Event *event)
{
    switch (event->type) {
        case SDL_KEYDOWN:         /* fallthru */
        case SDL_KEYUP:           /* fallthru */
        case SDL_TEXTINPUT:       /* fallthru */
            return is_event_windowid_ok(window, event);
        case SDL_MOUSEMOTION:     /* fallthru */
        case SDL_MOUSEBUTTONDOWN: /* fallthru */
        case SDL_MOUSEBUTTONUP:
            return window->focus && is_event_windowid_ok(window, event);
        case SDL_USEREVENT:
            return true;
        default:
            return false;
    }
}

static bool handle_status_bar_events(struct window *window,
        const SDL_Event *event)
{
    if (!is_event_windowid_ok(window, event)) {
        /* just in case */
        if (window->move_state.active) {
            signal_move_state(window);
        } else if (window->size_state.active) {
            signal_size_state(window);
        }
        return false;
    }

    switch (event->type) {
        case SDL_MOUSEMOTION:
            return handle_menu_mousemotion(window, &event->motion);
        case SDL_MOUSEBUTTONDOWN:
            return handle_menu_mousebuttondown(window, &event->button);
        case SDL_MOUSEBUTTONUP:
            return handle_menu_mousebuttonup(window, &event->button);
        case SDL_KEYDOWN:     /* fallthru */
        case SDL_KEYUP:       /* fallthru */
        case SDL_TEXTEDITING: /* fallthru */
        case SDL_TEXTINPUT:
            return handle_menu_keyboard(window, event);
        case SDL_WINDOWEVENT:
            return handle_menu_windowevent(window, &event->window);
        case SDL_QUIT:
            handle_quit();
            return false;
        default:
            return false;
    }
}

static void do_status_bar_loop(struct window *window)
{
    window->status_bar.in_menu = true;

    bool keep_going = true;
    while (keep_going) {
        SDL_Delay(window->delay);

        SDL_Event event;
        SDL_WaitEvent(&event);

        bool handled = false;
        if (is_ok_button_event(window, &event)
                && !window->move_state.moving
                && !window->size_state.sizing)
        {
             handled = handle_status_bar_buttons(window, &event);
        }

        if (event.type == SDL_MOUSEMOTION) {
            /* annoying mousemotion spam! */
            SDL_FlushEvent(SDL_MOUSEMOTION);
        }

        if (!handled) {
            /* so the user didnt click on a button */
            keep_going = handle_status_bar_events(window, &event);
        }

        redraw_window(window);
    }

    window->status_bar.in_menu = false;
}

static bool has_visible_subwindow(const struct window *window, unsigned index)
{
    return get_subwindow_by_index(window, index, true) != NULL;
}

static bool handle_mousemotion(const SDL_MouseMotionEvent *mouse)
{
    struct window *window = get_window_by_id(mouse->windowID);

    if (is_over_status_bar(&window->status_bar, mouse->x, mouse->y)) {
        do_status_bar_loop(window);
    }
    /* dont need other mousemotion events */
    SDL_FlushEvent(SDL_MOUSEMOTION);

    return false;
}

/* x and y are relative to window */
static bool get_colrow_from_xy(const struct subwindow *subwindow,
        int x, int y, int *col, int *row)
{
    SDL_Rect rect = {
        subwindow->full_rect.x + subwindow->inner_rect.x,
        subwindow->full_rect.y + subwindow->inner_rect.y,
        subwindow->inner_rect.w,
        subwindow->inner_rect.h
    };

    if (!is_point_in_rect(x, y, &rect)) {
        return false;
    }

    *col = (x - rect.x) / subwindow->font_width;
    *row = (y - rect.y) / subwindow->font_height;

    return true;
}

static uint8_t translate_key_mods(Uint16 mods)
{
#define TRANSLATE_K_MOD(m, k) ((m) & mods ? (k) : 0)
    uint8_t angband_mods =
        TRANSLATE_K_MOD(KMOD_SHIFT, KC_MOD_SHIFT)
        | TRANSLATE_K_MOD(KMOD_CTRL, KC_MOD_CONTROL)
        | TRANSLATE_K_MOD(KMOD_ALT, KC_MOD_ALT)
        | TRANSLATE_K_MOD(KMOD_GUI, KC_MOD_META);
#undef TRANSLATE_K_MOD
    return angband_mods;
}

static bool handle_mousebuttondown(const SDL_MouseButtonEvent *mouse)
{
    struct window *window = get_window_by_id(mouse->windowID);
    assert(window != NULL);

    struct subwindow *subwindow = get_subwindow_by_xy(window, mouse->x, mouse->y);
    if (subwindow == NULL) {
        /* not an error, the user clicked in some random place */
        return false;
    } else if (!subwindow->top) {
        bring_to_top(window, subwindow);
        redraw_window(window);
        return false;
    }

    /* terms that are not main do not react to events, and main term
     * lives in main window */
    if (window->index != MAIN_WINDOW) {
        return false;
    }

    /* all magic numbers are from ui-term.c and ui-context.c :) */

    int button;
    switch (mouse->button) {
        case SDL_BUTTON_LEFT:
            button = 1;
            break;
        case SDL_BUTTON_RIGHT:
            button = 2;
            break;
        default:
            /* XXX other buttons? */
            return false;
    }

    int col;
    int row;
    if (!get_colrow_from_xy(subwindow, mouse->x, mouse->y, &col, &row)) {
        return false;
    }

    uint8_t mods = translate_key_mods(SDL_GetModState());
    /* apparently mouse buttons dont get this */
    mods &= ~KC_MOD_META;

    button |= mods << 4;

    term *old = Term;
    Term_activate(subwindow->term);
//    Term_mousepress(col, row, button);
    Term_activate(old);

    return true;
}

static bool handle_keydown(const SDL_KeyboardEvent *key)
{
    uint8_t mods = translate_key_mods(key->keysym.mod);
    keycode_t ch = 0;

    /* SDL will give us both keydown and text input events in many cases.
     * Between this function and handle_text_input we need to make sure that
     * Term_keypress gets called exactly once for a given key press from the
     * user.
     * This function handles keys that don't produce text, and, if
     * g_kp_as_mod is true, the keypad and keypresses that will produce the
     * same characters as keypad keypresses.
     * Others should be handled in handle_text_input.
     */

    switch (key->keysym.sym) {
        /* arrow keys */
        case SDLK_UP:          ch = ARROW_UP;                        break;
        case SDLK_DOWN:        ch = ARROW_DOWN;                      break;
        case SDLK_LEFT:        ch = ARROW_LEFT;                      break;
        case SDLK_RIGHT:       ch = ARROW_RIGHT;                     break;
        /* text editing keys */
        case SDLK_BACKSPACE:   ch = KC_BACKSPACE;                    break;
        case SDLK_PAGEDOWN:    ch = KC_PGDOWN;                       break;
        case SDLK_PAGEUP:      ch = KC_PGUP;                         break;
        case SDLK_INSERT:      ch = KC_INSERT;                       break;
        case SDLK_DELETE:      ch = KC_DELETE;                       break;
        case SDLK_RETURN:      ch = KC_ENTER;                        break;
        case SDLK_ESCAPE:      ch = ESCAPE;                          break;
        case SDLK_HOME:        ch = KC_HOME;                         break;
        case SDLK_END:         ch = KC_END;                          break;
        case SDLK_TAB:         ch = KC_TAB;                          break;
        /* function keys */
        case SDLK_F1:          ch = KC_F1;                           break;
        case SDLK_F2:          ch = KC_F2;                           break;
        case SDLK_F3:          ch = KC_F3;                           break;
        case SDLK_F4:          ch = KC_F4;                           break;
        case SDLK_F5:          ch = KC_F5;                           break;
        case SDLK_F6:          ch = KC_F6;                           break;
        case SDLK_F7:          ch = KC_F7;                           break;
        case SDLK_F8:          ch = KC_F8;                           break;
        case SDLK_F9:          ch = KC_F9;                           break;
        case SDLK_F10:         ch = KC_F10;                          break;
        case SDLK_F11:         ch = KC_F11;                          break;
        case SDLK_F12:         ch = KC_F12;                          break;
        case SDLK_F13:         ch = KC_F13;                          break;
        case SDLK_F14:         ch = KC_F14;                          break;
        case SDLK_F15:         ch = KC_F15;                          break;
    }

    if (g_kp_as_mod) {
        /* If numlock is set and shift is not pressed, numpad numbers
         * produce regular numbers and not keypad numbers */
        uint8_t keypad_num_mod = ((key->keysym.mod & KMOD_NUM) && !(key->keysym.mod & KMOD_SHIFT))
            ? 0x00 : KC_MOD_KEYPAD;

        switch (key->keysym.sym) {
            /* Keypad */
            case SDLK_KP_0: ch = '0'; mods |= keypad_num_mod; break;
            case SDLK_KP_1: ch = '1'; mods |= keypad_num_mod; break;
            case SDLK_KP_2: ch = '2'; mods |= keypad_num_mod; break;
            case SDLK_KP_3: ch = '3'; mods |= keypad_num_mod; break;
            case SDLK_KP_4: ch = '4'; mods |= keypad_num_mod; break;
            case SDLK_KP_5: ch = '5'; mods |= keypad_num_mod; break;
            case SDLK_KP_6: ch = '6'; mods |= keypad_num_mod; break;
            case SDLK_KP_7: ch = '7'; mods |= keypad_num_mod; break;
            case SDLK_KP_8: ch = '8'; mods |= keypad_num_mod; break;
            case SDLK_KP_9: ch = '9'; mods |= keypad_num_mod; break;

            case SDLK_KP_MULTIPLY:
                ch = '*'; mods |= KC_MOD_KEYPAD; break;
            case SDLK_KP_PERIOD:
                ch = '.'; mods |= KC_MOD_KEYPAD; break;
            case SDLK_KP_DIVIDE:
                ch = '/'; mods |= KC_MOD_KEYPAD; break;
            case SDLK_KP_EQUALS:
                ch = '='; mods |= KC_MOD_KEYPAD; break;
            case SDLK_KP_MINUS:
                ch = '-'; mods |= KC_MOD_KEYPAD; break;
            case SDLK_KP_PLUS:
                ch = '+'; mods |= KC_MOD_KEYPAD; break;
            case SDLK_KP_ENTER:
                ch = KC_ENTER; mods |= KC_MOD_KEYPAD; break;

            /* Keys that produce the same character as keypad keys */
            case SDLK_ASTERISK: ch = '*'; break;
            case SDLK_PLUS: ch = '+'; break;
        }
        if((mods & KC_MOD_SHIFT)) {
            bool match = true;
            switch(key->keysym.sym) {
                /* Doesn't match every keyboard layout, unfortunately. */
                case SDLK_8: ch = '*'; break;
                case SDLK_EQUALS: ch = '+'; break;
                default: match = false;
            }
            if(match) {
                mods &= ~KC_MOD_SHIFT;
            }
        } else {
            switch(key->keysym.sym) {
                case SDLK_0: ch = '0'; break;
                case SDLK_1: ch = '1'; break;
                case SDLK_2: ch = '2'; break;
                case SDLK_3: ch = '3'; break;
                case SDLK_4: ch = '4'; break;
                case SDLK_5: ch = '5'; break;
                case SDLK_6: ch = '6'; break;
                case SDLK_7: ch = '7'; break;
                case SDLK_8: ch = '8'; break;
                case SDLK_9: ch = '9'; break;
                case SDLK_SLASH: ch = '/'; break;
                case SDLK_EQUALS: ch = '='; break;
                case SDLK_PERIOD: ch = '.'; break;
                case SDLK_MINUS: ch = '-'; break;
            }
        }
    } else if (key->keysym.sym == SDLK_KP_ENTER) {
        ch = KC_ENTER;
    }

    /* encode control */
    if (mods & KC_MOD_CONTROL) {
        bool match = true;
        switch (key->keysym.sym) {
            case SDLK_LEFTBRACKET:  ch = KTRL('[');  break;
            case SDLK_RIGHTBRACKET: ch = KTRL(']');  break;
            case SDLK_BACKSLASH:    ch = KTRL('\\'); break;
            case SDLK_a:            ch = KTRL('A'); break;
            case SDLK_b:            ch = KTRL('B'); break;
            case SDLK_c:            ch = KTRL('C'); break;
            case SDLK_d:            ch = KTRL('D'); break;
            case SDLK_e:            ch = KTRL('E'); break;
            case SDLK_f:            ch = KTRL('F'); break;
            case SDLK_g:            ch = KTRL('G'); break;
            case SDLK_h:            ch = KTRL('H'); break;
            case SDLK_i:            ch = KTRL('I'); break;
            case SDLK_j:            ch = KTRL('J'); break;
            case SDLK_k:            ch = KTRL('K'); break;
            case SDLK_l:            ch = KTRL('L'); break;
            case SDLK_m:            ch = KTRL('M'); break;
            case SDLK_n:            ch = KTRL('N'); break;
            case SDLK_o:            ch = KTRL('O'); break;
            case SDLK_p:            ch = KTRL('P'); break;
            case SDLK_q:            ch = KTRL('Q'); break;
            case SDLK_r:            ch = KTRL('R'); break;
            case SDLK_s:            ch = KTRL('S'); break;
            case SDLK_t:            ch = KTRL('T'); break;
            case SDLK_u:            ch = KTRL('U'); break;
            case SDLK_v:            ch = KTRL('V'); break;
            case SDLK_w:            ch = KTRL('W'); break;
            case SDLK_x:            ch = KTRL('X'); break;
            case SDLK_y:            ch = KTRL('Y'); break;
            case SDLK_z:            ch = KTRL('Z'); break;
            default: match = false;
        }
        if(match) {
            mods &= ~KC_MOD_CONTROL;
        }
    }


    if (ch) {
        Term_keypress(ch, mods);
        return true;
    } else {
        return false;
    }
}

static keycode_t utf8_to_codepoint(const char *utf8_string)
{
    /* hex  == binary
     * 0x00 == 00000000
     * 0x80 == 10000000
     * 0xc0 == 11000000
     * 0xe0 == 11100000
     * 0xf0 == 11110000
     * 0xf8 == 11111000
     * 0x3f == 00111111
     * 0x1f == 00011111
     * 0x0f == 00001111
     * 0x07 == 00000111 */

    keycode_t key = 0;

#define IS_UTF8_INFO(mask, result) (((unsigned char) utf8_string[0] & (mask)) == (result))
#define EXTRACT_UTF8_INFO(pos, mask, shift) (((unsigned char) utf8_string[(pos)] & (mask)) << (shift))
    /* 6 is the number of information bits in a utf8 continuation byte (10xxxxxx) */
    if (IS_UTF8_INFO(0x80, 0)) {
        key = utf8_string[0];
    } else if (IS_UTF8_INFO(0xe0, 0xc0)) {
        key = EXTRACT_UTF8_INFO(0, 0x1f, 6)
            | EXTRACT_UTF8_INFO(1, 0x3f, 0);
    } else if (IS_UTF8_INFO(0xf0, 0xe0)) {
        key = EXTRACT_UTF8_INFO(0, 0x0f, 12)
            | EXTRACT_UTF8_INFO(1, 0x3f, 6)
            | EXTRACT_UTF8_INFO(2, 0x3f, 0);
    } else if (IS_UTF8_INFO(0xf8, 0xf0)) {
        key = EXTRACT_UTF8_INFO(0, 0x07, 18)
            | EXTRACT_UTF8_INFO(1, 0x3f, 12)
            | EXTRACT_UTF8_INFO(2, 0x3f, 6)
            | EXTRACT_UTF8_INFO(3, 0x3f, 0);
    }
#undef IS_UTF8_INFO
#undef EXTRACT_UTF8_INFO

    return key;
}

static bool handle_text_input(const SDL_TextInputEvent *input)
{
    keycode_t ch = utf8_to_codepoint(input->text);

    /* Don't handle any characters that can be produced by the keypad if
     * they were handled in handle_keydown */
    if (g_kp_as_mod) {
        switch (ch) {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            case '/':
            case '*':
            case '-':
            case '+':
            case '.':
            case '=':
                return false;
        }
    }

    uint8_t mods = translate_key_mods(SDL_GetModState());

    /* Shift is already encoded in characters we receive here */
    if (!MODS_INCLUDE_SHIFT(ch)) {
        mods &= ~KC_MOD_SHIFT;
    }

    Term_keypress(ch, mods);

    return true;
}

static void wait_anykey(void)
{
    SDL_Event event;

    SDL_EventType expected = SDL_USEREVENT;
    while (true) {
        SDL_WaitEvent(&event);
        if (event.type == expected) {
            return;
        }

        switch (event.type) {
            case SDL_KEYDOWN:
                expected = SDL_KEYUP;
                break;;
            case SDL_MOUSEBUTTONDOWN:
                expected = SDL_MOUSEBUTTONUP;
                break;
            case SDL_MOUSEMOTION:
                SDL_FlushEvent(SDL_MOUSEMOTION);
                break;
            case SDL_QUIT:
                handle_quit();
                break;
            case SDL_WINDOWEVENT:
                handle_windowevent(&event.window);
                return;
        }
    }
}

static void handle_quit(void)
{
//    /* XXX copied from main-sdl.c */
//    if (character_generated && inkey_flag) {
//         /* no idea what that does :) */
//         msg_flag = false;
//         save_game();
//     }

    quit(NULL);
}

static bool get_event(void)
{
    SDL_Event event;
    if (!SDL_PollEvent(&event)) {
        return false;
    }

    switch (event.type) {
        case SDL_KEYDOWN:
            return handle_keydown(&event.key);
        case SDL_TEXTINPUT:
            return handle_text_input(&event.text);
        case SDL_MOUSEMOTION:
            return handle_mousemotion(&event.motion);
        case SDL_MOUSEBUTTONDOWN:
            return handle_mousebuttondown(&event.button);
        case SDL_WINDOWEVENT:
            handle_windowevent(&event.window);
            return false;
        case SDL_QUIT:
            handle_quit();
            return false;
        default:
            return false;
    }
}

static void refresh_angband_terms(void)
{
    if (!Setup.initialized) {
        return;
    }

    term *old = Term;
    Term_activate(term_screen);

    /* XXX XXX this is basically do_cmd_redraw(), just without EVENT_FLUSH_INPUT */
    {
        /* XXX XXX this works for refreshing monster's attrs */
        event_signal_point(EVENT_MAP, -1, -1);

        Term_flush();
//        verify_panel();

//        player->upkeep->notice |= (PN_COMBINE);
//        player->upkeep->update |= (PU_TORCH | PU_INVEN);
//        player->upkeep->update |= (PU_BONUS | PU_HP | PU_SPELLS);
//        player->upkeep->update |= (PU_UPDATE_VIEW | PU_MONSTERS);
//        player->upkeep->redraw |= (PR_BASIC | PR_EXTRA | PR_MAP | PR_INVEN |
//                                   PR_EQUIP | PR_MESSAGE | PR_MONSTER |
//                                   PR_OBJECT | PR_MONLIST | PR_ITEMLIST);

        Term_clear();
//        handle_stuff(player);
//        move_cursor_relative(player->grid.x, player->grid.y);

        Term_redraw_all();
    }

    /* Redraw */
    do_cmd_redraw();

    Term_activate(old);

    redraw_all_windows(false);
}

static errr term_xtra_event(int v)
{
    struct subwindow *subwindow = Term->data;
    assert(subwindow != NULL);

    redraw_all_windows(true);

    if (v) {
        while (true) {
            for (int i = 0; i < DEFAULT_IDLE_UPDATE_PERIOD; i++) {
                if (get_event()) {
                    return 0;
                }
                SDL_Delay(subwindow->window->delay);
            }
//            idle_update();
        }
    } else {
        (void) get_event();
    }

    return 0;
}

static errr term_xtra_flush(void)
{
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_WINDOWEVENT:
                handle_windowevent(&event.window);
                break;
        }
    }

    return 0;
}

static errr term_xtra_clear(void)
{
    struct subwindow *subwindow = Term->data;
    assert(subwindow != NULL);

    render_fill_rect(subwindow->window,
            subwindow->texture, &subwindow->inner_rect, &subwindow->color);

    subwindow->window->dirty = true;

    return 0;
}

static errr term_xtra_fresh(void)
{
    struct subwindow *subwindow = Term->data;
    assert(subwindow != NULL);

    if (!subwindow->window->status_bar.in_menu) {
        try_redraw_window(subwindow->window);
    }

    return 0;
}

static errr term_xtra_delay(int v)
{
    if (v > 0) {
        term_xtra_event(0);
        SDL_Delay(v);
    }

    return 0;
}

static errr term_xtra_react(void)
{
    init_colors();

    return 0;
}

static errr term_xtra_hook(int n, int v)
{
    switch (n) {
        case TERM_XTRA_EVENT:
            return term_xtra_event(v);
        case TERM_XTRA_DELAY:
            return term_xtra_delay(v);
        case TERM_XTRA_FLUSH:
            return term_xtra_flush();
        case TERM_XTRA_CLEAR:
            return term_xtra_clear();
        case TERM_XTRA_FRESH:
            return term_xtra_fresh();
        case TERM_XTRA_REACT:
            return term_xtra_react();
        default:
            return 0;
    }
}

static errr term_curs_hook(int col, int row)
{
    struct subwindow *subwindow = Term->data;
    assert(subwindow != NULL);

    /* Highlight the player */
    if (Term->minimap_active && cursor_x && cursor_y)
    {
        col = cursor_x + COL_MAP;
        row = cursor_y + ROW_MAP;
    }

    render_cursor(subwindow, col, row, false);

    subwindow->window->dirty = true;

    return 0;
}

static errr term_bigcurs_hook(int col, int row)
{
    struct subwindow *subwindow = Term->data;
    assert(subwindow != NULL);

    render_cursor(subwindow, col, row, true);

    subwindow->window->dirty = true;

    return 0;
}

static errr term_wipe_hook(int col, int row, int n)
{
    struct subwindow *subwindow = Term->data;
    assert(subwindow != NULL);

    SDL_Rect rect = {
        subwindow->inner_rect.x + col * subwindow->font_width,
        subwindow->inner_rect.y + row * subwindow->font_height,
        n * subwindow->font_width,
        subwindow->font_height
    };

    render_fill_rect(subwindow->window, subwindow->texture, &rect, &subwindow->color);

    subwindow->window->dirty = true;

    return 0;
}

static errr term_text_hook(int col, int row, int n, uint16_t a, const char *s)
{
    struct subwindow *subwindow = Term->data;
    assert(subwindow != NULL);

    SDL_Color fg = g_colors[a % MAX_COLORS];
    SDL_Color bg;

    switch (a / MAX_COLORS) {
        case BG_BLACK:
            bg = subwindow->color;
            break;
        case BG_SAME:
            bg = fg;
            break;
        case BG_DARK:
            bg = g_colors[DEFAULT_SHADE_COLOR];
            break;
        default:
            /* debugging */
            bg = g_colors[DEFAULT_ERROR_COLOR];
            break;
    }

    bg.a = subwindow->color.a;

    SDL_Rect rect = {
        subwindow->inner_rect.x + col * subwindow->font_width,
        subwindow->inner_rect.y + row * subwindow->font_height,
        n * subwindow->font_width,
        subwindow->font_height
    };

    render_fill_rect(subwindow->window, subwindow->texture, &rect, &bg);

    rect.w = subwindow->font_width;
    for (int i = 0; i < n; i++) {
        render_glyph_mono(subwindow->window,
                subwindow->font, subwindow->texture,
                rect.x, rect.y, &fg, (uint32_t) s[i]);
        rect.x += subwindow->font_width;
    }

    subwindow->window->dirty = true;

    return 0;
}

static errr term_pict_hook(int col, int row, int n,
        const uint16_t *ap, const char *cp, const uint16_t *tap, const char *tcp)
{
    struct subwindow *subwindow = Term->data;
    assert(subwindow != NULL);

    assert(subwindow->window->graphics.texture != NULL);

    for (int i = 0; i < n; i++) {
        render_tile_font_scaled(subwindow, col + i, row, tap[i], tcp[i], true);

        if (tap[i] == ap[i] && tcp[i] == cp[i]) {
            continue;
        }

        render_tile_font_scaled(subwindow, col + i, row, ap[i], cp[i], false);
    }

    subwindow->window->dirty = true;

    return 0;
}

static void term_view_map_shared(struct subwindow *subwindow,
        SDL_Texture *map, int w, int h)
{
    render_all(subwindow->window);

    SDL_Rect dst = {
        0, 0,
        w + 2 * DEFAULT_VISIBLE_BORDER,
        h + 2 * DEFAULT_VISIBLE_BORDER
    };
    SDL_Rect full = {
        0, 0,
        subwindow->window->full_rect.w,
        subwindow->window->full_rect.h
    };
    fit_rect_in_rect_proportional(&dst, &full);

    dst.x = (subwindow->window->full_rect.w - dst.w) / 2;
    dst.y = (subwindow->window->full_rect.h - dst.h) / 2;
    resize_rect(&dst,
            DEFAULT_VISIBLE_BORDER, DEFAULT_VISIBLE_BORDER,
            -DEFAULT_VISIBLE_BORDER, -DEFAULT_VISIBLE_BORDER);

    render_all(subwindow->window);
    SDL_SetRenderTarget(subwindow->window->renderer, NULL);
    SDL_RenderCopy(subwindow->window->renderer, map, NULL, &dst);

    /* render borders around the whole map */
    resize_rect(&dst,
            -DEFAULT_VISIBLE_BORDER, -DEFAULT_VISIBLE_BORDER,
            DEFAULT_VISIBLE_BORDER, DEFAULT_VISIBLE_BORDER);
    render_outline_rect_width(subwindow->window,
            NULL, &dst, &subwindow->borders.color, DEFAULT_VISIBLE_BORDER);

    SDL_RenderPresent(subwindow->window->renderer);

    wait_anykey();
}

static void term_view_map_tile(struct subwindow *subwindow)
{
    assert(subwindow->window->graphics.id != GRAPHICS_NONE);

    SDL_Rect tile = {
        0, 0, REASONABLE_MAP_TILE_WIDTH, REASONABLE_MAP_TILE_HEIGHT
    };
    SDL_Rect source = {
        0, 0,
        subwindow->window->graphics.tile_pixel_w,
        subwindow->window->graphics.tile_pixel_h
    };

    fit_rect_in_rect_proportional(&tile, &source);

    int w = tile.w * (subwindow->cols - (COL_MAP + 1));
    int h = tile.h * (subwindow->rows - (ROW_MAP + 1));

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

    SDL_Texture *map = make_subwindow_texture(subwindow->window, w, h);
    assert(map != NULL);

    render_clear(subwindow->window, map, &subwindow->color);

    for (int y = 0; y < subwindow->rows - (ROW_MAP + 1); y++) {
        tile.y = y * tile.w;
        for (int x = 0; x < subwindow->cols - (COL_MAP + 1); x++) {
            tile.x = x * tile.h;
            render_grid_cell_tile(subwindow, map, tile, x, y);
        }
    }

    SDL_Rect cursor = {cursor_x * tile.w, cursor_y * tile.h, tile.w,
                       tile.h};

    /* render cursor around player */
    render_outline_rect_width(subwindow->window,
            map, &cursor, &g_colors[DEFAULT_SUBWINDOW_CURSOR_COLOR],
            /* XXX some arbitrary values that look ok at the moment */
            MIN(MIN(tile.w / 4, tile.h / 4),
                DEFAULT_VISIBLE_BORDER));

    term_view_map_shared(subwindow, map, w, h);

    SDL_DestroyTexture(map);

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
}

static void term_view_map_text(struct subwindow *subwindow)
{
    int w = subwindow->font_width * (subwindow->cols - (COL_MAP + 1));
    int h = subwindow->font_height * (subwindow->rows - (ROW_MAP + 1));

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");

    SDL_Texture *map = make_subwindow_texture(subwindow->window, w, h);
    assert(map != NULL);

    render_clear(subwindow->window, map, &subwindow->color);

    for (int y = 0; y < subwindow->rows - (ROW_MAP + 1); y++) {
        for (int x = 0; x < subwindow->cols - (COL_MAP + 1); x++) {
            render_grid_cell_text(subwindow, map, x, y);
        }
    }

    SDL_Rect cursor = {
        cursor_x * subwindow->font_width,
        cursor_y * subwindow->font_height,
        subwindow->font_width,
        subwindow->font_height
    };

    /* render cursor around player */
    render_outline_rect_width(subwindow->window,
            map, &cursor, &g_colors[DEFAULT_SUBWINDOW_CURSOR_COLOR],
            /* XXX some arbitrary values that look reasonable at the moment */
            MIN(MIN(subwindow->font_width / 4,
                    subwindow->font_height / 4),
                DEFAULT_VISIBLE_BORDER));

    term_view_map_shared(subwindow, map, w, h);

    SDL_DestroyTexture(map);

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
}

static void term_view_map_hook(term *terminal)
{
    struct subwindow *subwindow = terminal->data;

    subwindow->term->view_map_hook = NULL;
    /* do_cmd_view_map(); waiting for a keypress inkey_ex();*/
    do_cmd_view_map_w();
    subwindow->term->view_map_hook = term_view_map_hook;

    if (subwindow->window->graphics.id == GRAPHICS_NONE) {
        term_view_map_text(subwindow);
    } else {
        term_view_map_tile(subwindow);
    }
}

static SDL_Texture *load_image(const struct window *window, const char *path)
{
    SDL_Surface *surface = IMG_Load(path);
    if (surface == NULL) {
        quit_fmt("cannot load image '%s': %s", path, IMG_GetError());
    }
    SDL_Texture *texture = SDL_CreateTextureFromSurface(window->renderer, surface);
    if (texture == NULL) {
        quit_fmt("cannot create texture from image '%s': %s", path, SDL_GetError());
    }
    SDL_FreeSurface(surface);

    return texture;
}

static void load_wallpaper(struct window *window, const char *path)
{
    if (window->wallpaper.mode == WALLPAPER_DONT_SHOW) {
        return;
    }
    if (window->wallpaper.mode == WALLPAPER_INVALID) {
        quit_fmt("invalid wallpaper mode in window %u", window->index);
    }

    SDL_Texture *wallpaper = load_image(window, path);
    assert(wallpaper != NULL);

    if (window->wallpaper.mode == WALLPAPER_TILED) {
        int w;
        int h;
        SDL_QueryTexture(wallpaper, NULL, NULL, &w, &h);

        SDL_Rect dst = {0, 0, w, h};

        /* make wallpaper 1/4 to 1/2 of the window's width and height
         * that would be 4 to 16 calls to RenderCopy in wallpaper rendering function */
        while (w < window->inner_rect.w / 4) {
            w *= 2;
        }
        while (h < window->inner_rect.h / 4) {
            h *= 2;
        }
        window->wallpaper.texture = make_subwindow_texture(window, w, h);
        window->wallpaper.w = w;
        window->wallpaper.h = h;

        SDL_Color color = {0};
        render_clear(window, window->wallpaper.texture, &color);

        for (dst.y = 0; dst.y < h; dst.y += dst.h) {
            for (dst.x = 0; dst.x < w; dst.x += dst.w) {
                SDL_RenderCopy(window->renderer, wallpaper, NULL, &dst);
            }
        }
        SDL_DestroyTexture(wallpaper);
    } else {
        SDL_QueryTexture(wallpaper, NULL, NULL,
                &window->wallpaper.w, &window->wallpaper.h);
        window->wallpaper.texture = wallpaper;
    }
}

static void load_default_wallpaper(struct window *window)
{
    if (window->wallpaper.mode == WALLPAPER_DONT_SHOW) {
        return;
    }

    char path[4096];
    path_build(path, sizeof(path), DEFAULT_WALLPAPER_DIR, DEFAULT_WALLPAPER);

    load_wallpaper(window, path);
}

static void load_default_window_icon(const struct window *window)
{
    char path[4096];
    path_build(path, sizeof(path), DEFAULT_WINDOW_ICON_DIR, DEFAULT_WINDOW_ICON);

    SDL_Surface *surface = IMG_Load(path);
    assert(surface != NULL);

    SDL_SetWindowIcon(window->window, surface);

    SDL_FreeSurface(surface);
}

static void load_graphics(struct window *window, graphics_mode *mode)
{
    assert(window->graphics.texture == NULL);

    current_graphics_mode = mode;
    use_graphics = mode->grafID;

    if (use_graphics == GRAPHICS_NONE) {
        tile_width = 1;
        tile_height = 1;
        tile_distorted = false;
    } else {
        char path[4096];
        path_build(path, sizeof(path), mode->path, mode->file);
        if (!file_exists(path)) {
            quit_fmt("cannot load graphics: file '%s' doesnt exist", path);
        }

        window->graphics.texture = load_image(window, path);
        assert(window->graphics.texture != NULL);

        window->graphics.tile_pixel_w = mode->cell_width;
        window->graphics.tile_pixel_h = mode->cell_height;

        window->graphics.overdraw_row = mode->overdrawRow;
        window->graphics.overdraw_max = mode->overdrawMax;

        /* Use ASCII symbol for distorted tiles */
        tile_distorted = is_tile_distorted(use_graphics, tile_width, tile_height);
    }

    if (Setup.initialized) {
        reset_visuals(true);
    }

    window->graphics.id = mode->grafID;
}

static void reload_all_graphics(graphics_mode *mode)
{
    if (mode == NULL) {
        return;
    }

    for (size_t i = 0; i < MAX_WINDOWS; i++) {
        struct window *window = get_window_direct(i);
        if (window == NULL) {
            continue;
        }
        
        free_graphics(&window->graphics);
        memset(&window->graphics, 0, sizeof(window->graphics));
        window->graphics.texture = NULL;

        load_graphics(window, mode);
    }
}

static const struct font_info *find_font_info(const char *name)
{
    for (size_t i = 0; i < N_ELEMENTS(g_font_info); i++) {
        if (g_font_info[i].loaded && streq(g_font_info[i].name, name)) {
            return &g_font_info[i];
        }
    }

    return NULL;
}

static void make_font_cache(const struct window *window, struct font *font)
{
    font->cache.texture = make_subwindow_texture(window,
            (int) ASCII_CACHE_SIZE * font->ttf.glyph.w, font->ttf.glyph.h);
    assert(font->cache.texture != NULL);
        
    /* fill texture with white transparent pixels */
    SDL_Color white = {0xFF, 0xFF, 0xFF, 0};
    render_clear(window, font->cache.texture, &white);
    /* restore the alpha; we will render glyphs in white */
    white.a = 0xFF;

    const int glyph_w = font->ttf.glyph.w;
    const int glyph_h = font->ttf.glyph.h;

    for (size_t i = 0; i < ASCII_CACHE_SIZE; i++) {
        SDL_Surface *surface = TTF_RenderGlyph_Blended(font->ttf.handle,
                (Uint16) g_ascii_codepoints_for_cache[i], white);
        if (surface == NULL) {
            quit_fmt("font cache rendering failed for '%c' (ASCII %lu) in font '%s': %s",
                g_ascii_codepoints_for_cache[i], (unsigned long) i, font->name, TTF_GetError());
        }

        SDL_Texture *texture = SDL_CreateTextureFromSurface(window->renderer, surface);
        if (texture == NULL)
            quit_fmt("cannot create texture for cache in font '%s': %s", font->name, SDL_GetError());

        SDL_Rect src = {0, 0, surface->w, surface->h};
        SDL_Rect dst = {glyph_w * i, 0, glyph_w, glyph_h};

        crop_rects(&src, &dst);

        SDL_RenderCopy(window->renderer, texture, &src, &dst);

        font->cache.rects[i] = dst;

        SDL_FreeSurface(surface);
        SDL_DestroyTexture(texture);
    }
}

static struct font *make_font(const struct window *window,
        const char *name, int size)
{
    const struct font_info *info = find_font_info(name);
    if (info == NULL) {
        return NULL;
    }

    struct font *font = mem_zalloc(sizeof(*font));

    font->index = info->index;
    font->path = string_make(info->path);
    font->name = string_make(info->name);
    font->size = size;

    font->cache.texture = NULL;

    load_font(font);
    make_font_cache(window, font);

    return font;
}

static bool reload_font(struct subwindow *subwindow,
        const struct font_info *info)
{
    struct font *new_font = make_font(subwindow->window, info->name, info->size);
    if (new_font == NULL) {
        return false;
    }

    subwindow->sizing_rect = subwindow->full_rect;
    if (!is_ok_col_row(subwindow,
                &subwindow->sizing_rect, new_font->ttf.glyph.w, new_font->ttf.glyph.h))
    {
        int min_cols = subwindow->index == MAIN_SUBWINDOW ? MIN_COLS_MAIN : MIN_COLS_OTHER;
        int min_rows = subwindow->index == MAIN_SUBWINDOW ? MIN_ROWS_MAIN : MIN_ROWS_OTHER;

        subwindow->sizing_rect.w = min_cols * new_font->ttf.glyph.w + 2 * DEFAULT_BORDER;
        subwindow->sizing_rect.h = min_rows * new_font->ttf.glyph.h + 2 * DEFAULT_BORDER;
    }

    if (subwindow->sizing_rect.w > subwindow->window->inner_rect.w
            || subwindow->sizing_rect.h > subwindow->window->inner_rect.h)
    {
        free_font(new_font);
        memset(&subwindow->sizing_rect, 0, sizeof(subwindow->sizing_rect));

        return false;
    }

    fit_rect_in_rect_by_xy(&subwindow->sizing_rect, &subwindow->window->inner_rect);

    free_font(subwindow->font);
    subwindow->font = new_font;

    resize_subwindow(subwindow);

    return true;
}

static void load_font(struct font *font)
{
    assert(font != NULL);
    assert(font->path != NULL);

    font->ttf.handle = TTF_OpenFont(font->path, font->size);
    if (font->ttf.handle == NULL) {
        quit_fmt("cannot open font '%s': %s", font->path, TTF_GetError());
    }

    font->ttf.glyph.h = TTF_FontHeight(font->ttf.handle);

    if (TTF_GlyphMetrics(font->ttf.handle, GLYPH_FOR_ADVANCE,
                NULL, NULL, NULL, NULL, &font->ttf.glyph.w) != 0)
    {
        quit_fmt("cannot query glyph metrics for font '%s': %s",
                font->path, TTF_GetError());
    }

    TTF_SetFontHinting(font->ttf.handle, DEFAULT_FONT_HINTING);
}

static void free_font(struct font *font)
{
    if (font->name != NULL) {
        mem_free(font->name);
    }
    if (font->path != NULL) {
        mem_free(font->path);
    }
    if (font->ttf.handle != NULL) {
        TTF_CloseFont(font->ttf.handle);
    }
    if (font->cache.texture != NULL) {
        SDL_DestroyTexture(font->cache.texture);
    }

    mem_free(font);
}

static bool is_ok_col_row(const struct subwindow *subwindow,
        const SDL_Rect *rect, int cell_w, int cell_h)
{
    const int min_col =
        subwindow->index == MAIN_SUBWINDOW ? MIN_COLS_MAIN : MIN_COLS_OTHER;
    const int min_row =
        subwindow->index == MAIN_SUBWINDOW ? MIN_ROWS_MAIN : MIN_ROWS_OTHER;

    if ((rect->w - DEFAULT_BORDER * 2) / cell_w < min_col) {
        return false;
    }
    if ((rect->h - DEFAULT_BORDER * 2) / cell_h < min_row) {
        return false;
    }

    return true;
}

static bool adjust_subwindow_geometry(const struct window *window,
        struct subwindow *subwindow)
{
    if (!subwindow->loaded && subwindow->config == NULL) {
        /* lets center it then */
        subwindow->full_rect.x = MAX(window->inner_rect.x,
                (window->inner_rect.w - subwindow->full_rect.w) / 2);
        subwindow->full_rect.y = MAX(window->inner_rect.y,
                (window->inner_rect.h - subwindow->full_rect.h) / 2);
    }

    /* coordinates of inner rect are relative to that of outer rect
     * (really, they are relative to subwindow's texture) */
    subwindow->inner_rect.x = 0;
    subwindow->inner_rect.y = 0;
    subwindow->inner_rect.w = subwindow->full_rect.w;
    subwindow->inner_rect.h = subwindow->full_rect.h;

    memset(&subwindow->sizing_rect, 0, sizeof(subwindow->sizing_rect));

    resize_rect(&subwindow->inner_rect,
            DEFAULT_BORDER, DEFAULT_BORDER,
            -DEFAULT_BORDER, -DEFAULT_BORDER);

    subwindow->borders.width = DEFAULT_VISIBLE_BORDER;

    subwindow->font_width = subwindow->font->ttf.glyph.w;
    subwindow->font_height = subwindow->font->ttf.glyph.h;

    subwindow->cols = subwindow->inner_rect.w / subwindow->font_width;
    subwindow->rows = subwindow->inner_rect.h / subwindow->font_height;

    subwindow->inner_rect.w = subwindow->cols * subwindow->font_width;
    subwindow->inner_rect.h = subwindow->rows * subwindow->font_height;

    subwindow->inner_rect.x = 
        (subwindow->full_rect.w - subwindow->inner_rect.w) / 2;
    subwindow->inner_rect.y =
        (subwindow->full_rect.h - subwindow->inner_rect.h) / 2;

    if (!is_ok_col_row(subwindow,
                &subwindow->full_rect, subwindow->font_width, subwindow->font_height))
    {
        return false;
    }

    if (!is_rect_in_rect(&subwindow->full_rect, &window->inner_rect)) {
        subwindow->borders.error = true;
    }

    return true;
}

static void sort_to_top_aux(struct window *window, size_t *next_subwindow,
        struct subwindow **subwindows, size_t num_subwindows,
        bool top, bool always_top)
{
    if (*next_subwindow == num_subwindows) {
        return;
    }

    for (size_t i = 0; i < N_ELEMENTS(window->subwindows); i++) {
        if (window->subwindows[i] != NULL
                && window->subwindows[i]->top == top
                && window->subwindows[i]->always_top == always_top)
        {
            subwindows[(*next_subwindow)++] = window->subwindows[i];
        }
    }

    assert(*next_subwindow <= num_subwindows);
}

static void sort_to_top(struct window *window)
{
    struct subwindow *tmp[N_ELEMENTS(window->subwindows)] = {NULL};
    assert(sizeof(window->subwindows) == sizeof(tmp));

    size_t num_subwindows = 0;
    for (size_t i = 0; i < N_ELEMENTS(window->subwindows); i++) {
        if (window->subwindows[i] != NULL) {
            num_subwindows++;
        }
    }

    /* and that's how we sort here :) */
    size_t current = 0;
    sort_to_top_aux(window, &current, tmp, num_subwindows, false, false);
    sort_to_top_aux(window, &current, tmp, num_subwindows, true,  false);
    sort_to_top_aux(window, &current, tmp, num_subwindows, false, true);
    sort_to_top_aux(window, &current, tmp, num_subwindows, true,  true);

    memcpy(window->subwindows, tmp, sizeof(window->subwindows));
}

static void bring_to_top(struct window *window, struct subwindow *subwindow)
{
    assert(subwindow->window == window);

    bool found_subwindow_in_window = false;
    for (size_t i = 0; i < N_ELEMENTS(window->subwindows); i++) {
        if (window->subwindows[i] != NULL) {
            window->subwindows[i]->top = false;
            if (window->subwindows[i] == subwindow) {
                found_subwindow_in_window = true;
            }
        }
    }
    assert(found_subwindow_in_window);

    subwindow->top = true;

    sort_to_top(window);
}

static void adjust_status_bar_geometry(struct window *window)
{
    struct status_bar *status_bar = &window->status_bar;

    status_bar->full_rect.x = 0;
    status_bar->full_rect.y = 0;
    status_bar->full_rect.w = window->full_rect.w;
    status_bar->full_rect.h = DEFAULT_LINE_HEIGHT(status_bar->font->ttf.glyph.h);
    status_bar->inner_rect = status_bar->full_rect;

    int border = (status_bar->full_rect.h - status_bar->font->ttf.glyph.h) / 2;
    resize_rect(&status_bar->inner_rect,
            border, border, -border, -border);
}

static struct subwindow *get_subwindow_by_index(const struct window *window,
        unsigned index, bool visible)
{
    for (size_t i = 0; i < N_ELEMENTS(window->subwindows); i++) {
        struct subwindow *subwindow = window->subwindows[i];

        if (subwindow != NULL
                && (visible ? subwindow->visible : true)
                && subwindow->index == index)
        {
            return subwindow;
        }
    }
    
    return NULL;
}

static struct subwindow *get_subwindow_by_xy(const struct window *window, int x, int y)
{
    /* checking subwindows in z order */
    for (size_t i = N_ELEMENTS(window->subwindows); i > 0; i--) {
        struct subwindow *subwindow = window->subwindows[i - 1];

        if (subwindow != NULL && subwindow->visible) {
            if (is_point_in_rect(x, y, &subwindow->full_rect)) {
                return subwindow;
            }
        }
    }

    return NULL;
}

static struct menu_panel *get_menu_panel_by_xy(struct menu_panel *menu_panel,
        int x, int y)
{
    while (menu_panel != NULL) {
        if (is_point_in_rect(x, y, &menu_panel->rect)) {
            return menu_panel;
        }
        menu_panel = menu_panel->next;
    }

    return NULL;
}

static bool is_over_status_bar(const struct status_bar *status_bar, int x, int y)
{
    return is_point_in_rect(x, y, &status_bar->full_rect);
}

static void make_button_bank(struct button_bank *bank)
{
    bank->buttons = mem_zalloc(sizeof(*bank->buttons) * MAX_BUTTONS);

    bank->size = MAX_BUTTONS;
    bank->number = 0;
}

static bool handle_button_open_subwindow(struct window *window,
        struct button *button, const SDL_Event *event)
{
    CHECK_BUTTON_DATA_TYPE(button, BUTTON_DATA_UNSIGNED);

    if (!click_status_bar_button(window, button, event)) {
        return false;
    }

    unsigned index = button->data.value.unsigned_value;
    struct subwindow *subwindow = NULL;
    
    subwindow = get_subwindow_by_index(window, index, false);
    if (subwindow != NULL) {
        subwindow->visible = !subwindow->visible;
        if (subwindow->visible) {
            bring_to_top(window, subwindow);
        }
    } else if (is_subwindow_loaded(index)) {
        subwindow = transfer_subwindow(window, index);
        subwindow->visible = true;
        bring_to_top(window, subwindow);
        refresh_angband_terms();
    } else {
        subwindow = make_subwindow(window, index);
        assert(subwindow != NULL);
        bring_to_top(window, subwindow);

        /* Reinitialize all subwindows */
        subwindows_reinit_flags();
        /* Set up the subwindows */
        subwindows_init_flags();

        refresh_angband_terms();
    }

    redraw_all_windows(false);

    return true;
}

static void close_status_bar_menu(struct status_bar *status_bar)
{
    if (status_bar->menu_panel != NULL) {
        free_menu_panel(status_bar->menu_panel);
        status_bar->menu_panel = NULL;
    }
}

static void make_default_status_buttons(struct status_bar *status_bar)
{
    SDL_Rect rect;
    struct button_data data;
    struct button_callbacks callbacks;

#define PUSH_BUTTON_LEFT_TO_RIGHT(cap) \
    get_string_metrics(status_bar->font, (cap), &rect.w, NULL); \
    rect.w += DEFAULT_BUTTON_BORDER * 2; \
    push_button(&status_bar->button_bank, status_bar->font, \
            (cap), data, callbacks, &rect, CAPTION_POSITION_CENTER); \
    rect.x += rect.w; \

    rect.x = status_bar->full_rect.x;
    rect.y = status_bar->full_rect.y;
    rect.w = 0;
    rect.h = status_bar->full_rect.h;

    callbacks.on_render = render_menu_button;
    callbacks.on_event = handle_menu_button;
    callbacks.on_menu = NULL;

    data.type = BUTTON_DATA_NONE;
    PUSH_BUTTON_LEFT_TO_RIGHT("Menu");

    callbacks.on_render = render_button_subwindows;

    data.type = BUTTON_DATA_UNSIGNED;

    if (status_bar->window->index == MAIN_WINDOW) {
        /* "A" button is not interactive, it's just for display */
        callbacks.on_event = ignore_status_bar_button;
        data.value.unsigned_value = MAIN_SUBWINDOW;
        /* the main term is called Angband in game options */
        PUSH_BUTTON_LEFT_TO_RIGHT("A");
    }

    callbacks.on_event = handle_button_open_subwindow;
    for (unsigned i = 1; i < N_ELEMENTS(status_bar->window->subwindows); i++) {
        data.value.unsigned_value = i;
        PUSH_BUTTON_LEFT_TO_RIGHT(format("%u", i));
    }
#undef PUSH_BUTTON_LEFT_TO_RIGHT

#define PUSH_BUTTON_RIGHT_TO_LEFT(cap) \
    get_string_metrics(status_bar->font, (cap), &rect.w, NULL); \
    rect.w += DEFAULT_BUTTON_BORDER * 2; \
    rect.x -= rect.w; \
    push_button(&status_bar->button_bank, status_bar->font, \
            (cap), data, callbacks, &rect, CAPTION_POSITION_CENTER); \

    rect.x = status_bar->full_rect.x + status_bar->full_rect.w;
    rect.y = status_bar->full_rect.y;
    rect.w = 0;
    rect.h = status_bar->full_rect.h;

    callbacks.on_render = render_button_movesize;
    callbacks.on_event = handle_button_movesize;
    callbacks.on_menu = NULL;

    data.type = BUTTON_DATA_INT;

    data.value.int_value = BUTTON_MOVESIZE_MOVING;
    PUSH_BUTTON_RIGHT_TO_LEFT("Move");

    data.value.int_value = BUTTON_MOVESIZE_SIZING;
    PUSH_BUTTON_RIGHT_TO_LEFT("Size");
#undef PUSH_BUTTON_RIGHT_TO_LEFT
}

static void reload_status_bar(struct status_bar *status_bar)
{
    close_status_bar_menu(status_bar);

    SDL_DestroyTexture(status_bar->texture);
    status_bar->texture = make_subwindow_texture(status_bar->window,
            status_bar->full_rect.w, status_bar->full_rect.h);
    assert(status_bar->texture != NULL);

    free_button_bank(&status_bar->button_bank);
    make_button_bank(&status_bar->button_bank);
    make_default_status_buttons(status_bar);

    render_status_bar(status_bar->window);
}

static void load_status_bar(struct window *window)
{
    if (window->status_bar.font == NULL) {
        if (window->config != NULL) {
            window->status_bar.font = make_font(window,
                    window->config->font_name,
                    window->config->font_size);
        } else {
            window->status_bar.font = make_font(window, DEFAULT_STATUS_BAR_FONT, 0);
        }
        assert(window->status_bar.font != NULL);
    } else {
        quit_fmt("font '%s' already loaded in status bar in window %u",
                window->status_bar.font->name, window->index);
    }

    adjust_status_bar_geometry(window);

    window->status_bar.texture = make_subwindow_texture(window,
            window->status_bar.full_rect.w, window->status_bar.full_rect.h);

    /* let's try renderer */
    if (SDL_SetRenderDrawColor(window->renderer,
                window->status_bar.color.r, window->status_bar.color.g,
                window->status_bar.color.b, window->status_bar.color.a) != 0) {
        quit_fmt("cannot set render color for status bar in window %u: %s",
                window->index, SDL_GetError());
    }
    /* well, renderer seems to work */
    if (SDL_SetRenderTarget(window->renderer, window->status_bar.texture) != 0) {
        quit_fmt("cannot set status bar texture as target in window %u: %s",
                window->index, SDL_GetError());
    }
    /* does it render? */
    if (SDL_RenderClear(window->renderer) != 0) {
        quit_fmt("cannot clear status bar texture in window %u: %s",
                window->index, SDL_GetError());
    }

    /* well, it probably works */
    window->status_bar.window = window;
}

static void fit_subwindow_in_window(const struct window *window,
        struct subwindow *subwindow)
{
    fit_rect_in_rect_by_xy(&subwindow->full_rect, &window->inner_rect);
    if (!is_rect_in_rect(&subwindow->full_rect, &window->inner_rect)) {
        subwindow->borders.error = true;
        render_borders(subwindow);
    }
}

static void resize_window(struct window *window, int w, int h)
{
    if (window->full_rect.w == w
            && window->full_rect.h == h)
    {
        return;
    }

    window->full_rect.w = w;
    window->full_rect.h = h;

    adjust_status_bar_geometry(window);
    adjust_window_geometry(window);
    
    clear_all_borders(window);
    for (size_t i = 0; i < N_ELEMENTS(window->subwindows); i++) {
        struct subwindow *subwindow = window->subwindows[i];
        if (subwindow != NULL) {
            fit_subwindow_in_window(window, subwindow);
        }
    }

    reload_status_bar(&window->status_bar);

    redraw_window(window);
}

static void adjust_window_geometry(struct window *window)
{
    window->inner_rect.x = 0;
    window->inner_rect.y = 0;
    window->inner_rect.w = window->full_rect.w;
    window->inner_rect.h = window->full_rect.h;

    resize_rect(&window->inner_rect,
            0, window->status_bar.full_rect.h, 0, 0);

    if (window->inner_rect.w <= 0
            || window->inner_rect.h <= 0) {
        quit_fmt("window %u is too small (%dx%d)",
                window->index, window->inner_rect.w, window->inner_rect.h);
    }
}

static void set_window_delay(struct window *window)
{
    assert(window->window != NULL);

    int display = SDL_GetWindowDisplayIndex(window->window);
    if (display < 0) {
        quit_fmt("cannot get display of window %u: %s",
                window->index, SDL_GetError());
    }

    SDL_DisplayMode mode;
    if (SDL_GetCurrentDisplayMode(display, &mode) != 0 ||
            mode.refresh_rate <= 0) {
        /* lets just guess; 60 fps is standard */
        mode.refresh_rate = 60;
    }
    /* delay in milliseconds; refresh rate in hz */
    window->delay = 1000 / mode.refresh_rate;
}

/* initialize miscellaneous things in window */
static void load_window(struct window *window)
{
    load_status_bar(window);
    adjust_window_geometry(window);
    make_button_bank(&window->status_bar.button_bank);
    make_default_status_buttons(&window->status_bar);
    set_window_delay(window);
    if (window->wallpaper.mode != WALLPAPER_DONT_SHOW) {
        if (window->config == NULL) {
            load_default_wallpaper(window);
        } else {
            load_wallpaper(window, window->config->wallpaper_path);
        }
    }
    load_default_window_icon(window);
    if (window->graphics.id != GRAPHICS_NONE) {
        load_graphics(window, get_graphics_mode(window->graphics.id, true));
    }

    render_clear(window, NULL, &window->color);
    render_status_bar(window);

    window->loaded = true;
}

static bool choose_pixelformat(struct window *window,
        const struct SDL_RendererInfo *info)
{
#define TRY_SET_WINDOW_PIXELFORMAT(format) \
    case SDL_PIXELFORMAT_ ##format: window->pixelformat = SDL_PIXELFORMAT_ ##format;

    for (size_t i = 0; i < info->num_texture_formats; i++) {
        switch (info->texture_formats[i]) {
            TRY_SET_WINDOW_PIXELFORMAT(ARGB8888); return true;
            TRY_SET_WINDOW_PIXELFORMAT(RGBA8888); return true;
            TRY_SET_WINDOW_PIXELFORMAT(ABGR8888); return true;
            TRY_SET_WINDOW_PIXELFORMAT(BGRA8888); return true;
        }
    }
#undef TRY_SET_WINDOW_PIXELFORMAT

    return false;
}

static void start_window(struct window *window)
{
    assert(!window->loaded);

    if (window->config == NULL) {
        window->window = SDL_CreateWindow(VERSION_NAME,
                window->full_rect.x, window->full_rect.y,
                window->full_rect.w, window->full_rect.h,
                SDL_WINDOW_FULLSCREEN_DESKTOP | SDL_WINDOW_RESIZABLE);
    } else {
        window->window = SDL_CreateWindow(VERSION_NAME,
                window->full_rect.x, window->full_rect.y,
                window->full_rect.w, window->full_rect.h,
                window->config->window_flags);
    }
    assert(window->window != NULL);

    if (window->config == NULL) {
        window->renderer = SDL_CreateRenderer(window->window,
                -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    } else {
        /* this is necessary for subwindows to have their own textures */
        window->config->renderer_flags |= SDL_RENDERER_TARGETTEXTURE;
        window->renderer = SDL_CreateRenderer(window->window,
                -1, window->config->renderer_flags);
    }
    if (window->renderer == NULL) {
        quit_fmt("cannot create renderer for window %u: %s",
                window->index, SDL_GetError());
    }

    SDL_RendererInfo info;
    if (SDL_GetRendererInfo(window->renderer, &info) != 0) {
        quit_fmt("cannot query renderer in window %u", window->index);
    }

    if (!choose_pixelformat(window, &info)) {
        quit_fmt("cannot choose pixelformat for window %u", window->index);
    }

    load_window(window);

    for (size_t i = 0; i < N_ELEMENTS(window->subwindows); i++) {
        if (window->subwindows[i] != NULL) {
            load_subwindow(window, window->subwindows[i]);
            window->subwindows[i]->visible = true;
        }
    }

    SDL_SetWindowMinimumSize(window->window,
            DEFAULT_WINDOW_MINIMUM_W, DEFAULT_WINDOW_MINIMUM_H);

    window->flags = SDL_GetWindowFlags(window->window);
    window->id = SDL_GetWindowID(window->window);
}

static void wipe_window_aux_config(struct window *window)
{
    window->config = mem_zalloc(sizeof(*window->config));
    assert(window->config != NULL);

    const struct window *main_window = get_window_direct(MAIN_WINDOW);
    assert(main_window != NULL);

    SDL_RendererInfo rinfo;
    if (SDL_GetRendererInfo(main_window->renderer, &rinfo) != 0) {
        quit_fmt("cannot get renderer info for main window: %s", SDL_GetError());
    }
    window->config->renderer_flags = rinfo.flags;
    window->config->renderer_index = -1;

    window->config->window_flags= SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;

    if (main_window->config == NULL) {
        char path[4096];
        path_build(path, sizeof(path), DEFAULT_WALLPAPER_DIR, DEFAULT_WALLPAPER);
        window->config->wallpaper_path = string_make(path);
        window->config->font_name = string_make(DEFAULT_STATUS_BAR_FONT);
    } else {
        window->config->wallpaper_path = string_make(main_window->config->wallpaper_path);
        window->config->font_name = string_make(main_window->config->font_name);
        window->config->font_size = main_window->config->font_size;
    }

    int display = SDL_GetWindowDisplayIndex(main_window->window);
    if (display < 0) {
        quit_fmt("cannot get display from main window: %s", SDL_GetError());
    }

    /* center it on the screen */
    SDL_DisplayMode mode;
    SDL_GetCurrentDisplayMode(display, &mode);
    window->full_rect.w = mode.w / 2;
    window->full_rect.h = mode.h / 2;
    window->full_rect.x = mode.w / 4;
    window->full_rect.y = mode.h / 4;

    if (current_graphics_mode != NULL) {
        window->graphics.id = current_graphics_mode->grafID;
    }
}

/* initialize window with suitable hardcoded defaults */
static void wipe_window(struct window *window, int display)
{
    unsigned index = window->index;
    memset(window, 0, sizeof(*window));
    window->index = index;

    for (size_t j = 0; j < N_ELEMENTS(window->subwindows); j++) {
        window->subwindows[j] = NULL;
    }

    SDL_DisplayMode mode;
    if (SDL_GetCurrentDisplayMode(display, &mode) != 0) {
        quit_fmt("cannot get display mode for window %u: %s",
                window->index, SDL_GetError());
    }

    window->pixelformat = SDL_PIXELFORMAT_UNKNOWN;

    window->full_rect.w = mode.w;
    window->full_rect.h = mode.h;

    window->color = g_colors[DEFAULT_WINDOW_BG_COLOR];
    window->alpha = DEFAULT_ALPHA_FULL;

    window->status_bar.font = NULL;

    window->wallpaper.texture = NULL;
    window->wallpaper.mode = WALLPAPER_TILED;

    window->status_bar.font = NULL;
    window->status_bar.color = g_colors[DEFAULT_STATUS_BAR_BG_COLOR];
    window->status_bar.button_bank.buttons = NULL;
    window->status_bar.menu_panel = NULL;
    window->status_bar.in_menu = false;

    window->graphics.texture = NULL;
    window->graphics.id = GRAPHICS_NONE;

    window->dirty = true;

    window->config = NULL;
    window->inited = true;
}

static void dump_subwindow(const struct subwindow *subwindow, ang_file *config)
{
#define DUMP_SUBWINDOW(sym, fmt, ...) \
    file_putf(config, "subwindow-" sym ":%u:" fmt "\n", subwindow->index, __VA_ARGS__)
    DUMP_SUBWINDOW("window", "%u", subwindow->window->index);
    DUMP_SUBWINDOW("full-rect", "%d:%d:%d:%d",
            subwindow->full_rect.x, subwindow->full_rect.y,
            subwindow->full_rect.w, subwindow->full_rect.h);
    DUMP_SUBWINDOW("font", "%d:%s",
            subwindow->font->size, subwindow->font->name);
    DUMP_SUBWINDOW("borders", "%s",
            subwindow->borders.visible ? "true" : "false");
    DUMP_SUBWINDOW("top", "%s:%s",
            subwindow->top ? "true" : "false",
            subwindow->always_top ? "true" : "false");
    DUMP_SUBWINDOW("alpha", "%u", subwindow->color.a);
#undef DUMP_SUBWINDOW
    file_put(config, "\n");
}

static void dump_window(const struct window *window, ang_file *config)
{
#define DUMP_WINDOW(sym, fmt, ...) \
    file_putf(config, "window-" sym ":%u:" fmt "\n", window->index, __VA_ARGS__)
    DUMP_WINDOW("display", "%d", SDL_GetWindowDisplayIndex(window->window));

    int x;
    int y;
    SDL_GetWindowPosition(window->window, &x, &y);
    DUMP_WINDOW("full-rect", "%d:%d:%d:%d",
            x, y, window->full_rect.w, window->full_rect.h);

    DUMP_WINDOW("fullscreen", "%s",
            (window->flags & SDL_WINDOW_FULLSCREEN_DESKTOP) ? "true" : "false");

    SDL_RendererInfo rinfo;
    SDL_GetRendererInfo(window->renderer, &rinfo);
    DUMP_WINDOW("renderer", "%s",
            (rinfo.flags & SDL_RENDERER_ACCELERATED) ? "hardware" : "software");

    if (window->config) {
        DUMP_WINDOW("wallpaper-path", "%s", window->config->wallpaper_path);
    } else {
        DUMP_WINDOW("wallpaper-path", "%s", "default");
    }
    DUMP_WINDOW("wallpaper-mode", "%s",
            window->wallpaper.mode == WALLPAPER_DONT_SHOW ? "none"     :
            window->wallpaper.mode == WALLPAPER_TILED     ? "tiled"    :
            window->wallpaper.mode == WALLPAPER_CENTERED  ? "centered" :
            window->wallpaper.mode == WALLPAPER_SCALED    ? "scaled"   :
            "ERROR");
    DUMP_WINDOW("status-bar-font", "%d:%s",
            window->status_bar.font->size, window->status_bar.font->name);

    DUMP_WINDOW("graphics-id", "%d", window->graphics.id);
    DUMP_WINDOW("tile-scale", "width:%d", tile_width);
    DUMP_WINDOW("tile-scale", "height:%d", tile_height);
#undef DUMP_WINDOW
    file_put(config, "\n");

    for (size_t i = 0; i < N_ELEMENTS(window->subwindows); i++) {
        struct subwindow *subwindow = window->subwindows[i];
        if (subwindow != NULL && subwindow->visible) {
            dump_subwindow(subwindow, config);
        }
    }
}

static void detach_subwindow_from_window(struct window *window,
        struct subwindow *subwindow)
{
    assert(subwindow->window == window);

    size_t i = 0;
    while (i < N_ELEMENTS(window->subwindows) && window->subwindows[i] != subwindow) {
        i++;
    }
    assert(i < N_ELEMENTS(window->subwindows));

    window->subwindows[i] = NULL;
}

static void attach_subwindow_to_window(struct window *window,
        struct subwindow *subwindow)
{
    assert(subwindow->window == window);

    size_t i = 0;
    while (i < N_ELEMENTS(window->subwindows) && window->subwindows[i] != NULL) {
        i++;
    }
    assert(i < N_ELEMENTS(window->subwindows));

    window->subwindows[i] = subwindow;
}

static struct subwindow *make_subwindow(struct window *window, unsigned index)
{
    struct subwindow *subwindow = get_new_subwindow(index);
    assert(subwindow != NULL);

    load_subwindow(window, subwindow);
    attach_subwindow_to_window(window, subwindow);
    load_term(subwindow);

    return subwindow;
}

static struct subwindow *transfer_subwindow(struct window *window, unsigned index)
{
    struct subwindow *subwindow = get_subwindow_direct(index);
    assert(subwindow != NULL);
    assert(subwindow->inited);
    assert(subwindow->loaded);

    detach_subwindow_from_window(subwindow->window, subwindow);
    subwindow->window = window;
    attach_subwindow_to_window(window, subwindow);

    SDL_DestroyTexture(subwindow->texture);
    subwindow->texture = make_subwindow_texture(window,
            subwindow->full_rect.w, subwindow->full_rect.h);
    assert(subwindow->texture != NULL);

    SDL_DestroyTexture(subwindow->aux_texture);
    subwindow->aux_texture = make_subwindow_texture(window, 1, 1);
    assert(subwindow->aux_texture != NULL);

    struct font *new_font = make_font(subwindow->window,
            subwindow->font->name, subwindow->font->size);
    assert(new_font != NULL);
    free_font(subwindow->font);
    subwindow->font = new_font;

    render_clear(window, subwindow->texture, &subwindow->color);

    subwindow->borders.error = false;
    render_borders(subwindow);

    fit_subwindow_in_window(window, subwindow);

    return subwindow;
}

static void load_subwindow(struct window *window, struct subwindow *subwindow)
{
    assert(window->loaded);
    assert(!subwindow->loaded);

    if (subwindow->font == NULL) {
        if (subwindow->config != NULL) {
            subwindow->font = make_font(window,
                    subwindow->config->font_name, subwindow->config->font_size);
        } else {
            subwindow->font = make_font(window, DEFAULT_FONT, 0);
        }
        assert(subwindow->font != NULL);
    }
    if (!adjust_subwindow_geometry(window, subwindow)) {
        quit_fmt("cannot adjust geometry of subwindow %u in window %u",
                subwindow->index, window->index);
    }
    subwindow->texture = make_subwindow_texture(window,
            subwindow->full_rect.w, subwindow->full_rect.h);
    assert(subwindow->texture != NULL);

    /* just a pixel for sizing rect */
    subwindow->aux_texture = make_subwindow_texture(window, 1, 1);
    assert(subwindow->aux_texture != NULL);

    /* same testing sequence as for status bar */
    if (SDL_SetRenderDrawColor(window->renderer,
                subwindow->color.r, subwindow->color.g,
                subwindow->color.b, subwindow->color.a) != 0)
    {
        quit_fmt("cannot set draw color for subwindow %u window %u: %s",
                subwindow->index, window->index, SDL_GetError());
    }
    if (SDL_SetRenderTarget(window->renderer, subwindow->texture) != 0) {
        quit_fmt("cannot set subwindow %u as render target in window %u: %s",
                subwindow->index, window->index, SDL_GetError());
    }
    if (SDL_RenderClear(window->renderer) != 0) {
        quit_fmt("cannot clear texture in subwindow %u window %u: %s",
                subwindow->index, window->index, SDL_GetError());
    }

    subwindow->window = window;
    subwindow->loaded = true;

    render_borders(subwindow);
}

static void clear_pw_flag(struct subwindow *subwindow)
{
    assert(subwindow->index < N_ELEMENTS(angband_term));

    uint32_t new_flags[N_ELEMENTS(window_flag)];
    assert(sizeof(new_flags) == sizeof(window_flag));
    memcpy(new_flags, window_flag, sizeof(new_flags));

    assert(subwindow->index < N_ELEMENTS(new_flags));
    new_flags[subwindow->index] = 0;

    subwindows_set_flags(new_flags, N_ELEMENTS(new_flags));
}

static void unload_term(struct subwindow *subwindow)
{
    if (!subwindow->linked) {
        return;
    }

    term_nuke(subwindow->term);
    mem_free(subwindow->term);
    angband_term[subwindow->index] = NULL;

    subwindow->term = NULL;
    subwindow->linked = false;
}

static void link_term(struct subwindow *subwindow)
{
    assert(subwindow->term != NULL);
    assert(subwindow->index < N_ELEMENTS(angband_term));
    assert(angband_term[subwindow->index] == NULL);

    subwindow->term->higher_pict = true;
    subwindow->term->soft_cursor = true;
    subwindow->term->complex_input = true;
    subwindow->term->never_frosh = true;

    subwindow->term->attr_blank = DEFAULT_ATTR_BLANK;
    subwindow->term->char_blank = DEFAULT_CHAR_BLANK;

    subwindow->term->xtra_hook = term_xtra_hook;
    subwindow->term->curs_hook = term_curs_hook;
    subwindow->term->bigcurs_hook = term_bigcurs_hook;
    subwindow->term->wipe_hook = term_wipe_hook;
    subwindow->term->text_hook = term_text_hook;
    subwindow->term->pict_hook = term_pict_hook;

    if (view_map_hook_mod) {
        subwindow->term->view_map_hook = term_view_map_hook;
    }

    subwindow->term->data = subwindow;
    angband_term[subwindow->index] = subwindow->term;
}

static void load_term(struct subwindow *subwindow)
{
    assert(!subwindow->linked);

    subwindow->term = mem_zalloc(sizeof(*subwindow->term));

    term_init(subwindow->term,
            subwindow->cols, subwindow->rows,
            subwindow->rows,
            SUBWINDOW_KEYQ_SIZE(subwindow));

    link_term(subwindow);

    term *old = Term;
    Term_activate(subwindow->term);
    Term_redraw();
    Term_activate(old);

    subwindow->linked = true;
}

/* initialize subwindow with suitable hardcoded defaults */
static bool wipe_subwindow(struct subwindow *subwindow)
{
    unsigned index = subwindow->index;
    memset(subwindow, 0, sizeof(*subwindow));
    subwindow->index = index;

    /* XXX 80x24 is essential for main */
    subwindow->full_rect.w = MIN_COLS_MAIN * DEFAULT_FONT_W + DEFAULT_BORDER * 2;
    subwindow->full_rect.h = MIN_ROWS_MAIN * DEFAULT_FONT_H + DEFAULT_BORDER * 2;
    if (subwindow->index != MAIN_SUBWINDOW) {
        subwindow->full_rect.w /= 2;
        subwindow->full_rect.h /= 2;
    }
    subwindow->cols = MIN_COLS_MAIN;
    subwindow->rows = MIN_ROWS_MAIN;

    subwindow->color = g_colors[DEFAULT_SUBWINDOW_BG_COLOR];
    subwindow->borders.color = g_colors[DEFAULT_SUBWINDOW_BORDER_COLOR];
    subwindow->borders.visible = true;

    subwindow->texture = NULL;
    subwindow->aux_texture = NULL;
    subwindow->window = NULL;
    subwindow->font = NULL;

    subwindow->term = NULL;

    subwindow->config = NULL;
    subwindow->inited = true;
    subwindow->visible = true;

    return true;
}

static void get_string_metrics(struct font *font, const char *str, int *w, int *h)
{
    assert(font != NULL);
    assert(font->ttf.handle != NULL);

    if (TTF_SizeUTF8(font->ttf.handle, str, w, h) != 0) {
        quit_fmt("cannot get string metrics for string '%s': %s", str, TTF_GetError());
    }
}

static int sort_cb_font_info(const void *infoa, const void *infob)
{
    int typea = ((struct font_info *) infoa)->type;
    int typeb = ((struct font_info *) infob)->type;
    
    const char *namea = ((struct font_info *) infoa)->name;
    const char *nameb = ((struct font_info *) infob)->name;

    if (typea != typeb) {
        /* raster (angband's .fon) fonts go last */
        return typea == FONT_TYPE_RASTER ? 1 : -1;
    } else if (typea == FONT_TYPE_VECTOR && typeb == FONT_TYPE_VECTOR) {
        /* vector (.ttf, etc) fonts go in alphabetical order */
        return strcmp(namea, nameb);
    } else {
        /* otherwise, we'll sort them numbers-wise (6x12x.fon before 6x13x.fon) */
        int wa = 0;
        int ha = 0;
        char facea[4 + 1] = {0};

        int wb = 0;
        int hb = 0;
        char faceb[4 + 1] = {0};

        sscanf(namea, "%dx%d%4[^.]", &wa, &ha, facea);
        sscanf(nameb, "%dx%d%4[^.]", &wb, &hb, faceb);

        if (wa < wb) {
            return -1;
        } else if (wa > wb) {
            return 1;
        } else if (ha < hb) {
            return -1;
        } else if (ha > hb) {
            return 1;
        } else {
            return strcmp(facea, faceb);
        }
    }
}

static bool is_font_file(const char *path)
{
    bool is_font = false;

    TTF_Font *font = TTF_OpenFont(path, 1);

    if (font != NULL) {
        if (TTF_FontFaceIsFixedWidth(font)) {
            is_font = true;
        }
        TTF_CloseFont(font);
    }

    return is_font;
}

static void free_menu_panel(struct menu_panel *menu_panel)
{
    while (menu_panel) {
        struct menu_panel *next = menu_panel->next;
        free_button_bank(&menu_panel->button_bank);
        mem_free(menu_panel);
        menu_panel = next;
    }
}

static void free_button_bank(struct button_bank *button_bank)
{
    for (size_t i = 0; i < button_bank->number; i++) {
        mem_free(button_bank->buttons[i].caption);
    }
    mem_free(button_bank->buttons);
    button_bank->buttons = NULL;
    button_bank->number = 0;
    button_bank->size = 0;
}

static void free_status_bar(struct status_bar *status_bar)
{
    if (status_bar->menu_panel != NULL) {
        free_menu_panel(status_bar->menu_panel);
        status_bar->menu_panel = NULL;
    }
    if (status_bar->button_bank.buttons != NULL) {
        free_button_bank(&status_bar->button_bank);
    }
    if (status_bar->texture != NULL) {
        SDL_DestroyTexture(status_bar->texture);
        status_bar->texture = NULL;
    }

    free_font(status_bar->font);
    status_bar->font = NULL;
}

static void free_font_info(struct font_info *font_info)
{
    if (font_info->name != NULL) {
        mem_free(font_info->name);
        font_info->name = NULL;
    }
    if (font_info->path != NULL) {
        mem_free(font_info->path);
        font_info->path = NULL;
    }
    font_info->loaded = false;
}

static void free_window_config(struct window_config *config)
{
    if (config->wallpaper_path != NULL) {
        mem_free(config->wallpaper_path);
    }
    if (config->font_name != NULL) {
        mem_free(config->font_name);
    }
    mem_free(config);
}

static void free_graphics(struct graphics *graphics)
{
    if (graphics->texture != NULL) {
        SDL_DestroyTexture(graphics->texture);
        graphics->texture = NULL;
    }
}

static void free_subwindow_config(struct subwindow_config *config)
{
    if (config->font_name != NULL) {
        mem_free(config->font_name);
    }
    mem_free(config);
}

static void free_subwindow(struct subwindow *subwindow)
{
    assert(subwindow->loaded);

    free_font(subwindow->font);
    subwindow->font = NULL;
    if (subwindow->texture != NULL) {
        SDL_DestroyTexture(subwindow->texture);
        subwindow->texture = NULL;
    }
    if (subwindow->aux_texture != NULL) {
        SDL_DestroyTexture(subwindow->aux_texture);
        subwindow->aux_texture = NULL;
    }
    if (subwindow->term != NULL) {
        unload_term(subwindow);
    }
    if (subwindow->config != NULL) {
        free_subwindow_config(subwindow->config);
        subwindow->config = NULL;
    }

    subwindow->window = NULL;
    subwindow->loaded = false;
    subwindow->inited = false;
}

static void free_window(struct window *window)
{
    assert(window->loaded);

    for (size_t i = 0; i < N_ELEMENTS(window->subwindows); i++) {
        struct subwindow *subwindow = window->subwindows[i];
        if (subwindow != NULL) {
            free_subwindow(subwindow);
            window->subwindows[i] = NULL;
        }
    }

    free_status_bar(&window->status_bar);

    if (window->wallpaper.texture != NULL) {
        SDL_DestroyTexture(window->wallpaper.texture);
        window->wallpaper.texture = NULL;
    }

    free_graphics(&window->graphics);

    if (window->renderer != NULL) {
        SDL_DestroyRenderer(window->renderer);
        window->renderer = NULL;
    }

    if (window->window != NULL) {
        SDL_DestroyWindow(window->window);
        window->window = NULL;
    }

    if (window->config != NULL) {
        free_window_config(window->config);
        window->config = NULL;
    }

    window->loaded = false;
    window->inited = false;
}

static void init_colors(void)
{
    size_t i;

    assert(N_ELEMENTS(g_colors) == N_ELEMENTS(angband_color_table));

    for (i = 0; i < N_ELEMENTS(g_colors); i++) {
        g_colors[i].r = angband_color_table[i][1];
        g_colors[i].g = angband_color_table[i][2];
        g_colors[i].b = angband_color_table[i][3];
        g_colors[i].a = DEFAULT_ALPHA_FULL;
    }
    for (i = 0; i < N_ELEMENTS(g_windows); i++)
        g_windows[i].color = g_colors[DEFAULT_WINDOW_BG_COLOR];
    for (i = 0; i < N_ELEMENTS(g_subwindows); i++)
    {
        /* Retain whatever customized alpha the subwindow has. */
        g_subwindows[i].color.r = g_colors[DEFAULT_SUBWINDOW_BG_COLOR].r;
        g_subwindows[i].color.g = g_colors[DEFAULT_SUBWINDOW_BG_COLOR].g;
        g_subwindows[i].color.b = g_colors[DEFAULT_SUBWINDOW_BG_COLOR].b;
    }
}

static void init_font_info(const char *directory)
{
    /* well maybe it will get ability to be reinitialized at some point */
    for (size_t i = 0; i < N_ELEMENTS(g_font_info); i++) {
        g_font_info[i].name = NULL;
        g_font_info[i].path = NULL;
        g_font_info[i].loaded = false;
    }

    ang_dir *dir = my_dopen(directory);
    assert(dir != NULL);

    char name[1024];
    char path[4096];

    size_t i = 0;
    while (i < N_ELEMENTS(g_font_info) && my_dread(dir, name, sizeof(name))) {
        path_build(path, sizeof(path), directory, name);

        if (is_font_file(path)) {
            g_font_info[i].name = string_make(name);
            g_font_info[i].path = string_make(path);
            g_font_info[i].loaded = true;
            if (suffix(path, ".fon") || suffix(path, ".FON")) {
                g_font_info[i].type = FONT_TYPE_RASTER;
                g_font_info[i].size = 0;
            } else {
                g_font_info[i].type = FONT_TYPE_VECTOR;
                g_font_info[i].size = DEFAULT_VECTOR_FONT_SIZE;
            }
            i++;
        }
    }
    assert(i > 0);

    sort(g_font_info, i, sizeof(g_font_info[0]), sort_cb_font_info);

    for (size_t j = 0; j < i; j++) {
        g_font_info[j].index = j;
    }

    /* checking font_page */
    if (i > MAX_BUTTONS) {
        font_page = true;
    }

    my_dclose(dir);
}

static void create_defaults(void)
{
    struct window *window = get_new_window(MAIN_WINDOW);
    assert(window != NULL);

    struct subwindow *subwindow = get_new_subwindow(MAIN_SUBWINDOW);
    assert(subwindow != NULL);

    assert(MAIN_SUBWINDOW < N_ELEMENTS(window->subwindows));
    window->subwindows[MAIN_SUBWINDOW] = subwindow;
}

static void quit_systems(void)
{
    SDL_StopTextInput();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

static void quit_hook(const char *s)
{
    dump_config_file();

    free_globals();
    quit_systems();
}

static void init_systems(void)
{
    if (SDL_Init(INIT_SDL_FLAGS) != 0) {
        quit_fmt("SDL_Init: %s", SDL_GetError());
    }
    if (IMG_Init(INIT_IMG_FLAGS) != INIT_IMG_FLAGS) {
        quit_fmt("IMG_Init: %s", IMG_GetError());
        SDL_Quit();
    }
    if (TTF_Init() != 0) {
        quit_fmt("TTF_Init: %s", TTF_GetError());
    }

    /* On (some?) Macs the touchpad sends both mouse events and touch events;
     * the latter interfere badly with the working of the status bar */
    SDL_EventState(SDL_FINGERMOTION, SDL_DISABLE);
    SDL_EventState(SDL_FINGERDOWN, SDL_DISABLE);
    SDL_EventState(SDL_FINGERUP, SDL_DISABLE);

    /* Ignore Keymap changed events since they are not handled */
    SDL_EventState(SDL_KEYMAPCHANGED, SDL_DISABLE);

    SDL_StartTextInput();
    SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");
#ifdef SDL_HINT_POLL_SENTINEL
    SDL_SetHint(SDL_HINT_POLL_SENTINEL, "0");
#endif
}

/*
 * Display warning message (see "z-util.c")
 * plog() hook to display a message box. similar to WIN32 client
 */
static void hack_plog(const char *str)
{
    if (SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, VERSION_NAME, str, NULL) < 0) {
        SDL_Log("error displaying message box");
    }
    printf("%s\n", str);
    return;
}

errr init_sdl2(void)
{
#ifdef ON_ANDROID
    SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");
#endif
    init_systems();
    init_globals();

    if (!init_graphics_modes()) {
        quit_systems();
        return 1;
    }

    if (!read_config_file()) {
        create_defaults();
    }

    start_windows();
    load_terms();

    /* Activate hooks */
    plog_aux = hack_plog;
    quit_aux = quit_hook;

    return 0;
}

/* the string ANGBAND_DIR_USER is freed before calling quit_hook(),
 * so we need to save the path to config file here */
static char g_config_file[4096];

static void init_globals(void)
{
    for (size_t i = 0; i < N_ELEMENTS(g_subwindows); i++) {
        g_subwindows[i].index = i;
    }
    for (size_t i = 0; i < N_ELEMENTS(g_windows); i++) {
        g_windows[i].index = i;
    }

    init_font_info(ANGBAND_DIR_FONTS);
    init_colors();

    path_build(g_config_file, sizeof(g_config_file),
            DEFAULT_CONFIG_FILE_DIR, DEFAULT_CONFIG_FILE);
}

static bool is_subwindow_loaded(unsigned index)
{
    const struct subwindow *subwindow = get_subwindow_direct(index);
    assert(subwindow != NULL);

    return subwindow->loaded;
}

static struct subwindow *get_subwindow_direct(unsigned index)
{
    size_t i;
    if (index < N_ELEMENTS(g_subwindows)
            && g_subwindows[index].index == index)
    {
        i = index;
    } else {
        for (i = 0; i < N_ELEMENTS(g_subwindows); i++) {
            if (g_subwindows[i].index == index) {
                break;
            }
        }
        if (i == N_ELEMENTS(g_subwindows)) {
            return NULL;
        }
    }

    return &g_subwindows[i];
}

static struct subwindow *get_new_subwindow(unsigned index)
{
    struct subwindow *subwindow = get_subwindow_direct(index);
    assert(subwindow != NULL);
    assert(!subwindow->inited);
    assert(!subwindow->loaded);
    assert(!subwindow->linked);

    if (!wipe_subwindow(subwindow)) {
        return NULL;
    }

    return subwindow;
}

static struct window *get_new_window(unsigned index)
{
    assert(index < N_ELEMENTS(g_windows));
    assert(g_windows[index].index == index);

    struct window *window = &g_windows[index];
    assert(!window->inited);
    assert(!window->loaded);

    wipe_window(window, DEFAULT_DISPLAY);

    return window;
}

static struct window *get_window_direct(unsigned index)
{
    assert(index < N_ELEMENTS(g_windows));

    if (g_windows[index].loaded) {
        assert(g_windows[index].index == index);
        return &g_windows[index];
    }

    return NULL;
}

static struct window *get_window_by_id(Uint32 id)
{
    for (size_t i = 0; i < N_ELEMENTS(g_windows); i++) {
        if (g_windows[i].loaded && g_windows[i].id == id) {
            return &g_windows[i];
        }
    }
    
    return NULL;
}

static void free_globals(void)
{
    for (size_t i = 0; i < N_ELEMENTS(g_font_info); i++) {
        free_font_info(&g_font_info[i]);
    }
    for (size_t i = 0; i < N_ELEMENTS(g_windows); i++) {
        if (g_windows[i].loaded) {
            free_window(&g_windows[i]);
        }
    }
    for (size_t i = 0; i < N_ELEMENTS(g_subwindows); i++) {
        assert(!g_subwindows[i].inited);
        assert(!g_subwindows[i].loaded);
        assert(!g_subwindows[i].linked);
    }
}

static void start_windows(void)
{
    for (size_t i = N_ELEMENTS(g_windows); i > 0; i--) {
        if (g_windows[i - 1].inited) {
            start_window(&g_windows[i - 1]);
        }
    }
}

static void load_terms(void)
{
    for (size_t i = 0; i < N_ELEMENTS(g_subwindows); i++) {
        if (g_subwindows[i].loaded) {
            load_term(&g_subwindows[i]);
        }
    }

    Term_activate(term_screen);
}

/* Config file stuff */

static void dump_config_file(void)
{
    ang_file *config = file_open(g_config_file, MODE_WRITE, FTYPE_TEXT);

    assert(config != NULL);

    for (size_t i = 0; i < N_ELEMENTS(g_windows); i++) {
        if (g_windows[i].loaded) {
            dump_window(&g_windows[i], config);
        }
    }
    file_putf(config, "kp-as-modifier:%d\n", (g_kp_as_mod) ? 1 : 0);
    file_putf(config, "map-modifier:%d\n", (view_map_hook_mod) ? 1 : 0);
    file_putf(config, "sound-volume:%d\n", sound_volume);
    file_putf(config, "music-volume:%d\n", music_volume);

    file_close(config);
}

/* XXX more bad style :) */
#define GET_WINDOW_FROM_INDEX \
    if (parser_getuint(parser, "index") >= N_ELEMENTS(g_windows)) { \
        return PARSE_ERROR_OUT_OF_BOUNDS; \
    } \
    struct window *window = &g_windows[parser_getuint(parser, "index")]; \

#define WINDOW_INIT_OK \
    if (!window->inited) { \
        return PARSE_ERROR_MISSING_RECORD_HEADER; \
    }

#define GET_SUBWINDOW_FROM_INDEX \
    if (parser_getuint(parser, "index") >= N_ELEMENTS(g_subwindows)) { \
        return PARSE_ERROR_OUT_OF_BOUNDS; \
    } \
    struct subwindow *subwindow = &g_subwindows[parser_getuint(parser, "index")]; \

#define SUBWINDOW_INIT_OK \
    if (!subwindow->inited) { \
        return PARSE_ERROR_MISSING_RECORD_HEADER; \
    }

static enum parser_error config_window_display(struct parser *parser)
{
    GET_WINDOW_FROM_INDEX;

    int display = parser_getint(parser, "display");

    if (display < 0 || display > SDL_GetNumVideoDisplays()) {
        return PARSE_ERROR_OUT_OF_BOUNDS;
    }

    wipe_window(window, display);

    window->config = mem_zalloc(sizeof(*window->config));

    window->config->window_flags = SDL_WINDOW_RESIZABLE;

    return PARSE_ERROR_NONE;
}

static enum parser_error config_window_fullscreen(struct parser *parser)
{
    GET_WINDOW_FROM_INDEX;
    WINDOW_INIT_OK;

    const char *mode = parser_getsym(parser, "fullscreen");
    if (streq(mode, "true")) {
        window->config->window_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    } else if (streq(mode, "false")) {
        ;
    } else {
        return PARSE_ERROR_INVALID_VALUE;
    }
    return PARSE_ERROR_NONE;
}

static enum parser_error config_window_rect(struct parser *parser)
{
    GET_WINDOW_FROM_INDEX;
    WINDOW_INIT_OK;
    window->full_rect.x = parser_getint(parser, "x");
    window->full_rect.y = parser_getint(parser, "y");
    window->full_rect.w = parser_getint(parser, "w");
    window->full_rect.h = parser_getint(parser, "h");

    return PARSE_ERROR_NONE;
}

static enum parser_error config_window_renderer(struct parser *parser)
{
    GET_WINDOW_FROM_INDEX;
    WINDOW_INIT_OK;
    const char *type = parser_getsym(parser, "type");

    if (streq(type, "hardware")) {
        window->config->renderer_flags = SDL_RENDERER_ACCELERATED;
    } else if (streq(type, "software")) {
        window->config->renderer_flags = SDL_RENDERER_SOFTWARE;
    } else {
        return PARSE_ERROR_INVALID_VALUE;
    }
    
    return PARSE_ERROR_NONE;
}

static enum parser_error config_window_wallpaper_path(struct parser *parser)
{
    GET_WINDOW_FROM_INDEX;
    WINDOW_INIT_OK;
    const char *path = parser_getstr(parser, "path");
    if (streq(path, "default")) {
        char buf[4096];
        path_build(buf, sizeof(buf), DEFAULT_WALLPAPER_DIR, DEFAULT_WALLPAPER);

        window->config->wallpaper_path = string_make(buf);
    } else {
        window->config->wallpaper_path = string_make(path);
    }

    return PARSE_ERROR_NONE;
}

static enum parser_error config_window_wallpaper_mode(struct parser *parser)
{
    GET_WINDOW_FROM_INDEX;
    WINDOW_INIT_OK;
    const char *mode = parser_getstr(parser, "mode");

    if (streq(mode, "none")) {
        window->wallpaper.mode = WALLPAPER_DONT_SHOW;
    } else if (streq(mode, "tiled")) {
        window->wallpaper.mode = WALLPAPER_TILED;
    } else if (streq(mode, "centered")) {
        window->wallpaper.mode = WALLPAPER_CENTERED;
    } else if (streq(mode, "scaled")) {
        window->wallpaper.mode = WALLPAPER_SCALED;
    } else {
        return PARSE_ERROR_INVALID_VALUE;
    }

    return PARSE_ERROR_NONE;
}

static enum parser_error config_window_font(struct parser *parser)
{
    GET_WINDOW_FROM_INDEX;
    WINDOW_INIT_OK;
    const char *name = parser_getstr(parser, "name");
    int size = parser_getint(parser, "size");

    if (find_font_info(name) == NULL) {
        return PARSE_ERROR_INVALID_VALUE;
    }
    window->config->font_name = string_make(name);
    window->config->font_size = size;

    return PARSE_ERROR_NONE;
}

static enum parser_error config_window_graphics(struct parser *parser)
{
    GET_WINDOW_FROM_INDEX;
    WINDOW_INIT_OK;

    int id = parser_getint(parser, "id");

    if (get_graphics_mode(id, true) == NULL) {
        return PARSE_ERROR_INVALID_VALUE;
    }

    window->graphics.id = id;

    return PARSE_ERROR_NONE;
}

static enum parser_error config_window_tile_scale(struct parser *parser)
{
    GET_WINDOW_FROM_INDEX;
    WINDOW_INIT_OK;

    if (window->graphics.id != GRAPHICS_NONE) {
        if (streq(parser_getsym(parser, "which"), "height")) {
            tile_height = parser_getint(parser, "scale");
        } else if (streq(parser_getsym(parser, "which"), "width")) {
            tile_width = parser_getint(parser, "scale");
        } else {
            return PARSE_ERROR_INVALID_OPTION;
        }
    }

    return PARSE_ERROR_NONE;
}

static enum parser_error config_subwindow_window(struct parser *parser)
{
    GET_SUBWINDOW_FROM_INDEX;

    unsigned windex = parser_getuint(parser, "windex");
    if (windex >= N_ELEMENTS(g_windows)) {
        return PARSE_ERROR_OUT_OF_BOUNDS;
    }
    struct window *window = &g_windows[windex];
    if (!window->inited) {
        return PARSE_ERROR_NON_SEQUENTIAL_RECORDS;
    }
    if (!wipe_subwindow(subwindow)) {
        return PARSE_ERROR_GENERIC;
    }
    subwindow->config = mem_zalloc(sizeof(*subwindow->config));

    subwindow->window = window;
    attach_subwindow_to_window(window, subwindow);

    return PARSE_ERROR_NONE;
}

static enum parser_error config_subwindow_rect(struct parser *parser)
{
    GET_SUBWINDOW_FROM_INDEX;
    SUBWINDOW_INIT_OK;

    subwindow->full_rect.x = parser_getint(parser, "x");
    subwindow->full_rect.y = parser_getint(parser, "y");
    subwindow->full_rect.w = parser_getint(parser, "w");
    subwindow->full_rect.h = parser_getint(parser, "h");

    return PARSE_ERROR_NONE;
}

static enum parser_error config_subwindow_font(struct parser *parser)
{
    GET_SUBWINDOW_FROM_INDEX;
    SUBWINDOW_INIT_OK;

    const char *name = parser_getstr(parser, "name");
    int size = parser_getint(parser, "size");

    if (find_font_info(name) == NULL) {
        /* TODO maybe its not really an error? the font file was
         * probably just deleted and now the ui wont event start... */
        return PARSE_ERROR_INVALID_VALUE;
    }
    subwindow->config->font_name = string_make(name);
    subwindow->config->font_size = size;

    return PARSE_ERROR_NONE;
}

static enum parser_error config_subwindow_borders(struct parser *parser)
{
    GET_SUBWINDOW_FROM_INDEX;
    SUBWINDOW_INIT_OK;

    const char *borders = parser_getsym(parser, "borders");
    if (streq(borders, "true")) {
        subwindow->borders.visible = true;
    } else if (streq(borders, "false")) {
        subwindow->borders.visible = false;
    } else {
        return PARSE_ERROR_INVALID_VALUE;
    }

    return PARSE_ERROR_NONE;
}

static enum parser_error config_subwindow_top(struct parser *parser)
{
    GET_SUBWINDOW_FROM_INDEX;
    SUBWINDOW_INIT_OK;

    const char *top = parser_getsym(parser, "top");
    if (streq(top, "true")) {
        subwindow->top = true;
    } else if (streq(top, "false")) {
        subwindow->top = false;
    } else {
        return PARSE_ERROR_INVALID_VALUE;
    }

    const char *always = parser_getsym(parser, "always");
    if (streq(always, "true")) {
        subwindow->always_top = true;
    } else if (streq(always, "false")) {
        subwindow->always_top = false;
    } else {
        return PARSE_ERROR_INVALID_VALUE;
    }

    return PARSE_ERROR_NONE;
}

static enum parser_error config_subwindow_alpha(struct parser *parser)
{
    GET_SUBWINDOW_FROM_INDEX;
    SUBWINDOW_INIT_OK;

    int alpha = parser_getint(parser, "alpha");

    if (alpha < 0 || alpha > DEFAULT_ALPHA_FULL) {
        return PARSE_ERROR_INVALID_VALUE;
    }

    subwindow->color.a = alpha;

    return PARSE_ERROR_NONE;
}
#undef GET_WINDOW_FROM_INDEX
#undef WINDOW_INIT_OK
#undef GET_SUBWINDOW_FROM_INDEX
#undef SUBWINDOW_INIT_OK

static enum parser_error config_kp_as_mod(struct parser *parser)
{
    g_kp_as_mod = parser_getint(parser, "enabled");
    return PARSE_ERROR_NONE;
}

static enum parser_error config_view_map_hook_mod(struct parser *parser)
{
    view_map_hook_mod = parser_getint(parser, "enabled");
    return PARSE_ERROR_NONE;
}

static enum parser_error config_sound_volume(struct parser *parser)
{
    sound_volume = parser_getint(parser, "sound_volume");
    return PARSE_ERROR_NONE;
}

static enum parser_error config_music_volume(struct parser *parser)
{
    music_volume = parser_getint(parser, "music_volume");
    return PARSE_ERROR_NONE;
}

static struct parser *init_parse_config(void)
{
    struct parser *parser = parser_new();
    
    parser_reg(parser, "window-display uint index int display",
            config_window_display);
    parser_reg(parser, "window-fullscreen uint index sym fullscreen",
            config_window_fullscreen);
    parser_reg(parser, "window-full-rect uint index int x int y int w int h",
            config_window_rect);
    parser_reg(parser, "window-renderer uint index sym type",
            config_window_renderer);
    parser_reg(parser, "window-wallpaper-path uint index str path",
            config_window_wallpaper_path);
    parser_reg(parser, "window-wallpaper-mode uint index str mode",
            config_window_wallpaper_mode);
    parser_reg(parser, "window-status-bar-font uint index int size str name",
            config_window_font);
    parser_reg(parser, "window-graphics-id uint index int id",
            config_window_graphics);
    parser_reg(parser, "window-tile-scale uint index sym which int scale",
            config_window_tile_scale);

    parser_reg(parser, "subwindow-window uint index uint windex",
            config_subwindow_window);
    parser_reg(parser, "subwindow-full-rect uint index int x int y int w int h",
            config_subwindow_rect);
    parser_reg(parser, "subwindow-font uint index int size str name",
            config_subwindow_font);
    parser_reg(parser, "subwindow-borders uint index sym borders",
            config_subwindow_borders);
    parser_reg(parser, "subwindow-top uint index sym top sym always",
            config_subwindow_top);
    parser_reg(parser, "subwindow-alpha uint index int alpha",
            config_subwindow_alpha);
    parser_reg(parser, "kp-as-modifier int enabled", config_kp_as_mod);
    parser_reg(parser, "map-modifier int enabled", config_view_map_hook_mod);
    parser_reg(parser, "sound-volume int sound_volume", config_sound_volume);
    parser_reg(parser, "music-volume int music_volume", config_music_volume);

    return parser;
}

static void print_error(const char *name, struct parser *parser)
{
    struct parser_state state;
    parser_getstate(parser, &state);

    fprintf(stderr, "parse error in %s line %d column %d: %s: %s\n",
            name,
            state.line,
            state.col,
            state.msg,
            parser_error_str[state.error]);
}

static bool read_config_file(void)
{
    ang_file *config = file_open(g_config_file, MODE_READ, FTYPE_TEXT);
    if (config == NULL) {
        /* not an error, its ok for a config file to not exist */
        return false;
    }

    char line[1024];
    struct parser *parser = init_parse_config();
    errr error = 0;

    while (file_getl(config, line, sizeof(line))) {
        error = parser_parse(parser, line);
        if (error != PARSE_ERROR_NONE) {
            print_error(g_config_file, parser);
            break;
        }
    }

    parser_destroy(parser);
    file_close(config);

    return error == PARSE_ERROR_NONE;
}

#else

/*
 * A font structure
 * Note that the data is only valid for a surface with matching
 * values for pitch & bpp. If a surface is resized the data _must_ be
 * recalculated.
 */
typedef struct sdl_Font sdl_Font;
struct sdl_Font
{
    int width;          /* The dimensions of this font (in pixels) */
    int height;
    char name[32];      /* The name of this font */
    Uint16 pitch;       /* Pitch of the surface this font is made for */
    Uint8 bpp;          /* Bytes per pixel of the surface */
    Uint8 something;    /* Padding */
    int *data;          /* The data */
    TTF_Font *sdl_font; /* The native font */
};

static sdl_Font SystemFont;

#define NUM_GLYPHS 256

/*
 * Window information
 * Each window has its own surface and coordinates
 *
 * Window border:
 *   COLOUR_DARK = no border
 *   COLOUR_WHITE to COLOUR_DEEP_L_BLUE = colored border
 *   COLOUR_SHADE (BASIC_COLORS) = default border
 */
typedef struct term_window term_window;
struct term_window
{
    term term_data;
    SDL_Surface *surface;   /* The surface for this window */
    SDL_Surface *tiles;     /* The appropriately sized tiles for this window */
    uint8_t Term_idx;          /* Index of term that relates to this */
    int top;                /* Window Coordinates on the main screen */
    int left;
    int keys;               /* Size of keypress storage */
    sdl_Font font;          /* Font info */
    char *req_font;         /* Requested font */
    int font_size;          /* Font size dimensions */
    int windowborders;      /* Window borders */
    int rows;               /* Dimension in tiles */
    int cols;
    int border;             /* Border width */
    int title_height;       /* Height of title bar */
    int width;              /* Dimension in pixels == tile_wid * cols + 2 x border */
    int height;
    int tile_wid;           /* Size in pixels of a char */
    int tile_hgt;
    bool visible;           /* Can we see this window? */
    SDL_Rect uRect;         /* The part that needs to be updated */
    bool minimap_active;    /* Are we looking at the minimap? */
    int max_rows;           /* Maximum number of lines */
};

typedef struct mouse_info mouse_info;
struct mouse_info
{
    int left;       /* Is it pressed? */
    int right;
    int leftx;      /* _IF_ left button is pressed these */
    int lefty;      /* show where it was pressed */
    int rightx;
    int righty;
    int x;          /* Current position of mouse */
    int y;
};

#define WINDOW_DRAW (SDL_USEREVENT + 1)

/*
 * The basic angband text colours in an sdl friendly form
 */
static SDL_Color text_colours[MAX_COLORS];

SDL_Color back_colour;    /* Background colour */
Uint32 back_pixel_colour;

typedef struct sdl_ButtonBank sdl_ButtonBank;
typedef struct sdl_Button sdl_Button;
typedef struct sdl_Window sdl_Window;

typedef void (*button_press_func)(sdl_Button *sender);
struct sdl_Button
{
    SDL_Rect pos;               /* Position & Size */
    bool selected;              /* Selected? */
    bool visible;               /* Visible? */
    button_press_func activate; /* A function to call when pressed */
    sdl_ButtonBank *owner;      /* Which bank is this in? */
    char caption[50];           /* Text for this button */
    SDL_Color unsel_colour;     /* Button unselected colour */
    SDL_Color sel_colour;       /* Selected colour */
    SDL_Color cap_colour;       /* Caption colour */
    void *data;                 /* Something */
    int tag;                    /* Something */
};

struct sdl_ButtonBank
{
    sdl_Button *buttons;        /* A collection of buttons */
    bool *used;                 /* What buttons are available? */
    sdl_Window *window;         /* The window that these buttons are on */
    bool need_update;
};

/*
 * Other 'windows' (basically a surface with a position and buttons on it)
 * Currently used for the top status bar and popup windows
 */
typedef void (*sdl_WindowCustomDraw)(sdl_Window *window);
struct sdl_Window
{
    int top;                            /* Position on main window */
    int left;
    int width;                          /* Dimensions */
    int height;
    bool visible;                       /* Visible? */
    SDL_Surface *surface;               /* SDL surface info */
    sdl_ButtonBank buttons;             /* Buttons */
    sdl_Font font;                      /* Font */
    SDL_Surface *owner;                 /* Who shall I display on */
    sdl_WindowCustomDraw draw_extra;    /* Stuff to draw on the surface */
    bool need_update;
};

/*
 * The main surface of the application
 */
static SDL_Surface *AppWin;

/*
 * The status bar
 */
static sdl_Window StatusBar;

/*
 * The Popup window
 */
static sdl_Window PopUp;
static bool popped;

/*
 * Term windows
 */
static term_window windows[ANGBAND_TERM_MAX];
static int Zorder[ANGBAND_TERM_MAX];

/*
 * Keep track of the mouse status
 */
static mouse_info mouse;

/*
 * The number pad consists of 10 keys, each with an SDL identifier
 */
#define is_numpad(k) \
((k == SDLK_KP0) || (k == SDLK_KP1) || (k == SDLK_KP2) || (k == SDLK_KP3) || \
 (k == SDLK_KP4) || (k == SDLK_KP5) || (k == SDLK_KP6) || \
 (k == SDLK_KP7) || (k == SDLK_KP8) || (k == SDLK_KP9) || (k == SDLK_KP_ENTER))

static int SnapRange = 5;   /* Window snap range (pixels) */
static int StatusHeight;    /* The height in pixels of the status bar */
static int SelectedTerm;    /* Current selected Term */

static int AboutSelect;     /* About button */
static int TermSelect;      /* Term selector button */
static int FontSelect;      /* Font selector button */
static int VisibleSelect;   /* Hide/unhide window button*/
static int MoreSelect;      /* Other options button */
static int QuitSelect;      /* Quit button */

/* For saving the icon for the About Box */
static SDL_Surface *mratt = NULL;

/* Buttons on the 'More' panel */
static int MoreOK;          /* Accept changes */
static int MoreFullscreen;  /* Fullscreen toggle button */
static int MoreNiceGfx;     /* Nice graphics toggle button */
static int MoreSnapPlus;    /* Increase snap range */
static int MoreSnapMinus;   /* Decrease snap range */
static int MoreFontSizePlus;    /* Increase font size range */
static int MoreFontSizeMinus;   /* Decrease font size range */
static int MoreSoundVolumePlus;      /* Increase sound volume */
static int MoreSoundVolumeMinus;     /* Decrease sound volume */
static int MoreMusicVolumePlus;      /* Increase music volume */
static int MoreMusicVolumeMinus;     /* Decrease music volume */
static int MoreWindowBordersPlus;  /* Increase window borders */
static int MoreWindowBordersMinus; /* Decrease window borders */

static bool Moving;             /* Moving a window */
static bool Sizing;             /* Sizing a window */
static SDL_Rect SizingSpot;     /* Rect to describe the sizing area */
static bool Sizingshow = false; /* Is the resize thingy displayed? */
static SDL_Rect SizingRect;     /* Rect to describe the current resize window */

static SDL_Surface *GfxSurface = NULL;  /* A surface for the graphics */

static int MoreWidthPlus;   /* Increase tile width */
static int MoreWidthMinus;  /* Decrease tile width */
static int MoreHeightPlus;  /* Increase tile height */
static int MoreHeightMinus; /* Decrease tile height */
static int *GfxButtons;     /* Graphics mode buttons */
static int SelectedGfx;     /* Current selected gfx */


/*
 * Fill in an SDL_Rect structure.
 * Note it also returns the value adjusted
 */
static SDL_Rect *sdl_RECT(int x, int y, int w, int h, SDL_Rect *rect)
{
    rect->x = x;
    rect->y = y;
    rect->w = w;
    rect->h = h;
    
    return rect;
}


/*
 * Is a point(x, y) in a rectangle?
 */
static bool point_in(SDL_Rect *rect, int x, int y)
{
    if (x < rect->x) return false;
    if (y < rect->y) return false;
    if (x >= rect->x + rect->w) return false;
    if (y >= rect->y + rect->h) return false;
    
    /* Must be inside */
    return true;
}


/*
 * Draw an outline box
 * Given the top, left, width & height
 */
static void sdl_DrawBox(SDL_Surface *surface, SDL_Rect *rect, SDL_Color colour, int width)
{
    SDL_Rect rc;
    int left = rect->x;
    int right = rect->x + rect->w - width;
    int top = rect->y;
    int bottom = rect->y + rect->h - width;
    Uint32 pixel_colour = SDL_MapRGB(surface->format, colour.r, colour.g, colour.b);
    
    /* Top left -> Top Right */
    sdl_RECT(left, top, rect->w, width, &rc);
    SDL_FillRect(surface, &rc, pixel_colour);
    
    /* Bottom left -> Bottom Right */
    sdl_RECT(left, bottom, rect->w, width, &rc);
    SDL_FillRect(surface, &rc, pixel_colour);
    
    /* Top left -> Bottom left */
    sdl_RECT(left, top, width, rect->h, &rc);
    SDL_FillRect(surface, &rc, pixel_colour);
    
    /* Top right -> Bottom right */
    sdl_RECT(right, top, width, rect->h, &rc);
    SDL_FillRect(surface, &rc, pixel_colour);
}


/*
 * Get the width and height of a given font file
 */
static errr sdl_CheckFont(const char *fontname, int font_size, int *width, int *height)
{
    char buf[MSG_LEN];
    TTF_Font *ttf_font;

    /* Build the path */
    path_build(buf, sizeof(buf), ANGBAND_DIR_FONTS, fontname);

    /* Attempt to load it */
    ttf_font = TTF_OpenFont(buf, font_size);

    /* Bugger */
    if (!ttf_font) return (-1);

    /* Get the size */
    if (TTF_SizeText(ttf_font, "M", width, height)) return (-1);

    /* Finished with the font */
    TTF_CloseFont(ttf_font);

    return (0);
}


/*
 * The sdl_Font routines
 */


/*
 * Free any memory assigned by Create()
 */
static void sdl_FontFree(sdl_Font *font)
{
    /* Finished with the font */
    TTF_CloseFont(font->sdl_font);
}


/*
 * Create new font data with font fontname, optimizing the data
 * for the surface given
 */
static errr sdl_FontCreate(sdl_Font *font, const char *fontname, int font_size, SDL_Surface *surface)
{
    char buf[MSG_LEN];
    TTF_Font *ttf_font;

    /* Build the path */
    path_build(buf, sizeof(buf), ANGBAND_DIR_FONTS, fontname);

    /* Attempt to load it */
    ttf_font = TTF_OpenFont(buf, font_size);

    /* Bugger */
    if (!ttf_font) return (-1);

    /* Get the size */
    if (TTF_SizeText(ttf_font, "M", &font->width, &font->height)) return (-1);

    /* Get the best quality */
    TTF_SetFontHinting(ttf_font, TTF_HINTING_LIGHT);

    /* Fill in some of the font struct */
    if (font->name != fontname) my_strcpy(font->name, fontname, 30);
    font->pitch = surface->pitch;
    font->bpp = surface->format->BytesPerPixel;
    font->sdl_font = ttf_font;

    /* Success */
    return (0); 
}


/*
 * Draw some text onto a surface, allowing shaded backgrounds.
 * The surface is first checked to see if it is compatible with
 * this font, if it isn't then the font will be 're-precalculated'.
 *
 * You can, I suppose, use one font on many surfaces, but it is
 * definitely not recommended. One font per surface is good enough.
 */
static errr sdl_mapFontDraw(sdl_Font *font, int font_size, SDL_Surface *surface, SDL_Color colour,
    SDL_Color bg, int x, int y, int n, const char *s)
{
    Uint8 bpp = surface->format->BytesPerPixel;
    Uint16 pitch = surface->pitch;
    SDL_Rect rc;
    SDL_Surface *text;

    if ((bpp != font->bpp) || (pitch != font->pitch))
        sdl_FontCreate(font, font->name, font_size, surface);

    /* Lock the window surface (if necessary) */
    if (SDL_MUSTLOCK(surface))
    {
        if (SDL_LockSurface(surface) < 0) return (-1);
    }

    sdl_RECT(x, y, n * font->width, font->height, &rc);
    text = TTF_RenderText_Shaded(font->sdl_font, s, colour, bg);
    if (text)
    {
        SDL_BlitSurface(text, NULL, surface, &rc);
        SDL_FreeSurface(text);
    }

    /* Unlock the surface */
    if (SDL_MUSTLOCK(surface)) SDL_UnlockSurface(surface);

    /* Success */
    return (0);
}


/*
 * Draw some text onto a surface
 * The surface is first checked to see if it is compatible with
 * this font, if it isn't then the font will be 're-precalculated'
 *
 * You can, I suppose, use one font on many surfaces, but it is
 * definitely not recommended. One font per surface is good enough.
 */
static errr sdl_FontDraw(sdl_Font *font, SDL_Surface *surface, SDL_Color colour, int x, int y,
    int n, const char *s)
{
    Uint8 bpp = surface->format->BytesPerPixel;
    Uint16 pitch = surface->pitch;
    SDL_Rect rc;
    SDL_Surface *text;

    if ((bpp != font->bpp) || (pitch != font->pitch))
        sdl_FontCreate(font, font->name, 0, surface);

    /* Lock the window surface (if necessary) */
    if (SDL_MUSTLOCK(surface))
    {
        if (SDL_LockSurface(surface) < 0) return (-1);
    }

    sdl_RECT(x, y, n * font->width, font->height, &rc);
    text = TTF_RenderText_Solid(font->sdl_font, s, colour);
    if (text)
    {
        SDL_BlitSurface(text, NULL, surface, &rc);
        SDL_FreeSurface(text);
    }

    /* Unlock the surface */
    if (SDL_MUSTLOCK(surface)) SDL_UnlockSurface(surface);

    /* Success */
    return (0);
}


/*
 * Draw a button on the screen
 */
static void sdl_ButtonDraw(sdl_Button *button)
{
    SDL_Surface *surface = button->owner->window->surface;
    sdl_Font *font = &button->owner->window->font;
    SDL_Color colour = button->selected? button->sel_colour: button->unsel_colour;
    
    if (!button->visible) return;

    SDL_FillRect(surface, &button->pos, SDL_MapRGB(surface->format, colour.r, colour.g, colour.b));

    if (strlen(button->caption))
    {
        size_t len = strlen(button->caption);
        unsigned max = button->pos.w / font->width;
        int n = MIN(len, max);
        int l = n * font->width / 2;
        int x = button->pos.x + ((button->pos.w) / 2) - l;
        
        sdl_FontDraw(font, surface, button->cap_colour, x, button->pos.y + 1, n, button->caption);
    }
}


/*
 * Adjust the position of a button
 */
static void sdl_ButtonMove(sdl_Button *button, int x, int y)
{
    button->pos.x = x;
    button->pos.y = y;
    button->owner->need_update = true;
}


/*
 * Adjust the size of a button
 */
static void sdl_ButtonSize(sdl_Button *button, int w, int h)
{
    button->pos.w = w;
    button->pos.h = h;
    button->owner->need_update = true;
}


/*
 * Set the caption
 */
static void sdl_ButtonCaption(sdl_Button *button, const char *s)
{
    my_strcpy(button->caption, s, sizeof(button->caption));
    button->owner->need_update = true;
}


/*
 * Set the visibility of a button
 */
static void sdl_ButtonVisible(sdl_Button *button, bool visible)
{
    if (button->visible != visible)
    {
        button->visible = visible;
        
        button->owner->need_update = true;
    }
}


/*
 * Maximum amount of buttons in a bank
 */
#define MAX_BUTTONS 60


/*
 * The button_bank package
 */


/*
 * Initialize it
 */
static void sdl_ButtonBankInit(sdl_ButtonBank *bank, sdl_Window *window)
{
    bank->window = window;
    bank->buttons = mem_zalloc(MAX_BUTTONS * sizeof(sdl_Button));
    bank->used = mem_zalloc(MAX_BUTTONS * sizeof(bool));
    bank->need_update = true;
}


/*
 * Clear the bank
 */
static void sdl_ButtonBankFree(sdl_ButtonBank *bank)
{
    mem_free(bank->buttons);
    mem_free(bank->used);
}


/*
 * Draw all the buttons on the screen
 */
static void sdl_ButtonBankDrawAll(sdl_ButtonBank *bank)
{
    int i;
    
    for (i = 0; i < MAX_BUTTONS; i++)
    {
        sdl_Button *button = &bank->buttons[i];
        
        if (!bank->used[i]) continue;
        if (!button->visible) continue;
        
        sdl_ButtonDraw(button);
    }
    bank->need_update = false;
}


/*
 * Get a new button index
 */
static int sdl_ButtonBankNew(sdl_ButtonBank *bank)
{
    int i = 0;
    sdl_Button *new_button;

    while (bank->used[i] && (i < MAX_BUTTONS)) i++;

    if (i == MAX_BUTTONS)
    {
        /* Bugger! */
        return (-1);
    }

    /* Get the button */
    new_button = &bank->buttons[i];

    /* Mark the button as used */
    bank->used[i] = true;

    /* Clear it */
    memset(new_button, 0, sizeof(sdl_Button));

    /* Mark it as mine */
    new_button->owner = bank;

    /* Default colours */
    if ((statusbar_color > 0) && (statusbar_color < BASIC_COLORS))
    {
        new_button->unsel_colour.r = text_colours[statusbar_color].r;
        new_button->unsel_colour.g = text_colours[statusbar_color].g;
        new_button->unsel_colour.b = text_colours[statusbar_color].b;
    }
    else
    {
        new_button->unsel_colour.r = d_color_r;
        new_button->unsel_colour.g = d_color_g;
        new_button->unsel_colour.b = d_color_b;
    }

    new_button->sel_colour.r = 210;
    new_button->sel_colour.g = 210;
    new_button->sel_colour.b = 110;
    new_button->cap_colour.r = 0;
    new_button->cap_colour.g = 0;
    new_button->cap_colour.b = 0;

    /* Success */
    return (i);
}


/*
 * Retrieve button 'idx' or NULL
 */
static sdl_Button *sdl_ButtonBankGet(sdl_ButtonBank *bank, int idx)
{
    /* Check the index */
    if ((idx < 0) || (idx >= MAX_BUTTONS)) return (NULL);
    if (!bank->used[idx]) return (NULL);
    
    /* Return it */
    return &bank->buttons[idx];
}


/*
 * Examine and respond to mouse presses
 * Return if we 'handled' the click
 */
static bool sdl_ButtonBankMouseDown(sdl_ButtonBank *bank, int x, int y)
{
    int i;
    
    /* Check every button */
    for (i = 0; i < MAX_BUTTONS; i++)
    {
        sdl_Button *button = &bank->buttons[i];
        
        /* Discard some */
        if (!bank->used[i]) continue;
        if (!button->visible) continue;
        
        /* Check the coordinates */
        if (point_in(&button->pos, x, y))
        {
            button->selected = true;
            
            /* Draw it */
            bank->need_update = true;
            
            return true;
        }
    }
    return false;
}


/*
 * Respond to a mouse button release
 */
static bool sdl_ButtonBankMouseUp(sdl_ButtonBank *bank, int x, int y)
{
    int i;
    
    /* Check every button */
    for (i = 0; i < MAX_BUTTONS; i++)
    {
        sdl_Button *button = &bank->buttons[i];
        
        /* Discard some */
        if (!bank->used[i]) continue;
        if (!button->visible) continue;
        
        /* Check the coordinates */
        if (point_in(&button->pos, x, y))
        {
            /* Has this butoon been 'selected'? */
            if (button->selected)
            {
                /* Activate the button (usually) */
                if (button->activate) (*button->activate)(button);
                
                /* Now not selected */
                button->selected = false;
                
                /* Draw it */
                bank->need_update = true;
                
                return true;
            }
        }
        else
        {
            /* This button was 'selected' but the release of the */
            /* mouse button was outside the area of this button */
            if (button->selected)
            {
                /* Now not selected */
                button->selected = false;
                
                /* Draw it */
                bank->need_update = true;
            }
        }
    }
    
    return false;
}


/*
 * sdl_Window functions
 */
static void sdl_WindowFree(sdl_Window* window)
{
    if (window->surface)
    {
        SDL_FreeSurface(window->surface);
        sdl_ButtonBankFree(&window->buttons);
        sdl_FontFree(&window->font);
        memset(window, 0, sizeof(sdl_Window));
    }
}


/*
 * Initialize a window
 */
static void sdl_WindowInit(sdl_Window* window, int w, int h, SDL_Surface *owner,
    const char *fontname)
{
    sdl_WindowFree(window);
    window->owner = owner;
    window->width = w;
    window->height = h;
    window->surface = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h,
        owner->format->BitsPerPixel, owner->format->Rmask, owner->format->Gmask,
        owner->format->Bmask, owner->format->Amask);
    sdl_ButtonBankInit(&window->buttons, window);
    sdl_FontCreate(&window->font, fontname, 0, window->surface);
    window->visible = true;
    window->need_update = true;
}


static void sdl_WindowBlit(sdl_Window* window)
{
    SDL_Rect rc;
    
    if (!window->visible) return;
    
    sdl_RECT(window->left, window->top, window->width, window->height, &rc);
    
    SDL_BlitSurface(window->surface, NULL, window->owner, &rc);
    SDL_UpdateRects(window->owner, 1, &rc);
}


static void sdl_WindowText(sdl_Window* window, SDL_Color c, int x, int y, const char *s)
{
    sdl_FontDraw(&window->font, window->surface, c, x, y, strlen(s), s);
}


static void sdl_WindowUpdate(sdl_Window* window)
{
    if ((window->need_update || window->buttons.need_update) && (window->visible))
    {
        SDL_Event Event;
        
        SDL_FillRect(window->surface, NULL, back_pixel_colour);
        
        if (window->draw_extra) (*window->draw_extra)(window);
        
        sdl_ButtonBankDrawAll(&window->buttons);
        
        window->need_update = false;
        
        memset(&Event, 0, sizeof(SDL_Event));
        
        Event.type = WINDOW_DRAW;
        
        Event.user.data1 = (void*)window;
        
        SDL_PushEvent(&Event);
    }
}


static void term_windowFree(term_window* win)
{
    if (win->surface)
    {
        SDL_FreeSurface(win->surface);
        win->surface = NULL;

        /* Invalidate the gfx surface */
        if (win->tiles)
        {
            SDL_FreeSurface(win->tiles);
            win->tiles = NULL;
        }

        term_nuke(&win->term_data);
    }
    
    sdl_FontFree(&win->font);
}


static errr save_prefs(void);


/*
 * Display warning message (see "z-util.c")
 */
static void hook_plog(const char *str)
{
#ifdef WINDOWS
    /* Warning */
    if (str) MessageBox(NULL, str, "Warning", MB_ICONEXCLAMATION | MB_OK);
#else
    printf("%s\n", str);
#endif
}


static void hook_quit(const char *str)
{
    int i;
    static bool quitting = false;

    /* Don't re-enter if already quitting */
    if (quitting) return;
    quitting = true;
    
    save_prefs();
    
    string_free(sdl_settings_file);
    
    /* Free the surfaces of the windows */
    for (i = 0; i < ANGBAND_TERM_MAX; i++)
    {
        term_windowFree(&windows[i]);
        string_free(windows[i].req_font);
    }

    /* Free the graphics surfaces */
    if (GfxSurface) SDL_FreeSurface(GfxSurface);

    /* Free the 'System font' */
    sdl_FontFree(&SystemFont);
    
    /* Free the statusbar window */
    sdl_WindowFree(&StatusBar);
    
    /* Free the popup window */
    sdl_WindowFree(&PopUp);
    
    /* Free the main surface */
    SDL_FreeSurface(AppWin);

    mem_free(GfxButtons);
    close_graphics_modes();

    /* Shut down the font library */
    TTF_Quit();
    
    /* Shut down SDL */
    SDL_Quit();
    
    for (i = 0; i < MAX_FONTS; i++)
        string_free(FontList[i]);

    /* Free resources */
    textui_cleanup();
    cleanup_angband();
    close_sound();

    /* Cleanup network stuff */
    Net_cleanup();

#ifdef WINDOWS
    /* Cleanup WinSock */
    WSACleanup();
#endif
}


#ifdef WINDOWS
static BOOL CtrlHandler(DWORD fdwCtrlType)
{
    switch (fdwCtrlType)
    {
        case CTRL_CLOSE_EVENT:
            quit(NULL);
            return FALSE;
        default:
            return FALSE;
    }
}
#endif


static void BringToTop(void)
{
    int i, idx;
    
    for (idx = 0; idx < ANGBAND_TERM_MAX; idx++)
    {
        if (Zorder[idx] == SelectedTerm) break;
    }

    if (idx == ANGBAND_TERM_MAX) return;
    
    for (i = idx; i < ANGBAND_TERM_MAX - 1; i++)
    {
        Zorder[i] = Zorder[i + 1];
    }
    
    Zorder[ANGBAND_TERM_MAX - 1] = SelectedTerm;
}


/*
 * Validate a file
 */
static void validate_file(const char *s)
{
    if (!file_exists(s))
        quit_fmt("Cannot find required file:\n%s", s);
}


/*
 * Find a window that is under the points x,y on
 * the main screen
 */

static int sdl_LocateWin(int x, int y)
{
    int i;
    
    for (i = ANGBAND_TERM_MAX - 1; i >= 0; i--)
    {
        term_window *win = &windows[Zorder[i]];
        SDL_Rect rc;
        
        if (!win->visible) continue;
        if (!point_in(sdl_RECT(win->left, win->top, win->width, win->height, &rc), x, y))
            continue;
        
        return (Zorder[i]);
    }
    
    return (-1);
}


static void draw_statusbar(sdl_Window *window)
{
    char buf[128];
    term_window *win = &windows[SelectedTerm];
    int fw = window->font.width;
    int x = 1;
    sdl_Button *button;
    SDL_Rect rc;
    SDL_Color c;

    sdl_RECT(0, StatusBar.height - 1, StatusBar.width, 1, &rc);

    c.r = d_color_r;
    c.g = d_color_g;
    c.b = d_color_b;

    if ((statusbar_color > 0) && (statusbar_color < BASIC_COLORS))
    {
        c.r = text_colours[statusbar_color].r;
        c.g = text_colours[statusbar_color].g;
        c.b = text_colours[statusbar_color].b;
    }
    SDL_FillRect(StatusBar.surface, &rc, SDL_MapRGB(StatusBar.surface->format, c.r, c.g, c.b));

    button = sdl_ButtonBankGet(&StatusBar.buttons, AboutSelect);
    x += button->pos.w + 20;

    sdl_WindowText(&StatusBar, c, x, 1, "Term:");
    x += 5 * fw;
    
    button = sdl_ButtonBankGet(&StatusBar.buttons, TermSelect);
    button->pos.x = x;
    x += button->pos.w + 10;

    my_strcpy(buf, format("(%dx%d)", win->cols, win->rows), sizeof(buf));
    sdl_WindowText(&StatusBar, c, x, 1, buf);
    x += strlen(buf) * fw + 20;
    
    sdl_WindowText(&StatusBar, c, x, 1, "Visible:");
    x += 8 * fw;
    
    button = sdl_ButtonBankGet(&StatusBar.buttons, VisibleSelect);
    button->pos.x = x;
    x += button->pos.w + 20;
    
    button = sdl_ButtonBankGet(&StatusBar.buttons, FontSelect);
    if (button->visible) sdl_WindowText(&StatusBar, c, x, 1, "Font:");
    x += 5 * fw;
    
    button->pos.x = x;
    x += button->pos.w + 20;
    
    button = sdl_ButtonBankGet(&StatusBar.buttons, MoreSelect);
    button->pos.x = x;
    
    x += button->pos.w + 20;
}


static void sdl_BlitWin(term_window *win)
{
    SDL_Rect rc;

    if (!win->surface) return;
    if (!win->visible) return;
    if (win->uRect.x == -1) return;

    /* Select the area to be updated */
    sdl_RECT(win->left + win->uRect.x, win->top + win->uRect.y, win->uRect.w, win->uRect.h, &rc);
    
    SDL_BlitSurface(win->surface, &win->uRect, AppWin, &rc);
    SDL_UpdateRects(AppWin, 1, &rc);

    /* Mark the update as complete */
    win->uRect.x = -1;
}


static void sdl_SizingSpot(term_window *win, bool relative, SDL_Rect *prc)
{
    int xoffset = (relative? 0: win->left);
    int yoffset = (relative? 0: win->top);

    sdl_RECT(xoffset + win->width - 10, yoffset + win->height - 10, 8, 8, prc);
}


static void sdl_BlitAll(void)
{
    SDL_Rect rc;
    sdl_Window *window = &StatusBar;
    int i;
    SDL_Color colour;

    colour.r = d_color_r;
    colour.g = d_color_g;
    colour.b = d_color_b;

    SDL_FillRect(AppWin, NULL, back_pixel_colour);

    for (i = 0; i < ANGBAND_TERM_MAX; i++)
    {
        term_window *win = &windows[Zorder[i]];
        
        if (!win->surface) continue;
        if (!win->visible) continue;

        sdl_RECT(win->left, win->top, win->width, win->height, &rc);
        SDL_BlitSurface(win->surface, NULL, AppWin, &rc);

        if (Zorder[i] == SelectedTerm)
        {
            sdl_SizingSpot(win, false, &SizingSpot);

            if (Sizing)
            {
                int grabsize = 10;

                rc = SizingRect;
                sdl_RECT(SizingRect.x + SizingRect.w - grabsize,
                    SizingRect.y + SizingRect.h - grabsize, grabsize, grabsize, &SizingSpot);
            }
        }

        /* Paranoia: always redraw the borders of the window */
        if ((win->windowborders >= 0) && (win->windowborders < BASIC_COLORS))
            sdl_DrawBox(AppWin, &rc, text_colours[win->windowborders], win->border);
        else
            sdl_DrawBox(AppWin, &rc, colour, win->border);
    }

    sdl_RECT(window->left, window->top, window->width, window->height, &rc);

    SDL_BlitSurface(window->surface, NULL, AppWin, &rc);

    SDL_UpdateRect(AppWin, 0, 0, AppWin->w, AppWin->h);
}


static void RemovePopUp(void)
{
    PopUp.visible = false;
    popped = false;

    // Hack -- weather
    if (player->weather_type != 0)
    {
        // Decrease "icky" depth
        player->screen_save_depth--;
        Send_icky();
    }

    sdl_BlitAll();
}


static void QuitActivate(sdl_Button *sender)
{
    SDL_Event Event;
    
    Event.type = SDL_QUIT;
    
    SDL_PushEvent(&Event);
}


static void SetStatusButtons(void)
{
    term_window *win = &windows[SelectedTerm];
    sdl_Button *button = sdl_ButtonBankGet(&StatusBar.buttons, TermSelect);
    sdl_Button *fontbutton = sdl_ButtonBankGet(&StatusBar.buttons, FontSelect);
    sdl_Button *visbutton = sdl_ButtonBankGet(&StatusBar.buttons, VisibleSelect);
    
    sdl_ButtonCaption(button, angband_term_name[SelectedTerm]);
    
    if (!win->visible)
    {
        sdl_ButtonVisible(fontbutton, false);
        sdl_ButtonCaption(visbutton, "No");
    }
    else
    {
        sdl_ButtonVisible(fontbutton, true);
        sdl_ButtonCaption(fontbutton, win->font.name);
        sdl_ButtonCaption(visbutton, "Yes");
    }
}


static void TermFocus(int idx)
{
    if (SelectedTerm == idx) return;
    
    SelectedTerm = idx;
    
    BringToTop();
    
    SetStatusButtons();
    
    sdl_BlitAll();
}


static void AboutDraw(sdl_Window *win)
{
    SDL_Rect rc;
    SDL_Rect icon;
    
    /* Wow - a different colour! */
    SDL_Color colour = {160, 60, 60, 0};

    sdl_RECT(0, 0, win->width, win->height, &rc);
    
    /* Draw a nice box */
    SDL_FillRect(win->surface, &win->surface->clip_rect,
        SDL_MapRGB(win->surface->format, 255, 255, 255));
    sdl_DrawBox(win->surface, &win->surface->clip_rect, colour, 5);
    if (mratt)
    {
        sdl_RECT((win->width - mratt->w) / 2, 5, mratt->w, mratt->h, &icon);
        SDL_BlitSurface(mratt, NULL, win->surface, &icon);
    }
    sdl_WindowText(win, colour, 20, 150,
        format("You are playing %s", version_build(VERSION_NAME, true)));
    sdl_WindowText(win, colour, 20, 160, "See https://tangaria.com");
}


static void AboutActivate(sdl_Button *sender)
{
    int width = 350;
    int height = 200;
    
    sdl_WindowInit(&PopUp, width, height, AppWin, StatusBar.font.name);
    PopUp.left = (AppWin->w / 2) - width / 2;
    PopUp.top = (AppWin->h / 2) - height / 2;
    PopUp.draw_extra = AboutDraw;
    
    popped = true;

    // Hack -- weather
    if (player->weather_type != 0)
    {
        // Increase "icky" depth
        player->screen_save_depth++;
        Send_icky();
    }
}


static void SelectTerm(sdl_Button *sender)
{
    RemovePopUp();
    
    TermFocus(sender->tag);
}


static int get_term_namewidth(void)
{
    int i, maxl = 0;

    for (i = 0; i < ANGBAND_TERM_MAX; i++)
    {
        int l = strlen(angband_term_name[i]);
        if (l > maxl) maxl = l;
    }

    return (maxl * StatusBar.font.width + 20);
}


static void TermActivate(sdl_Button *sender)
{
    int i;
    int width, height = ANGBAND_TERM_MAX * (StatusBar.font.height + 1);
    
    width = get_term_namewidth();

    sdl_WindowInit(&PopUp, width, height, AppWin, StatusBar.font.name);
    PopUp.left = sender->pos.x;
    PopUp.top = sender->pos.y;

    for (i = 0; i < ANGBAND_TERM_MAX; i++)
    {
        int h = PopUp.font.height;
        int b = sdl_ButtonBankNew(&PopUp.buttons);
        sdl_Button *button = sdl_ButtonBankGet(&PopUp.buttons, b);
        sdl_ButtonSize(button, width - 2 , h);
        sdl_ButtonMove(button, 1, i * (h + 1));
        sdl_ButtonCaption(button, angband_term_name[i]);
        sdl_ButtonVisible(button, true);
        button->tag = i;
        button->activate = SelectTerm;
    }
    popped = true;

    // Hack -- weather
    if (player->weather_type != 0)
    {
        // Increase "icky" depth
        player->screen_save_depth++;
        Send_icky();
    }
}


static void ResizeWin(term_window* win, int w, int h);
static void term_data_link_sdl(term_window *win);


static void VisibleActivate(sdl_Button *sender)
{
    term_window *window = &windows[SelectedTerm];
    
    if (SelectedTerm == 0) return;

    /* Reinitialize all subwindows */
    subwindows_reinit_flags();

    if (window->visible)
    {
        window->visible = false;
        term_windowFree(window);
        angband_term[SelectedTerm] = NULL;
    }
    else
    {
        window->visible = true;
        ResizeWin(window, window->width, window->height);
    }

    /* Set up the subwindows */
    subwindows_init_flags();
    
    SetStatusButtons();
    sdl_BlitAll();

    /* Push a key to force redraw */
    Term_key_push(ESCAPE);
}


static void SelectFont(sdl_Button *sender)
{
    term_window *window = &windows[SelectedTerm];
    int w, h;
    
    sdl_FontFree(&window->font);
    string_free(window->req_font);

    window->req_font = string_make(sender->caption);
    
    sdl_CheckFont(window->req_font, window->font_size, &w, &h);

    /* Invalidate the gfx surface */
    if (window->tiles)
    {
        SDL_FreeSurface(window->tiles);
        window->tiles = NULL;
    }
    
    ResizeWin(window, (w * window->cols) + (2 * window->border),
        (h * window->rows) + window->border + window->title_height);
    
    SetStatusButtons();
    
    RemovePopUp();
}


static int get_font_namewidth(void)
{
    int i, maxl = 0;

    for (i = 0; i < num_fonts; i++)
    {
        int l = strlen(FontList[i]);
        if (l > maxl) maxl = l;
    }

    return (maxl * StatusBar.font.width + 20);
}


static void FontActivate(sdl_Button *sender)
{
    int i;
    int width, height = num_fonts * (StatusBar.font.height + 1);

    width = get_font_namewidth();

    sdl_WindowInit(&PopUp, width, height, AppWin, StatusBar.font.name);
    PopUp.left = sender->pos.x;
    PopUp.top = sender->pos.y;

    for (i = 0; i < num_fonts; i++)
    {
        int h = PopUp.font.height;
        int b = sdl_ButtonBankNew(&PopUp.buttons);
        sdl_Button *button = sdl_ButtonBankGet(&PopUp.buttons, b);
        sdl_ButtonSize(button, width - 2 , h);
        sdl_ButtonMove(button, 1, i * (h + 1));
        sdl_ButtonCaption(button, FontList[i]);
        sdl_ButtonVisible(button, true);
        button->activate = SelectFont;
    }
    popped = true;

    // Hack -- weather
    if (player->weather_type != 0)
    {
        // Increase "icky" depth
        player->screen_save_depth++;
        Send_icky();
    }
}


static errr load_gfx(void);
static bool do_update_w = false;
static bool do_update = false;


static void SelectGfx(sdl_Button *sender)
{
    SelectedGfx = sender->tag;
}


/*
 * Compute tile width/height multipliers to display the best possible tiles
 */
static void apply_nice_graphics(term_window* win)
{
    graphics_mode *mode = get_graphics_mode(use_graphics, true);
    int nice_tile_wid = 0, nice_tile_hgt = 0;

    if (!nicegfx) return;

    if (mode && mode->grafID)
    {
        if (mode->file[0])
        {
            char *end;

            nice_tile_wid = strtol(mode->file, &end, 10);
            nice_tile_hgt = strtol(end + 1, NULL, 10);
        }
        if ((nice_tile_wid == 0) || (nice_tile_hgt == 0))
        {
            nice_tile_wid = mode->cell_width;
            nice_tile_hgt = mode->cell_height;
        }
    }
    if ((nice_tile_wid == 0) || (nice_tile_hgt == 0))
    {
        nice_tile_wid = win->tile_wid;
        nice_tile_hgt = win->tile_hgt;
    }

    if (nice_tile_wid >= win->tile_wid * 2)
        tile_width = nice_tile_wid / win->tile_wid;
    if (nice_tile_hgt >= win->tile_hgt * 2)
        tile_height = nice_tile_hgt / win->tile_hgt;
}


static void AcceptChanges(sdl_Button *sender)
{
    sdl_Button *button;
    bool do_video_reset = false;

    if (use_graphics != SelectedGfx)
    {
        do_update = true;

        use_graphics = SelectedGfx;
    }

    if (!use_graphics) reset_tile_params();

    button = sdl_ButtonBankGet(&PopUp.buttons, MoreNiceGfx);
    if (button->tag != nicegfx)
    {
        nicegfx = !nicegfx;
        do_update = true;
    }

    load_gfx();

    /* Reset visuals */
    reset_visuals(true);

    /* Invalidate all the gfx surfaces */
    if (do_update)
    {
        int i;

        for (i = 0; i < ANGBAND_TERM_MAX; i++)
        {
            term_window *win = &windows[i];

            if (win->tiles)
            {
                SDL_FreeSurface(win->tiles);
                win->tiles = NULL;
            }
        }
    }

    button = sdl_ButtonBankGet(&PopUp.buttons, MoreFullscreen);

    if (button->tag != fullscreen)
    {
        fullscreen = !fullscreen;

        do_video_reset = true;
    }

    SetStatusButtons();

    RemovePopUp();

    /* Hacks */
    if (do_update_w)
    {
        ResizeWin(&windows[SelectedTerm], windows[SelectedTerm].width, windows[SelectedTerm].height);

        /* Show on the screen */
        sdl_BlitAll();
    }

    if (do_update)
    {
        /* Redraw */
        if (Setup.initialized)
        {
            ResizeWin(&windows[0], windows[0].width, windows[0].height);
            do_cmd_redraw();
        }

        /* This will set up the window correctly */
        else
            ResizeWin(&windows[0], windows[0].width, windows[0].height);
    }

    if (do_video_reset)
    {
        SDL_Event Event;

        memset(&Event, 0, sizeof(SDL_Event));

        Event.type = SDL_VIDEORESIZE;
        Event.resize.w = screen_w;
        Event.resize.h = screen_h;

        SDL_PushEvent(&Event);
    }

    do_update_w = false;
    do_update = false;
}


static void FlipTag(sdl_Button *sender)
{
    if (sender->tag)
    {
        sender->tag = 0;
        sdl_ButtonCaption(sender, "Off");
    }
    else
    {
        sender->tag = 1;
        sdl_ButtonCaption(sender, "On");
    }
}


static void SnapChange(sdl_Button *sender)
{
    SnapRange += sender->tag;
    if (SnapRange < 0) SnapRange = 0;
    if (SnapRange > 20) SnapRange = 20;
    PopUp.need_update = true;
}


static void WidthChange(sdl_Button *sender)
{
    tile_width += sender->tag;
    if (tile_width < 1) tile_width = 1;
    if (tile_width > 12) tile_width = 12;
    do_update = true;
}


static void HeightChange(sdl_Button *sender)
{
    tile_height += sender->tag;
    if (tile_height < 1) tile_height = 1;
    if (tile_height > 8) tile_height = 8;
    do_update = true;
}


static void FontSizeChange(sdl_Button *sender)
{
    term_window *window = &windows[SelectedTerm];

    window->font_size += sender->tag;
    if (window->font_size < 4) window->font_size = 4;
    if (window->font_size > 64) window->font_size = 64;
    do_update_w = true;
    do_update = true;
}


static void WindowBordersChange(sdl_Button *sender)
{
    term_window *window = &windows[SelectedTerm];

    window->windowborders += sender->tag;
    if (window->windowborders < 0) window->windowborders = BASIC_COLORS;
    if (window->windowborders > BASIC_COLORS) window->windowborders = 0;
    do_update_w = true;
    do_update = true;
}


static void SoundVolumeChange(sdl_Button *sender)
{
    sound_volume += sender->tag;
    if (sound_volume < 0) sound_volume = 0;
    if (sound_volume > 100) sound_volume = 100;
}


static void MusicVolumeChange(sdl_Button *sender)
{
    music_volume += sender->tag;
    if (music_volume < 0) music_volume = 0;
    if (music_volume > 100) music_volume = 100;
}


static void MoreDraw(sdl_Window *win)
{
    term_window *window = &windows[SelectedTerm];
    SDL_Rect rc;
    sdl_Button *button;
    int y = 20;
    graphics_mode *mode;

    /* Wow - a different colour! */
    SDL_Color colour = {160, 60, 60, 0};
    SDL_Color select_colour = {210, 110, 110, 0};

    sdl_RECT(0, 0, win->width, win->height, &rc);

    /* Draw a nice box */
    sdl_DrawBox(win->surface, &rc, colour, 5);

    button = sdl_ButtonBankGet(&win->buttons, MoreWidthMinus);
    sdl_ButtonVisible(button, SelectedGfx? true: false);

    button = sdl_ButtonBankGet(&win->buttons, MoreWidthPlus);
    sdl_ButtonVisible(button, SelectedGfx? true: false);

    button = sdl_ButtonBankGet(&win->buttons, MoreHeightMinus);
    sdl_ButtonVisible(button, SelectedGfx? true: false);

    button = sdl_ButtonBankGet(&win->buttons, MoreHeightPlus);
    sdl_ButtonVisible(button, SelectedGfx? true: false);

    if (SelectedGfx)
    {
        sdl_WindowText(win, colour, 20, y, format("Tile width is %d.", tile_width));
        button = sdl_ButtonBankGet(&win->buttons, MoreWidthMinus);
        sdl_ButtonMove(button, 150, y);

        button = sdl_ButtonBankGet(&win->buttons, MoreWidthPlus);
        sdl_ButtonMove(button, 180, y);

        y += 20;

        sdl_WindowText(win, colour, 20, y, format("Tile height is %d.", tile_height));
        button = sdl_ButtonBankGet(&win->buttons, MoreHeightMinus);
        sdl_ButtonMove(button, 150, y);

        button = sdl_ButtonBankGet(&win->buttons, MoreHeightPlus);
        sdl_ButtonMove(button, 180, y);

        y += 20;
    }

    button = sdl_ButtonBankGet(&win->buttons, MoreNiceGfx);
    sdl_WindowText(win, colour, 20, y, "Nice graphics is:");

    sdl_ButtonMove(button, 150, y);
    y += 20;

    /* Allow only in initial phase */
    if (!Setup.initialized)
    {
        sdl_WindowText(win, colour, 20, y, "Selected Graphics:");

        mode = get_graphics_mode(SelectedGfx, false);
        if (mode && mode->grafID)
            sdl_WindowText(win, select_colour, 150, y, mode->menuname);
        else
            sdl_WindowText(win, select_colour, 150, y, "None");

        y += 20;

        sdl_WindowText(win, colour, 20, y, "Available Graphics:");

        mode = graphics_modes;
        while (mode)
        {
            if (mode->menuname[0])
            {
                button = sdl_ButtonBankGet(&win->buttons, GfxButtons[mode->grafID]);
                sdl_ButtonMove(button, 150, y);
                y += 20;
            }
            mode = mode->pNext;
        }
    }

    button = sdl_ButtonBankGet(&win->buttons, MoreFullscreen);
    sdl_WindowText(win, colour, 20, y, "Fullscreen is:");

    sdl_ButtonMove(button, 150, y);
    y += 20;

    sdl_WindowText(win, colour, 20, y, format("Window borders is %d.", window->windowborders));
    button = sdl_ButtonBankGet(&win->buttons, MoreWindowBordersMinus);
    sdl_ButtonMove(button, 150, y);

    button = sdl_ButtonBankGet(&win->buttons, MoreWindowBordersPlus);
    sdl_ButtonMove(button, 180, y);

    y += 20;

    sdl_WindowText(win, colour, 20, y, format("Sound Volume is %d.", sound_volume));
    button = sdl_ButtonBankGet(&win->buttons, MoreSoundVolumeMinus);
    sdl_ButtonMove(button, 150, y);

    button = sdl_ButtonBankGet(&win->buttons, MoreSoundVolumePlus);
    sdl_ButtonMove(button, 180, y);

    y += 20;

    sdl_WindowText(win, colour, 20, y, format("Music Volume is %d.", music_volume));
    button = sdl_ButtonBankGet(&win->buttons, MoreMusicVolumeMinus);
    sdl_ButtonMove(button, 150, y);

    button = sdl_ButtonBankGet(&win->buttons, MoreMusicVolumePlus);
    sdl_ButtonMove(button, 180, y);

    y += 20;

    sdl_WindowText(win, colour, 20, y, format("Snap range is %d.", SnapRange));
    button = sdl_ButtonBankGet(&win->buttons, MoreSnapMinus);
    sdl_ButtonMove(button, 150, y);

    button = sdl_ButtonBankGet(&win->buttons, MoreSnapPlus);
    sdl_ButtonMove(button, 180, y);

    y += 20;

    sdl_WindowText(win, colour, 20, y, format("Font size is %d.", window->font_size));
    button = sdl_ButtonBankGet(&win->buttons, MoreFontSizeMinus);
    sdl_ButtonMove(button, 150, y);

    button = sdl_ButtonBankGet(&win->buttons, MoreFontSizePlus);
    sdl_ButtonMove(button, 180, y);
}


static int get_gfx_namewidth(void)
{
    int maxl = 0, l;
    graphics_mode *mode = graphics_modes;

    while (mode)
    {
        if (mode->menuname[0])
        {
            l = strlen(mode->menuname);
            if (l > maxl) maxl = l;
        }
        mode = mode->pNext;
    }

    return (maxl * StatusBar.font.width + 20);
}


static void MoreActivate(sdl_Button *sender)
{
    int width = 300;
    int height = 320;
    sdl_Button *button;
    graphics_mode *mode;
    SDL_Color ucolour = {160, 60, 60, 0};
    SDL_Color scolour = {210, 110, 110, 0};
    int gfx_namewidth = get_gfx_namewidth();

    sdl_WindowInit(&PopUp, width, height, AppWin, StatusBar.font.name);
    PopUp.left = (AppWin->w / 2) - width / 2;
    PopUp.top = (AppWin->h / 2) - height / 2;
    PopUp.draw_extra = MoreDraw;

    SelectedGfx = use_graphics;

    MoreWidthPlus = sdl_ButtonBankNew(&PopUp.buttons);
    button = sdl_ButtonBankGet(&PopUp.buttons, MoreWidthPlus);

    button->unsel_colour = ucolour;
    button->sel_colour = scolour;
    sdl_ButtonSize(button, 20, PopUp.font.height + 2);
    sdl_ButtonCaption(button, "+");
    button->tag = 1;
    sdl_ButtonVisible(button, SelectedGfx? true: false);
    button->activate = WidthChange;

    MoreWidthMinus = sdl_ButtonBankNew(&PopUp.buttons);
    button = sdl_ButtonBankGet(&PopUp.buttons, MoreWidthMinus);

    button->unsel_colour = ucolour;
    button->sel_colour = scolour;
    sdl_ButtonSize(button, 20, PopUp.font.height + 2);
    sdl_ButtonCaption(button, "-");
    button->tag = -1;
    sdl_ButtonVisible(button, SelectedGfx? true: false);
    button->activate = WidthChange;

    MoreHeightPlus = sdl_ButtonBankNew(&PopUp.buttons);
    button = sdl_ButtonBankGet(&PopUp.buttons, MoreHeightPlus);

    button->unsel_colour = ucolour;
    button->sel_colour = scolour;
    sdl_ButtonSize(button, 20, PopUp.font.height + 2);
    sdl_ButtonCaption(button, "+");
    button->tag = 1;
    sdl_ButtonVisible(button, SelectedGfx? true: false);
    button->activate = HeightChange;

    MoreHeightMinus = sdl_ButtonBankNew(&PopUp.buttons);
    button = sdl_ButtonBankGet(&PopUp.buttons, MoreHeightMinus);

    button->unsel_colour = ucolour;
    button->sel_colour = scolour;
    sdl_ButtonSize(button, 20, PopUp.font.height + 2);
    sdl_ButtonCaption(button, "-");
    button->tag = -1;
    sdl_ButtonVisible(button, SelectedGfx? true: false);
    button->activate = HeightChange;

    MoreNiceGfx = sdl_ButtonBankNew(&PopUp.buttons);
    button = sdl_ButtonBankGet(&PopUp.buttons, MoreNiceGfx);

    button->unsel_colour = ucolour;
    button->sel_colour = scolour;
    sdl_ButtonSize(button, 50, PopUp.font.height + 2);
    sdl_ButtonVisible(button, true);
    sdl_ButtonCaption(button, nicegfx? "On": "Off");
    button->tag = nicegfx;
    button->activate = FlipTag;

    /* Allow only in initial phase */
    if (!Setup.initialized)
    {
        mode = graphics_modes;
        while (mode)
        {
            if (mode->menuname[0])
            {
                GfxButtons[mode->grafID] = sdl_ButtonBankNew(&PopUp.buttons);
                button = sdl_ButtonBankGet(&PopUp.buttons, GfxButtons[mode->grafID]);

                button->unsel_colour = ucolour;
                button->sel_colour = scolour;
                sdl_ButtonSize(button, gfx_namewidth, PopUp.font.height + 2);
                sdl_ButtonVisible(button, true);
                sdl_ButtonCaption(button, mode->menuname);
                button->tag = mode->grafID;
                button->activate = SelectGfx;
            }
            mode = mode->pNext;
        }
    }

    MoreFullscreen = sdl_ButtonBankNew(&PopUp.buttons);
    button = sdl_ButtonBankGet(&PopUp.buttons, MoreFullscreen);

    button->unsel_colour = ucolour;
    button->sel_colour = scolour;
    sdl_ButtonSize(button, 50, PopUp.font.height + 2);
    sdl_ButtonVisible(button, true);
    sdl_ButtonCaption(button, fullscreen? "On": "Off");
    button->tag = fullscreen;
    button->activate = FlipTag;
    
    MoreWindowBordersPlus = sdl_ButtonBankNew(&PopUp.buttons);
    button = sdl_ButtonBankGet(&PopUp.buttons, MoreWindowBordersPlus);

    button->unsel_colour = ucolour;
    button->sel_colour = scolour;
    sdl_ButtonSize(button, 20, PopUp.font.height + 2);
    sdl_ButtonCaption(button, "+");
    button->tag = 1;
    sdl_ButtonVisible(button, true);
    button->activate = WindowBordersChange;

    MoreWindowBordersMinus = sdl_ButtonBankNew(&PopUp.buttons);
    button = sdl_ButtonBankGet(&PopUp.buttons, MoreWindowBordersMinus);

    button->unsel_colour = ucolour;
    button->sel_colour = scolour;
    sdl_ButtonSize(button, 20, PopUp.font.height + 2);
    sdl_ButtonCaption(button, "-");
    button->tag = -1;
    sdl_ButtonVisible(button, true);
    button->activate = WindowBordersChange;

    MoreSoundVolumePlus = sdl_ButtonBankNew(&PopUp.buttons);
    button = sdl_ButtonBankGet(&PopUp.buttons, MoreSoundVolumePlus);

    button->unsel_colour = ucolour;
    button->sel_colour = scolour;
    sdl_ButtonSize(button, 20, PopUp.font.height + 2);
    sdl_ButtonCaption(button, "+");
    button->tag = 5;
    sdl_ButtonVisible(button, true);
    button->activate = SoundVolumeChange;

    MoreSoundVolumeMinus = sdl_ButtonBankNew(&PopUp.buttons);
    button = sdl_ButtonBankGet(&PopUp.buttons, MoreSoundVolumeMinus);

    button->unsel_colour = ucolour;
    button->sel_colour = scolour;
    sdl_ButtonSize(button, 20, PopUp.font.height + 2);
    sdl_ButtonCaption(button, "-");
    button->tag = -5;
    sdl_ButtonVisible(button, true);
    button->activate = SoundVolumeChange;

    MoreMusicVolumePlus = sdl_ButtonBankNew(&PopUp.buttons);
    button = sdl_ButtonBankGet(&PopUp.buttons, MoreMusicVolumePlus);

    button->unsel_colour = ucolour;
    button->sel_colour = scolour;
    sdl_ButtonSize(button, 20, PopUp.font.height + 2);
    sdl_ButtonCaption(button, "+");
    button->tag = 5;
    sdl_ButtonVisible(button, true);
    button->activate = MusicVolumeChange;

    MoreMusicVolumeMinus = sdl_ButtonBankNew(&PopUp.buttons);
    button = sdl_ButtonBankGet(&PopUp.buttons, MoreMusicVolumeMinus);

    button->unsel_colour = ucolour;
    button->sel_colour = scolour;
    sdl_ButtonSize(button, 20, PopUp.font.height + 2);
    sdl_ButtonCaption(button, "-");
    button->tag = -5;
    sdl_ButtonVisible(button, true);
    button->activate = MusicVolumeChange;

    MoreSnapPlus = sdl_ButtonBankNew(&PopUp.buttons);
    button = sdl_ButtonBankGet(&PopUp.buttons, MoreSnapPlus);

    button->unsel_colour = ucolour;
    button->sel_colour = scolour;
    sdl_ButtonSize(button, 20, PopUp.font.height + 2);
    sdl_ButtonCaption(button, "+");
    button->tag = 1;
    sdl_ButtonVisible(button, true);
    button->activate = SnapChange;

    MoreSnapMinus = sdl_ButtonBankNew(&PopUp.buttons);
    button = sdl_ButtonBankGet(&PopUp.buttons, MoreSnapMinus);

    button->unsel_colour = ucolour;
    button->sel_colour = scolour;
    sdl_ButtonSize(button, 20, PopUp.font.height + 2);
    sdl_ButtonCaption(button, "-");
    button->tag = -1;
    sdl_ButtonVisible(button, true);
    button->activate = SnapChange;

    MoreFontSizePlus = sdl_ButtonBankNew(&PopUp.buttons);
    button = sdl_ButtonBankGet(&PopUp.buttons, MoreFontSizePlus);

    button->unsel_colour = ucolour;
    button->sel_colour = scolour;
    sdl_ButtonSize(button, 20, PopUp.font.height + 2);
    sdl_ButtonCaption(button, "+");
    button->tag = 1;
    sdl_ButtonVisible(button, true);
    button->activate = FontSizeChange;

    MoreFontSizeMinus = sdl_ButtonBankNew(&PopUp.buttons);
    button = sdl_ButtonBankGet(&PopUp.buttons, MoreFontSizeMinus);

    button->unsel_colour = ucolour;
    button->sel_colour = scolour;
    sdl_ButtonSize(button, 20, PopUp.font.height + 2);
    sdl_ButtonCaption(button, "-");
    button->tag = -1;
    sdl_ButtonVisible(button, true);
    button->activate = FontSizeChange;

    MoreOK = sdl_ButtonBankNew(&PopUp.buttons);
    button = sdl_ButtonBankGet(&PopUp.buttons, MoreOK);

    button->unsel_colour = ucolour;
    button->sel_colour = scolour;
    sdl_ButtonSize(button, 50, PopUp.font.height + 2);
    sdl_ButtonVisible(button, true);
    sdl_ButtonCaption(button, "OK");
    sdl_ButtonMove(button, width / 2 - 25, height - 40);
    button->activate = AcceptChanges;

    popped = true;

    // Hack -- weather
    if (player->weather_type != 0)
    {
        // Increase "icky" depth
        player->screen_save_depth++;
        Send_icky();
    }
}


static errr Term_xtra_sdl_clear(void);


/* Window size bounds checking */
static void check_bounds_resize(term_window *win, int *cols, int *rows, int *max_rows, int *width,
    int *height)
{
    int dummy;

    /* Get the amount of columns & rows */
    *cols = dummy = (*width - (win->border * 2)) / win->tile_wid;
    *rows = *max_rows = (*height - win->border - win->title_height) / win->tile_hgt;

    check_term_resize((win->Term_idx == 0), cols, rows);
    check_term_resize((win->Term_idx == 0), &dummy, max_rows);

    /* Calculate the width & height */
    *width = (*cols * win->tile_wid) + (win->border * 2);
    *height = (*max_rows * win->tile_hgt) + win->border + win->title_height;
}


/*
 * Make a window with size (x,y) pixels
 * Note: The actual size of the window may end up smaller.
 * This may be called when a window wants resizing,
 * is made visible, or the font has changed.
 * This function doesn't go in for heavy optimization, and doesn't need it-
 * it may initialize a few too many redraws or whatnot, but everything gets done!
 */
static void ResizeWin(term_window *win, int w, int h)
{
    /* Don't bother */
    if (!win->visible) return;

    win->border = 2;
    win->title_height = StatusHeight;

    /* No font - a new font is needed -> get dimensions */
    if (!win->font.data)
    {
        /* Get font dimensions */
        sdl_CheckFont(win->req_font, win->font_size, &win->tile_wid, &win->tile_hgt);

        /* Oops */
        if (!win->tile_wid || !win->tile_hgt)
        {
			quit_fmt("Unable to find font '%s'.\n"
                "Note that there are new extended font files ending in 'x' in %s.\n"
                "Please check %s and edit if necessary.",
                win->req_font, ANGBAND_DIR_FONTS, sdl_settings_file);
        }

        /* Apply nice graphics */
        if (win->Term_idx == 0)
        {
            apply_nice_graphics(win);
            tile_distorted = is_tile_distorted(use_graphics, tile_width, tile_height);
        }
    }

    /* Initialize the width & height */
    win->width = w;
    win->height = h;

    /* Window size bounds checking */
    check_bounds_resize(win, &win->cols, &win->rows, &win->max_rows, &win->width, &win->height);

    /* Delete the old surface */
    if (win->surface) SDL_FreeSurface(win->surface);

    /* Create a new surface */
    win->surface = SDL_CreateRGBSurface(SDL_SWSURFACE, win->width, win->height,
        AppWin->format->BitsPerPixel, AppWin->format->Rmask, AppWin->format->Gmask,
        AppWin->format->Bmask, AppWin->format->Amask);

    /* Fill it */
    if ((win->windowborders >= 0) && (win->windowborders < BASIC_COLORS))
    {
        SDL_FillRect(win->surface, NULL, SDL_MapRGB(AppWin->format,
            text_colours[win->windowborders].r, text_colours[win->windowborders].g,
            text_colours[win->windowborders].b));
    }
    else
        SDL_FillRect(win->surface, NULL, SDL_MapRGB(AppWin->format, d_color_r, d_color_g, d_color_b));

    /* Label it */
    sdl_FontDraw(&SystemFont, win->surface, back_colour, 1, 1,
        strlen(angband_term_name[win->Term_idx]), angband_term_name[win->Term_idx]);

    /* Mark the whole window for redraw */
    sdl_RECT(0, 0, win->width, win->height, &win->uRect);

    /* Create the font if we need to */
    if (!win->font.data)
        sdl_FontCreate(&win->font, win->req_font, win->font_size, win->surface);

    /* This window was never visible before, or needs resizing */
    if (!angband_term[win->Term_idx])
    {
        term *old = Term;

        /* Initialize the term data */
        term_data_link_sdl(win);

        /* Make it visible to angband */
        angband_term[win->Term_idx] = &win->term_data;

        /* Activate it */
        Term_activate((term*)&win->term_data);

        /* Redraw */
        Term_redraw();

        /* Restore */
        Term_activate(old);
    }
    else
    {
        term *old = Term;

        /* Activate it */
        Term_activate((term*)&win->term_data);

        /* Resize */
        Term_resize(win->cols, win->rows, win->max_rows);

        /* Redraw */
        Term_redraw();

        /* Restore */
        Term_activate(old);
    }

    /* Calculate the hotspot */
    if (win->Term_idx == SelectedTerm) sdl_SizingSpot(win, false, &SizingSpot);

    StatusBar.need_update = true;

    /* Dungeon size */
    if (win->Term_idx == 0)
        net_term_resize(win->cols, win->rows, win->max_rows);

    /* Hack -- redraw all windows */
    if (Setup.initialized) do_cmd_redraw();
}


static errr load_prefs(void)
{
    char buf[MSG_LEN];
    ang_file *fff;

    /* Build the path */
    path_build(buf, sizeof(buf), ANGBAND_DIR_USER, "sdlinit.txt");
    sdl_settings_file = string_make(buf);

    /* Open the file */
    fff = file_open(buf, MODE_READ, FTYPE_TEXT);

    /* Check it */
    if (!fff) return (1);

    /* Process the file */
    while (file_getl(fff, buf, sizeof(buf)))
    {
        char *s;
        if (!buf[0]) continue;

        if (buf[0] == '#') continue;

        s = strchr(buf, '=');
        s++;
        while (!isalnum(*s)) s++;

        if (strstr(buf, "Resolution"))
        {
            screen_w = atoi(s);
            s = strchr(buf, 'x');
            screen_h = atoi(s + 1);
        }
        else if (strstr(buf, "Fullscreen"))
            fullscreen = atoi(s);
        else if (strstr(buf, "SoundVolume"))
            sound_volume = atoi(s);
        else if (strstr(buf, "MusicVolume"))
            music_volume = atoi(s);
        else if (strstr(buf, "DefaultColor"))
            sscanf(s, "%d,%d,%d", &d_color_r, &d_color_g, &d_color_b);
        else if (strstr(buf, "StatusBarColor"))
            statusbar_color = atoi(s);
        else if (strstr(buf, "NiceGraphics"))
            nicegfx = atoi(s);
        else if (strstr(buf, "Graphics"))
            use_graphics = atoi(s);
        else if (strstr(buf, "TileWidth"))
            tile_width = atoi(s);
        else if (strstr(buf, "TileHeight"))
            tile_height = atoi(s);
    }

    if (screen_w < MIN_SCREEN_WIDTH) screen_w = MIN_SCREEN_WIDTH;
    if (screen_h < MIN_SCREEN_HEIGHT) screen_h = MIN_SCREEN_HEIGHT;

    if (sound_volume < 0) sound_volume = 0;
    if (sound_volume > 100) sound_volume = 100;

    if (music_volume < 0) music_volume = 0;
    if (music_volume > 100) music_volume = 100;

    if ((d_color_r < 0) || (d_color_r > 255) || (d_color_g < 0) || 
        (d_color_g > 255) || (d_color_b < 0) || (d_color_b > 255))
    {
        d_color_r = 160;
        d_color_g = 160;
        d_color_b = 60;
    }

    if ((statusbar_color < 0) || (statusbar_color >= BASIC_COLORS)) statusbar_color = 0;

    file_close(fff);

    tile_distorted = is_tile_distorted(use_graphics, tile_width, tile_height);

    return (0);
}


static errr load_window_prefs(void)
{
    char buf[MSG_LEN];
    ang_file *fff;
    term_window *win;
    int i, w, h, b = 2;

    /* Initialize the windows with default values */
    for (i = 0; i < ANGBAND_TERM_MAX; i++)
    {
        win = &windows[i];

        /* Clear the data */
        memset(win, 0, sizeof(term_window));

        /* Who? */
        win->Term_idx = i;

        /* Default font */
        win->req_font = string_make(DEFAULT_FONT_FILE);

        /* Default font size */
        win->font_size = DEFAULT_FONT_SIZE;

        /* Default window borders */
        win->windowborders = BASIC_COLORS;

        /* Default width & height */
        sdl_CheckFont(win->req_font, win->font_size, &w, &h);
        win->width = (NORMAL_WID * w) + (b * 2);
        win->height = (NORMAL_HGT * h) + b + StatusHeight;

        /* Default values */
        if (i == 0)
        {
            win->top = StatusHeight;
            win->keys = 1024;
            win->visible = true;
        }
        else
        {
            win->top = windows[0].top + windows[0].height + (i * 10);
            win->left = (i - 1) * 10;
            win->keys = 32;
            win->visible = false;
        }
    }

    /* Open the file */
    fff = file_open(sdl_settings_file, MODE_READ, FTYPE_TEXT);

    /* Check it */
    if (!fff) return (1);

    /* Process the file */
    while (file_getl(fff, buf, sizeof(buf)))
    {
        char *s;
        if (!buf[0]) continue;

        if (buf[0] == '#') continue;

        s = strchr(buf, '=');
        s++;
        while (!isalnum(*s)) s++;

        if (strstr(buf, "Window"))
            win = &windows[atoi(s)];
        else if (strstr(buf, "Visible"))
            win->visible = atoi(s);
        else if (strstr(buf, "Left"))
            win->left = atoi(s);
        else if (strstr(buf, "Top"))
            win->top = atoi(s);
        else if (strstr(buf, "Width"))
            win->width = atoi(s);
        else if (strstr(buf, "Height"))
            win->height = atoi(s);
        else if (strstr(buf, "Keys"))
            win->keys = atoi(s);
        else if (strstr(buf, "WinBorders"))
            win->windowborders = atoi(s);
        else if (strstr(buf, "FontSize"))
            win->font_size = atoi(s);
        else if (strstr(buf, "Font"))
        {
            string_free(win->req_font);
            win->req_font = string_make(s);
        }
    }

    file_close(fff);

    return (0);
}


static errr save_prefs(void)
{
    ang_file *fff;
    int i;

    /* Open the file */
    fff = file_open(sdl_settings_file, MODE_WRITE, FTYPE_TEXT);

    /* Check it */
    if (!fff) return (1);

    file_putf(fff, "Resolution = %dx%d\n", screen_w, screen_h);
    file_putf(fff, "Fullscreen = %d\n", fullscreen);
    file_putf(fff, "SoundVolume = %d\n", sound_volume);
    file_putf(fff, "MusicVolume = %d\n", music_volume);
    file_putf(fff, "DefaultColor = %d,%d,%d\n", d_color_r, d_color_g, d_color_b);
    file_putf(fff, "StatusBarColor = %d\n", statusbar_color);
    file_putf(fff, "NiceGraphics = %d\n", nicegfx);
    file_putf(fff, "Graphics = %d\n", use_graphics);
    file_putf(fff, "TileWidth = %d\n", tile_width);
    file_putf(fff, "TileHeight = %d\n", tile_height);

    for (i = 0; i < ANGBAND_TERM_MAX; i++)
    {
        term_window *win = &windows[i];

        file_putf(fff, "\nWindow = %d\n", i);
        file_putf(fff, "Visible = %d\n", (int)win->visible);
        file_putf(fff, "Left = %d\n", win->left);
        file_putf(fff, "Top = %d\n", win->top);
        file_putf(fff, "Width = %d\n", win->width);
        file_putf(fff, "Height = %d\n", win->height);
        file_putf(fff, "Keys = %d\n", win->keys);
        file_putf(fff, "WinBorders = %d\n", win->windowborders);
        file_putf(fff, "FontSize = %d\n", win->font_size);
        file_putf(fff, "Font = %s\n", win->req_font);
    }   

    file_close(fff);

    /* Done */
    return (0);
}


static void set_update_rect(term_window *win, SDL_Rect *rc);


static void DrawSizeWidget(void)
{
    Uint32 colour = SDL_MapRGB(AppWin->format, 30, 160, 70);

    SDL_FillRect(AppWin, &SizingSpot, colour);
    SDL_UpdateRects(AppWin, 1, &SizingSpot);
}


static int Movingx;
static int Movingy;


/*
 * Is What within Range units of Origin
 */
#define closeto(Origin, What, Range) \
    ((ABS((Origin) - (What))) < (Range))


/*
 * This function keeps the 'mouse' info up to date,
 * and reacts to mouse buttons appropriately.
 */
static void sdl_HandleMouseEvent(SDL_Event *event)
{
    term_window *win;

    switch (event->type)
    {
        /* Mouse moved */
        case SDL_MOUSEMOTION:
        {
            mouse.x = event->motion.x;
            mouse.y = event->motion.y;
            win = &windows[SelectedTerm];

            /* We are moving or resizing a window */
            if (Moving)
            {
                int i;

                /* Move the window */
                win->left = (mouse.x - Movingx);
                win->top = (mouse.y - Movingy);

                /* Left bounds check */
                if (win->left < 0)
                {
                    win->left = 0;
                    Movingx = mouse.x;
                }

                /* Right bounds check */
                if ((win->left + win->width) > AppWin->w)
                {
                    win->left = AppWin->w - win->width;
                    Movingx = mouse.x - win->left;
                }

                /* Top bounds check */
                if (win->top < StatusHeight)
                {
                    win->top = StatusHeight;
                    Movingy = mouse.y - win->top;
                }

                /* Bottom bounds check */
                if ((win->top + win->height) > AppWin->h)
                {
                    win->top = AppWin->h - win->height;
                    Movingy = mouse.y - win->top;
                }

                for (i = 0; i < ANGBAND_TERM_MAX; i++)
                {
                    term_window *snapper = &windows[i];

                    /* cannot snap to self... */
                    if (i == SelectedTerm) continue;

                    /* cannot snap to the invisible */
                    if (!snapper->visible) continue;

                    /* Check the windows are across from each other */
                    if ((snapper->top < win->top + win->height) &&
                        (win->top < snapper->top + snapper->height))
                    {
                        /* Lets try to the left... */
                        if (closeto(win->left, snapper->left + snapper->width, SnapRange))
                        {
                            win->left = snapper->left + snapper->width;
                            Movingx = mouse.x - win->left;
                        }

                        /* Maybe to the right */
                        if (closeto(win->left + win->width, snapper->left, SnapRange))
                        {
                            win->left = snapper->left - win->width;
                            Movingx = mouse.x - win->left;
                        }
                    }
                    
                    /* Check the windows are above/below each other */
                    if ((snapper->left < win->left + win->width) &&
                        (win->left < snapper->left + snapper->width))
                    {
                        /* Lets try to the top... */
                        if (closeto(win->top, snapper->top + snapper->height, SnapRange))
                        {
                            win->top = snapper->top + snapper->height;
                            Movingy = mouse.y - win->top;
                        }

                        /* Maybe to the bottom */
                        if (closeto(win->top + win->height, snapper->top, SnapRange))
                        {
                            win->top = snapper->top - win->height;
                            Movingy = mouse.y - win->top;
                        }
                    }
                }

                /* Show on the screen */
                sdl_BlitAll();
            }
            else if (Sizing)
            {
                int dummy_cols, dummy_rows, dummy_max_rows;
                int rect_width, rect_height;

                /* Calculate the dimensions of the sizing rectangle */
                rect_width = win->width - win->left + (mouse.x - Movingx);
                rect_height = win->height - win->top + (mouse.y - Movingy);

                /* Window size bounds checking */
                check_bounds_resize(win, &dummy_cols, &dummy_rows, &dummy_max_rows, &rect_width,
                    &rect_height);

                /* Adjust the sizing rectangle */
                SizingRect.w = rect_width;
                SizingRect.h = rect_height;

                /* Show on the screen */                
                sdl_BlitAll();
            }

            else if (!popped)
            {
                /* Have a look for the corner stuff */
                if (point_in(&SizingSpot, mouse.x, mouse.y))
                {
                    if (!Sizingshow)
                    {
                        /* Indicate the hotspot */
                        Sizingshow = true;
                        DrawSizeWidget();
                    }
                }

                /* Remove the hotspot */
                else if (Sizingshow)
                {
                    SDL_Rect rc;

                    Sizingshow = false;
                    sdl_SizingSpot(win, true, &rc);
                    set_update_rect(win, &rc);
                    sdl_BlitWin(win);
                }
            }

            break;
        }
            
        /* A button has been pressed */
        case SDL_MOUSEBUTTONDOWN:
        {
            sdl_Window *window;
            bool res;
            int idx = sdl_LocateWin(mouse.x, mouse.y);
            
            if (event->button.button == SDL_BUTTON_LEFT)
            {
                mouse.left = 1;
                mouse.leftx = event->button.x;
                mouse.lefty = event->button.y;

                /* Pop up window gets priority */
                if (popped) window = &PopUp; else window = &StatusBar;

                /* React to a button press */
                res = sdl_ButtonBankMouseDown(&window->buttons,
                    mouse.x - window->left, mouse.y - window->top);

                /* If pop-up window active and no reaction, cancel the popup */
                if (popped && !res)
                {
                    RemovePopUp();
                    break;
                }

                /* Has this mouse press been handled */
                if (res) break;

                /* Is the mouse press in a term_window? */
                if (idx < 0) break;

                /* The 'focused' window has changed */
                if (idx != SelectedTerm) TermFocus(idx);

                /* A button press has happened on the focused term window */
                win = &windows[idx];

                /* Check for mouse press in the title bar... */
                if (mouse.y < win->top + win->title_height)
                {
                    /* Let's get moving */
                    Moving = true;

                    /* Remember where we started */
                    Movingx = mouse.x - win->left;
                    Movingy = mouse.y - win->top;
                }

                /* ...or the little hotspot in the bottom right corner... */
                else if (point_in(&SizingSpot, mouse.x, mouse.y))
                {
                    /* Let's get sizing */
                    Sizing = true;

                    /* Create the sizing rectangle */
                    sdl_RECT(win->left, win->top, win->width, win->height, &SizingRect);

                    /* Remember where we started */
                    Movingx = mouse.x - win->left;
                    Movingy = mouse.y - win->top;
                }
            }
            else if (event->button.button == SDL_BUTTON_RIGHT)
            {
                mouse.right = 1;
                mouse.rightx = event->button.x;
                mouse.righty = event->button.y;

                /* Right-click always cancels the popup */
                if (popped) popped = false;
            }

            break;
        }

        case SDL_MOUSEBUTTONUP:
        {
            /* Handle release of left button */
            if (event->button.button == SDL_BUTTON_LEFT)
            {
                sdl_Window *window;
                bool res;
                mouse.left = 0;

                /* Pop up window gets priority */
                if (popped) window = &PopUp; else window = &StatusBar;

                /* React to a button release */
                res = sdl_ButtonBankMouseUp(&window->buttons, mouse.x - window->left,
                    mouse.y - window->top);

                /* Cancel popup */
                if (popped && !res) RemovePopUp();

                /* Finish moving */
                if (Moving)
                {
                    Moving = false;

                    /* Update */
                    sdl_BlitAll();
                }

                /* Finish sizing */
                if (Sizing)
                {
                    /* Sort out the window */
                    ResizeWin(&windows[SelectedTerm], SizingRect.w, SizingRect.h);
                    Sizing = false;
                    Sizingshow = false;

                    /* Update */
                    sdl_BlitAll();
                }
            }
            else if (event->button.button == SDL_BUTTON_RIGHT)
                mouse.right = 0;

            break;
        }
    }
}


/*
 * Handle keypresses.
 *
 * We treat left and right modifier keys as equivalent.
 * We ignore any key without a valid SDLK index.
 */
static void sdl_keypress(SDL_keysym keysym)
{
    uint16_t key_code = keysym.unicode;
    SDLKey key_sym = keysym.sym;
    int ch = 0;

    /* Store the value of various modifier keys */
    bool mc = ((keysym.mod & KMOD_CTRL) > 0);
	bool ms = ((keysym.mod & KMOD_SHIFT) > 0);
	bool ma = ((keysym.mod & KMOD_ALT) > 0);
	bool mm = ((keysym.mod & KMOD_META) > 0);
    /*bool mg = ((keysym.mod & KMOD_MODE) > 0);*/
    bool kp = false;
    uint8_t mods = ((ma? KC_MOD_ALT: 0) | (mm? KC_MOD_META: 0));

    /* Hack -- for keyboards with Alt-Gr translated into KMOD_RALT | KMOD_LCTRL */
    /*if ((keysym.mod & KMOD_RALT) && (keysym.mod & KMOD_LCTRL)) mg = true;*/

    /* Ignore if main term is not initialized */
    if (!Term) return;

    /* Handle all other valid SDL keys */
    switch (key_sym)
    {
        /* Keypad */
        case SDLK_KP0: ch = '0'; kp = true; break;
        case SDLK_KP1: ch = '1'; kp = true; break;
        case SDLK_KP2: ch = '2'; kp = true; break;
        case SDLK_KP3: ch = '3'; kp = true; break;
        case SDLK_KP4: ch = '4'; kp = true; break;
        case SDLK_KP5: ch = '5'; kp = true; break;
        case SDLK_KP6: ch = '6'; kp = true; break;
        case SDLK_KP7: ch = '7'; kp = true; break;
        case SDLK_KP8: ch = '8'; kp = true; break;
        case SDLK_KP9: ch = '9'; kp = true; break;
        case SDLK_KP_PERIOD: ch = '.'; kp = true; break;
        case SDLK_KP_DIVIDE: ch = '/'; kp = true; break;
        case SDLK_KP_MULTIPLY: ch = '*'; kp = true; break;
        case SDLK_KP_MINUS: ch = '-'; kp = true; break;
        case SDLK_KP_PLUS: ch = '+'; kp = true; break;
        case SDLK_KP_ENTER: ch = KC_ENTER; kp = true; break;
        case SDLK_KP_EQUALS: ch = '='; kp = true; break;

        /* Have these to get consistent ctrl-shift behaviour */
        /*case SDLK_0: if ((!ms || mc || ma) && !mg) ch = '0'; break;
        case SDLK_1: if ((!ms || mc || ma) && !mg) ch = '1'; break;
        case SDLK_2: if ((!ms || mc || ma) && !mg) ch = '2'; break;
        case SDLK_3: if ((!ms || mc || ma) && !mg) ch = '3'; break;
        case SDLK_4: if ((!ms || mc || ma) && !mg) ch = '4'; break;
        case SDLK_5: if ((!ms || mc || ma) && !mg) ch = '5'; break;
        case SDLK_6: if ((!ms || mc || ma) && !mg) ch = '6'; break;
        case SDLK_7: if ((!ms || mc || ma) && !mg) ch = '7'; break;
        case SDLK_8: if ((!ms || mc || ma) && !mg) ch = '8'; break;
        case SDLK_9: if ((!ms || mc || ma) && !mg) ch = '9'; break;*/

        case SDLK_UP: ch = ARROW_UP; break;
        case SDLK_DOWN: ch = ARROW_DOWN; break;
        case SDLK_RIGHT: ch = ARROW_RIGHT; break;
        case SDLK_LEFT: ch = ARROW_LEFT; break;

        case SDLK_INSERT: ch = KC_INSERT; break;
        case SDLK_HOME: ch = KC_HOME; break;
        case SDLK_PAGEUP: ch = KC_PGUP; break;
        case SDLK_DELETE: ch = KC_DELETE; break;
        case SDLK_END: ch = KC_END; break;
        case SDLK_PAGEDOWN: ch = KC_PGDOWN; break;
        case SDLK_ESCAPE: ch = ESCAPE; break;
        case SDLK_BACKSPACE: ch = KC_BACKSPACE; break;
        case SDLK_RETURN: ch = KC_ENTER; break;
        case SDLK_TAB: ch = KC_TAB; break;

        case SDLK_F1: ch = KC_F1; break;
        case SDLK_F2: ch = KC_F2; break;
        case SDLK_F3: ch = KC_F3; break;
        case SDLK_F4: ch = KC_F4; break;
        case SDLK_F5: ch = KC_F5; break;
        case SDLK_F6: ch = KC_F6; break;
        case SDLK_F7: ch = KC_F7; break;
        case SDLK_F8: ch = KC_F8; break;
        case SDLK_F9: ch = KC_F9; break;
        case SDLK_F10: ch = KC_F10; break;
        case SDLK_F11: ch = KC_F11; break;
        case SDLK_F12: ch = KC_F12; break;
        case SDLK_F13: ch = KC_F13; break;
        case SDLK_F14: ch = KC_F14; break;
        case SDLK_F15: ch = KC_F15; break;

        default: break;
    }

    if (ch)
    {
        if (kp) mods |= KC_MOD_KEYPAD;
        if (mc) mods |= KC_MOD_CONTROL;
        if (ms) mods |= KC_MOD_SHIFT;
        Term_keypress(ch, mods);
    }
    else if (key_code)
    {
        /* If the keycode is 7-bit ASCII (except numberpad) send directly to the game */
        if (mc && ((key_sym == SDLK_TAB) || (key_sym == SDLK_RETURN) ||
            (key_sym == SDLK_BACKSPACE) || MODS_INCLUDE_CONTROL(key_code)))
        {
            mods |= KC_MOD_CONTROL;
        }
        if (ms && MODS_INCLUDE_SHIFT(key_code)) mods |= KC_MOD_SHIFT;

        Term_keypress(key_code, mods);
    }
}


static void init_windows(void);
static void init_morewindows(void);


/*
 * Handle a single message sent to the application.
 *
 * Functions that are either called from a separate thread or which need to
 * create a separate thread (such as sounds) need to pass messages to this
 * function in order to execute most operations.  See the useage of
 * "SDL_USEREVENT".
 */
static errr sdl_HandleEvent(SDL_Event *event)
{
    /* Handle message */
    switch (event->type)
    {
        /* Keypresses */
        case SDL_KEYDOWN:
        {
            /* Handle keypress */
            sdl_keypress(event->key.keysym);
            break;
        }
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
        {
            /* Handle mouse stuff */
            sdl_HandleMouseEvent(event);
            break;
        }

        case SDL_MOUSEMOTION:
        {
            int i;
            SDL_Event events[10];

            /*
             * If there are a bundle of mouse movements pending,
             * we'll just take every tenth one - this makes a
             * simple approach to dragging practical, for instance.
             */  
            i = SDL_PeepEvents(events, 10, SDL_GETEVENT,
                SDL_EVENTMASK(SDL_MOUSEMOTION));
            if (i > 0) *event = events[i - 1];

            /* Handle mouse stuff */
            sdl_HandleMouseEvent(event);
            break;
        }
            
        /* Shut down the game */
        /* XXX - check for stuck inside menu etc... */
        case SDL_QUIT:
        {
            save_prefs();

            quit(NULL);
            break;
        }
            
        /* Resize the application */
        case SDL_VIDEORESIZE:
        {
            /* Free the surface */
            SDL_FreeSurface(AppWin);

            if (!fullscreen)
            {
                /* Make sure */
                vflags &= ~(SDL_FULLSCREEN);
                vflags |= SDL_RESIZABLE;

                screen_w = event->resize.w;
                screen_h = event->resize.h;

                if (screen_w < MIN_SCREEN_WIDTH) screen_w = MIN_SCREEN_WIDTH;
                if (screen_h < MIN_SCREEN_HEIGHT) screen_h = MIN_SCREEN_HEIGHT;

                /* Resize the application surface */
                AppWin = SDL_SetVideoMode(screen_w, screen_h, 0, vflags);
            }
            else
            {
                /* Make sure */
                vflags |= SDL_FULLSCREEN;
                vflags &= ~(SDL_RESIZABLE);

                AppWin = SDL_SetVideoMode(full_w, full_h, 0, vflags);
            }
            init_windows();
            init_morewindows();
            sdl_BlitAll();

            break;
        }
            
        case WINDOW_DRAW:
        {
            /* Redraw window that have asked */
            sdl_Window *window = (sdl_Window*)event->user.data1;
            sdl_WindowBlit(window);
            break;
        }

        default:
        {
            /* Do nothing */
            break;
        }
    }   
    sdl_WindowUpdate(&StatusBar);
    sdl_WindowUpdate(&PopUp);
    return (0);
}


/*
 * Update the redraw rect
 * A simple but effective way to keep track of what
 * parts of a window need to updated.
 * Any new areas that are updated before a blit are simply combined
 * into a new larger rectangle to encompass all changes.
 */
static void set_update_rect(term_window *win, SDL_Rect *rc)
{
    /* No outstanding update areas yet? */
    if (win->uRect.x == -1)
    {
        /* Simple copy */
        win->uRect = *rc;
    }
    else
    {
        /* Combine the old update area with the new */
        int x = MIN(win->uRect.x, rc->x);
        int y = MIN(win->uRect.y, rc->y);
        int x2 = MAX(win->uRect.x + win->uRect.w, rc->x + rc->w);
        int y2 = MAX(win->uRect.y + win->uRect.h, rc->y + rc->h);
        sdl_RECT(x, y, x2 - x, y2 - y, &win->uRect);
    }
}


/*
 * Clear a terminal window
 */
static errr Term_xtra_sdl_clear(void)
{
    term_window *win = (term_window*)(Term->data);
    SDL_Rect rc;

    /* Oops */
    if (!win->surface) return (1);

    /* Create the fill area */
    sdl_RECT(win->border, win->title_height, win->width - (2 * win->border),
         win->height - win->border - win->title_height, &rc);

    /* Fill the rectangle */
    SDL_FillRect(win->surface, &rc, back_pixel_colour);

    /* Rectangle to update */
    set_update_rect(win, &rc);

    /* Success */
    return (0);
}


/*
 * Process at least one event
 */
static errr Term_xtra_sdl_event(int v)
{
    SDL_Event event;
    errr error = 0;

    /* Wait or check for an event */
    if (v)
    {
        /* Wait for an event */
        if (SDL_WaitEvent(&event))
        {
            /* Handle it */
            error = sdl_HandleEvent(&event);
        }
        else return (1);
    }
    else
    {
        /* Get a single pending event */
        if (SDL_PollEvent(&event))
        {
            /* Handle it */
            error = sdl_HandleEvent(&event);
        }
    }

    /* Note success or failure */
    return (error);
}


/*
 * Process all pending events
 */
static errr Term_xtra_sdl_flush(void)
{
    SDL_Event event;

    /* Get all pending events */
    while (SDL_PollEvent(&event))
    {
        /* Handle them (ignore errors) */
        sdl_HandleEvent(&event);
    }

    /* Done */
    return (0);
}


/*
 * Delay for "x" milliseconds
 */
static errr Term_xtra_sdl_delay(int v)
{
    /* Sleep */
    if (v > 0)
    {
        Term_xtra_sdl_event(0);
        SDL_Delay(v);
    }

    /* Success */
    return (0);
}


static errr get_sdl_rect(term_window *win, int col, int row, bool translate, SDL_Rect *prc)
{
    /* Make the destination rectangle */
    sdl_RECT(col * win->tile_wid, row * win->tile_hgt, win->tile_wid, win->tile_hgt, prc);

    /* Stretch for bigtile mode */
    if (!Term->minimap_active)
    {
	    prc->w *= tile_width;
	    prc->h *= tile_height;
    }

    /* Translate it */
    prc->x += win->border;
    prc->y += win->title_height;

    /* Success */
    return (0);
}


/*
 * Displays the "normal" cursor
 */
static errr Term_curs_sdl(int col, int row)
{
    term_window *win = (term_window*)(Term->data);
    SDL_Color colour = text_colours[COLOUR_YELLOW];
    SDL_Rect rc;

    /* Make a rectangle */
    sdl_RECT(col * win->tile_wid, row * win->tile_hgt, win->tile_wid, win->tile_hgt, &rc);

    /* Translate it */
    rc.x += win->border;
    rc.y += win->title_height;

    /* Paranoia */
    if (rc.y > win->height) return (-1);

    /* Draw it */
    sdl_DrawBox(win->surface, &rc, colour, 1);

    /* Update area */
    set_update_rect(win, &rc);

    /* Success */
    return (0);
}


/*
 * Displays the "big" cursor
 */
static errr Term_bigcurs_sdl(int col, int row)
{
    term_window *win = (term_window*)(Term->data);
    SDL_Color colour = text_colours[COLOUR_YELLOW];
    SDL_Rect rc;
    /*uint16_t a, ta;
    char c, tc;
    int j = 0;*/

    if (Term->minimap_active)
    {
        /* Normal cursor in map window */
        Term_curs_sdl(col, row);
        return 0;
    }

    /* Make a rectangle */
    if (get_sdl_rect(win, col, row, true, &rc)) return (1);

    /* If we are using overdraw, draw a double height cursor (disabled for now) */
    /*if (!Term_info(col, row, &a, &c, &ta, &tc)) j = (a & 0x7F);
    if (overdraw && (j > ROW_MAP + 1) && (j >= overdraw) && (j <= overdraw_max))
    {
        rc.y -= rc.h;
        rc.h = (rc.h << 1);
    }*/

    /* Draw it */
    sdl_DrawBox(win->surface, &rc, colour, 1);

    /* Update area */
    set_update_rect(win, &rc);

    /* Success */
    return (0);
}


static errr Term_xtra_sdl(int n, int v)
{
    switch (n)
    {
        /* Process an event */
        case TERM_XTRA_EVENT:
            return (Term_xtra_sdl_event(v));

        /* Flush all events */
        case TERM_XTRA_FLUSH:
            return (Term_xtra_sdl_flush());

        /* Clear the screen */
        case TERM_XTRA_CLEAR:
            return (Term_xtra_sdl_clear());

        /* Show or hide the cursor */
        case TERM_XTRA_SHAPE:
        {
            int x, y;

            /* Obtain the cursor */
            Term_locate(&x, &y);

            /* Show or hide the cursor */
            Term_curs_sdl(x, y);
            return (0);
        }

        case TERM_XTRA_FRESH:
        {
            /* Get the current window data */
            term_window *win = (term_window*)(Term->data);

            /* Blat it! */
            sdl_BlitWin(win);

            /* Done */
            return (0);
        }

        case TERM_XTRA_DELAY:
            return (Term_xtra_sdl_delay(v));

        case TERM_XTRA_REACT:
        {
            int i;

            /* Re-initialize the colours */
            back_colour.r = angband_color_table[COLOUR_DARK][1];
            back_colour.g = angband_color_table[COLOUR_DARK][2];
            back_colour.b = angband_color_table[COLOUR_DARK][3];
            back_pixel_colour = SDL_MapRGB(AppWin->format, back_colour.r, back_colour.g,
                back_colour.b);
            for (i = 0; i < MAX_COLORS; i++)
            {
                text_colours[i].r = angband_color_table[i][1];
                text_colours[i].g = angband_color_table[i][2];
                text_colours[i].b = angband_color_table[i][3];
            }

            if (use_graphics != v)
            {
                use_graphics = v;
                if (!use_graphics) reset_tile_params();
                load_gfx();
                reset_visuals(true);

                /* Redraw */
                if (Setup.initialized)
                    do_cmd_redraw();

                /* Apply nice graphics */
                else
                {
                    apply_nice_graphics(&windows[0]);
                    tile_distorted = is_tile_distorted(use_graphics, tile_width, tile_height);
                }
            }
        }
    }

    return (1);
}


static errr Term_wipe_sdl(int col, int row, int n)
{
    term_window *win = (term_window*)(Term->data);
    SDL_Rect rc;

    /* Build the area to black out */
    rc.x = col * win->tile_wid;
    rc.y = row * win->tile_hgt;
    rc.w = win->tile_wid * n;
    rc.h = win->tile_hgt;

    /* Translate it */
    rc.x += win->border;
    rc.y += win->title_height;

    /* Paranoia */
    if (rc.y > win->height) return (-1);

    /* Wipe it */
    SDL_FillRect(win->surface, &rc, back_pixel_colour);

    /* Update */
    set_update_rect(win, &rc);

    return (0);
}


/*
 * Do a 'stretched blit'
 * SDL has no support for stretching... What a bastard!
 */
static void sdl_StretchBlit(SDL_Surface *src, SDL_Rect *srcRect, SDL_Surface *dest,
    SDL_Rect *destRect)
{
    int x, y;
    int sx, sy, dx, dy;
    Uint8 *ps, *pd;

    for (y = 0; y < destRect->h; y++)
    {
        for (x = 0; x < destRect->w; x++)
        {
            /* Actual source coords */
            sx = (srcRect->w * x / (destRect->w)) + srcRect->x;
            sy = (srcRect->h * y / (destRect->h)) + srcRect->y;

            /* Find a source pixel */
            ps = (Uint8 *)src->pixels + (sx * src->format->BytesPerPixel) + (sy * src->pitch);

            /* Actual destination pixel coords */
            dx = x + destRect->x;
            dy = y + destRect->y;

            /* Destination pixel */
            pd = (Uint8 *)dest->pixels + (dx * dest->format->BytesPerPixel) + (dy * dest->pitch);

            switch (dest->format->BytesPerPixel)
            {
                case 1:
                {
                    *pd = *ps;
                    break;
                }
                case 2:
                {
                    Uint16 *ps16 = (Uint16*) ps;
                    Uint16 *pd16 = (Uint16*) pd;
                    *pd16 = *ps16;
                    break;
                }
                case 3:
                case 4:
                {
                    Uint32 *ps32 = (Uint32*) ps;
                    Uint32 *pd32 = (Uint32*) pd;
                    *pd32 = *ps32;
                }
            }
        }
    }
}


/*
 * Make the 'pre-stretched' tiles for this window
 * Assumes the tiles surface was freed elsewhere
 */
static errr sdl_BuildTileset(term_window *win)
{
    int x, y;
    int ta, td;
    int xx, yy;
    graphics_mode *info;
    int dwid, dhgt;
    SDL_Surface *surface = GfxSurface;

    info = get_graphics_mode(use_graphics, true);
    if (!(info && info->grafID)) return (1);

    if (!surface) return (1);

    if (Term->minimap_active)
    {
        dwid = win->tile_wid;
        dhgt = win->tile_hgt;
    }
    else
    {
        dwid = win->tile_wid * tile_width;
        dhgt = win->tile_hgt * tile_height;
    }

    /* Calculate the number of tiles across & down */
    ta = surface->w / info->cell_width;
    td = surface->h / info->cell_height;

    /* Calculate the size of the new surface */
    x = ta * dwid;
    y = td * dhgt;

    /* Make it */
    win->tiles = SDL_CreateRGBSurface(SDL_SWSURFACE, x, y, surface->format->BitsPerPixel,
        surface->format->Rmask, surface->format->Gmask, surface->format->Bmask,
        surface->format->Amask);

    /* Bugger */
    if (!win->tiles) return (1);

    /* For every tile... */
    for (xx = 0; xx < ta; xx++)
    {
        for (yy = 0; yy < td; yy++)
        {
            SDL_Rect src, dest;

            /* Source rectangle (on surface) */
            sdl_RECT(xx * info->cell_width, yy * info->cell_height, info->cell_width,
                info->cell_height, &src);

            /* Destination rectangle (win->tiles) */
			sdl_RECT(xx * dwid, yy * dhgt, dwid, dhgt, &dest);

            /* Do the stretch thing */
            sdl_StretchBlit(surface, &src, win->tiles, &dest);
        }
    }

    return (0);
}


/*
 * Draw a tile, given its position and dimensions.
 *
 * If "prc" is not null, only draw the portion of the tile inside that rectangle. We suppose here
 * that the two rectangles "rc" and "prc" are overlapping.
 */
static void sdl_DrawTile(term_window *win, int col, int row, SDL_Rect rc, SDL_Rect *prc, uint16_t a,
    char c, bool background)
{
    int j = (a & 0x7F);
    SDL_Rect src;

    /* Get the dimensions of the graphic surface */
    src.w = rc.w;
    src.h = rc.h;

    /* Default background to darkness */
    src.x = 0;
    src.y = 0;

    /* Use the terrain picture only if mapped */
    if ((a & 0x80) || !background)
    {
        src.x = (c & 0x7F) * src.w;
        src.y = j * src.h;
    }

    /* If we are using overdraw, draw the top rectangle */
    if (overdraw && (row > ROW_MAP + 1) && (j >= overdraw) && (j <= overdraw_max))
    {
        /* Double the height */
        src.y -= rc.h;
        rc.y -= rc.h;
        rc.h = (rc.h << 1);
        src.h = rc.h;
        SDL_BlitSurface(win->tiles, &src, win->surface, &rc);
    }

    /* Draw a portion of the tile */
    else if (prc)
    {
        int dx = prc->x - rc.x;
        int dy = prc->y - rc.y;

        if (dx > 0)
        {
            src.x += dx;
            rc.x = prc->x;
        }
        rc.w -= abs(dx);
        src.w = rc.w;
        if (dy > 0)
        {
            src.y += dy;
            rc.y = prc->y;
        }
        rc.h -= abs(dy);
        src.h = rc.h;
        SDL_BlitSurface(win->tiles, &src, win->surface, &rc);
    }

    /* Draw the tile */
    else
        SDL_BlitSurface(win->tiles, &src, win->surface, &rc);
}


/*
 * Draw foreground and background tiles, given their position and dimensions.
 *
 * If "prc" is not null, only draw the portion of the tiles inside that rectangle. We suppose here
 * that the two rectangles "rc" and "prc" are overlapping.
 */
static void sdl_DrawTiles(term_window *win, int col, int row, SDL_Rect rc, SDL_Rect *prc, uint16_t a,
    char c, uint16_t ta, char tc)
{
    /* Draw the terrain tile */
    sdl_DrawTile(win, col, row, rc, prc, ta, tc, true);

    /* If foreground is the same as background, we're done */
    if ((ta == a) && (tc == c)) return;

    /* Draw the foreground tile */
    sdl_DrawTile(win, col, row, rc, prc, a, c, false);
}


/*
 * Draw some text to a window
 */
static errr Term_text_sdl_aux(int col, int row, int n, uint16_t a, const char *s)
{
    term_window *win = (term_window*)(Term->data);
    SDL_Color colour = text_colours[a % MAX_COLORS];
    SDL_Color bg = text_colours[COLOUR_DARK];
    int x = col * win->tile_wid;
    int y = row * win->tile_hgt;
    char buf[256];

    /* Paranoia */
    if (n > win->cols) return (-1);

    /* Translate */
    x += win->border;
    y += win->title_height;

    /* Not much point really... */
    if (!win->visible) return (0);

    /* Clear the way */
    Term_wipe_sdl(col, row, n);

    /* Take a copy of the incoming string, but truncate it at n chars */
    my_strcpy(buf, s, sizeof(buf));
    buf[n] = '\0';

    /* Handle background */
    switch (a / MAX_COLORS)
    {
        /* Default Background */
        case BG_BLACK: break;

        /* Background same as foreground */
        case BG_SAME: bg = colour; break;

        /* Highlight Background */
        case BG_DARK: bg = text_colours[COLOUR_SHADE]; break;
    }

    /* Draw it */
    return (sdl_mapFontDraw(&win->font, win->font_size, win->surface, colour, bg, x, y, n, buf));
}


/*
 * Draw some text to a window.
 *
 * For double-height tiles, we redraw all double-height tiles below.
 */
static errr Term_text_sdl(int col, int row, int n, uint16_t a, const char *s)
{
    term_window *win = (term_window*)(Term->data);
    SDL_Rect rc;
    int i;
    uint16_t fa, ta;
    char fc, tc;
    int tile_wid = 1, tile_hgt = 1;

    /* Large tile mode */
    if (!Term->minimap_active)
    {
        tile_wid = tile_width;
        tile_hgt = tile_height;
    }

    /* Redraw the current text */
    Term_text_sdl_aux(col, row, n, a, s);

    /* Redraw the bottom tiles (recursively) */
    for (i = 0; i < n; i++)
    {
        int j = 1, tilex, tiley;

        while (j)
        {
            /* Get the position of the jth tile below the ith character */
            tilex = COL_MAP + ((col - COL_MAP + i) / tile_wid) * tile_wid;
            tiley = ROW_MAP + ((row - ROW_MAP) / tile_hgt + j) * tile_hgt;

            if (overdraw && (tiley > ROW_MAP + 1) && !Term_info(tilex, tiley, &fa, &fc, &ta, &tc))
            {
                int row = (fa & 0x7F);
                int trow = (ta & 0x7F);

                if (((trow >= overdraw) && (trow <= overdraw_max)) ||
                    ((row >= overdraw) && (row <= overdraw_max)))
                {
                    get_sdl_rect(win, tilex, tiley, false, &rc);
                    set_update_rect(win, &rc);
                    sdl_DrawTiles(win, tilex, tiley, rc, NULL, fa, fc, ta, tc);
                    j++;
                }
                else j = 0;
            }
            else j = 0;
        }
    }

    /* Highlight the player */
    if (Term->minimap_active && (win->Term_idx == 0) && cursor_x && cursor_y)
        Term_curs_sdl(cursor_x + COL_MAP, cursor_y + ROW_MAP);

    /* Success */
    return 0;
}


/*
 * Put some gfx on the screen.
 *
 * Called with n > 1 only if always_pict is true, which is never the case.
 *
 * For double-height tiles, we redraw the tile just above and all double-height tiles below.
 */
static errr Term_pict_sdl(int col, int row, int n, const uint16_t *ap, const char *cp,
    const uint16_t *tap, const char *tcp)
{
    /* Get the right window */
    term_window *win = (term_window*)(Term->data);
    SDL_Rect rc, rc2;
	int i;
    uint16_t a, ta;
    char c, tc;
    int tile_wid = 1, tile_hgt = 1;

    /* Large tile mode */
    if (!Term->minimap_active)
    {
        tile_wid = tile_width;
        tile_hgt = tile_height;
    }

    /* Toggle minimap view */
    if (win->minimap_active != Term->minimap_active)
    {
        win->minimap_active = Term->minimap_active;
        if (win->tiles)
        {
            SDL_FreeSurface(win->tiles);
            win->tiles = NULL;
        }
    }

    /* First time a pict is requested we load the tileset in */
    if (!win->tiles)
    {
        sdl_BuildTileset(win);
        if (!win->tiles) return (1);
    }

    /* Make the destination rectangle */
    if (get_sdl_rect(win, col, row, true, &rc)) return (1);

    /* Blit 'em! (it) */
    for (i = 0; i < n; i++)
    {
        int j = 1;

        /* Update area */
        set_update_rect(win, &rc);

        /* Clear the way */
        SDL_FillRect(win->surface, &rc, back_pixel_colour);

        /* Redraw the top tile */
        if (overdraw && !Term_info(col + i * tile_wid, row - tile_hgt, &a, &c, &ta, &tc))
        {
            if (a & 0x80)
            {
                get_sdl_rect(win, col + i * tile_wid, row - tile_hgt, false, &rc2);
                set_update_rect(win, &rc2);
                sdl_DrawTiles(win, col + i * tile_wid, row - tile_hgt, rc2, NULL, a, c, ta, tc);
            }
            else
            {
                int tx, ty;

                for (tx = col + i * tile_wid; tx < col + (i + 1) * tile_wid; tx++)
                {
                    for (ty = row - tile_hgt; ty < row; ty++)
                    {
                        Term_info(tx, ty, &a, &c, &ta, &tc);
                        Term_text_sdl_aux(tx, ty, 1, a, &c);
                    }
                }
            }
        }

        /* Draw the terrain and foreground tiles */
        sdl_DrawTiles(win, col + i * tile_wid, row, rc, NULL, ap[i], cp[i], tap[i], tcp[i]);

        /* Redraw the bottom tile (recursively) */
        while (j)
        {
            if (overdraw && (row + j * tile_hgt > ROW_MAP + 1) &&
                !Term_info(col + i * tile_wid, row + j * tile_hgt, &a, &c, &ta, &tc))
            {
                int frow = (a & 0x7F);
                int trow = (ta & 0x7F);

                if (((trow >= overdraw) && (trow <= overdraw_max)) ||
                    ((frow >= overdraw) && (frow <= overdraw_max)))
                {
                    get_sdl_rect(win, col + i * tile_wid, row + j * tile_hgt, false, &rc2);
                    set_update_rect(win, &rc2);
                    sdl_DrawTiles(win, col + i * tile_wid, row + j * tile_hgt, rc2, NULL, a, c,
                        ta, tc);
                    j++;
                }
                else j = 0;
            }
            else j = 0;
        }

        /* Advance */
        rc.x += rc.w;
    }

    /* Highlight the player */
    if (Term->minimap_active && (win->Term_idx == 0) && cursor_x && cursor_y)
        Term_curs_sdl(cursor_x + COL_MAP, cursor_y + ROW_MAP);

    return (0);
}


/*
 * Create and initialize the Term contained within this window.
 */
static void term_data_link_sdl(term_window *win)
{
    term *t = &win->term_data;

    /* Initialize the term */
    term_init(t, win->cols, win->rows, win->max_rows, win->keys);

    t->higher_pict = true;

    /* Use a "software" cursor */
    t->soft_cursor = true;

    /* Erase with "white space" */
    t->attr_blank = COLOUR_WHITE;
    t->char_blank = ' ';

    /* Differentiate between BS/^h, Tab/^i, etc. */
    t->complex_input = true;

    /* Never refresh one row */
    t->never_frosh = true;

    /* Ignore the init/nuke hooks */

    /* Prepare the template hooks */
    t->xtra_hook = Term_xtra_sdl;
    t->curs_hook = Term_curs_sdl;
    t->bigcurs_hook = Term_bigcurs_sdl;
    t->wipe_hook = Term_wipe_sdl;
    t->text_hook = Term_text_sdl;
    t->pict_hook = Term_pict_sdl;

    /* Remember where we came from */
    t->data = win;
}


/*
 * Initialize the status bar:
 *  Populate it with some buttons
 *  Set the custom draw function for the bar
 */
static void init_morewindows(void)
{
    char buf[128];
    sdl_Button *button;
    int x;

    popped = false;

    /* Make sure */
    sdl_WindowFree(&PopUp);

    /* Initialize the status bar */
    sdl_WindowInit(&StatusBar, AppWin->w, StatusHeight, AppWin, DEFAULT_FONT_FILE);

    /* Custom drawing function */
    StatusBar.draw_extra = draw_statusbar;

    /* Don't overlap the buttons */
    if (AppWin->w >= 720) my_strcpy(buf, version_build(VERSION_NAME, true), sizeof(buf));
    else my_strcpy(buf, "About...", sizeof(buf));

    AboutSelect = sdl_ButtonBankNew(&StatusBar.buttons);
    button = sdl_ButtonBankGet(&StatusBar.buttons, AboutSelect);

    /* Initialize the 'about' button */
    sdl_ButtonSize(button, StatusBar.font.width * strlen(buf) + 5, StatusHeight - 2);
    sdl_ButtonMove(button, 1, 1);
    sdl_ButtonVisible(button, true);
    sdl_ButtonCaption(button, buf);
    button->activate = AboutActivate;

    /* New button */
    TermSelect = sdl_ButtonBankNew(&StatusBar.buttons);
    button = sdl_ButtonBankGet(&StatusBar.buttons, TermSelect);

    /* Initialize the 'term' button */
    sdl_ButtonSize(button, get_term_namewidth(), StatusHeight - 2);
    x = 100 + (StatusBar.font.width * 5);
    sdl_ButtonMove(button, x, 1);
    sdl_ButtonVisible(button, true);
    button->activate = TermActivate;

    /* Another new button */
    VisibleSelect = sdl_ButtonBankNew(&StatusBar.buttons);
    button = sdl_ButtonBankGet(&StatusBar.buttons, VisibleSelect);

    /* Initialize the 'visible' button */
    sdl_ButtonSize(button, 50, StatusHeight - 2);
    x = 200 + (StatusBar.font.width * 8);
    sdl_ButtonMove(button, x, 1);
    sdl_ButtonVisible(button, true);
    button->activate = VisibleActivate;

    /* Another new button */
    FontSelect = sdl_ButtonBankNew(&StatusBar.buttons);
    button = sdl_ButtonBankGet(&StatusBar.buttons, FontSelect);

    /* Initialize the 'font_select' button */
    sdl_ButtonSize(button, get_font_namewidth(), StatusHeight - 2);
    sdl_ButtonMove(button, 400, 1);
    button->activate = FontActivate;

    /* Another new button */
    MoreSelect = sdl_ButtonBankNew(&StatusBar.buttons);
    button = sdl_ButtonBankGet(&StatusBar.buttons, MoreSelect);

    /* Initialize the 'more' button */
    sdl_ButtonSize(button, 50, StatusHeight - 2);
    sdl_ButtonMove(button, 400, 1);
    sdl_ButtonVisible(button, true);
    sdl_ButtonCaption(button, "Options");
    button->activate = MoreActivate;

    /* Another new button */
    QuitSelect = sdl_ButtonBankNew(&StatusBar.buttons);
    button = sdl_ButtonBankGet(&StatusBar.buttons, QuitSelect);

    /* Initialize the 'quit' button */
    sdl_ButtonSize(button, 50, StatusHeight - 2);
    sdl_ButtonMove(button, AppWin->w - 51, 1);
    sdl_ButtonCaption(button, "Quit");
    button->activate = QuitActivate;
    sdl_ButtonVisible(button, true);

    SetStatusButtons();

    TermFocus(0);
}


/*
 * The new streamlined graphics loader.
 * Only uses colour keys.
 * Much more tolerant of different bit-planes
 */
static errr load_gfx(void)
{
    char buf[MSG_LEN];
    const char *filename = NULL;
    SDL_Surface *temp;
    graphics_mode *mode;

    if (GfxSurface && is_current_graphics_mode(use_graphics)) return (0);

    mode = get_graphics_mode(use_graphics, true);
    if (mode && mode->grafID) filename = mode->file;

    /* Free the old surfaces */
    if (GfxSurface)
    {
        SDL_FreeSurface(GfxSurface);
        GfxSurface = NULL;
    }

    /* This may be called when GRAPHICS_NONE is set */
    if (!filename) return (0);

    /* Find and load the file into a temporary surface */
    path_build(buf, sizeof(buf), mode->path, filename);
    temp = IMG_Load(buf);
    if (!temp) return (1);

    /* Change the surface type to the current video surface format */
    GfxSurface = SDL_DisplayFormatAlpha(temp);

    overdraw = mode->overdrawRow;
    overdraw_max = mode->overdrawMax;

    /* All good */
    return (0);
}


/*
 * Initialize the graphics
 */
static void init_gfx(void)
{
    graphics_mode *mode;

    /* Check for existence of required files */
    mode = graphics_modes;
    while (mode)
    {
        char path[MSG_LEN];

        /* Check the graphic file */
        if (mode->file[0])
        {
            path_build(path, sizeof(path), mode->path, mode->file);

            if (!file_exists(path))
            {
                plog_fmt("cannot find file %s - graphics mode '%s' will be disabled.", path,
                    mode->menuname);
                mode->file[0] = 0;
            }
        }
        mode = mode->pNext;
    }

    /* Check availability (default to no graphics) */
    mode = get_graphics_mode(use_graphics, true);
    if (!(mode && mode->grafID && mode->file[0]))
    {
        use_graphics = GRAPHICS_NONE;
        reset_tile_params();
    }

    /* Load the graphics stuff in */
    load_gfx();
}

/*
 * Create the windows
 * Called sometime after load_prefs()
 */
static void init_windows(void)
{
    int i;

    for (i = 0; i < ANGBAND_TERM_MAX; i++)
    {
        term_window *win = &windows[i];

        /* Only bother with visible windows */
        if (win->visible)
        {
            /* Left bounds check */
            if (win->left < 0) win->left = 0;

            /* Right bounds check */
            if ((win->left + win->width) > AppWin->w)
            {
                if (win->width > AppWin->w) win->width = AppWin->w;
                win->left = AppWin->w - win->width;
            }

            /* Top bounds check */
            if (win->top < StatusHeight) win->top = StatusHeight;

            /* Bottom bounds check */
            if ((win->top + win->height) > AppWin->h)
            {
                if (win->height > AppWin->h) win->height = AppWin->h;
                win->top = AppWin->h - win->height;
            }

            /* Invalidate the gfx surface */
            if (win->tiles)
            {
                SDL_FreeSurface(win->tiles);
                win->tiles = NULL;
            }

            /* This will set up the window correctly */
            ResizeWin(win, win->width, win->height);
        }
        else
        {
            /* Doesn't exist */
            angband_term[i] = NULL;
        }

        /* Term 0 is at the top */
        Zorder[i] = ANGBAND_TERM_MAX - i - 1;

        /* Hack -- set ANGBAND_FONTNAME for main window */
        if (i == 0) ANGBAND_FONTNAME = win->req_font;
    }

    /* Good to go... */
    Term_activate(term_screen);
}


/*
 * Set up some SDL stuff
 */
static void init_sdl_local(void)
{
    const SDL_VideoInfo *VideoInfo;
    int i;
    int h, w;
    char path[MSG_LEN];

    /* Get information about the video hardware */
    VideoInfo = SDL_GetVideoInfo();

    /* Require at least 256 colors */
    if (VideoInfo->vfmt->BitsPerPixel < 8)
        quit_fmt("This %s port requires lots of colors.", version_build(VERSION_NAME, true));

    full_w = VideoInfo->current_w;
    full_h = VideoInfo->current_h;

    /* Use a software surface - A tad inefficient, but stable... */
    vflags |= SDL_SWSURFACE;

    /* Set fullscreen flag */
    if (fullscreen) vflags |= SDL_FULLSCREEN;

    /* otherwise we make this surface resizable */
    else vflags |= SDL_RESIZABLE;

    /* Create the main window */
    AppWin = SDL_SetVideoMode(fullscreen? full_w: screen_w, fullscreen? full_h: screen_h, 0, vflags);

    /* Handle failure */
    if (!AppWin)
    {
        quit_fmt("Failed to create %dx%d window at %d bpp!", screen_w, screen_h,
            VideoInfo->vfmt->BitsPerPixel);
    }

    /* Set the window caption */
    SDL_WM_SetCaption(version_build(VERSION_NAME, true), NULL);

    /* Enable key repeating; use defaults */
    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

    /* Enable Unicode (so we can read key codes) */
    SDL_EnableUNICODE(1);

    /* Build a color for "black" that matches the pixel depth of this surface */
    back_colour.r = angband_color_table[COLOUR_DARK][1];
    back_colour.g = angband_color_table[COLOUR_DARK][2];
    back_colour.b = angband_color_table[COLOUR_DARK][3];
    back_pixel_colour = SDL_MapRGB(AppWin->format, back_colour.r, back_colour.g, back_colour.b);

    /* Initialize the colours */
    for (i = 0; i < MAX_COLORS; i++)
    {
        text_colours[i].r = angband_color_table[i][1];
        text_colours[i].g = angband_color_table[i][2];
        text_colours[i].b = angband_color_table[i][3];
    }

    /* Get the height of the status bar */
    sdl_CheckFont(DEFAULT_FONT_FILE, DEFAULT_FONT_SIZE, &w, &h);
    StatusHeight = h + 3;

    /* Font used for window titles */
    sdl_FontCreate(&SystemFont, DEFAULT_FONT_FILE, DEFAULT_FONT_SIZE, AppWin);

    /* Get the icon for display in the About box */
    path_build(path, sizeof(path), ANGBAND_DIR_ICONS, "att-128.png");
    if (file_exists(path))
        mratt = IMG_Load(path);
}


/*
 * Font sorting function
 *
 * Orders by width, then height, then face
 */
static int cmp_font(const void *f1, const void *f2)
{
    const char *font1 = *(const char **)f1;
    const char *font2 = *(const char **)f2;
    int width1 = 0;
    int width2 = 0;
    int height1 = 0;
    int height2 = 0;
    char face1[5] = {0};
    char face2[5] = {0};

    sscanf(font1, "%dx%d%4s.", &width1, &height1, face1);
    sscanf(font2, "%dx%d%4s.", &width2, &height2, face2);

    if (width1 == NULL && height1 == NULL && width2 == NULL && height2 == NULL)
        return strcmp(font1, font2);

    if (width1 < width2)
        return -1;
    else if (width1 > width2)
        return 1;
    else
    {
        if (height1 < height2)
            return -1;
        else if (height1 > height2)
            return 1;
        else
            return strcmp(face1, face2);
    }
}


static void init_paths(void)
{
    int i;
    char path[MSG_LEN];
    char buf[MSG_LEN];
    ang_dir *dir;

    /* Build the filename */
    path_build(path, sizeof(path), ANGBAND_DIR_FONTS, DEFAULT_FONT_FILE);

    /* Hack -- validate the basic font */
    validate_file(path);

    for (i = 0; i < MAX_FONTS; i++)
        FontList[i] = NULL;

    /* Open the fonts directory */
    dir = my_dopen(ANGBAND_DIR_FONTS);
    if (!dir) return;

    /* Read every font to the limit */
    while (my_dread(dir, buf, sizeof(buf)))
    {
        /* Check for file extension */
        if (suffix(buf, ".ttf") || suffix(buf, ".TTF"))
            FontList[num_fonts++] = string_make(buf);

        if (suffix(buf, ".fon") || suffix(buf, ".FON"))
            FontList[num_fonts++] = string_make(buf);

        /* Don't grow to long */
        if (num_fonts == MAX_FONTS) break;
    }

    sort(FontList, num_fonts, sizeof(FontList[0]), cmp_font);

    /* Done */
    my_dclose(dir);
}


/*
 * The SDL port's "main()" function.
 */
errr init_sdl(void)
{
    /* Remove W8080 warnings: SDL_Swap16/64 is declared but never used */
    SDL_Swap16(0);
    SDL_Swap64(0);

    /* Activate hook */
    plog_aux = hook_plog;

    /* Initialize SDL:  Timer, video, and audio functions */
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        plog_fmt("Couldn't initialize SDL: %s\n", SDL_GetError());
        return (2);
    }

    /* Initialize the TTF library */
    if (TTF_Init() < 0)
    {
        plog_fmt("Couldn't initialize TTF: %s\n", SDL_GetError());
        SDL_Quit();
        return (2);
    }

    /* Init some extra paths */
    init_paths();

    /* Load possible graphics modes */
    init_graphics_modes();
    GfxButtons = mem_zalloc(sizeof(int) * (graphics_mode_high_id + 1));

    /* Load prefs */
    load_prefs();

    /* Get sdl going */
    init_sdl_local();

    /* Load window prefs */
    load_window_prefs();

    /* Prepare the windows */
    init_windows();

    /* Prepare the gfx */
    init_gfx();

    /* Prepare some more windows(!) */
    init_morewindows();

    /* Show on the screen */
    sdl_BlitAll();

    /* Activate hook */
    quit_aux = hook_quit;

#ifdef WINDOWS
    /* Register a control handler */
    if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, true))
        quit("Could not set control handler");
#endif

    /* Paranoia */
    return (0);
}
#endif/* USE_SDL */

#endif 

