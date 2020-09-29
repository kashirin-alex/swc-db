/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_core_comm_Buffers_h
#define swc_core_comm_Buffers_h

#include <asio.hpp>
#include "swcdb/core/Serializable.h"
#include "swcdb/core/Buffer.h"
#include "swcdb/core/comm/CommHeader.h"

#include <memory>
#include <string>
#include <vector>

namespace SWC { namespace Comm {


class Buffers final {
  public:

  typedef std::shared_ptr<Buffers> Ptr;

  static Ptr make(uint32_t reserve=0);

  static Ptr make(const Serializable& params, uint32_t reserve=0);

  static Ptr make(const Serializable& params, StaticBuffer& buffer, 
                  uint32_t reserve=0);

  static Ptr make(StaticBuffer& buffer, uint32_t reserve=0);

  static Ptr create_error_message(int error, const char *msg, uint16_t len);


  Buffers(uint32_t reserve=0);

  Buffers(const Serializable& params, uint32_t reserve=0);

  Buffers(const Serializable& params, StaticBuffer& buffer, 
          uint32_t reserve=0);

  Buffers(StaticBuffer& buffer, uint32_t reserve=0);

  ~Buffers();

  void set_data(uint32_t sz);

  void set_data(const Serializable& params, uint32_t reserve);

  void write_header();

  std::vector<asio::const_buffer> get_buffers();

  void append_i8(uint8_t ival);

  void append_i32(uint32_t ival);

  CommHeader    header;
  uint8_t*      data_ptr;

  private:

  StaticBuffer  buf_header;
  StaticBuffer  buf_data;
  StaticBuffer  buf_ext;


};


}} // namespace SWC::Comm



#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/comm/Buffers.cc"
#endif 

#endif // swc_core_comm_Buffers_h
