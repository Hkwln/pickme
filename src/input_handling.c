#include "../include/input_handling.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include <xkbcommon/xkbcommon.h>

#include "../include/ui.h"

#define QUERY_MAX 256 /* longest query we keep, in bytes */

/* Everything the keyboard side owns. */
static struct wl_keyboard* keyboard;
static struct xkb_context* xkb_ctx;    /* xkbcommon's "environment"     */
static struct xkb_keymap* xkb_keymap;  /* the layout the compositor sent */
static struct xkb_state* xkb_state;    /* the map + which keys are held  */

static char query[QUERY_MAX];
static size_t query_len;

const char* input_query(void)
{
    return query;
}

/* ---------- the query string ---------- */

static void insert_text(const char* text)
{
    const size_t n = strlen(text);
    if (query_len + n >= sizeof query) return; /* full: drop the key */
    memcpy(query + query_len, text, n);
    query_len += n;
    query[query_len] = '\0';
}

/* Backspace has to remove one UTF-8 character, not one byte: "ä" is two
 * bytes, and cutting it in half would leave invalid UTF-8 behind. */
static void delete_last_char(void)
{
    if (query_len == 0) return;
    size_t i = query_len - 1;
    while (i > 0 && ((unsigned char)query[i] & 0xC0) == 0x80) i--; /* 10xxxxxx
                                                                    * continuation bytes */
    query_len = i;
    query[query_len] = '\0';
}

/* ---------- decoding ---------- */

static int modifier_active(const char* name)
{
    return xkb_state != NULL &&
           xkb_state_mod_name_is_active(xkb_state, name,
                                        XKB_STATE_MODS_EFFECTIVE) > 0;
}

/* Turn one raw Wayland keycode into the single thing we care about.
 * Fills `utf8` with the character when the result is ACTION_CHAR.
 * This is the only function that talks to xkbcommon. */
static enum key_action decode_key(uint32_t key, char* utf8, size_t utf8_size)
{
    if (xkb_state == NULL) return ACTION_OTHER;

    /* Wayland sends evdev keycodes; xkbcommon wants them offset by 8. */
    const xkb_keycode_t code = (xkb_keycode_t)key + 8;
    const xkb_keysym_t sym = xkb_state_key_get_one_sym(xkb_state, code);

    switch (sym) {
    case XKB_KEY_Return:
    case XKB_KEY_KP_Enter:
        return ACTION_ENTER;
    case XKB_KEY_Escape:
        return ACTION_ESCAPE;
    case XKB_KEY_BackSpace:
        return ACTION_BACKSPACE;
    case XKB_KEY_Up:
        return ACTION_UP;
    case XKB_KEY_Down:
        return ACTION_DOWN;
    default:
        break;
    }

    /* Ctrl/Alt + key is a shortcut, not text (Ctrl+C must not type "c"). */
    if (modifier_active(XKB_MOD_NAME_CTRL) || modifier_active(XKB_MOD_NAME_ALT))
        return ACTION_OTHER;

    /* Everything else: let xkbcommon produce the text this key types. It
     * already accounts for layout, shift, caps lock, alt-gr, ... */
    const int n = xkb_state_key_get_utf8(xkb_state, code, utf8, utf8_size);
    if (n <= 0) return ACTION_OTHER;

    /* Control characters are not text. */
    if ((unsigned char)utf8[0] < 0x20 || (unsigned char)utf8[0] == 0x7F)
        return ACTION_OTHER;

    return ACTION_CHAR;
}

/* ---------- wl_keyboard events ---------- */

/* The compositor hands us its keymap as a shared-memory fd. We compile it
 * with xkbcommon and keep the resulting state. */
static void handle_keymap(void* data, struct wl_keyboard* kb, uint32_t format,
                          int32_t fd, uint32_t size)
{
    (void)data;
    (void)kb;

    if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1) {
        fprintf(stderr, "pickme: unsupported keymap format %u\n", format);
        close(fd); /* xkb_v1 is the only format in existence */
        return;
    }

    char* map = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (map == MAP_FAILED) {
        fprintf(stderr, "pickme: could not map the keymap\n");
        close(fd);
        return;
    }

    if (xkb_ctx == NULL) xkb_ctx = xkb_context_new(XKB_CONTEXT_NO_FLAGS);

    struct xkb_keymap* new_keymap =
        xkb_keymap_new_from_string(xkb_ctx, map, XKB_KEYMAP_FORMAT_TEXT_V1,
                                   XKB_KEYMAP_COMPILE_NO_FLAGS);
    munmap(map, size);
    close(fd);

    if (new_keymap == NULL) {
        fprintf(stderr, "pickme: the compositor's keymap did not compile\n");
        return;
    }

    struct xkb_state* new_state = xkb_state_new(new_keymap);
    if (new_state == NULL) {
        xkb_keymap_unref(new_keymap);
        return;
    }

    if (xkb_state) xkb_state_unref(xkb_state);
    if (xkb_keymap) xkb_keymap_unref(xkb_keymap);
    xkb_state = new_state;
    xkb_keymap = new_keymap;
}

/* Focus notifications. We do not need them: the compositor only sends key
 * events while we are focused anyway. They must exist (non-NULL) though. */
static void handle_enter(void* data, struct wl_keyboard* kb, uint32_t serial,
                         struct wl_surface* surface, struct wl_array* keys)
{
    (void)data;
    (void)kb;
    (void)serial;
    (void)surface;
    (void)keys;
}

