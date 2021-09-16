/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_ranger_queries_update_CommonMeta_h
#define swcdb_ranger_queries_update_CommonMeta_h


namespace SWC { namespace Ranger { namespace Query { namespace Update {


class CommonMeta : public BaseMeta {
  public:
  typedef std::shared_ptr<CommonMeta>     Ptr;
  typedef std::function<void(const Ptr&)> Cb_t;

  SWC_CAN_INLINE
  static Ptr make(const RangePtr& range, Cb_t&& cb) {
    return Ptr(new CommonMeta(range, std::move(cb)));
  }

  const Cb_t cb;

  SWC_CAN_INLINE
  CommonMeta(const RangePtr& a_range, Cb_t&& a_cb)
            : BaseMeta(a_range), cb(std::move(a_cb)) {
  }

  virtual ~CommonMeta() { }

  virtual void response(int err=Error::OK) override {
    struct Task {
      Ptr hdlr;
      SWC_CAN_INLINE
      Task(Ptr&& a_hdlr) noexcept : hdlr(std::move(a_hdlr)) { }
      void operator()() { hdlr->cb(hdlr); }
    };
    if(is_last_rsp(err)) {
      Env::Rgr::post(
        Task(std::dynamic_pointer_cast<CommonMeta>(shared_from_this())));
    }
  }

  virtual void callback() override {
    cb(std::dynamic_pointer_cast<CommonMeta>(shared_from_this()));
  }

};


}}}}


#endif // swcdb_ranger_queries_update_CommonMeta_h
