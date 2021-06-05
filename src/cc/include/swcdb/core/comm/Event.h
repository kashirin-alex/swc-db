/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_core_comm_Event_h
#define swcdb_core_comm_Event_h


#include "swcdb/core/Time.h"
#include "swcdb/core/Buffer.h"
#include "swcdb/core/comm/Header.h"


namespace SWC { namespace Comm {

class Event final {

  public:

  /** Enumeration for event types.*/
  enum Type : uint8_t {
    ERROR       = 0x00,  ///< %Error event
    MESSAGE     = 0x01,  ///< Request/response message event
    DISCONNECT  = 0x02,  ///< Connection disconnected event
  };

  typedef std::shared_ptr<Event> Ptr;

  static Ptr make(Type type, int error) {
    return std::make_shared<Event>(type, error);
  }

  explicit Event(Type type_, int error_) noexcept
                : type(type_), error(error_), expiry_ms(0) {
  }

  //~Event() { }

  SWC_CAN_INLINE
  void received() noexcept {
    if(header.timeout_ms)
      expiry_ms = Time::now_ms() + header.timeout_ms - 1;
  }

  void decode_buffers();

  bool expired(int64_t within=0) const noexcept;

  int32_t response_code();

  void print(std::ostream& out) const;

  Type                type;
  int                 error;
  int64_t             expiry_ms;
  StaticBuffer        data;     //!< Primary data buffer
  StaticBuffer        data_ext; //!< Extended buffer
  Header              header;
};

}} // namespace SWC::Comm


#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/comm/Event.cc"
#endif

#endif // swcdb_core_comm_Event_h
