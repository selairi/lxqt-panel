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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h> 

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include <time.h>

#include <wayland-client.h>
#include "../src_protocols/foreign-toplevel.h"
#include "../src_protocols/foreign-toplevel.c"

#include "dbus.h"
#include "windowlist.h"
#include "manager.h"

static inline struct Manager* _(void *data)
{
  return (struct Manager*)data;
}

/**This is a window object.
 */
struct Handler {
  memory_guarded struct Manager *manager;
  struct Window window;
  memory_guarded struct WindowList *windowlist;
};

static memory_take_possession struct Handler *handler_new(struct Manager *manager, memory_guarded struct zwlr_foreign_toplevel_handle_v1 *zwlr_foreign_toplevel_handle_v1, memory_guarded struct wl_output *output, memory_guarded struct WindowList *windowlist)
{
  uint32_t id = manager->count++;
  struct Handler *self = (struct Handler*)malloc(sizeof(struct Handler));
  self->manager = manager;
  window_init(&self->window, id, zwlr_foreign_toplevel_handle_v1, manager->seat); 
  window_set_output(&self->window, output);
  self->windowlist = windowlist;
  windowlist_add(windowlist, &self->window);
  return self;
}

static void handler_delete(memory_take_possession struct Handler *handler)
{
  windowlist_remove(handler->windowlist, window_id(&handler->window));
  window_clear(&handler->window);
  free(handler);
}

static inline struct Handler* handler(memory_guarded void *data)
{
  return (struct Handler*)data;
}

///////////////////////////////////////
// Implementación de struct zwlr_foreign_toplevel_handle_v1_listener *listener
/**
 * title change
 *
 * This event is emitted whenever the title of the toplevel
 * changes.
 */
void top_level_handle_listener_title(memory_guarded void *data,
		      memory_guarded struct zwlr_foreign_toplevel_handle_v1 *zwlr_foreign_toplevel_handle_v1,
		      memory_guarded const char *title)
{
    printf("\n\t\t[top_level_handle_listener_title] %s\n", title);
    window_set_title(&handler(data)->window, strdup(title));
    dbus_send_signal_with_value(handler(data)->manager->dbus, "WindowTitleChanged", window_id(&handler(data)->window),(char*)title);
}

/**
 * app-id change
 *
 * This event is emitted whenever the app-id of the toplevel
 * changes.
 */
void top_level_handle_listener_app_id(memory_guarded void *data,
		       memory_guarded struct zwlr_foreign_toplevel_handle_v1 *zwlr_foreign_toplevel_handle_v1,
		       memory_guarded const char *app_id)
{
    printf("\t\t[top_level_handle_listener_app_id] %s\n", app_id);
    window_set_app_id(&handler(data)->window, strdup(app_id));
    dbus_send_signal_with_value(handler(data)->manager->dbus, "WindowAppIdChanged", window_id(&handler(data)->window), (char*)app_id);
}

/**
 * toplevel entered an output
 *
 * This event is emitted whenever the toplevel becomes visible on
 * the given output. A toplevel may be visible on multiple outputs.
 */
void top_level_handle_listener_output_enter(memory_guarded void *data,
        memory_guarded struct zwlr_foreign_toplevel_handle_v1 *zwlr_foreign_toplevel_handle_v1,
        memory_guarded struct wl_output *output)
{
    printf("\t\t[top_level_handle_listener_output_enter]\n");
    window_set_output(&handler(data)->window, output);
}

/**
 * toplevel left an output
 *
 * This event is emitted whenever the toplevel stops being
 * visible on the given output. It is guaranteed that an
 * entered-output event with the same output has been emitted
 * before this event.
 */
void top_level_handle_listener_output_leave(memory_guarded void *data,
        memory_guarded struct zwlr_foreign_toplevel_handle_v1 *zwlr_foreign_toplevel_handle_v1,
        memory_guarded struct wl_output *output)
{
    printf("\t\t[top_level_handle_listener_output_leave]\n");
}

/**
 * the toplevel state changed
 *
 * This event is emitted immediately after the
 * zlw_foreign_toplevel_handle_v1 is created and each time the
 * toplevel state changes, either because of a compositor action or
 * because of a request in this protocol.
 */
