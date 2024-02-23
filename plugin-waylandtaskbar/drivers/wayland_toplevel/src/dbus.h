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

#ifndef __DBUS_H__
#define __DBUS_H__

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <systemd/sd-bus.h>
#include <poll.h>
#include <stdbool.h> 
#include "memory.h"
#include "windowlist.h"
#include <wayland-client.h>

struct DBus {
  sd_bus_slot *m_slot memory_ref_count;
  sd_bus *m_bus memory_ref_count;
  char *m_interface_name memory_owner;
  struct WindowList *windowlist memory_guarded;
  struct wl_display *display memory_guarded;
};

void dbus_init(memory_guarded struct DBus *dbus, memory_guarded struct WindowList *windowlist, memory_guarded struct wl_display *display);
void dbus_finish(memory_guarded struct DBus *dbus); 
void dbus_init_struct_pollfd(memory_guarded struct DBus *dbus, memory_guarded struct pollfd *fds);
void dbus_process_poll_event(memory_guarded struct DBus *dbus, memory_guarded struct pollfd *fds);
int dbus_send_signal(memory_guarded struct DBus *dbus, memory_guarded char *signal_name, uint32_t window_id);
int dbus_send_signal_with_value(memory_guarded struct DBus *dbus, memory_guarded char *signal_name, uint32_t window_id, memory_guarded char *signal_value);

#endif 
