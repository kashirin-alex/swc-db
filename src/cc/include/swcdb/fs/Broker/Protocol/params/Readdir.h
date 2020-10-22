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


class ReaddirReq : public Serializable {
  public:

  ReaddirReq();

  ReaddirReq(const std::string& dirname);

  std::string dirname;

  private:

  size_t internal_encoded_length() const;

  void internal_encode(uint8_t** bufp) const;

  void internal_decode(const uint8_t** bufp, size_t* remainp);

};


class ReaddirRsp : public Serializable {
  public:

  FS::DirentList& listing;

  ReaddirRsp(FS::DirentList& listing)
            : listing(listing) {
  }

  private:

  size_t internal_encoded_length() const;

  void internal_encode(uint8_t** bufp) const;

  void internal_decode(const uint8_t** bufp, size_t* remainp);
  
};

}}}}}


#if defined(SWC_IMPL_SOURCE) or \
    (defined(FS_BROKER_APP) and !defined(BUILTIN_FS_BROKER))
#include "swcdb/fs/Broker/Protocol/params/Readdir.cc"
#endif 


#endif // swcdb_fs_Broker_Protocol_params_Readdir_h
