/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_Protocol_params_Create_h
#define swcdb_fs_Broker_Protocol_params_Create_h


#include "swcdb/core/comm/Serializable.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Params {


class CreateReq final : public Serializable {
  public:

  SWC_CAN_INLINE
  CreateReq() noexcept { }

  SWC_CAN_INLINE
  CreateReq(const std::string& a_fname, uint32_t a_flags, int32_t a_bufsz,
            uint8_t a_replication, int64_t a_blksz)
            : fname(a_fname), flags(a_flags), bufsz(a_bufsz),
              replication(a_replication), blksz(a_blksz) { }

  ~CreateReq() noexcept { }

  std::string fname;
  uint32_t    flags;
  int32_t     bufsz;
  uint8_t     replication;
  int64_t     blksz;

  private:

  size_t internal_encoded_length() const override;

  void internal_encode(uint8_t** bufp) const override;

  void internal_decode(const uint8_t** bufp, size_t* remainp) override;

};

}}}}}


#if defined(SWC_IMPL_SOURCE) or \
    (defined(FS_BROKER_APP) and !defined(BUILTIN_FS_BROKER))
#include "swcdb/fs/Broker/Protocol/params/Create.cc"
#endif


#endif // swcdb_fs_Broker_Protocol_params_Create_h
