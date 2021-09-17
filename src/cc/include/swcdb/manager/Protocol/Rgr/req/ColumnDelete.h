/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_manager_Protocol_rgr_req_ColumnDelete_h
#define swcdb_manager_Protocol_rgr_req_ColumnDelete_h


namespace SWC { namespace Comm { namespace Protocol {
namespace Rgr { namespace Req {


class ColumnDelete : public client::ConnQueue::ReqBase  {
  public:
  typedef std::shared_ptr<ColumnDelete> Ptr;

  ColumnDelete(const Manager::Ranger::Ptr& rgr,
               const DB::Schema::Ptr& schema,
               uint64_t req_id);

  virtual ~ColumnDelete() noexcept { }

  void handle(ConnHandlerPtr conn, const Event::Ptr& ev) override;

  void handle_no_conn() override;

  void remove();

  private:

  Manager::Ranger::Ptr  rgr;
  DB::Schema::Ptr       schema;
  uint64_t              req_id;
};

}}}}}

#endif // swcdb_manager_Protocol_rgr_req_ColumnDelete_h
