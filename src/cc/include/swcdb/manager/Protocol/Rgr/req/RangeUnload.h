
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */ 

#ifndef swc_manager_Protocol_rgr_req_RangeUnload_h
#define swc_manager_Protocol_rgr_req_RangeUnload_h



namespace SWC { namespace Protocol { namespace Rgr { namespace Req {


class RangeUnload : public Comm::client::ConnQueue::ReqBase {
  public:

  RangeUnload(const Manager::Ranger::Ptr& rgr,
              const Manager::Column::Ptr& col, 
              const Manager::Range::Ptr& range,
              uint32_t timeout=60000);

  virtual ~RangeUnload();

  bool valid() override;

  void handle(Comm::ConnHandlerPtr conn, const Comm::Event::Ptr& ev) override;

  void handle_no_conn() override;

  void unloaded(int err);

  private:
  
  const Manager::Ranger::Ptr rgr;
  const Manager::Column::Ptr col;
  const Manager::Range::Ptr  range;
   
};

}}}}

#endif // swc_manager_Protocol_rgr_req_RangeUnload_h
