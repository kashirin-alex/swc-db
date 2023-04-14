/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_manager_Protocol_rgr_req_ColumnUpdate_h
#define swcdb_manager_Protocol_rgr_req_ColumnUpdate_h



namespace SWC { namespace Comm { namespace Protocol {
namespace Rgr { namespace Req {

class ColumnUpdate : public client::ConnQueue::ReqBase {
  public:
  typedef std::shared_ptr<ColumnUpdate> Ptr;

  ColumnUpdate(const Manager::Ranger::Ptr& rgr,
               const Manager::Column::Ptr& col,
               const DB::Schema::Ptr& schema,
               uint64_t req_id);

  ColumnUpdate(ColumnUpdate&&) = delete;
  ColumnUpdate(const ColumnUpdate&) = delete;
  ColumnUpdate& operator=(ColumnUpdate&&) = delete;
  ColumnUpdate& operator=(const ColumnUpdate&) = delete;

  virtual ~ColumnUpdate() noexcept { }

  void handle(ConnHandlerPtr conn, const Event::Ptr& ev) override;

  void handle_no_conn() override;

  void updated();

  private:
  Manager::Ranger::Ptr   rgr;
  Manager::Column::Ptr   col;
  DB::Schema::Ptr        schema;
  uint64_t               req_id;

};

}}}}}

#endif // swcdb_manager_Protocol_rgr_req_ColumnUpdate_h
