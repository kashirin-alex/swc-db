/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_core_Compact_h
#define swcdb_core_Compact_h



//! The SWC-DB C++ namespace 'SWC'
namespace SWC { 


/**
 * \defgroup Core The Core-Components Group
 * @brief A group with all related to SWC-DB Core-Components (libswcdb_core, libswcdb_core_config, libswcdb_core_comm).
 *
 * 
 */

/**
 * @brief The SWC-DB Core Components C++ namespace 'SWC::Core'
 *
 * \ingroup Core
 */
namespace Core { }
}



/*!
 *  \addtogroup Core
 *  @{
 */


#define SWC_ATTRIBS(attrs) __attribute__(attrs)
#define SWC_ATTR_NOTHROW __nothrow__
#define SWC_ATTR_INLINE __always_inline__ //, __artificial__

#define SWC_NOEXCEPT noexcept(true)


#ifdef SWC_IMPL_SOURCE
# define SWC_SHOULD_INLINE  \
  SWC_ATTRIBS((SWC_ATTR_INLINE)) \
  extern inline
#else 
# define SWC_SHOULD_INLINE
#endif


# define SWC_SHOULD_NOT_INLINE  \
  SWC_ATTRIBS((__noinline__, __noclone__))


# define SWC_CAN_INLINE  \
  SWC_ATTRIBS((SWC_ATTR_INLINE)) \
  inline



#include <stddef.h>
#include <stdint.h>

#include "swcdb/core/Malloc.h"

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <memory>
#include <string>


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


/*! @} End of Core Group*/




#include "swcdb/core/BitFieldInt.h" // for u/int24/40/48/56_t


#if defined SWC_IMPL_SOURCE && defined SWC_MALLOC
#include "swcdb/core/MallocImpl.cc"
#endif 

#endif // swcdb_core_Compact_h