void top_level_handle_listener_state(memory_guarded void *data,
        memory_guarded struct zwlr_foreign_toplevel_handle_v1 *zwlr_foreign_toplevel_handle_v1,
        memory_guarded struct wl_array *state)
{
  printf("\t\t[top_level_handle_listener_state]\n");
  enum zwlr_foreign_toplevel_handle_v1_state  *s;
  struct Window *window = &handler(data)->window;
  wl_array_for_each(s, state) {
    printf("\t\t\tState: %d ", *s);
    if(window_app_id(window) != NULL) printf(" %s ", window_app_id(window));
    if(*s == ZWLR_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_MAXIMIZED) {
      printf("maximized\n");
      window_set_state(window, MAXIMIZED);
      dbus_send_signal(handler(data)->manager->dbus, "WindowMaximized", window_id(&handler(data)->window));
    } else if(*s == ZWLR_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_MINIMIZED) {
      printf("minimized\n");
      window_set_state(window, MINIMIZED);
      dbus_send_signal(handler(data)->manager->dbus, "WindowMinimized", window_id(&handler(data)->window));
    } else if(*s == ZWLR_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_ACTIVATED) {
      printf("activated\n");
      window_set_active_window(window_id(&handler(data)->window));
      dbus_send_signal(handler(data)->manager->dbus, "WindowActivated", window_id(&handler(data)->window));
    } else if(*s == ZWLR_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_FULLSCREEN) {
      printf("fullscreen\n");
      window_set_state(window, FULLSCREEN);
      dbus_send_signal(handler(data)->manager->dbus, "WindowFullscreened", window_id(&handler(data)->window));
    }
  }
}

/**
 * all information about the toplevel has been sent
 *
 * This event is sent after all changes in the toplevel state
 * have been sent.
 *
 * This allows changes to the zwlr_foreign_toplevel_handle_v1
 * properties to be seen as atomic, even if they happen via
 * multiple events.
 */
void top_level_handle_listener_done(memory_guarded void *data,
        memory_guarded struct zwlr_foreign_toplevel_handle_v1 *zwlr_foreign_toplevel_handle_v1)
{
  printf("\t\t[top_level_handle_listener_done]"); 
  if(window_app_id(&handler(data)->window) != NULL) { printf(" %s %d", window_app_id(&handler(data)->window), window_id(&handler(data)->window)); }
  printf("\n");
}

/**
 * this toplevel has been destroyed
 *
 * This event means the toplevel has been destroyed. It is
 * guaranteed there won't be any more events for this
 * zwlr_foreign_toplevel_handle_v1. The toplevel itself becomes
 * inert so any requests will be ignored except the destroy
 * request.
 */
void top_level_handle_listener_closed(memory_take_possession void *data,
        memory_guarded struct zwlr_foreign_toplevel_handle_v1 *zwlr_foreign_toplevel_handle_v1)
{
  printf("\t\t[top_level_handle_listener_closed]");
  dbus_send_signal(handler(data)->manager->dbus, "WindowClosed", window_id(&handler(data)->window));
  if(window_app_id(&handler(data)->window) != NULL) { printf(" %s %d", window_app_id(&handler(data)->window), window_id(&handler(data)->window)); }
  printf("\n");
  printf("\t\t\tDeleting object... ");
  handler_delete(handler(data));
  printf(" deleted.\n");
}

/**
 * parent change
 *
 * This event is emitted whenever the parent of the toplevel
 * changes.
 *
 * No event is emitted when the parent handle is destroyed by the
 * client.
 * @since 3
 */
void top_level_handle_listener_parent(memory_guarded void *data,
        memory_guarded struct zwlr_foreign_toplevel_handle_v1 *zwlr_foreign_toplevel_handle_v1,
        memory_guarded struct zwlr_foreign_toplevel_handle_v1 *parent)
{
    printf("\t\t[top_level_handle_listener_parent]\n");
}


///////////////////////////////////////
// Implementación del zwlr_foreign_toplevel_manager_v1_listener

void top_level_listener_toplevel(memory_guarded void *data,
			 memory_guarded struct zwlr_foreign_toplevel_manager_v1 *zwlr_foreign_toplevel_manager_v1,
			 memory_guarded struct zwlr_foreign_toplevel_handle_v1 *toplevel)
{
    printf("\t[top_level_listener_toplevel] Init\n");

    struct Handler *handler_obj = handler_new(_(data), toplevel, _(data)->output, &_(data)->windowlist);
    zwlr_foreign_toplevel_handle_v1_add_listener(toplevel, &(_(data)->top_level_handle_listener), handler_obj);
    dbus_send_signal(handler_obj->manager->dbus, "WindowOpened", window_id(&handler_obj->window));
}

