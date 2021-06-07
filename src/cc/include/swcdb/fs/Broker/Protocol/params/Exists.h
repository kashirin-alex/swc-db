/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_Protocol_params_Exists_h
#define swcdb_fs_Broker_Protocol_params_Exists_h


#include "swcdb/core/comm/Serializable.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Params {


class ExistsReq final : public Serializable {
  public:

  SWC_CAN_INLINE
  ExistsReq() noexcept { }

  SWC_CAN_INLINE
  ExistsReq(const std::string& fname) : fname(fname) { }

  std::string fname;

  private:

  size_t internal_encoded_length() const override;

  void internal_encode(uint8_t** bufp) const override;

  void internal_decode(const uint8_t** bufp, size_t* remainp) override;

};




class ExistsRsp final : public Serializable {
  public:

  SWC_CAN_INLINE
  ExistsRsp() noexcept { }

  SWC_CAN_INLINE
  ExistsRsp(bool exists) noexcept : exists(exists) { }

  bool exists;

  private:

  size_t internal_encoded_length() const override;

  void internal_encode(uint8_t** bufp) const override;

  void internal_decode(const uint8_t** bufp, size_t* remainp) override;

};

}}}}}


#if defined(SWC_IMPL_SOURCE) or \
    (defined(FS_BROKER_APP) and !defined(BUILTIN_FS_BROKER))
#include "swcdb/fs/Broker/Protocol/params/Exists.cc"
#endif


#endif // swcdb_fs_Broker_Protocol_params_Exists_h
