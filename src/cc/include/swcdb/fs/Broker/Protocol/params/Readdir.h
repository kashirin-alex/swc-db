/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_fs_Broker_Protocol_params_Readdir_h
#define swc_fs_Broker_Protocol_params_Readdir_h


#include "swcdb/core/Serializable.h"
#include "swcdb/fs/Dirent.h"


namespace SWC { namespace FS { namespace Protocol { namespace Params {


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
  
  ReaddirRsp();

  ReaddirRsp(DirentList &listing);

  void get_listing(DirentList &listing);

  private:

  size_t internal_encoded_length() const;

  void internal_encode(uint8_t** bufp) const;

  void internal_decode(const uint8_t** bufp, size_t* remainp);
  
  DirentList m_listing;
};

}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/fs/Broker/Protocol/params/Readdir.cc"
#endif 


#endif // swc_fs_Broker_Protocol_params_Readdir_h
