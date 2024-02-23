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


#ifndef __MEMORY_H__
#define __MEMORY_H__

#define memory_guarded
#define memory_owner
#define memory_take_possession
#define memory_owner_of(mem)
#define memory_ref_count
#define memory_ptr_inout
#define memory_ptr_out

#define m_g
#define m_o
#define m_t
#define m_o_(mem)
#define m_r

#define _cleanup_(f) __attribute__((cleanup(f)))

#define UNUSED(var) (void)var

// Example:
//
// struct Example {
// 	memory_guarded char *name; // This pointer must not be freed by this object
// 	memory_owner   char *id;     // This object must be freed by this object
// };
// 
// memory_take_possession struct Example *example_new(
//     memory_guarded        char *name, 
//     memory_take_possession char *id
// ) 
// {
// 	struct Example *ex = (struct Example *)malloc(sizeof(struct Example));
// 	ex->name = name; ex->id = id;
// 	return ex;
// }
// 
// void example_delete(memory_owner struct Example *ex) {
// 	free(ex->id);
// 	free(ex);
// }
// 
// memory_guarded char *example_id(memory_guarded struct Example *ex) {
// 	return ex->id;
// }
// 
// int main() {
//   const char *name = "Hello";
//   struct Example *ex = example_new((char*)name, strdup("World"));
//   printf("%s\n", example_id(ex));
//   example_delete(ex);
// }
#endif
