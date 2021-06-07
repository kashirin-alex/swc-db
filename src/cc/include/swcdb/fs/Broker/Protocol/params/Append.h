/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_fs_Broker_Protocol_params_Append_h
#define swcdb_fs_Broker_Protocol_params_Append_h


#include "swcdb/core/comm/Serializable.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Params {


class AppendReq final : public Serializable {
  public:

  SWC_CAN_INLINE
  AppendReq() noexcept : fd(-1), flags(0) { }

  SWC_CAN_INLINE
  AppendReq(int32_t fd, uint8_t flags) noexcept : fd(fd), flags(flags) { }

  int32_t fd;
  uint8_t flags;

  private:

  size_t internal_encoded_length() const override;

  void internal_encode(uint8_t** bufp) const override;

  void internal_decode(const uint8_t** bufp, size_t* remainp) override;

};




class AppendRsp final : public Serializable {
  public:

  SWC_CAN_INLINE
  AppendRsp() noexcept : amount(0) { }

  SWC_CAN_INLINE
  AppendRsp(uint64_t offset, uint32_t amount) noexcept
            : offset(offset), amount(amount) { }

  uint64_t offset {};
  uint32_t amount {};

  private:

  size_t internal_encoded_length() const override;

  void internal_encode(uint8_t** bufp) const override;

  void internal_decode(const uint8_t** bufp, size_t* remainp) override;

};

}}}}}


#if defined(SWC_IMPL_SOURCE) or \
    (defined(FS_BROKER_APP) and !defined(BUILTIN_FS_BROKER))
#include "swcdb/fs/Broker/Protocol/params/Append.cc"
#endif


#endif // swcdb_fs_Broker_Protocol_params_Append_h
