#ifndef UI_H
#define UI_H

#include <wayland-client.h>

/* Forward declarations for the xdg-shell objects. Their full definitions
 * live in the generated header "xdg-shell-client-protocol.h" (see README /
 * build instructions) — ui.c includes it. Other files only ever hold
 * pointers to these structs, so forward declarations are enough here. */
struct xdg_wm_base;
struct xdg_surface;
struct xdg_toplevel;
struct wl_buffer;
struct wl_shm_pool;

struct state {
    struct wl_display* display;       /* the connection to the compositor  */
    struct wl_registry* registry;     /* list of offered globals */
    struct wl_compositor* compositor; /* factory for wl_surface */
    struct xdg_wm_base* wm_base;      /* factory for real windows */
    struct wl_shm* shm;               /* factory for shared-memory buffers */
    struct wl_seat* seat;             /* input (bound if present) */

    struct wl_surface* surface;        /* the "canvas" of our window */
    struct xdg_surface* xdg_surface;   /* protocol glue for the toplevel */
    struct xdg_toplevel* xdg_toplevel; /* the actual window with titlebar */

    struct wl_shm_pool* pool;     /* shm pool both buffers live in */
    void* map;                    /* mmap'ed memory holding them */
    struct wl_buffer* buffers[2]; /* front/back pixel buffers */
    void* pixels[2];              /* draw target for each buffer */

    int width, height; /* fixed 600x400 */
    int running;       /* 0 = window was closed */

    /* --- what the window shows right now ------------------------------
     * These are BORROWED pointers: ui.c never copies or frees them, so
     * whoever sets them must keep the memory alive until they set them
     * again. */
    const char* query; /* text on the input line */
    char** list;       /* the suggestion rows */
    int list_count;    /* how many rows list[] holds */
    int selected;      /* index of the highlighted row */

    /* --- repaint bookkeeping ------------------------------------------
     * The compositor reads our pixels asynchronously, so we must not draw
     * into a buffer while it is still showing it. Two buffers mean one is
     * always free (see create_shm_buffers in ui.c for why one is not
     * enough). */
    int buffer_busy[2]; /* 1 = compositor is still using that buffer */
    int next_buffer;    /* slot to try first on the next redraw */
    int redraw_pending; /* 1 = a redraw was asked for while both were busy */
};

/* Connect to the compositor and create the window.
 * Returns 0 on success, -1 on error. */
int connect_display(void);

/* Block dispatching Wayland events until the window is closed.
 * Returns 0 on success (window closed), -1 on error. */
int run_ui(void);

/* Release every object created by connect_display. */
void cleanup_ui(void);

/* ---- what the window should display --------------------------------- */

/* Text on the input line. Not copied — must stay alive. */
void ui_set_query(const char* text);

/* The suggestion rows. Not copied — must stay alive. */
void ui_set_list(char** names, int count);

/* The highlighted row (clamped to the list). */
void ui_set_selected(int index);
int ui_selected(void);
void ui_move_selection(int delta);

/* Repaint into the buffer and commit it to the compositor. Cheap enough to
 * call on every key press. */
void ui_redraw(void);

/* Leave run_ui() (Escape, or "we launched something, get out of the way"). */
void ui_quit(void);

#endif  // UI_H
