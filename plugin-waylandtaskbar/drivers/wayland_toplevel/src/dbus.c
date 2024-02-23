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

#include "dbus.h"

#define DBUS_INTERFACE_PREFIX "lxqt.WindowsList."

static memory_take_possession char *get_dbus_interface_name()
{
  // dbus->m_interface_name is DBUS_INTERFACE_PREFIX + enviroment variable WAYLAND_DISPLAY. 
  // Character '-' is ignored. sd_bus doesn't support the '-' symbol.
  char *wayland_interface = getenv("WAYLAND_DISPLAY");
  if(wayland_interface == NULL)
    wayland_interface = "wayland"; 
  char *buffer = (char*)malloc(sizeof(char)*(strlen(DBUS_INTERFACE_PREFIX) + strlen(wayland_interface) + 1));
  sprintf(buffer, "%s%s", DBUS_INTERFACE_PREFIX, wayland_interface);
  // Ignore '-' character:
  char *ptr1, *ptr2;
  ptr1 = ptr2 = buffer;
  while(*ptr1 != '\0') {
    if(*ptr1 == '-') ptr1++;
    *ptr2 = *ptr1; 
    ptr1++; ptr2++;
  }
  *ptr2 = '\0';
  printf("m_interface_name %s\n", buffer);
  return buffer;
}


static int get_property_callback(sd_bus *bus, const char *path, const char *interface, const char *property, sd_bus_message *reply, void *userdata, sd_bus_error *ret_error) 
{
  UNUSED(bus);
  UNUSED(path);
  UNUSED(interface);
  UNUSED(userdata);
  UNUSED(ret_error);

  int r = -1;
  printf("[get_property] Property: %s\n", property);
  if(! strcmp("ProtocolVersion", property))
    r = sd_bus_message_append(reply, "u", 0);
  return r;
}


/** Callback for dbus WindowTitle.
 */
static int window_title_callback(
    memory_guarded sd_bus_message *m, 
    memory_guarded void *userdata, 
    sd_bus_error *ret_error) 
{
  UNUSED(ret_error);
  struct DBus *dbus = (struct DBus *)userdata; 
  uint32_t id;
  int r;

  // Read the id
  r = sd_bus_message_read(m, "u", &id);
  if (r < 0) {
    fprintf(stderr, "Failed to parse parameters: %s\n", strerror(-r));
    return r;
  }
  struct Window *window = windowlist_get(dbus->windowlist, id);
  char *title = NULL;

  if(window != NULL) {
    title = window_title(window);
  }

  // Reply with the name
  return sd_bus_reply_method_return(m, "s", memory_guarded title);
}

/** Callback for dbus WindowTitle.
 */
static int window_state_callback(
    memory_guarded sd_bus_message *m, 
    memory_guarded void *userdata, 
    sd_bus_error *ret_error) 
{
  UNUSED(ret_error);
  struct DBus *dbus = (struct DBus *)userdata; 
  uint32_t id;

  // Read the id
  int r = sd_bus_message_read(m, "u", &id);
  if (r < 0) {
    fprintf(stderr, "Failed to parse parameters: %s\n", strerror(-r));
    return r;
  }
  
  struct Window *window = windowlist_get(dbus->windowlist, id);
  enum State state = NORMAL;
  char *reply = NULL;
  if(window != NULL) {
    state = window_state(window);
    switch(state) {
      case NORMAL: reply = "NORMAL"; break;
      case MAXIMIZED: reply = "MAXIMIZED"; break;
      case MINIMIZED: reply = "MINIMIZED"; break;
      case FULLSCREEN: reply = "FULLSCREEN"; break;
    }
  }

  // Reply with the name
  return sd_bus_reply_method_return(m, "s", memory_guarded reply);
}

/** Callback for dbus WindowTitle.
 */
