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


#ifndef __LIST_H__
#define __LIST_H__

#include <stdint.h>
#include <stdlib.h>
#include "window.h"

struct WindowList {
  memory_owner_of(memory_guarded) struct Window **windows;
  uint32_t size;
  uint32_t real_size;
};

void windowlist_init(memory_guarded struct WindowList *list);
void windowlist_clear(memory_guarded struct WindowList *list);

void windowlist_add(memory_guarded struct WindowList *list, memory_guarded struct Window *window);
void windowlist_remove(memory_guarded struct WindowList *list, uint32_t id);

memory_guarded struct Window *windowlist_get(memory_guarded struct WindowList *list, uint32_t id); 
memory_take_possession uint32_t *windowlist_get_ids(memory_guarded struct WindowList *list);

#define windowlist_foreach(list,n,window) for(window=list->windows[0],n=0; n<list->size; window=list->windows[++n])

#endif
