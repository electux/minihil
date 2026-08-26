#include <gio/gio.h>

#if defined (__ELF__) && ( __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 6))
# define SECTION __attribute__ ((section (".gresource.resources"), aligned (sizeof(void *) > 8 ? sizeof(void *) : 8)))
#else
# define SECTION
#endif

static const SECTION union { const guint8 data[781]; const double alignment; void * const ptr;}  resources_resource_data = {
  "\107\126\141\162\151\141\156\164\000\000\000\000\000\000\000\000"
  "\030\000\000\000\254\000\000\000\000\000\000\050\005\000\000\000"
  "\000\000\000\000\002\000\000\000\002\000\000\000\003\000\000\000"
  "\003\000\000\000\324\265\002\000\377\377\377\377\254\000\000\000"
  "\001\000\114\000\260\000\000\000\264\000\000\000\014\240\142\334"
  "\004\000\000\000\264\000\000\000\011\000\166\000\300\000\000\000"
  "\346\002\000\000\144\252\051\316\003\000\000\000\346\002\000\000"
  "\010\000\114\000\360\002\000\000\364\002\000\000\173\242\170\174"
  "\000\000\000\000\364\002\000\000\003\000\114\000\370\002\000\000"
  "\374\002\000\000\104\246\353\017\002\000\000\000\374\002\000\000"
  "\014\000\114\000\010\003\000\000\014\003\000\000\057\000\000\000"
  "\003\000\000\000\163\164\171\154\145\056\143\163\163\000\000\000"
  "\026\002\000\000\000\000\000\000\164\145\170\164\166\151\145\167"
  "\040\164\145\170\164\040\173\012\040\040\040\040\142\141\143\153"
  "\147\162\157\165\156\144\055\143\157\154\157\162\072\040\043\061"
  "\062\061\062\061\064\073\012\040\040\040\040\143\157\154\157\162"
  "\072\040\043\060\060\146\146\066\066\073\012\040\040\040\040\146"
  "\157\156\164\055\146\141\155\151\154\171\072\040\155\157\156\157"
  "\163\160\141\143\145\073\012\040\040\040\040\146\157\156\164\055"
  "\163\151\172\145\072\040\061\063\160\170\073\012\175\012\012\056"
  "\162\145\154\141\171\055\143\141\162\144\040\173\012\040\040\040"
  "\040\142\157\162\144\145\162\072\040\061\160\170\040\163\157\154"
  "\151\144\040\043\144\060\144\060\144\065\073\012\040\040\040\040"
  "\142\157\162\144\145\162\055\162\141\144\151\165\163\072\040\070"
  "\160\170\073\012\040\040\040\040\142\141\143\153\147\162\157\165"
  "\156\144\055\143\157\154\157\162\072\040\043\146\067\146\067\146"
  "\071\073\012\175\012\012\056\162\145\154\141\171\055\143\141\162"
  "\144\040\154\141\142\145\154\040\173\012\040\040\040\040\143\157"
  "\154\157\162\072\040\043\061\062\061\062\061\064\073\012\175\012"
  "\012\056\162\145\154\141\171\055\143\141\162\144\072\144\151\163"
  "\141\142\154\145\144\040\154\141\142\145\154\040\173\012\040\040"
  "\040\040\143\157\154\157\162\072\040\043\067\146\070\143\070\144"
  "\073\012\175\012\012\056\154\145\144\055\151\156\144\151\143\141"
  "\164\157\162\040\173\012\040\040\040\040\146\157\156\164\055\163"
  "\151\172\145\072\040\062\064\160\170\073\012\175\012\012\056\162"
  "\145\154\141\171\055\143\141\162\144\040\154\141\142\145\154\056"
  "\154\145\144\055\157\156\040\173\012\040\040\040\040\143\157\154"
  "\157\162\072\040\043\062\145\143\143\067\061\073\012\040\040\040"
  "\040\164\145\170\164\055\163\150\141\144\157\167\072\040\060\040"
  "\060\040\061\060\160\170\040\043\062\145\143\143\067\061\054\040"
  "\060\040\060\040\062\060\160\170\040\043\062\145\143\143\067\061"
  "\073\012\175\012\012\056\162\145\154\141\171\055\143\141\162\144"
  "\040\154\141\142\145\154\056\154\145\144\055\157\146\146\040\173"
  "\012\040\040\040\040\143\157\154\157\162\072\040\043\142\144\143"
  "\063\143\067\073\012\040\040\040\040\164\145\170\164\055\163\150"
  "\141\144\157\167\072\040\156\157\156\145\073\012\175\012\000\000"
  "\050\165\165\141\171\051\145\154\145\143\164\165\170\057\000\000"
  "\004\000\000\000\151\157\057\000\002\000\000\000\155\151\156\151"
  "\150\151\154\144\145\163\153\057\001\000\000\000" };

static GStaticResource static_resource = { resources_resource_data.data, sizeof (resources_resource_data.data) - 1 /* nul terminator */, NULL, NULL, NULL };

G_MODULE_EXPORT
GResource *resources_get_resource (void);
GResource *resources_get_resource (void)
{
  return g_static_resource_get_resource (&static_resource);
}
/* GLIB - Library of useful routines for C programming
 * Copyright (C) 1995-1997  Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Modified by the GLib Team and others 1997-2000.  See the AUTHORS
 * file for a list of people on the GLib Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GLib at ftp://ftp.gtk.org/pub/gtk/.
 */