static int window_app_id_callback(
    memory_guarded sd_bus_message *m, 
    memory_guarded void *userdata, 
    sd_bus_error *ret_error) 
{
  UNUSED(ret_error);
  struct DBus *dbus = (struct DBus *)userdata; 
  uint32_t id;

  // Read the id
  int r = sd_bus_message_read(m, "u", &id);
  if (r < 0) {
    fprintf(stderr, "Failed to parse parameters: %s\n", strerror(-r));
    return r;
  }
  
  struct Window *window = windowlist_get(dbus->windowlist, id);
  char *app_id = NULL;
  if(window != NULL) {
    app_id = window_app_id(window);
  }

  // Reply with the name
  return sd_bus_reply_method_return(m, "s", memory_guarded app_id);
}

/** Do an action.
 */
static int window_set_action_callback(
    memory_guarded sd_bus_message *m, 
    memory_guarded void *userdata,
    void(*action_func)(struct Window *window, uint32_t id),
    sd_bus_error *ret_error) 
{
  UNUSED(ret_error);
  struct DBus *dbus = (struct DBus *)userdata; 
  uint32_t id;

  // Read the id
  int r = sd_bus_message_read(m, "u", &id);
  if (r < 0) {
    fprintf(stderr, "Failed to parse parameters: %s\n", strerror(-r));
    return r;
  }
  
  struct Window *window = windowlist_get(dbus->windowlist, id);
  if(window != NULL) {
    printf("[window_set_action_callback] action_func start\n");
    action_func(window, id);
    printf("[window_set_action_callback] action_func end\n");
    //wl_display_dispatch(dbus->display);
    wl_display_roundtrip(dbus->display);
  }

  // Reply with the state
  printf("[window_set_action_callback] dbus reply\n");
  int ret = sd_bus_reply_method_return(m, "u", 0);
  sd_bus_flush(dbus->m_bus);
  printf("[window_set_action_callback] dbus reply flush\n");
  return ret;
}

static void window_change_fullscreen_callback_func(struct Window *window, uint32_t id)
{
  UNUSED(id);
  window_send_set_maximized(window, window_state(window) != MAXIMIZED);
}
/** Callback for dbus WindowChangeFullscreen.
 */
static int window_change_fullscreen_callback(
    memory_guarded sd_bus_message *m, 
    memory_guarded void *userdata, 
    sd_bus_error *ret_error) 
{
  return window_set_action_callback(m, userdata, 
      window_change_fullscreen_callback_func, ret_error);
}


static void window_change_maximized_callback_func(struct Window *window, uint32_t id)
{
  UNUSED(id);
  window_send_set_maximized(window, window_state(window) != MAXIMIZED);
}
/** Callback for dbus WindowChangeMaximized.
 */
static int window_change_maximized_callback(
    memory_guarded sd_bus_message *m, 
    memory_guarded void *userdata, 
    sd_bus_error *ret_error) 
{
  return window_set_action_callback(m, userdata, 
      window_change_maximized_callback_func, ret_error);
}

static void window_change_minimized_callback_func(struct Window *window, uint32_t id)
{
  UNUSED(id);
  window_send_set_minimized(window, window_state(window) != MINIMIZED);
}

/** Callback for dbus WindowChangeMinimized.
 */
static int window_change_minimized_callback(
    memory_guarded sd_bus_message *m, 
    memory_guarded void *userdata, 
    sd_bus_error *ret_error) 
{
  return window_set_action_callback(m, userdata, 
      window_change_minimized_callback_func, ret_error);
}

static void window_set_active_callback_func(struct Window *window, uint32_t id)
{
  if(id != window_get_active_window()) {
    window_send_set_active(window);
  }
}
/** Callback for dbus WindowSetActive.
 */
static int window_set_active_callback(
    memory_guarded sd_bus_message *m, 
    memory_guarded void *userdata, 
    sd_bus_error *ret_error) 
{
  return window_set_action_callback(m, userdata, 
      window_set_active_callback_func, ret_error);
}

