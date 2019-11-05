/* -*- c++ -*-
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

/// @file
/// Declarations for Write parameters.

#ifndef swc_lib_fs_Broker_Protocol_params_Write_h
#define swc_lib_fs_Broker_Protocol_params_Write_h

#include "swcdb/lib/core/Serializable.h"


namespace SWC { namespace FS { namespace Protocol { namespace Params {


class WriteReq : public Serializable {
  public:

  WriteReq() {}

  WriteReq(const std::string &fname, uint32_t flags,
	          int32_t replication, int64_t blksz)
            : m_fname(fname), m_flags(flags),
	            m_replication(replication), m_blksz(blksz) {
  }

  const std::string get_name() { return m_fname; }

  uint32_t get_flags() { return m_flags; }

  int32_t get_replication() { return m_replication; }

  int64_t get_block_size() { return m_blksz; }

  private:

  uint8_t encoding_version() const {
    return 1; 
  }

  size_t encoded_length_internal() const override {
    return 16 + Serialization::encoded_length_vstr(m_fname);
  }

  void encode_internal(uint8_t **bufp) const override {
    Serialization::encode_i32(bufp, m_flags);
    Serialization::encode_i32(bufp, m_replication);
    Serialization::encode_i64(bufp, m_blksz);
    Serialization::encode_vstr(bufp, m_fname);
  }

  void decode_internal(uint8_t version, const uint8_t **bufp,
			     size_t *remainp) override {
    (void)version;
    m_flags = Serialization::decode_i32(bufp, remainp);
    m_replication = (int32_t)Serialization::decode_i32(bufp, remainp);
    m_blksz = (int64_t)Serialization::decode_i64(bufp, remainp);
    m_fname.clear();
    m_fname.append(Serialization::decode_vstr(bufp, remainp));
  }

  /// File name
  std::string m_fname;

  /// Write flags
  uint32_t m_flags;

  /// Replication
  int32_t m_replication;

  /// Block size
  int64_t m_blksz;
};

}}}}

#endif // swc_lib_fs_Broker_Protocol_params_Write_h
