#ifndef INPUT_HANDLING_H
#define INPUT_HANDLING_H

#include <wayland-client.h>

/* Every key press we can make sense of, after decoding the raw Wayland
 * keycode through xkbcommon. */
enum key_action {
    ACTION_OTHER,     /* a key we do not use (F-keys, dead keys, Ctrl+C, ...) */
    ACTION_CHAR,      /* a printable character: `utf8` holds it              */
    ACTION_BACKSPACE, /* delete the last character of the query              */
    ACTION_ENTER,     /* launch / accept the highlighted row                */
    ACTION_ESCAPE,    /* close the window                                   */
    ACTION_UP,        /* move the highlight up                              */
    ACTION_DOWN       /* move the highlight down                            */
};

/* =====================================================================
 * INTEGRATION POINT — this is where input meets YOUR logic.
 *
 * input_handling.c ships a weak default (Escape closes the window).
 * Define your own `on_key` anywhere you like (e.g. logic.c) and the linker
 * picks yours instead; nothing else in this module has to change.
 *
 *   void on_key(enum key_action action, const char* utf8)
 *   {
 *       if (action == ACTION_ESCAPE) { ui_quit(); return; }
 *       if (action != ACTION_CHAR && action != ACTION_BACKSPACE) return;
 *
 *       // re-run your search for input_query() and hand the result over:
 *       //     ui_set_list(names, count);
 *   }
 *
 * After on_key returns, input_handling.c repaints the window once — so you
 * only ever have to update state, never call ui_redraw() yourself.
 * Around Enter: `ui_selected()` tells you which row was highlighted.
 * ===================================================================== */
void on_key(enum key_action action, const char* utf8);

/* The text typed so far (owned by input_handling.c, do not free). */
const char* input_query(void);

/* Start decoding keys from this seat. Called for you by ui.c. */
void input_init(struct wl_seat* seat);
void input_cleanup(void);

#endif  // INPUT_HANDLING_H