static void handle_leave(void* data, struct wl_keyboard* kb, uint32_t serial,
                         struct wl_surface* surface)
{
    (void)data;
    (void)kb;
    (void)serial;
    (void)surface;
}

/* Shift/caps/alt state changed: hand it to xkbcommon so later key lookups
 * are correct ("A" vs "a"). */
static void handle_modifiers(void* data, struct wl_keyboard* kb, uint32_t serial,
                             uint32_t mods_depressed, uint32_t mods_latched,
                             uint32_t mods_locked, uint32_t group)
{
    (void)data;
    (void)kb;
    (void)serial;
    if (xkb_state == NULL) return;
    xkb_state_update_mask(xkb_state, mods_depressed, mods_latched, mods_locked,
                          0, 0, group);
}

/* The compositor's repeat settings. Hyprland (wlroots) repeats keys on the
 * server side, so we only need to notice this event, not act on it. */
static void handle_repeat_info(void* data, struct wl_keyboard* kb,
                               int32_t rate, int32_t delay)
{
    (void)data;
    (void)kb;
    (void)rate;
    (void)delay;
}

/* A key went down (or repeated). This is the whole point of the module. */
static void handle_key(void* data, struct wl_keyboard* kb, uint32_t serial,
                       uint32_t time, uint32_t key, uint32_t state)
{
    (void)data;
    (void)kb;
    (void)serial;
    (void)time;

    if (state == WL_KEYBOARD_KEY_STATE_RELEASED) return; /* we only act on down */

    if (xkb_state == NULL) {
        fprintf(stderr, "pickme: key press before the keymap arrived\n");
        return;
    }

    char utf8[8] = {0};
    const enum key_action action = decode_key(key, utf8, sizeof utf8);

    if (getenv("PICKME_DEBUG") != NULL)
        fprintf(stderr, "pickme: key %u -> action %d%s%s\n", key, (int)action,
                action == ACTION_CHAR ? " text=" : "",
                action == ACTION_CHAR ? utf8 : "");

    /* 1. update the state this module owns */
    switch (action) {
    case ACTION_CHAR:
        insert_text(utf8);
        break;
    case ACTION_BACKSPACE:
        delete_last_char();
        break;
    case ACTION_UP:
        ui_move_selection(-1);
        break;
    case ACTION_DOWN:
        ui_move_selection(+1);
        break;
    default:
        break;
    }

    /* 2. let the application react (re-run the search, launch, ...) */
    on_key(action, (action == ACTION_CHAR) ? utf8 : NULL);

    /* 3. repaint once, with whatever state we and the app ended up in */
    ui_set_query(query);
    ui_redraw();
}

static const struct wl_keyboard_listener keyboard_listener = {
    .keymap = handle_keymap,
    .enter = handle_enter,
    .leave = handle_leave,
    .key = handle_key,
    .modifiers = handle_modifiers,
    .repeat_info = handle_repeat_info,
};

/* ---------- wl_seat ---------- */

static void release_keyboard(void)
{
    if (keyboard == NULL) return;
    /* wl_keyboard.release only exists from version 3 on; the old destroy is
     * just a local free and works on any version. */
    if (wl_keyboard_get_version(keyboard) >= 3)
        wl_keyboard_release(keyboard);
    else
        wl_keyboard_destroy(keyboard);
    keyboard = NULL;
}

/* The seat tells us which input devices exist. We want the keyboard. */
static void handle_seat_capabilities(void* data, struct wl_seat* seat,
                                     uint32_t capabilities)
{
    (void)data;

    if ((capabilities & WL_SEAT_CAPABILITY_KEYBOARD) && keyboard == NULL) {
        keyboard = wl_seat_get_keyboard(seat);
        wl_keyboard_add_listener(keyboard, &keyboard_listener, NULL);
    } else if (!(capabilities & WL_SEAT_CAPABILITY_KEYBOARD)) {
        if (keyboard == NULL)
            fprintf(stderr,
                    "pickme: the compositor's seat offers no keyboard\n");
        release_keyboard();
    }
}

static void handle_seat_name(void* data, struct wl_seat* seat, const char* name)
{
    (void)data;
    (void)seat;
    (void)name;
}

static const struct wl_seat_listener seat_listener = {
    .capabilities = handle_seat_capabilities,
    .name = handle_seat_name,
};

/* ---------- public API ---------- */

void input_init(struct wl_seat* seat)
{
    if (seat == NULL) return;
    wl_seat_add_listener(seat, &seat_listener, NULL);
    /* The first capabilities event arrives on the next dispatch (run_ui). */
}

void input_cleanup(void)
{
    release_keyboard();
    if (xkb_state) {
        xkb_state_unref(xkb_state);
        xkb_state = NULL;
    }
    if (xkb_keymap) {
        xkb_keymap_unref(xkb_keymap);
        xkb_keymap = NULL;
    }
    if (xkb_ctx) {
        xkb_context_unref(xkb_ctx);
        xkb_ctx = NULL;
    }
}

/* ---------------------------------------------------------------------
 * Default behaviour, so the window is usable before your logic exists:
 * Escape closes it, everything else is ignored. Define `on_key` yourself
 * (e.g. in logic.c) and GCC links yours instead of this weak one.
 * ------------------------------------------------------------------- */
__attribute__((weak)) void on_key(enum key_action action, const char* utf8)
{
    (void)utf8;
    if (action == ACTION_ESCAPE) ui_quit();
}
