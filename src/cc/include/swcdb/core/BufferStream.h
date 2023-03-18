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
  BufferStreamOut(size_t a_pre_alloc = 12582912,
                  size_t a_commit_size = 8388608) noexcept
                  : error(Error::OK),
                    pre_alloc(a_pre_alloc),
                    commit_size(a_commit_size),
                    buffer() {
  }

  BufferStreamOut(BufferStreamOut&&) = delete;

  BufferStreamOut(const BufferStreamOut&) = delete;

  BufferStreamOut& operator=(const BufferStreamOut&) = delete;

  BufferStreamOut& operator=(BufferStreamOut&&) = delete;

  virtual ~BufferStreamOut() noexcept { }

  virtual bool SWC_PURE_FUNC empty() const;

  virtual bool SWC_PURE_FUNC full() const;

  virtual size_t SWC_PURE_FUNC available();

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

  BufferStreamOut_ZSTD_OnAdd(BufferStreamOut_ZSTD_OnAdd&&) = delete;

  BufferStreamOut_ZSTD_OnAdd(const BufferStreamOut_ZSTD_OnAdd&) = delete;

  BufferStreamOut_ZSTD_OnAdd& operator=(const BufferStreamOut_ZSTD_OnAdd&) = delete;

  BufferStreamOut_ZSTD_OnAdd& operator=(BufferStreamOut_ZSTD_OnAdd&&) = delete;

  virtual ~BufferStreamOut_ZSTD_OnAdd() noexcept;

  virtual bool SWC_PURE_FUNC empty() const override;

  virtual bool SWC_PURE_FUNC full() const override;

  virtual size_t SWC_PURE_FUNC available() override;

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

  BufferStreamOut_ZSTD(BufferStreamOut_ZSTD&&) = delete;

  BufferStreamOut_ZSTD(const BufferStreamOut_ZSTD&) = delete;

  BufferStreamOut_ZSTD& operator=(const BufferStreamOut_ZSTD&) = delete;

  BufferStreamOut_ZSTD& operator=(BufferStreamOut_ZSTD&&) = delete;

  virtual ~BufferStreamOut_ZSTD() noexcept;

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

  BufferStreamOut_ENCODER(BufferStreamOut_ENCODER&&) = delete;

  BufferStreamOut_ENCODER(const BufferStreamOut_ENCODER&) = delete;

  BufferStreamOut_ENCODER& operator=(const BufferStreamOut_ENCODER&) = delete;

  BufferStreamOut_ENCODER& operator=(BufferStreamOut_ENCODER&&) = delete;

  virtual ~BufferStreamOut_ENCODER() noexcept { }

  virtual void get(StaticBuffer& output) override;

};




// STREAM-IN

class BufferStreamIn {
  public:
  int  error;

  SWC_CAN_INLINE
  BufferStreamIn() noexcept : error(Error::OK), buffer() { }

  BufferStreamIn(BufferStreamIn&&) = delete;

  BufferStreamIn(const BufferStreamIn&) = delete;

  BufferStreamIn& operator=(const BufferStreamIn&) = delete;

  BufferStreamIn& operator=(BufferStreamIn&&) = delete;

  virtual ~BufferStreamIn() noexcept { }

  virtual bool SWC_PURE_FUNC empty() const;

  virtual void add(StaticBuffer& inbuffer);

  virtual void put_back(const uint8_t* ptr, size_t len);

  virtual bool get(StaticBuffer& output);

  protected:
  DynamicBuffer   buffer;

};



class BufferStreamIn_ZSTD : public BufferStreamIn {
  public:
  BufferStreamIn_ZSTD();

  BufferStreamIn_ZSTD(BufferStreamIn_ZSTD&&) = delete;

  BufferStreamIn_ZSTD(const BufferStreamIn_ZSTD&) = delete;

  BufferStreamIn_ZSTD& operator=(const BufferStreamIn_ZSTD&) = delete;

  BufferStreamIn_ZSTD& operator=(BufferStreamIn_ZSTD&&) = delete;

  virtual ~BufferStreamIn_ZSTD() noexcept;

  virtual bool SWC_PURE_FUNC empty() const override;

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
