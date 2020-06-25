/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#ifndef swc_core_Serializable_h
#define swc_core_Serializable_h

#include "swcdb/core/Compat.h"

namespace SWC {


class Serializable {

  public:

  ~Serializable() { }

  virtual size_t encoded_length() const;

  virtual void encode(uint8_t** bufp) const;

  virtual void decode(const uint8_t** bufp, size_t* remainp);


  protected:

  virtual size_t internal_encoded_length() const = 0;

  virtual void internal_encode(uint8_t** bufp) const = 0;

  virtual void internal_decode(const uint8_t** bufp, size_t* remainp) = 0;


};


}



#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/Serializable.cc"
#endif 


#endif
