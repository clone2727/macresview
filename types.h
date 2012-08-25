/* macresview - A simple Mac resource fork dumper
 *
 * macresview is the legal property of its developers, whose names can be
 * found in the AUTHORS file distributed with this source
 * distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef TYPES_H
#define TYPES_H

#include <limits.h>

// Standard types
// If this doesn't work, too bad :P

typedef unsigned char byte;
typedef signed char int8;
typedef unsigned int uint;

#if USHRT_MAX == 65535
typedef unsigned short uint16;
typedef signed short int16;
#elif UINT_MAX == 65535
typedef unsigned int uint16;
typedef signed int int16;
#elif ULONG_MAX == 65535
typedef unsigned long uint16;
typedef signed long int16;
#else
	#error "Could not detect 16-bit integer type"
#endif

#if USHRT_MAX == 4294967295
typedef unsigned short uint32;
typedef signed short int32;
#elif UINT_MAX == 4294967295
typedef unsigned int uint32;
typedef signed int int32;
#elif ULONG_MAX == 4294967295
typedef unsigned long uint32;
typedef signed long int32;
#else
	#error "Could not detect 32-bit integer type"
#endif

#endif
