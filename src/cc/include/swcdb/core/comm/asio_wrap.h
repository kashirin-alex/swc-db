/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */



#ifndef swcdb_core_comm_asio_wrap_h
#define swcdb_core_comm_asio_wrap_h


#pragma GCC diagnostic push


#pragma GCC diagnostic ignored  "-Wold-style-cast"
#pragma GCC diagnostic ignored  "-Wnull-dereference"
#pragma GCC diagnostic ignored  "-Wzero-as-null-pointer-constant"


#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic ignored  "-Wnoexcept"
#pragma GCC diagnostic ignored  "-Wsuggest-override"
#pragma GCC diagnostic ignored  "-Wuseless-cast"
#pragma GCC diagnostic ignored  "-Wstrict-null-sentinel"
#pragma GCC diagnostic ignored  "-Wduplicated-branches"
#pragma GCC diagnostic ignored  "-Wsuggest-attribute=const"
#pragma GCC diagnostic ignored  "-Wsuggest-attribute=pure"
#pragma GCC diagnostic ignored  "-Wsuggest-attribute=malloc"
#pragma GCC diagnostic ignored  "-Wsuggest-attribute=cold"
#pragma GCC diagnostic ignored  "-Weffc++"
#endif


#include <asio.hpp>
#include <asio/ssl.hpp>



#pragma GCC diagnostic pop


#endif // swcdb_core_comm_asio_wrap_h
