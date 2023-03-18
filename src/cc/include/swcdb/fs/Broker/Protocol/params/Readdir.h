/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Broker_Protocol_params_Readdir_h
#define swcdb_fs_Broker_Protocol_params_Readdir_h


#include "swcdb/core/comm/Serializable.h"
#include "swcdb/fs/Dirent.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Params {


class ReaddirReq final : public Serializable {
  public:

  SWC_CAN_INLINE
  ReaddirReq() noexcept : dirname() { }

  SWC_CAN_INLINE
  ReaddirReq(const std::string& a_dirname) : dirname(a_dirname) { }

  ~ReaddirReq() noexcept { }

  std::string dirname;

  private:

  size_t SWC_PURE_FUNC internal_encoded_length() const override;

  void internal_encode(uint8_t** bufp) const override;

  void internal_decode(const uint8_t** bufp, size_t* remainp) override;

};


class ReaddirRsp final : public Serializable {
  public:

  FS::DirentList listing;

  SWC_CAN_INLINE
  ReaddirRsp() noexcept: listing() { }

  ~ReaddirRsp() noexcept { }

  private:

  size_t SWC_PURE_FUNC internal_encoded_length() const override;

  void internal_encode(uint8_t** bufp) const override;

  void internal_decode(const uint8_t** bufp, size_t* remainp) override;

};

}}}}}


#if defined(SWC_IMPL_SOURCE) or \
    (defined(FS_BROKER_APP) and !defined(BUILTIN_FS_BROKER))
#include "swcdb/fs/Broker/Protocol/params/Readdir.cc"
#endif


#endif // swcdb_fs_Broker_Protocol_params_Readdir_h
