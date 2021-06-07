/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_Protocol_params_Pread_h
#define swcdb_fs_Broker_Protocol_params_Pread_h


#include "swcdb/core/comm/Serializable.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Params {


class PreadReq final : public Serializable {
  public:

  SWC_CAN_INLINE
  PreadReq() noexcept : fd(-1) { }

  SWC_CAN_INLINE
  PreadReq(int32_t fd, uint64_t offset, uint32_t amount) noexcept
          : fd(fd), offset(offset), amount(amount) { }

  int32_t   fd;
  uint64_t  offset;
  uint32_t  amount;

  private:

  size_t internal_encoded_length() const override;

  void internal_encode(uint8_t** bufp) const override;

  void internal_decode(const uint8_t** bufp, size_t* remainp) override;

};


}}}}}


#if defined(SWC_IMPL_SOURCE) or \
    (defined(FS_BROKER_APP) and !defined(BUILTIN_FS_BROKER))
#include "swcdb/fs/Broker/Protocol/params/Pread.cc"
#endif


#endif // swcdb_fs_Broker_Protocol_params_Pread_h
