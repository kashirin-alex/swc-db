/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_ranger_callbacks_ColumnsUnloadAll_h
#define swcdb_ranger_callbacks_ColumnsUnloadAll_h


namespace SWC { namespace Ranger { namespace Callback {


class ColumnsUnloadAll : public ColumnsUnload {
  public:

  typedef std::shared_ptr<ColumnsUnloadAll> Ptr;

  bool                validation;
  std::promise<void>  promise;

  ColumnsUnloadAll(bool validation);

  virtual ~ColumnsUnloadAll() { }

  void unloaded(RangePtr range) override;

  void unloaded(const ColumnPtr& col) override;

  void complete() override {
    promise.set_value();
  }

  SWC_CAN_INLINE
  void wait() {
    promise.get_future().wait();
  }

};


}
}}
#endif // swcdb_ranger_callbacks_ColumnsUnloadAll_h