static void window_close_callback_func(struct Window *window, uint32_t id)
{
  UNUSED(id);
  window_send_close(window);
}
/** Callback for dbus WindowClose.
 */
static int window_close_callback(
    memory_guarded sd_bus_message *m, 
    memory_guarded void *userdata, 
    sd_bus_error *ret_error) 
{  
  return window_set_action_callback(m, userdata, 
      window_close_callback_func, ret_error);
}

/** Callback for dbus WindowTitle.
 */
static int window_list_callback(
    memory_guarded sd_bus_message *call, 
    memory_guarded void *userdata, 
    sd_bus_error *ret_error) 
{
  UNUSED(ret_error);
  struct DBus *dbus = (struct DBus *)userdata; 
  // Reply with the ids
  _cleanup_(sd_bus_message_unrefp) sd_bus_message *reply = NULL;
  int r = sd_bus_message_new_method_return(call, &reply);
  if (r < 0) return r;
  uint32_t *ids = windowlist_get_ids(dbus->windowlist);
  r = sd_bus_message_append_array(reply, 'u', memory_guarded ids, dbus->windowlist->size*sizeof(uint32_t));
  free(ids);
  if (r < 0) return r;
  return sd_bus_message_send(reply);
}

static const sd_bus_vtable watcher_vtable[] = {
  SD_BUS_VTABLE_START(0),
  SD_BUS_PROPERTY("ProtocolVersion", "u", get_property_callback, 0, SD_BUS_VTABLE_PROPERTY_CONST),
  SD_BUS_METHOD("WindowTitle", "u", "s", window_title_callback, SD_BUS_VTABLE_UNPRIVILEGED),
  SD_BUS_METHOD("WindowAppId", "u", "s", window_app_id_callback, SD_BUS_VTABLE_UNPRIVILEGED),
  SD_BUS_METHOD("WindowState", "u", "s", window_state_callback, SD_BUS_VTABLE_UNPRIVILEGED),
  SD_BUS_METHOD("WindowList", "", "au", window_list_callback, SD_BUS_VTABLE_UNPRIVILEGED),
  SD_BUS_METHOD("WindowChangeFullscreen", "u", "u", window_change_fullscreen_callback, SD_BUS_VTABLE_UNPRIVILEGED),
  SD_BUS_METHOD("WindowChangeMaximized", "u", "u", window_change_maximized_callback, SD_BUS_VTABLE_UNPRIVILEGED),
  SD_BUS_METHOD("WindowChangeMinimized", "u", "u", window_change_minimized_callback, SD_BUS_VTABLE_UNPRIVILEGED),
  SD_BUS_METHOD("WindowSetActive", "u", "u", window_set_active_callback, SD_BUS_VTABLE_UNPRIVILEGED),
  SD_BUS_METHOD("WindowClose", "u", "u", window_close_callback, SD_BUS_VTABLE_UNPRIVILEGED),
  SD_BUS_SIGNAL("WindowTitleChanged", "us", 0),
  SD_BUS_SIGNAL("WindowAppIdChanged", "us", 0),
  SD_BUS_SIGNAL("WindowOpened", "u", 0),
  SD_BUS_SIGNAL("WindowClosed", "u", 0),
  SD_BUS_SIGNAL("WindowActivated", "u", 0),
  SD_BUS_SIGNAL("WindowMaximized", "u", 0),
  SD_BUS_SIGNAL("WindowMinimized", "u", 0),
  SD_BUS_SIGNAL("WindowFullscreened", "u", 0),
  SD_BUS_VTABLE_END
};

void dbus_finish(struct DBus *dbus) 
{
  sd_bus_slot_unref(dbus->m_slot);
  sd_bus_unref(dbus->m_bus);
  free(dbus->m_interface_name);
}

