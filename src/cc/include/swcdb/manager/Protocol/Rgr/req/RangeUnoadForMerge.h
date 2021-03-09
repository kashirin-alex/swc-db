/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_manager_Protocol_rgr_req_RangeUnoadForMerge_h
#define swcdb_manager_Protocol_rgr_req_RangeUnoadForMerge_h



namespace SWC { namespace Comm { namespace Protocol {
namespace Rgr { namespace Req {


class RangeUnoadForMerge : public client::ConnQueue::ReqBase {
  public:

  RangeUnoadForMerge(
    const Manager::Ranger::Ptr& rgr,
    const Manager::ColumnHealthCheck::ColumnMerger::RangesMerger::Ptr& merger,
    const Manager::Range::Ptr& range,
    uint32_t timeout=60000);

  virtual ~RangeUnoadForMerge();

  bool valid() override;

  void handle(ConnHandlerPtr conn, const Event::Ptr& ev) override;

  void handle_no_conn() override;

  private:

  const Manager::Ranger::Ptr                                        rgr;
  const Manager::ColumnHealthCheck::ColumnMerger::RangesMerger::Ptr merger;
  const Manager::Range::Ptr                                         range;

};

}}}}}

#endif // swcdb_manager_Protocol_rgr_req_RangeUnload_h
