/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_core_BufferStream_h
#define swcdb_core_BufferStream_h


#include "swcdb/core/Buffer.h"
#include "swcdb/core/Encoder.h"
#include <zstd.h>


namespace SWC { namespace Core {



// STREAM-OUT

class BufferStreamOut {
  public:
  int error;

  SWC_CAN_INLINE
  BufferStreamOut(size_t pre_alloc = 12582912, size_t commit_size = 8388608)
                  noexcept  : error(Error::OK),
                              pre_alloc(pre_alloc), commit_size(commit_size) {
  }

  virtual ~BufferStreamOut() { }

  virtual bool empty() const;

  virtual bool full() const;

  virtual size_t available();

  virtual void add(const uint8_t* ptr, size_t len);

  virtual void get(StaticBuffer& output);

  protected:
  size_t          pre_alloc;
  size_t          commit_size;
  DynamicBuffer   buffer;
};



class BufferStreamOut_ZSTD_OnAdd : public BufferStreamOut {
  public:
  BufferStreamOut_ZSTD_OnAdd(int level=0,
                             size_t pre_alloc = 12582912,
                             size_t commit_size = 8388608);

  virtual ~BufferStreamOut_ZSTD_OnAdd();

  virtual bool empty() const override;

  virtual bool full() const override;

  virtual size_t available() override;

  virtual void add(const uint8_t* ptr, size_t len) override;

  virtual void get(StaticBuffer& output) override;

  private:
  ZSTD_CStream* const cstream;
  bool                has_data;
  size_t              plain_size;
  DynamicBuffer       tmp_buff;
  ZSTD_outBuffer      out_buff;

};



class BufferStreamOut_ZSTD : public BufferStreamOut {
  public:
  BufferStreamOut_ZSTD(int level=0,
                       size_t pre_alloc = 12582912,
                       size_t commit_size = 8388608);

  virtual ~BufferStreamOut_ZSTD();

  virtual void get(StaticBuffer& output) override;

  private:
  ZSTD_CStream* const cstream;

};



class BufferStreamOut_ENCODER : public BufferStreamOut {
  public:
  const Encoder::Type encoder;
  BufferStreamOut_ENCODER(Encoder::Type encoder,
                          size_t pre_alloc = 12582912,
                          size_t commit_size = 8388608);

  virtual ~BufferStreamOut_ENCODER() { }

  virtual void get(StaticBuffer& output) override;

};




// STREAM-IN

class BufferStreamIn {
  public:
  int  error;

  SWC_CAN_INLINE
  BufferStreamIn() noexcept : error(Error::OK) { }

  virtual ~BufferStreamIn() {}

  virtual bool empty() const;

  virtual void add(StaticBuffer& inbuffer);

  virtual void put_back(const uint8_t* ptr, size_t len);

  virtual bool get(StaticBuffer& output);

  protected:
  DynamicBuffer   buffer;

};



class BufferStreamIn_ZSTD : public BufferStreamIn {
  public:
  BufferStreamIn_ZSTD();

  virtual ~BufferStreamIn_ZSTD();

  virtual bool empty() const override;

  virtual void add(StaticBuffer& inbuffer) override;

  virtual bool get(StaticBuffer& output) override;

  private:
  ZSTD_DStream* const dstream;
  DynamicBuffer       buffer_enc;
  size_t              offset;
  bool                frame_complete;
  DynamicBuffer       tmp_buff;

};



}} // namespace SWC::Core



#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/BufferStream.cc"
#endif


#endif // swcdb_core_BufferStream_h
