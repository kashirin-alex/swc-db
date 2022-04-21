/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Cells/CellValueSerialFieldUpdate.h"


namespace SWC { namespace DB { namespace Cell {
namespace Serial { namespace Value {



std::ostream& FieldUpdate::print(std::ostream& out) const {
  if(is_no_add_field())
    out << "CTRL_NO_ADD_FIELD ";
  if(is_delete_field())
    out << "CTRL_DELETE_FIELD ";
  return out;
}

std::ostream& FieldUpdate_MATH::print(std::ostream& out) const {
  FieldUpdate::print(out);
  switch(op) {
    case OP::EQUAL:
      return out << "EQUAL";
    case OP::PLUS:
      return out << "PLUS";
    case OP::MULTIPLY:
      return out << "MULTIPLY";
    case OP::DIVIDE:
      return out << "DIVIDE";
    default:
      return out << "UNKNOWN";
  }
}

std::ostream& FieldUpdate_LIST::print(std::ostream& out) const {
  FieldUpdate::print(out);
  switch(op) {
    case OP::REPLACE:
      return out << "REPLACE";
    case OP::APPEND:
      return out << "APPEND";
    case OP::PREPEND:
      return out << "PREPEND";
    case OP::INSERT:
      return out << "INSERT:" << pos;
    case OP::OVERWRITE:
      return out << "OVERWRITE:" << pos;
    case OP::ERASE:
      return out << "ERASE:" << pos;
    case OP::BY_INDEX:
      return out << "BY_INDEX";
    default:
      return out << "UNKNOWN";
  }
}



}}}}}
