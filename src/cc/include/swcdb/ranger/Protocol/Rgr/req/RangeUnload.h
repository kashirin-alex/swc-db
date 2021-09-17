/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_ranger_protocol_rgr_req_RangeUnload_h
#define swcdb_ranger_protocol_rgr_req_RangeUnload_h



namespace SWC { namespace Comm { namespace Protocol {
namespace Rgr { namespace Req {


class RangeUnload : public client::ConnQueue::ReqBase {
  public:
  typedef std::shared_ptr<RangeUnload>   Ptr;

  RangeUnload(const Ranger::RangePtr& range,
              const Ranger::Callback::RangeLoad::Ptr& req,
              uint32_t timeout=60000);

  virtual ~RangeUnload() noexcept { }

  void handle(ConnHandlerPtr conn, const Event::Ptr& ev) override;

  bool valid() override;

  void handle_no_conn() override;

  void unloaded(int err);

  private:

  Ranger::Callback::RangeLoad::Ptr  req;
  Ranger::RangePtr                  range;

};

}}}}}

#endif // swcdb_ranger_protocol_rgr_req_RangeUnload_h
