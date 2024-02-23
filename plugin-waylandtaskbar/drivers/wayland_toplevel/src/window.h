/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * LXQt - a lightweight, Qt based, desktop toolset
 * https://lxqt.org
 *
 * Copyright: 2024 LXQt team
 *
 * Authors:
 *   P.L. Lucas <selairi@gmail.com>
 *
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 * END_COMMON_COPYRIGHT_HEADER */


#ifndef __WINDOW_H__
#define __WINDOW_H__

#include <stdint.h>
#include <stdbool.h>
#include "memory.h"
#include "wayland_window_actions.h"

enum State {
  NORMAL, MAXIMIZED, MINIMIZED, FULLSCREEN
};

struct Window {
  uint32_t id;
  memory_owner char *title;
  memory_owner char *app_id;
  enum State state;
  struct WaylandActions wayland_actions;
};

void window_init(memory_guarded  struct Window *window, uint32_t id, memory_guarded struct zwlr_foreign_toplevel_handle_v1 *zwlr_foreign_toplevel_handle_v1, memory_guarded struct wl_seat *seat);
void window_clear(memory_guarded struct Window *window);

uint32_t window_get_active_window();
void window_set_active_window(uint32_t id);

uint32_t window_id(memory_guarded struct Window *window);

memory_guarded char *window_title(memory_guarded struct Window *window);
void window_set_title(
    memory_guarded struct Window *window, 
    memory_take_possession char *title
);

memory_guarded char *window_app_id(memory_guarded struct Window *window);
void window_set_app_id(
    memory_guarded struct Window *window, 
    memory_take_possession char *app_id
);

enum State window_state(memory_guarded struct Window *window);
void window_set_state(memory_guarded struct Window *window, enum State state);
void window_send_set_fullscreen(memory_guarded struct Window *window, bool value);
void window_send_set_maximized(memory_guarded struct Window *window, bool value);
void window_send_set_minimized(memory_guarded struct Window *window, bool value);
void window_send_set_active(memory_guarded struct Window *window);
void window_send_close(memory_guarded struct Window *window);

void window_set_output(memory_guarded struct Window *window, memory_guarded struct wl_output *output);
#endif
