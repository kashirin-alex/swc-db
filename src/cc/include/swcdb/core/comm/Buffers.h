/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_core_comm_Buffers_h
#define swcdb_core_comm_Buffers_h

#include "swcdb/core/Buffer.h"
#include "swcdb/core/comm/asio_wrap.h"
#include "swcdb/core/comm/Serializable.h"
#include "swcdb/core/comm/Header.h"
#include "swcdb/core/comm/Event.h"


namespace SWC { namespace Comm {


class Buffers final {
  public:

  typedef std::shared_ptr<Buffers> Ptr;

  /* Make Common */
  static Ptr make(uint32_t reserve=0);

  static Ptr make(const Serializable& params, uint32_t reserve=0);

  static Ptr make(const Serializable& params, StaticBuffer& buffer,
                  uint32_t reserve=0);

  static Ptr make(StaticBuffer& buffer, uint32_t reserve=0);

  /* Make Request */
  static Ptr make(const Serializable& params, uint32_t reserve,
                  uint64_t cmd, uint32_t timeout);

  static Ptr make(const Serializable& params, StaticBuffer& buffer,
                  uint32_t reserve,
                  uint64_t cmd, uint32_t timeout);

  /* Make Response */
  static Ptr make(const Event::Ptr& ev, uint32_t reserve=0);

  static Ptr make(const Event::Ptr& ev,
                  const Serializable& params, uint32_t reserve=0);

  static Ptr make(const Event::Ptr& ev,
                  const Serializable& params, StaticBuffer& buffer,
                  uint32_t reserve=0);

  static Ptr make(const Event::Ptr& ev,
                  StaticBuffer& buffer, uint32_t reserve=0);

  static Ptr create_error_message(const Event::Ptr& ev,
                                  int error, const char *msg, uint16_t len);

  /* Init Common */
  Buffers(uint32_t reserve=0);

  Buffers(const Serializable& params, uint32_t reserve=0);

  Buffers(const Serializable& params, StaticBuffer& buffer,
          uint32_t reserve=0);

  Buffers(StaticBuffer& buffer, uint32_t reserve=0);

  /* Init Request */
  Buffers(const Serializable& params, uint32_t reserve,
          uint64_t cmd, uint32_t timeout);

  Buffers(const Serializable& params, StaticBuffer& buffer,
          uint32_t reserve,
          uint64_t cmd, uint32_t timeout);

  /* Init Response */
  Buffers(const Event::Ptr& ev, uint32_t reserve=0);

  Buffers(const Event::Ptr& ev,
          const Serializable& params, uint32_t reserve=0);

  Buffers(const Event::Ptr& ev,
          StaticBuffer& buffer, uint32_t reserve=0);

  Buffers(const Event::Ptr& ev,
          const Serializable& params, StaticBuffer& buffer,
          uint32_t reserve=0);

  ~Buffers();

  void set_data(uint32_t sz);

  void set_data(const Serializable& params, uint32_t reserve);

  void prepare(Core::Encoder::Type encoder);

  bool expired() const noexcept;

  uint8_t write_header();

  std::vector<asio::const_buffer> get_buffers();

  void append_i8(uint8_t ival) noexcept;

  void append_i32(uint32_t ival) noexcept;

  Header    header;
  int64_t   expiry_ms;
  uint8_t*  data_ptr;

  private:

  uint8_t       buf_header[Header::MAX_LENGTH];
  StaticBuffer  buf_data;
  StaticBuffer  buf_ext;


};


}} // namespace SWC::Comm



#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/comm/Buffers.cc"
#endif

#endif // swcdb_core_comm_Buffers_h
