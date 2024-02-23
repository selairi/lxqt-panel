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


#include "windowlist.h"
#include <stdlib.h>
#include <stdio.h>

void windowlist_init(struct WindowList *list)
{
  list->real_size = 8;
  list->windows = (struct Window **)malloc(sizeof(struct Window *)*list->real_size);
  list->size = 0;
}

void windowlist_clear(struct WindowList *list)
{
  free(list->windows);
  list->windows = NULL;
  list->size = 0;
  list->real_size = 0;
}

void windowlist_add(struct WindowList *list, struct Window *window)
{
  list->size++;
  if(list->size > list->real_size) {
    list->real_size += 8;
    struct Window **windows = realloc(list->windows, sizeof(struct Window*)*list->real_size);
    if(windows != NULL) {
      list->windows = windows;
    } else {
      list->size--;
      list->real_size -= 8;
      printf("Error: No memory left\n");
    }
  }
  list->windows[list->size - 1] = window;
}

void windowlist_remove(struct WindowList *list, uint32_t id)
{
  uint32_t n;
  for(n = 0; n < list->size; n++) {
    if(window_id(list->windows[n]) == id) break;
  }
  for(n += 1; n < list->size; n++) {
    list->windows[n - 1] = list->windows[n];
  }
  list->size--;
  if(list->real_size > 8 && list->size < (list->real_size - 10)) {
    list->real_size -= 8;
    list->windows = realloc(list->windows, sizeof(struct Window*)*list->real_size);
  }
}

struct Window *windowlist_get(struct WindowList *list, uint32_t id)
{
  // bsearch in window list
  struct Window *item;
  int cmp, i;
  //printf("[windowlist_get]: Size %d\n", list->size);
  int low = 0, upper = list->size;
  while(low < upper) {
    i = (low + upper) >> 1; // i = (low + upper) / 2;
    //printf("\tWindow %d: id %d size: %ld\n", i, list->windows[i]->id, sizeof(struct Window*));
    item = list->windows[i];
    cmp = id - item->id;
    if(cmp < 0) upper = i;
    else if(cmp > 0) low = i + 1;
    else return item;
  }
  return NULL;
}



memory_take_possession uint32_t *windowlist_get_ids(memory_guarded struct WindowList *list)
{
  uint32_t n, *res = (uint32_t*)malloc(sizeof(uint32_t)*list->size);
  struct Window *window;
  if(res != NULL) {
    windowlist_foreach(list,n,window) { res[n] = window_id(window); }
  }
  return res;
}
