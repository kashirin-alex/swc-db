/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/core/Error.h"
#include "swcdb/core/BufferStream.h"


namespace SWC { namespace Core {


// STREAM-OUT

BufferStreamOut::BufferStreamOut(size_t pre_alloc, size_t commit_size)
                : error(Error::OK),
                  pre_alloc(pre_alloc), commit_size(commit_size) {
}

bool BufferStreamOut::empty() const {
  return !buffer.fill();
}

bool BufferStreamOut::full() const {
  return buffer.fill() >= commit_size;
}

size_t BufferStreamOut::available() {
  return buffer.fill();
}

void BufferStreamOut::add(const uint8_t* ptr, size_t len) {
  buffer.add(ptr, len);
}

void BufferStreamOut::get(StaticBuffer& output) {
  output.set(buffer);
  buffer.ensure(pre_alloc);
}



BufferStreamOut_ZSTD_OnAdd::BufferStreamOut_ZSTD_OnAdd(
                      int level, size_t pre_alloc, size_t commit_size)
                      : BufferStreamOut(pre_alloc, commit_size),
                        cstream(ZSTD_createCStream()),
                        has_data(false), plain_size(0) {
  if(ZSTD_isError(
      ZSTD_initCStream(cstream, level ? level : ZSTD_CLEVEL_DEFAULT)))
    error = Error::ENCODER_ENCODE;
  else if(ZSTD_isError(
      ZSTD_CCtx_setParameter(cstream, ZSTD_c_checksumFlag, 1)))
    error = Error::ENCODER_ENCODE;
}

BufferStreamOut_ZSTD_OnAdd::~BufferStreamOut_ZSTD_OnAdd() {
  ZSTD_freeCStream(cstream);
}

bool BufferStreamOut_ZSTD_OnAdd::empty() const {
  return !has_data && BufferStreamOut::empty();
}

bool BufferStreamOut_ZSTD_OnAdd::full() const {
  return plain_size >= BufferStreamOut::commit_size;
}

size_t BufferStreamOut_ZSTD_OnAdd::available() {
  return BufferStreamOut::available() + (has_data ? tmp_buff.size : 0);
}

void BufferStreamOut_ZSTD_OnAdd::add(const uint8_t* ptr, size_t len) {
  has_data = true;
  plain_size += len;
  tmp_buff.ensure(ZSTD_compressBound(len));
  out_buff.dst = tmp_buff.base;
  out_buff.size = tmp_buff.size;
  out_buff.pos = 0;
  ZSTD_inBuffer inBuff = { ptr, len, 0 };
  if(ZSTD_isError(ZSTD_compressStream(cstream, &out_buff, &inBuff))) {
    error = Error::ENCODER_ENCODE;

  } else if (out_buff.pos) {
    BufferStreamOut::add(tmp_buff.base, out_buff.pos);
    size_t remain;
    out_buff.pos = 0;
    do {
      remain = ZSTD_flushStream(cstream, &out_buff);
      if(ZSTD_isError(remain)) {
        error = Error::ENCODER_ENCODE;
        return;
      }
      if(out_buff.pos) {
        BufferStreamOut::add(tmp_buff.base, out_buff.pos);
        out_buff.pos = 0;
      }
    } while (remain);
  }
}

void BufferStreamOut_ZSTD_OnAdd::get(StaticBuffer& output) {
  tmp_buff.ensure(ZSTD_compressBound(128));
  out_buff.dst = tmp_buff.base;
  out_buff.size = tmp_buff.size;
  out_buff.pos = 0;
  size_t remain;
  do {
    remain = ZSTD_endStream(cstream, &out_buff);
    if(ZSTD_isError(remain)) {
      error = Error::ENCODER_ENCODE;
      return;
    }
    if(out_buff.pos) {
      BufferStreamOut::add(tmp_buff.base, out_buff.pos);
      out_buff.pos = 0;
    }
  } while (remain);

  BufferStreamOut::get(output);
  plain_size = 0;
  has_data = false;
}



BufferStreamOut_ZSTD::BufferStreamOut_ZSTD(
                    int level, size_t pre_alloc, size_t commit_size)
                    : BufferStreamOut(pre_alloc, commit_size),
                      cstream(ZSTD_createCStream()) {
  if(ZSTD_isError(
      ZSTD_initCStream(cstream, level ? level : ZSTD_CLEVEL_DEFAULT)))
    error = Error::ENCODER_ENCODE;
  else if(ZSTD_isError(
      ZSTD_CCtx_setParameter(cstream, ZSTD_c_checksumFlag, 1)))
    error = Error::ENCODER_ENCODE;
  else if(ZSTD_isError(
      ZSTD_CCtx_setParameter(cstream, ZSTD_c_contentSizeFlag, 1)))
    error = Error::ENCODER_ENCODE;
}

BufferStreamOut_ZSTD::~BufferStreamOut_ZSTD() {
  ZSTD_freeCStream(cstream);
}

void BufferStreamOut_ZSTD::get(StaticBuffer& output) {
  ZSTD_inBuffer input = {buffer.base, buffer.fill(), 0};

  DynamicBuffer tmp_buff(ZSTD_compressBound(input.size));
  ZSTD_outBuffer out_buff = {tmp_buff.base, tmp_buff.size, 0};

  size_t remain = ZSTD_compressStream2(
    cstream, &out_buff, &input, ZSTD_e_end);
  if(remain || ZSTD_isError(remain)) {
    error = Error::ENCODER_ENCODE;
    return;
  }
  tmp_buff.ptr += out_buff.pos;
  buffer.clear();
  output.set(tmp_buff);
}



BufferStreamOut_ENCODER::BufferStreamOut_ENCODER(
                Encoder::Type encoder, size_t pre_alloc, size_t commit_size)
                : BufferStreamOut(pre_alloc, commit_size),
                  encoder(encoder) {
}

void BufferStreamOut_ENCODER::get(StaticBuffer& output) {
  DynamicBuffer tmp_buff;
  size_t sz_enc;
  Encoder::encode(error, encoder,
                  buffer.base, buffer.fill(),
                  &sz_enc, tmp_buff,
                  0, true, true);
  if(!sz_enc) {
    error = Error::ENCODER_ENCODE;
  } else {
    buffer.clear();
    output.set(tmp_buff);
  }
}






// STREAM-IN

BufferStreamIn::BufferStreamIn() : error(Error::OK) { }

bool BufferStreamIn::empty() const {
  return !buffer.fill();
  }

void BufferStreamIn::add(StaticBuffer& inbuffer) {
  if(buffer.fill()) {
    buffer.add(inbuffer.base, inbuffer.size);
    inbuffer.free();
  } else {
    buffer.take_ownership(inbuffer);
  }
}

void BufferStreamIn::put_back(const uint8_t* ptr, size_t len) {
  buffer.add(ptr, len);
}

bool BufferStreamIn::get(StaticBuffer& output) {
  if(buffer.fill()) {
    output.set(buffer);
    return true;
  }
  return false;
}



BufferStreamIn_ZSTD::BufferStreamIn_ZSTD()
              : dstream(ZSTD_createDStream()),
                offset(0), frame_complete(true) {
  if(ZSTD_isError(ZSTD_initDStream(dstream)))
    error = Error::ENCODER_ENCODE;
}

BufferStreamIn_ZSTD::~BufferStreamIn_ZSTD() {
  ZSTD_freeDStream(dstream);
}

bool BufferStreamIn_ZSTD::empty() const {
  return !buffer.fill() && !buffer_enc.fill();
}

void BufferStreamIn_ZSTD::add(StaticBuffer& inbuffer) {
  if(buffer_enc.fill()) {
    if(offset) {
      StaticBuffer tmp(
        buffer_enc.base + offset, buffer_enc.fill() - offset, true);
      buffer_enc.clear();
      buffer_enc.add(tmp.base, tmp.size);
    }
    buffer_enc.add(inbuffer.base, inbuffer.size);
    inbuffer.free();
  } else {
    buffer_enc.take_ownership(inbuffer);
  }
}

bool BufferStreamIn_ZSTD::get(StaticBuffer& output) {
  size_t fill = buffer_enc.fill();
  if(fill <= offset)
    return false;
  size_t remain;
  if(frame_complete) {
    remain = ZSTD_getFrameContentSize(buffer_enc.base+offset, fill-offset);
    if(remain == ZSTD_CONTENTSIZE_ERROR || remain == ZSTD_CONTENTSIZE_UNKNOWN)
      remain = 262144;
  } else {
    remain = 262144;
  }
  buffer.ensure(remain);
  tmp_buff.ensure(remain);
  ZSTD_outBuffer out_buff = {tmp_buff.base, tmp_buff.size, 0};
  ZSTD_inBuffer in_buff = {buffer_enc.base, fill, offset};
  frame_complete = false;
  do {
    remain = ZSTD_decompressStream(dstream, &out_buff, &in_buff);
    if(!out_buff.pos || ZSTD_isError(remain))
      break;
    buffer.add(tmp_buff.base, out_buff.pos);
    out_buff.pos = 0;
  } while(remain);

  if(!remain)
    frame_complete = true;

  if(in_buff.size > in_buff.pos) {
    offset = in_buff.pos;
  } else {
    buffer_enc.clear();
    offset = 0;
  }
  if(buffer.fill()) {
    output.set(buffer);
    return true;
  }
  return false;
}


}} // namespace SWC::Core
