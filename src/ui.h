
#ifndef UI_H
#define UI_H

#include <wayland-client.h>
struct state {
    struct wl_compositor* compositor;
    struct xdg_wm_base* wm_base;
    struct wl_shm* shm;
    struct wl_seat* seat;
    struct wl_surface* surface;
    struct wl_keyboard* keyboard;
    int width, height;  // fixed 600*400
};
/* returns -1 on success*/
int connect_display(void);

#endif  // UI_H
