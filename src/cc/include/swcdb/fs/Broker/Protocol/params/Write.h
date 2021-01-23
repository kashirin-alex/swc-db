/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_Protocol_params_Write_h
#define swcdb_fs_Broker_Protocol_params_Write_h


#include "swcdb/core/comm/Serializable.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Params {


class WriteReq : public Serializable {
  public:

  WriteReq();

  WriteReq(const std::string& fname, uint32_t flags,
            uint8_t replication, int64_t blksz);

  std::string fname;
  uint32_t    flags;
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
#include "swcdb/fs/Broker/Protocol/params/Write.cc"
#endif


#endif // swcdb_fs_Broker_Protocol_params_Write_h
