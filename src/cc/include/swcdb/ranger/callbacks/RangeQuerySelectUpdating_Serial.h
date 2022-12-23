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
                                    u_fields(
                                      spec.updating->value,
                                      spec.updating->vlen,
                                      false
                                    ) {
    only_keys = false;
    opfields_found.reserve(u_fields.count);
    opfields_missing.reserve(u_fields.count);
  }

  virtual ~RangeQuerySelectUpdating_Serial() noexcept { }

  void update_cell_value(DB::Cells::Cell& cell) override {
    StaticBuffer v;
    DB::Types::Encoder encoder = cell.get_value(v, false);
    const uint8_t* ptr = v.base;
    size_t remain = v.size;

    DB::Cell::Serial::Value::FieldsWriter wfields;
    wfields.ensure(remain + spec.updating->vlen);

    while(remain) {
      switch(DB::Cell::Serial::Value::read_type(&ptr, &remain)) {
        case DB::Cell::Serial::Value::Type::INT64: {
          DB::Cell::Serial::Value::Field_INT64 field(&ptr, &remain);
          auto opfield = u_fields.find_matching_type_and_id(&field);
          if(opfield) {
            opfields_found.push_back(opfield);
            if(opfield->ufield->is_delete_field())
              break;
            reinterpret_cast<DB::Cell::Serial::Value::FieldUpdate_MATH*>
              (opfield->ufield)->apply(opfield->field, field);
          }
          wfields.add(&field);
          break;
        }
        case DB::Cell::Serial::Value::Type::DOUBLE: {
          DB::Cell::Serial::Value::Field_DOUBLE field(&ptr, &remain);
          auto opfield = u_fields.find_matching_type_and_id(&field);
          if(opfield) {
            opfields_found.push_back(opfield);
            if(opfield->ufield->is_delete_field())
              break;
            reinterpret_cast<DB::Cell::Serial::Value::FieldUpdate_MATH*>
              (opfield->ufield)->apply(opfield->field, field);
          }
          wfields.add(&field);
          break;
        }
        case DB::Cell::Serial::Value::Type::BYTES: {
          DB::Cell::Serial::Value::Field_BYTES field(&ptr, &remain);
          auto opfield = u_fields.find_matching_type_and_id(&field);
          if(opfield) {
            opfields_found.push_back(opfield);
            if(opfield->ufield->is_delete_field())
              break;
            reinterpret_cast<DB::Cell::Serial::Value::FieldUpdate_LIST*>
              (opfield->ufield)->apply(opfield->field, field);
          }
          wfields.add(&field);
          break;
        }
        case DB::Cell::Serial::Value::Type::KEY: {
          DB::Cell::Serial::Value::Field_KEY field(&ptr, &remain);
          auto opfield = u_fields.find_matching_type_and_id(&field);
          if(opfield) {
            opfields_found.push_back(opfield);
            if(opfield->ufield->is_delete_field())
              break;

          }
          wfields.add(&field);
          break;
        }
        case DB::Cell::Serial::Value::Type::LIST_INT64: {
          DB::Cell::Serial::Value::Field_LIST_INT64 field(&ptr, &remain);
          auto opfield = u_fields.find_matching_type_and_id(&field);
          if(opfield) {
            opfields_found.push_back(opfield);
            if(opfield->ufield->is_delete_field())
              break;
            reinterpret_cast<DB::Cell::Serial::Value::FieldUpdate_LIST_INT64*>
              (opfield->ufield)->apply(opfield->field, field);
          }
          wfields.add(&field);
          break;
        }
        case DB::Cell::Serial::Value::Type::LIST_BYTES: {
          DB::Cell::Serial::Value::Field_LIST_BYTES field(&ptr, &remain);
          auto opfield = u_fields.find_matching_type_and_id(&field);
          if(opfield) {
            opfields_found.push_back(opfield);
            if(opfield->ufield->is_delete_field())
              break;
            reinterpret_cast<DB::Cell::Serial::Value::FieldUpdate_LIST_BYTES*>
              (opfield->ufield)->apply(opfield->field, field);
          }
          wfields.add(&field);
          break;
        }
        default:
          continue;
      }
    }

    u_fields.get_not_in(opfields_found, opfields_missing);
    for(auto opfield : opfields_missing) {
      if(!opfield->ufield->is_no_add_field() &&
         !opfield->ufield->is_delete_field())
        wfields.add(opfield->field);
    }

    encoder == DB::Types::Encoder::DEFAULT
      ? cell.set_value(wfields.base, wfields.fill(), true)
      : cell.set_value(encoder, wfields.base, wfields.fill());

    opfields_found.clear();
    opfields_missing.clear();
  }

  private:
  const DB::Cell::Serial::Value::FieldsUpdaterMap u_fields;
  DB::Cell::Serial::Value::FieldUpdateOpPtrs      opfields_found;
  DB::Cell::Serial::Value::FieldUpdateOpPtrs      opfields_missing;

};


}}}
#endif // swcdb_ranger_callbacks_RangeQuerySelectUpdating_Serial_h
