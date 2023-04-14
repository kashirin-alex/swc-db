/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_manager_Protocol_rgr_req_RangeLoad_h
#define swcdb_manager_Protocol_rgr_req_RangeLoad_h

namespace SWC { namespace Comm { namespace Protocol {
namespace Rgr { namespace Req {


class RangeLoad : public client::ConnQueue::ReqBase {
  public:
  typedef std::shared_ptr<RangeLoad> Ptr;

  RangeLoad(const Manager::Ranger::Ptr& rgr,
            const Manager::Column::Ptr& col,
            const Manager::Range::Ptr& range,
            const DB::Schema::Ptr& schema);

  RangeLoad(RangeLoad&&) = delete;
  RangeLoad(const RangeLoad&) = delete;
  RangeLoad& operator=(RangeLoad&&) = delete;
  RangeLoad& operator=(const RangeLoad&) = delete;

  virtual ~RangeLoad() noexcept { }

  void handle(ConnHandlerPtr conn, const Event::Ptr& ev) override;

  bool valid() override;

  void handle_no_conn() override;

  void loaded(int err, bool failure,
              const DB::Cells::Interval& intval, int64_t revision);

  private:

  Manager::Ranger::Ptr rgr;
  Manager::Column::Ptr col;
  Manager::Range::Ptr  range;
  int64_t              schema_revision;

};

}}}}}

#endif // swcdb_manager_Protocol_rgr_req_RangeLoad_h
