/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_core_comm_Event_h
#define swcdb_core_comm_Event_h


#include "swcdb/core/Time.h"
#include "swcdb/core/Buffer.h"
#include "swcdb/core/Exception.h"
#include "swcdb/core/Serialization.h"
#include "swcdb/core/comm/Header.h"


namespace SWC { namespace Comm {

class Event final {

  private:

  SWC_CAN_INLINE
  explicit Event(int a_error) noexcept : expiry_ms(0), error(a_error) { }

  public:

  typedef std::shared_ptr<Event> Ptr;

  SWC_CAN_INLINE
  static Ptr make(int error) {
    return Ptr(new Event(error));
  }

  ~Event() noexcept;

  SWC_CAN_INLINE
  void received() noexcept {
    if(header.timeout_ms)
      expiry_ms = Time::now_ms() + header.timeout_ms - 1;
  }

  void decode_buffers();

  SWC_CAN_INLINE
  bool expired(int64_t within=0) const noexcept {
    return expiry_ms && Time::now_ms() > (expiry_ms - within);
  }

  int32_t response_code() const noexcept;

  void print(std::ostream& out) const;

  int64_t             expiry_ms;
  StaticBuffer        data;     //!< Primary data buffer
  StaticBuffer        data_ext; //!< Extended buffer
  Header              header;
  int                 error;

};



SWC_CAN_INLINE
void Event::decode_buffers() {
  int err = Error::OK;
  uint8_t n = 1;
  header.data.decode(err, data);
  if(!err && header.buffers > 1) {
    ++n;
    header.data_ext.decode(err, data_ext);
  }

  if(err) {
    error = Error::REQUEST_TRUNCATED_PAYLOAD;
    data.free();
    data_ext.free();
    SWC_LOG_OUT(LOG_WARN,
      SWC_LOG_OSTREAM
        << "decode, REQUEST ENCODER_DECODE: n(" << int(n) << ") ";
      print(SWC_LOG_OSTREAM);
    );
  }
}



}} // namespace SWC::Comm


#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/comm/Event.cc"
#endif

#endif // swcdb_core_comm_Event_h
