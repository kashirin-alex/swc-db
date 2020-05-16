/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_core_COMPAT_H
#define swc_core_COMPAT_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <cstddef> // for std::size_t and std::ptrdiff_t
#include <memory>

// SWC_ATTRIBS(SWC_NOTHROW, SWC_INLINE)
#define SWC_ATTRIBS(attrs) __attribute__(attrs)
#define SWC_NOTHROW (__nothrow__)
#define SWC_INLINE (__always_inline__, __artificial__)


#if defined (__GLIBC__) && (__GLIBC__ >= 2) && (__GLIBC_MINOR__ >= 23)
#define USE_READDIR_R 0
#else
#define USE_READDIR_R 1
#endif

#ifdef SWC_USE_ABORT
#define SWC_ABORT abort()
#else
#define SWC_ABORT raise(SIGABRT)
#endif


#include "swcdb/core/Version.h"
#include "swcdb/core/BitFieldInt.h" // for u/int24/40/48/56_t

#endif
