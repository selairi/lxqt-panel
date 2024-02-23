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


#ifndef __WAYLAND_WINDOW_ACTIONS__
#define __WAYLAND_WINDOW_ACTIONS__

#include <stdbool.h>
#include "memory.h"
#include <wayland-client.h>
#include "../src_protocols/foreign-toplevel.h"

struct WaylandActions {
  memory_guarded struct wl_output *output;
  memory_guarded struct wl_seat *seat;
  memory_guarded struct zwlr_foreign_toplevel_handle_v1 *zwlr_foreign_toplevel_handle_v1;
};

void wayland_window_action_set_fullscreen(memory_guarded struct WaylandActions *manager, bool value);
void wayland_window_action_set_maximized(memory_guarded struct WaylandActions *manager, bool value);
void wayland_window_action_set_minimized(memory_guarded struct WaylandActions *manager, bool value);
void wayland_window_action_set_active(memory_guarded struct WaylandActions *manager);
void wayland_window_action_close(memory_guarded struct WaylandActions *manager);

#endif
