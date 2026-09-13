
#include "../include/input_handling.h"

#include <wayland-client-protocol.h>
// to map the raw keycodes to characters
#include <xkbcommon/xkbcommon.h>
static struct xkb_state* xkb_state;
#if 0
// map key codes to e.g key_up, key_down key_enter
void get_key(struct wl_seat* seat)
{
    state->wl_keyboard = wl_seat_get_keyboard(seat);
    wl_keyboard_add_listener(state->wl_keyboard, &wl_keyboard_listener, state)
}
#endif