void top_level_listener_finished(memory_guarded void *data,
    memory_guarded struct zwlr_foreign_toplevel_manager_v1 *zwlr_foreign_toplevel_manager_v1
    )
{
  windowlist_clear(&_(data)->windowlist);
  free(_(data)->toplevel);
}
///////////////////////////////////////

static void registry_listener_global(
    memory_guarded void *data, 
    memory_guarded struct wl_registry *registry, 
    uint32_t id, const char *interface, uint32_t version
)
{
  printf("Got a registry event for %s id %d\n", interface, id);
  if(!strcmp(interface, "zwlr_foreign_toplevel_manager_v1")) {
    printf("\tGet toplevel\n");
    _(data)->toplevel = wl_registry_bind(registry, id, &zwlr_foreign_toplevel_manager_v1_interface, version);
    zwlr_foreign_toplevel_manager_v1_add_listener(_(data)->toplevel, &(_(data)->top_level_listener), data);
  } else if(!strcmp(interface, "wl_output")) {
    printf("\tGet wl_output\n");
    _(data)->output = wl_registry_bind(registry, id, &wl_output_interface, version);
  } else if(!strcmp(interface, "wl_seat")) {
    printf("\tGet wl_seat\n");
    _(data)->seat = wl_registry_bind(registry, id, &wl_seat_interface, version);
  }
}

static void registry_listener_remove(
    memory_guarded void *data,
    memory_guarded struct wl_registry *registry, 
    uint32_t id
)
{
  printf("Got a registry losing event for %d\n", id);
}

int main(int argc, char **argv)
{

  struct wl_display *display = wl_display_connect(NULL);
  if (display == NULL) {
    fprintf(stderr, "Can't connect to display\n");
    exit(1);
  }
  printf("connected to display\n");

  struct Manager manager;
  manager.top_level_listener.toplevel = top_level_listener_toplevel;
  manager.top_level_listener.finished = top_level_listener_finished;

  manager.top_level_handle_listener.title = top_level_handle_listener_title;
  manager.top_level_handle_listener.app_id = top_level_handle_listener_app_id;
  manager.top_level_handle_listener.output_enter = top_level_handle_listener_output_enter;
  manager.top_level_handle_listener.output_leave = top_level_handle_listener_output_leave;
  manager.top_level_handle_listener.state = top_level_handle_listener_state;
  manager.top_level_handle_listener.done = top_level_handle_listener_done;
  manager.top_level_handle_listener.closed = top_level_handle_listener_closed;
  manager.top_level_handle_listener.parent = top_level_handle_listener_parent;

  manager.count = 0;

  windowlist_init(&manager.windowlist);

  struct DBus dbus;
  dbus_init(&dbus, &manager.windowlist, display);
  manager.dbus = &dbus;
  
  // Se crea el escuchador de wl_registry
  struct wl_registry_listener registry_listener;
  registry_listener.global = registry_listener_global;
  registry_listener.global_remove = registry_listener_remove;

  // Se obtiene el objeto wl_registry 
  struct wl_registry *registry = wl_display_get_registry(display);
  // Se le añade el escuchador recién creado
  wl_registry_add_listener(registry, &registry_listener, &manager);

  // Se solicita que se envíen las peticiones desde el cliente al servidor
  wl_display_dispatch(display);
  // Se bloquea hasta que se procesen los eventos
  wl_display_roundtrip(display);

  bool running = true;
  struct pollfd fds[2];
  long timeout_msecs = -1;
  int ret;

  fds[0].fd = wl_display_get_fd(display);
  fds[0].events = POLLIN;
  fds[0].revents = 0;

  dbus_init_struct_pollfd(&dbus, &(fds[1]));
 
  while (running /*wl_display_dispatch(display)*/) {
    // Bucle de eventos
    ret = poll(fds, 2, (int)timeout_msecs);
    if(ret > 0) {
      if(fds[0].revents) {
        // Process Wayland events
        running = wl_display_dispatch(display) != 0;
        fds[0].revents = 0;
      }
      if(fds[1].revents) {
        // Process DBus events
        printf("[main] dbus process start\n");
        dbus_process_poll_event(&dbus, &(fds[1]));
        printf("[main] dbus process end\n");
      }
    } else if(ret == 0) {
      printf("Timeout\n");
    } else {
      printf("poll failed %d\n", ret);
    }
  }

  wl_display_disconnect(display);
  printf("disconnected from display\n");
  dbus_finish(&dbus);
  return 0;
}
