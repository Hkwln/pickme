
#include "ui.h"

#include <stdio.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>

static void registry_global(void* data, struct wl_registry* registry,
                            uint32_t name, const char* interface,
                            uint32_t version)
{
    // setup a registry to bind wl_compositor, wl_seat and wl_shm
    // use wl_compositor_create_surface() to create surface
    // use w1shell or xdg_wm base to create toplevel window
}

static void registry_global_remove(void* data, struct wl_registry* registry,
                                   uint32_t name)
{ /* ignore */
}
static const struct wl_registry_listener registry_listener = {
    .global = registry_global,
    .global_remove = registry_global_remove,
};
// use a fixed 600 x 400 window size
// draw  a characcter into a buffer ( 1 bit = 1 pixel)
int connect_display(void)
{
    struct state s = {.width = 600, .height = 400};
    struct wl_display* display = wl_display_connect("minimal launcher");
    if (!display) {
        fputs("can't connect to Wayland\n", stderr);
        return 1;
    }
    struct wl_registry* registry = wl_display_get_registry(display);
    wl_registry_add_listener(registry, &registry_listener, &registry);
    wl_display_roundtrip(display);
    wl_display_roundtrip(display);

    if (!s.compositor || !s.wm_base || !s.shm || !s.seat) {
        fputs("missing required Wayland global\n", stderr);
        return 1;
    }
    return -1;
}
