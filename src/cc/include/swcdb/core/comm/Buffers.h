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
#include "swcdb/core/Serialization.h"


namespace SWC { namespace Comm {


class Buffers final {
  public:

  typedef std::shared_ptr<Buffers> Ptr;

  /* Make Common */
  SWC_CAN_INLINE
  static Ptr make(uint32_t reserve=0) {
    return Ptr(new Buffers(reserve));
  }

  SWC_CAN_INLINE
  static Ptr make(const Serializable& params, uint32_t reserve=0) {
    return Ptr(new Buffers(params, reserve));
  }

  SWC_CAN_INLINE
  static Ptr make(const Serializable& params, StaticBuffer& buffer,
                  uint32_t reserve=0) {
    return Ptr(new Buffers(params, buffer, reserve));
  }

  SWC_CAN_INLINE
  static Ptr make(StaticBuffer& buffer, uint32_t reserve=0) {
    return Ptr(new Buffers(buffer, reserve));
  }


  /* Make Request */
  SWC_CAN_INLINE
  static Ptr make(const Serializable& params, uint32_t reserve,
                  uint64_t cmd, uint32_t timeout) {
    return Ptr(new Buffers(params, reserve, cmd, timeout));
  }

  SWC_CAN_INLINE
  static Ptr make(const Serializable& params, StaticBuffer& buffer,
                  uint32_t reserve, uint64_t cmd, uint32_t timeout) {
    return Ptr(new Buffers(params, buffer, reserve, cmd, timeout));
  }


  /* Make Response */
  SWC_CAN_INLINE
  static Ptr make(const Event::Ptr& ev, uint32_t reserve=0) {
    return Ptr(new Buffers(ev, reserve));
  }

  SWC_CAN_INLINE
  static Ptr make(const Event::Ptr& ev,
                  const Serializable& params, uint32_t reserve=0) {
    return Ptr(new Buffers(ev, params, reserve));
  }

  SWC_CAN_INLINE
  static Ptr make(const Event::Ptr& ev,
                  const Serializable& params, StaticBuffer& buffer,
                  uint32_t reserve=0) {
    return Ptr(new Buffers(ev, params, buffer, reserve));
  }

  SWC_CAN_INLINE
  static Ptr make(const Event::Ptr& ev,
                  StaticBuffer& buffer, uint32_t reserve=0) {
    return Ptr(new Buffers(ev, buffer, reserve));
  }

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

  ~Buffers() noexcept;

  void set_data(uint32_t sz);

  void set_data(const Serializable& params, uint32_t reserve);

  SWC_CAN_INLINE
  void append_i8(uint8_t ival) noexcept {
    Serialization::encode_i8(&data_ptr, ival);
  }

  SWC_CAN_INLINE
  void append_i32(uint32_t ival) noexcept {
    Serialization::encode_i32(&data_ptr, ival);
  }

  SWC_CAN_INLINE
  void prepare(Core::Encoder::Type encoder) {
    if(buf_data.size) {
      header.data.encode(encoder, buf_data);
      if(buf_ext.size)
        header.data_ext.encode(encoder, buf_ext);
    }
  }

  SWC_CAN_INLINE
  bool expired() const noexcept {
    return expiry_ms && Time::now_ms() > expiry_ms;
  }

  Core::Vector<asio::const_buffer> get_buffers();

  Header    header;
  int64_t   expiry_ms;
  uint8_t*  data_ptr;

  private:

  uint8_t       buf_header[Header::MAX_LENGTH];
  StaticBuffer  buf_data;
  StaticBuffer  buf_ext;


};



SWC_CAN_INLINE
Buffers::Ptr
Buffers::create_error_message(const Event::Ptr& ev,
                              int error, const char *msg, uint16_t len) {
  auto cbp = Buffers::make(ev, 4 + Serialization::encoded_length_bytes(len));
  Serialization::encode_i32(&cbp->data_ptr, error);
  Serialization::encode_bytes(&cbp->data_ptr, msg, len);
  return cbp;
}

/* Init Common */
SWC_CAN_INLINE
Buffers::Buffers(uint32_t reserve)
                : expiry_ms(0) {
  if(reserve)
    set_data(reserve);
}

SWC_CAN_INLINE
Buffers::Buffers(const Serializable& params, uint32_t reserve)
                : expiry_ms(0) {
  set_data(params, reserve);
}

SWC_CAN_INLINE
Buffers::Buffers(const Serializable& params, StaticBuffer& buffer,
                 uint32_t reserve)
                : expiry_ms(0), buf_ext(buffer) {
  set_data(params, reserve);
}

SWC_CAN_INLINE
Buffers::Buffers(StaticBuffer& buffer, uint32_t reserve)
                : expiry_ms(0), buf_ext(buffer) {
  if(reserve)
    set_data(reserve);
}


/* Init Request */
SWC_CAN_INLINE
Buffers::Buffers(const Serializable& params, uint32_t reserve,
                 uint64_t cmd, uint32_t timeout)
                : header(cmd, timeout), expiry_ms(0) {
  set_data(params, reserve);
}

SWC_CAN_INLINE
Buffers::Buffers(const Serializable& params, StaticBuffer& buffer,
                 uint32_t reserve,
                 uint64_t cmd, uint32_t timeout)
                : header(cmd, timeout), expiry_ms(0),
                  buf_ext(buffer) {
  set_data(params, reserve);
}


/* Init Response */
SWC_CAN_INLINE
Buffers::Buffers(const Event::Ptr& ev, uint32_t reserve)
                : header(ev->header), expiry_ms(ev->expiry_ms) {
  if(reserve)
    set_data(reserve);
}

SWC_CAN_INLINE
Buffers::Buffers(const Event::Ptr& ev,
                 const Serializable& params, uint32_t reserve)
                : header(ev->header), expiry_ms(ev->expiry_ms) {
  set_data(params, reserve);
}

SWC_CAN_INLINE
Buffers::Buffers(const Event::Ptr& ev,
                 const Serializable& params, StaticBuffer& buffer,
                 uint32_t reserve)
                : header(ev->header), expiry_ms(ev->expiry_ms),
                  buf_ext(buffer) {
  set_data(params, reserve);
}

SWC_CAN_INLINE
Buffers::Buffers(const Event::Ptr& ev,
                 StaticBuffer& buffer, uint32_t reserve)
                : header(ev->header), expiry_ms(ev->expiry_ms),
                  buf_ext(buffer) {
  if(reserve)
    set_data(reserve);
}



}} // namespace SWC::Comm



#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/comm/Buffers.cc"
#endif

#endif // swcdb_core_comm_Buffers_h
