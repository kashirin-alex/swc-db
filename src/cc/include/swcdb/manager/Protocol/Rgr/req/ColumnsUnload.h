/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_manager_Protocol_rgr_req_ColumnsUnload_h
#define swcdb_manager_Protocol_rgr_req_ColumnsUnload_h


namespace SWC { namespace Comm { namespace Protocol {
namespace Rgr { namespace Req {


class ColumnsUnload : public client::ConnQueue::ReqBase  {
  public:
  typedef std::shared_ptr<ColumnsUnload> Ptr;

  ColumnsUnload(const Manager::Ranger::Ptr& rgr,
                cid_t cid_begin, cid_t cid_end);

  virtual ~ColumnsUnload() noexcept { }

  void handle(ConnHandlerPtr conn, const Event::Ptr& ev) override;

  void handle_no_conn() override;

  private:

  const Manager::Ranger::Ptr  rgr;

};

}}}}}

#endif // swcdb_manager_Protocol_rgr_req_ColumnsUnload_h
