/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/core/comm/Buffers.h"
#include "swcdb/core/Checksum.h"

namespace SWC { namespace Comm {

namespace {
static const uint16_t BUFFER_CHUNK_SZ = 4096;
}



void Buffers::set_data(uint32_t sz) {
  if(buf_data.size)
    header.data.reset();

  buf_data.reallocate(sz);
  data_ptr = buf_data.base;
}

void Buffers::set_data(const Serializable& params, uint32_t reserve) {
  set_data(reserve + params.encoded_length());

  data_ptr = buf_data.base + reserve;
  params.encode(&data_ptr);
  data_ptr = buf_data.base;
}

std::vector<asio::const_buffer> Buffers::get_buffers() {
  uint8_t buf_header_len = header.encoded_length();
  {
    uint8_t* buf = buf_header;
    header.encode(&buf); // write_header
  }

  size_t nchunks = 1;

  size_t buf_data_chunks = 0;
  bool buf_data_not_aligned = false;
  if(buf_data.size)
    nchunks += (buf_data_chunks += buf_data.size / BUFFER_CHUNK_SZ)
            + (buf_data_not_aligned = buf_data.size % BUFFER_CHUNK_SZ);

  size_t buf_ext_chunks = 0;
  bool buf_ext_not_aligned = false;
  if(buf_ext.size)
    nchunks += (buf_ext_chunks += buf_ext.size / BUFFER_CHUNK_SZ)
            + (buf_ext_not_aligned = buf_ext.size % BUFFER_CHUNK_SZ);

  std::vector<asio::const_buffer> buffers(nchunks);
  auto it = buffers.begin();
  *it = asio::const_buffer(buf_header, buf_header_len);

  if(buf_data.size) {
    auto p = buf_data.base;
    for(size_t i = 0; i<buf_data_chunks; ++i, p+=BUFFER_CHUNK_SZ)
      *++it = asio::const_buffer(p, BUFFER_CHUNK_SZ);
    if(buf_data_not_aligned)
      *++it = asio::const_buffer(p, (buf_data.base + buf_data.size) - p);
  }
  if(buf_ext.size) {
    auto p = buf_ext.base;
    for(size_t i = 0; i<buf_ext_chunks; ++i, p+=BUFFER_CHUNK_SZ)
      *++it = asio::const_buffer(p, BUFFER_CHUNK_SZ);
    if(buf_ext_not_aligned)
      *++it = asio::const_buffer(p, (buf_ext.base + buf_ext.size) - p);
  }
  return buffers;
}


}} // namespace SWC::Comm
