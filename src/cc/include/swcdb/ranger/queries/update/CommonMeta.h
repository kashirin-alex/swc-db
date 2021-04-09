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

  static Ptr make(const RangePtr& range, Cb_t cb) {
    return std::make_shared<CommonMeta>(range, std::move(cb));
  }

  const Cb_t cb;

  CommonMeta(const RangePtr& range, Cb_t&& cb)
            : BaseMeta(range), cb(std::move(cb)) {
  }

  virtual ~CommonMeta() { }

  virtual void response(int err=Error::OK) override {
    if(is_last_rsp(err)) {
      Env::Rgr::post(
        [hdlr=std::dynamic_pointer_cast<CommonMeta>(shared_from_this())](){
          hdlr->cb(hdlr);
      });
    }
  }

  virtual void callback() override {
    cb(std::dynamic_pointer_cast<CommonMeta>(shared_from_this()));
  }

};


}}}}


#endif // swcdb_ranger_queries_update_CommonMeta_h
