/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_ranger_callbacks_RangeQuerySelectUpdating_Serial_h
#define swcdb_ranger_callbacks_RangeQuerySelectUpdating_Serial_h


#include "swcdb/ranger/callbacks/RangeQuerySelectUpdating.h"


namespace SWC { namespace Ranger { namespace Callback {


class RangeQuerySelectUpdating_Serial final
    : public RangeQuerySelectUpdating {
  public:

  typedef std::shared_ptr<RangeQuerySelectUpdating_Serial> Ptr;

  SWC_CAN_INLINE
  RangeQuerySelectUpdating_Serial(const Comm::ConnHandlerPtr& conn,
                                  const Comm::Event::Ptr& ev,
                                  DB::Specs::Interval&& req_spec,
                                  const RangePtr& a_range)
                                  : RangeQuerySelectUpdating(
                                      conn,
                                      ev,
                                      std::move(req_spec),
                                      a_range
                                    ),
                                    fields_for_update(
                                      spec.updating->value,
                                      spec.updating->vlen,
                                      false
                                    ) {
    only_keys = false;
  }

  virtual ~RangeQuerySelectUpdating_Serial() noexcept { }

  void update_cell_value(DB::Cells::Cell& cell) override {
    // TODO: (it is only rewrites and fill not-found)

    const uint8_t* ptr = cell.value;
    size_t remain = cell.vlen;

    DB::Cell::Serial::Value::FieldsWriter wfields;
    wfields.ensure(cell.vlen + spec.updating->vlen);

    Core::Vector<DB::Cell::Serial::Value::Field*, uint32_t> found_fields;
    found_fields.reserve(fields_for_update.count);

    while(remain) {
      switch(DB::Cell::Serial::Value::read_type(&ptr, &remain)) {
        case DB::Cell::Serial::Value::Type::INT64: {
          DB::Cell::Serial::Value::Field_INT64 field(&ptr, &remain);
          auto ufield = fields_for_update.find_matching_type_and_id(&field);
          if(ufield) {
            found_fields.push_back(ufield);

          }
          wfields.add(&field);
          break;
        }
        case DB::Cell::Serial::Value::Type::DOUBLE: {
          DB::Cell::Serial::Value::Field_DOUBLE field(&ptr, &remain);
          auto ufield = fields_for_update.find_matching_type_and_id(&field);
          if(ufield) {
            found_fields.push_back(ufield);

          }
          wfields.add(&field);
          break;
        }
        case DB::Cell::Serial::Value::Type::BYTES: {
          DB::Cell::Serial::Value::Field_BYTES field(&ptr, &remain);
          auto ufield = fields_for_update.find_matching_type_and_id(&field);
          if(ufield) {
            found_fields.push_back(ufield);

          }
          wfields.add(&field);
          break;
        }
        case DB::Cell::Serial::Value::Type::KEY: {
          DB::Cell::Serial::Value::Field_KEY field(&ptr, &remain);
          auto ufield = fields_for_update.find_matching_type_and_id(&field);
          if(ufield) {
            found_fields.push_back(ufield);

          }
          wfields.add(&field);
          break;
        }
        case DB::Cell::Serial::Value::Type::LIST_INT64: {
          DB::Cell::Serial::Value::Field_LIST_INT64 field(&ptr, &remain);
          auto ufield = fields_for_update.find_matching_type_and_id(&field);
          if(ufield) {
            found_fields.push_back(ufield);

          }
          wfields.add(&field);
          break;
        }
        case DB::Cell::Serial::Value::Type::LIST_BYTES: {
          DB::Cell::Serial::Value::Field_LIST_BYTES field(&ptr, &remain);
          auto ufield = fields_for_update.find_matching_type_and_id(&field);
          if(ufield) {
            found_fields.push_back(ufield);

          }
          wfields.add(&field);
          break;
        }
        default:
          continue;
      }
    }

    auto missing_fields(fields_for_update.get_not_in(found_fields));
    for(auto field : missing_fields) {
      wfields.add(field);
    }

    cell.set_value(wfields.base, wfields.fill(), true);
  }

  private:
  DB::Cell::Serial::Value::FieldsReaderMap fields_for_update;

};


}}}
#endif // swcdb_ranger_callbacks_RangeQuerySelectUpdating_Serial_h
