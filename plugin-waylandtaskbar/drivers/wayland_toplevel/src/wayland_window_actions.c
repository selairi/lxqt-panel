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


#include "wayland_window_actions.h"
#include "manager.h"

void wayland_window_action_close(memory_guarded struct WaylandActions *manager)
{
  if(manager->output != NULL) {
    zwlr_foreign_toplevel_handle_v1_close(manager->zwlr_foreign_toplevel_handle_v1);
  }
}

void wayland_window_action_set_fullscreen(memory_guarded struct WaylandActions *manager, bool value)
{
  if(manager->output != NULL) {
    if(value)
      zwlr_foreign_toplevel_handle_v1_set_fullscreen(manager->zwlr_foreign_toplevel_handle_v1, manager->output);
    else
      zwlr_foreign_toplevel_handle_v1_unset_fullscreen(manager->zwlr_foreign_toplevel_handle_v1);
  }
}

void wayland_window_action_set_maximized(memory_guarded struct WaylandActions *manager, bool value)
{
  if(manager->output != NULL) {
    if(value)
      zwlr_foreign_toplevel_handle_v1_set_maximized(manager->zwlr_foreign_toplevel_handle_v1);
    else
      zwlr_foreign_toplevel_handle_v1_unset_maximized(manager->zwlr_foreign_toplevel_handle_v1);
  }
}

void wayland_window_action_set_minimized(memory_guarded struct WaylandActions *manager, bool value)
{
  if(manager->output != NULL) {
    if(value)
      zwlr_foreign_toplevel_handle_v1_set_minimized(manager->zwlr_foreign_toplevel_handle_v1);
    else
      zwlr_foreign_toplevel_handle_v1_unset_minimized(manager->zwlr_foreign_toplevel_handle_v1);
  }
}

void wayland_window_action_set_active(memory_guarded struct WaylandActions *manager)
{
  zwlr_foreign_toplevel_handle_v1_activate(manager->zwlr_foreign_toplevel_handle_v1, manager->seat);
}
