/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_core_comm_Serializable_h
#define swcdb_core_comm_Serializable_h


#include "swcdb/core/Exception.h"
#include "swcdb/core/Serialization.h"


namespace SWC { namespace Comm {


class Serializable {

  public:

  size_t encoded_length() const;

  void encode(uint8_t** bufp) const;

  void decode(const uint8_t** bufp, size_t* remainp);


  protected:

  virtual ~Serializable() { };

  virtual size_t internal_encoded_length() const = 0;

  virtual void internal_encode(uint8_t** bufp) const = 0;

  virtual void internal_decode(const uint8_t** bufp, size_t* remainp) = 0;


};


SWC_CAN_INLINE
extern
size_t Serializable::encoded_length() const {
  size_t length = internal_encoded_length();
  return Serialization::encoded_length_vi32(length) + length;
}

SWC_CAN_INLINE
extern
void Serializable::encode(uint8_t** bufp) const {
  Serialization::encode_vi32(bufp, internal_encoded_length());
  internal_encode(bufp);
}

SWC_CAN_INLINE
extern
void Serializable::decode(const uint8_t** bufp, size_t* remainp) {
  size_t len = Serialization::decode_vi32(bufp, remainp);
  if(len > *remainp)
    SWC_THROWF(Error::PROTOCOL_ERROR,
               "Buffer-Trunclated remain=%lu len=%lu", *remainp, len);
  *remainp -= len;

  const uint8_t* end = *bufp + len;
  internal_decode(bufp, &len);

  if(len)
    SWC_THROWF(Error::PROTOCOL_ERROR,
                "Bad Decode missing=%lu in buffer", len);
  if(*bufp > end)
    SWC_THROWF(Error::PROTOCOL_ERROR,
               "Bad Decode buffer overrun by=%ld", *bufp - end);
}


}} // namespace SWC::Comm



#endif // swcdb_core_comm_Serializable_h