void dbus_init_struct_pollfd(struct DBus *dbus, struct pollfd *fds)
{
  if(dbus->m_bus == NULL) {
    printf("DBus error. m_bus == NULL\n");;
    exit(1);
  }

  fds->fd = sd_bus_get_fd(dbus->m_bus);
  if(fds->fd < 0) {
    printf("DBus fd cannot be read.\n");;
    exit(1);
  }
  int events = sd_bus_get_events(dbus->m_bus);
  if(events >= 0)
    fds->events = POLLIN; //sd_bus_get_events(m_bus);
  else {
    printf("DBus events cannot be read.\n");
    exit(1);
  }
  fds->revents = 0;
}


void dbus_process_poll_event(struct DBus *dbus, struct pollfd *fds)
{
  if(dbus->m_bus == NULL) {
    printf("DBus error. m_bus == NULL\n");
    exit(1);
  }
  // Process DBus events
  printf("[dbus_process_poll_event] start sd_bus_proccess %d\n", sd_bus_get_events(dbus->m_bus));
  int r = sd_bus_process(dbus->m_bus, NULL);
  if (r < 0) {
    dbus_finish(dbus);
    printf("DBus error. Failed to process bus: %s\n", strerror(-r));
    exit(1);
  }
  printf("[dbus_process_poll_event] end sd_bus_proccess\n");
  fds->events = (short)sd_bus_get_events(dbus->m_bus);
  fds->revents = 0;
}


void dbus_init(struct DBus *dbus, struct WindowList *windowlist, memory_guarded struct wl_display *display)
{
  printf("Starting DBus ");
  dbus->m_bus = NULL;
  dbus->m_slot = NULL;
  dbus->m_interface_name = get_dbus_interface_name();
  dbus->windowlist = windowlist;
  dbus->display = display;
  //Guard_sd_bus_error error;
  int r = sd_bus_open_user(&(dbus->m_bus));
  if (r < 0) {
    dbus_finish(dbus);
    printf("DBus error. Failed to connect to system bus: %s\n", strerror(-r));
    exit(1);
  }

  printf("DBus has been started.\n");

  // Init StatusNotifierHost service.
  r = sd_bus_add_object_vtable(dbus->m_bus,
      &(dbus->m_slot),
      "/WindowsList",    // object path 
      dbus->m_interface_name, // interface name 
      watcher_vtable,
      dbus);
  if(r < 0) {
    dbus_finish(dbus);
    printf("DBus error. %s Failed to issue method call: %s\n", dbus->m_interface_name, strerror(-r));
    exit(1);
  }

  r = sd_bus_request_name(dbus->m_bus, dbus->m_interface_name, 0);
  if (r < 0) {
    dbus_finish(dbus);
    printf("DBus error. Failed to acquire service name: %s\n", strerror(-r));
    exit(1);
  }

  r = 0;
  while(r == 0) {
    r = sd_bus_process(dbus->m_bus, NULL);
    if (r < 0) {
      dbus_finish(dbus);
      printf("DBus error. Failed to process bus: %s\n", strerror(-r));
      exit(1);
    }
  }
  r = sd_bus_wait(dbus->m_bus, (uint64_t) -1);
}

int dbus_send_signal(memory_guarded struct DBus *dbus, memory_guarded char *signal_name, uint32_t window_id)
{
  int r = sd_bus_emit_signal(dbus->m_bus, "/WindowsList", dbus->m_interface_name, signal_name, "u", window_id);
  if(r < 0) {
    fprintf(stderr, "Failed to send %s signal. Error: %s\n", signal_name,  strerror(-r));
  }
  return r;
}

int dbus_send_signal_with_value(memory_guarded struct DBus *dbus, memory_guarded char *signal_name, uint32_t window_id, memory_guarded char *signal_value)
{
  int r = sd_bus_emit_signal(dbus->m_bus, "/WindowsList", dbus->m_interface_name, signal_name, "us", window_id, signal_value);
  if(r < 0) {
    fprintf(stderr, "Failed to send %s signal. Error: %s\n", signal_name,  strerror(-r));
  }
  return r;
}

