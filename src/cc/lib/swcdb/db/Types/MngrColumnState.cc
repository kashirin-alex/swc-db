/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/db/Types/MngrColumnState.h"


namespace SWC { namespace DB { namespace Types {


namespace {
  const char MngrColumn_State_NOTSET[]    = "NOTSET";
  const char MngrColumn_State_OK[]        = "OK";
  const char MngrColumn_State_LOADING[]   = "LOADING";
  const char MngrColumn_State_DELETED[]   = "DELETED";
  const char MngrColumn_State_UNKNOWN[]   = "UNKNOWN";
}


const char* to_string(MngrColumn::State state) noexcept {
  switch(state) {

    case MngrColumn::State::NOTSET:
      return MngrColumn_State_NOTSET;

    case MngrColumn::State::OK:
      return MngrColumn_State_OK;

    case MngrColumn::State::LOADING:
      return MngrColumn_State_LOADING;

    case MngrColumn::State::DELETED:
      return MngrColumn_State_DELETED;

    default:
      return MngrColumn_State_UNKNOWN;
  }
}


}}}
