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


#include "window.h"
#include <stdlib.h>
#include <stdio.h>

static uint32_t active_window = 0;

void window_init(struct Window *window, uint32_t id, struct zwlr_foreign_toplevel_handle_v1 *zwlr_foreign_toplevel_handle_v1, struct wl_seat *seat)
{
  window->id = id;
  window->title = NULL;
  window->app_id = NULL;
  window->state = NORMAL;
  window->wayland_actions.output = NULL;
  window->wayland_actions.seat = seat;
  window->wayland_actions.zwlr_foreign_toplevel_handle_v1 = zwlr_foreign_toplevel_handle_v1;
}


void window_clear(struct Window *window)
{
  free(window->title);
  free(window->app_id);
  window->title = window-> app_id = NULL;
}


uint32_t window_get_active_window()
{
  return active_window;
}


void window_set_active_window(uint32_t id)
{
  active_window = id;
}

void window_set_title(struct Window *window, char *title)
{
  if(window->title != NULL) free(window->title);
  window->title = title;
}

void window_set_app_id(struct Window *window, char *app_id)
{
  window->app_id = app_id;
}

void window_set_state(struct Window *window, enum State state)
{
  window->state = state;
}

void window_send_set_fullscreen(memory_guarded struct Window *window, bool value)
{
  wayland_window_action_set_fullscreen(&window->wayland_actions, value);
  window->state = value ? FULLSCREEN : NORMAL;
}

void window_send_set_maximized(memory_guarded struct Window *window, bool value)
{
  wayland_window_action_set_maximized(&window->wayland_actions, value);
  window->state = value ? MAXIMIZED : NORMAL;
}

void window_send_set_minimized(memory_guarded struct Window *window, bool value)
{
  wayland_window_action_set_minimized(&window->wayland_actions, value);
  window->state = value ? MINIMIZED : NORMAL;
}

void window_send_set_active(memory_guarded struct Window *window)
{
  wayland_window_action_set_active(&window->wayland_actions);
  window->state = window->state == MINIMIZED ? NORMAL : window->state;
}

void window_send_close(memory_guarded struct Window *window)
{
  wayland_window_action_close(&window->wayland_actions);
}

void window_set_output(memory_guarded struct Window *window, memory_guarded struct wl_output *output)
{
  window->wayland_actions.output = output;
}

uint32_t window_id(struct Window *window) {return window->id;}
char *window_title(struct Window *window) {return window->title;}
char *window_app_id(struct Window *window) {return window->app_id;}
enum State window_state(struct Window *window) {return window->state;}