#ifndef __G_CONSTRUCTOR_H__
#define __G_CONSTRUCTOR_H__

/*
  If G_HAS_CONSTRUCTORS is true then the compiler support *both* constructors and
  destructors, in a usable way, including e.g. on library unload. If not you're on
  your own.

  Some compilers need #pragma to handle this, which does not work with macros,
  so the way you need to use this is (for constructors):

  #ifdef G_DEFINE_CONSTRUCTOR_NEEDS_PRAGMA
  #pragma G_DEFINE_CONSTRUCTOR_PRAGMA_ARGS(my_constructor)
  #endif
  G_DEFINE_CONSTRUCTOR(my_constructor)
  static void my_constructor(void) {
   ...
  }

*/

#ifndef __GTK_DOC_IGNORE__

#if  __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 7)

#define G_HAS_CONSTRUCTORS 1

#define G_DEFINE_CONSTRUCTOR(_func) static void __attribute__((constructor)) _func (void);
#define G_DEFINE_DESTRUCTOR(_func) static void __attribute__((destructor)) _func (void);

#elif defined (_MSC_VER)

/*
 * Only try to include gslist.h if not already included via glib.h,
 * so that items using gconstructor.h outside of GLib (such as
 * GResources) continue to build properly.
 */
#ifndef __G_LIB_H__
#include "gslist.h"
#endif

#include <stdlib.h>

#define G_HAS_CONSTRUCTORS 1

/* We do some weird things to avoid the constructors being optimized
 * away on VS2015 if WholeProgramOptimization is enabled. First we
 * make a reference to the array from the wrapper to make sure its
 * references. Then we use a pragma to make sure the wrapper function
 * symbol is always included at the link stage. Also, the symbols
 * need to be extern (but not dllexport), even though they are not
 * really used from another object file.
 */

/* We need to account for differences between the mangling of symbols
 * for x86 and x64/ARM/ARM64 programs, as symbols on x86 are prefixed
 * with an underscore but symbols on x64/ARM/ARM64 are not.
 */
#ifdef _M_IX86
#define G_MSVC_SYMBOL_PREFIX "_"
#else
#define G_MSVC_SYMBOL_PREFIX ""
#endif

#define G_DEFINE_CONSTRUCTOR(_func) G_MSVC_CTOR (_func, G_MSVC_SYMBOL_PREFIX)
#define G_DEFINE_DESTRUCTOR(_func) G_MSVC_DTOR (_func, G_MSVC_SYMBOL_PREFIX)

#define G_MSVC_CTOR(_func,_sym_prefix) \
  static void _func(void); \
  extern int (* _array ## _func)(void);              \
  int _func ## _wrapper(void);              \
  int _func ## _wrapper(void) { _func(); g_slist_find (NULL,  _array ## _func); return 0; } \
  __pragma(comment(linker,"/include:" _sym_prefix # _func "_wrapper")) \
  __pragma(section(".CRT$XCU",read)) \
  __declspec(allocate(".CRT$XCU")) int (* _array ## _func)(void) = _func ## _wrapper;

#define G_MSVC_DTOR(_func,_sym_prefix) \
  static void _func(void); \
  extern int (* _array ## _func)(void);              \
  int _func ## _constructor(void);              \
  int _func ## _constructor(void) { atexit (_func); g_slist_find (NULL,  _array ## _func); return 0; } \
   __pragma(comment(linker,"/include:" _sym_prefix # _func "_constructor")) \
  __pragma(section(".CRT$XCU",read)) \
  __declspec(allocate(".CRT$XCU")) int (* _array ## _func)(void) = _func ## _constructor;

#elif defined(__SUNPRO_C)

/* This is not tested, but i believe it should work, based on:
 * http://opensource.apple.com/source/OpenSSL098/OpenSSL098-35/src/fips/fips_premain.c
 */

#define G_HAS_CONSTRUCTORS 1

#define G_DEFINE_CONSTRUCTOR_NEEDS_PRAGMA 1
#define G_DEFINE_DESTRUCTOR_NEEDS_PRAGMA 1

#define G_DEFINE_CONSTRUCTOR_PRAGMA_ARGS(_func) \
  init(_func)
#define G_DEFINE_CONSTRUCTOR(_func) \
  static void _func(void);

#define G_DEFINE_DESTRUCTOR_PRAGMA_ARGS(_func) \
  fini(_func)
#define G_DEFINE_DESTRUCTOR(_func) \
  static void _func(void);

#else

/* constructors not supported for this compiler */

#endif

#endif /* __GTK_DOC_IGNORE__ */
#endif /* __G_CONSTRUCTOR_H__ */

#ifdef G_HAS_CONSTRUCTORS

#ifdef G_DEFINE_CONSTRUCTOR_NEEDS_PRAGMA
#pragma G_DEFINE_CONSTRUCTOR_PRAGMA_ARGS(resourcesresource_constructor)
#endif
G_DEFINE_CONSTRUCTOR(resourcesresource_constructor)
#ifdef G_DEFINE_DESTRUCTOR_NEEDS_PRAGMA
#pragma G_DEFINE_DESTRUCTOR_PRAGMA_ARGS(resourcesresource_destructor)
#endif
G_DEFINE_DESTRUCTOR(resourcesresource_destructor)

#else
#warning "Constructor not supported on this compiler, linking in resources will not work"
#endif

static void resourcesresource_constructor (void)
{
  g_static_resource_init (&static_resource);
}

static void resourcesresource_destructor (void)
{
  g_static_resource_fini (&static_resource);
}
