/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_core_comm_CommBuf_h
#define swc_core_comm_CommBuf_h

#include <asio.hpp>
#include "swcdb/core/Serializable.h"
#include "swcdb/core/StaticBuffer.h"
#include "swcdb/core/comm/CommHeader.h"

#include <memory>
#include <string>
#include <vector>

namespace SWC {


class CommBuf final {
  public:

  typedef std::shared_ptr<CommBuf> Ptr;

  static Ptr make(uint32_t reserve=0);

  static Ptr make(const Serializable& params, uint32_t reserve=0);

  static Ptr make(const Serializable& params, StaticBuffer& buffer, 
                  uint32_t reserve=0);

  static Ptr make(StaticBuffer& buffer, uint32_t reserve=0);

  static Ptr create_error_message(int error, const char *msg, uint16_t len);


  CommBuf(uint32_t reserve=0);

  CommBuf(const Serializable& params, uint32_t reserve=0);

  CommBuf(const Serializable& params, StaticBuffer& buffer, 
          uint32_t reserve=0);

  CommBuf(StaticBuffer& buffer, uint32_t reserve=0);

  ~CommBuf();

  void set_data(uint32_t sz);

  void set_data(const Serializable& params, uint32_t reserve);

  void write_header();

  std::vector<asio::const_buffer> get_buffers();

  void append_i32(uint32_t ival);
  
  CommHeader    header;
  uint8_t*      data_ptr;

  private:

  StaticBuffer  buf_header;
  StaticBuffer  buf_data;
  StaticBuffer  buf_ext;


};


} // namespace SWC



#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/comm/CommBuf.cc"
#endif 

#endif // swc_core_comm_CommBuf_h
