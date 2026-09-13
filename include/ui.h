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

    struct wl_shm_pool* pool; /* shm pool our buffer lives in */
    struct wl_buffer* buffer; /* the visible pixels */
    void* pixels;             /* mmap'ed memory we can draw into */

    int width, height; /* fixed 600x400 */
    int running;       /* 0 = window was closed */
};

/* Connect to the compositor and create the (blank) window.
 * Returns 0 on success, -1 on error. */
int connect_display(void);

/* Block dispatching Wayland events until the window is closed.
 * Returns 0 on success (window closed), -1 on error. */
int run_ui(void);

/* Release every object created by connect_display. */
void cleanup_ui(void);

#endif  // UI_H
