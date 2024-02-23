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


#ifndef __MANAGER_H__
#define __MANAGER_H__

#include <wayland-client.h>
#include "../src_protocols/foreign-toplevel.h"
#include "dbus.h"
#include "windowlist.h"
#include "memory.h"

struct Manager {
  memory_guarded struct zwlr_foreign_toplevel_manager_v1 *toplevel;
  struct zwlr_foreign_toplevel_manager_v1_listener top_level_listener;
  struct zwlr_foreign_toplevel_handle_v1_listener top_level_handle_listener;
  uint32_t count;
  struct WindowList windowlist;
  memory_guarded struct wl_output *output;
  memory_guarded struct wl_seat *seat;
  memory_guarded struct DBus *dbus;
};



#endif
