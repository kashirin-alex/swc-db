/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_manager_Protocol_rgr_req_RangeUnload_h
#define swcdb_manager_Protocol_rgr_req_RangeUnload_h



namespace SWC { namespace Comm { namespace Protocol {
namespace Rgr { namespace Req {


class RangeUnload : public client::ConnQueue::ReqBase {
  public:
  typedef std::shared_ptr<RangeUnload> Ptr;

  RangeUnload(const Manager::Ranger::Ptr& rgr,
              const Manager::Column::Ptr& col,
              const Manager::Range::Ptr& range,
              bool ignore_error=false,
              uint32_t timeout=60000);

  virtual ~RangeUnload() noexcept { }

  bool valid() override;

  void handle(ConnHandlerPtr conn, const Event::Ptr& ev) override;

  void handle_no_conn() override;

  void unloaded(int err);

  private:

  const Manager::Ranger::Ptr rgr;
  const Manager::Column::Ptr col;
  const Manager::Range::Ptr  range;
  bool                       ignore_error;

};

}}}}}

#endif // swcdb_manager_Protocol_rgr_req_RangeUnload_h
