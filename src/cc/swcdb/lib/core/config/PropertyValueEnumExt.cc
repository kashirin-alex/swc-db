/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#include "swcdb/lib/core/Compat.h"
#include "swcdb/lib/core/Logger.h"
#include "swcdb/lib/core/Error.h"

#include "PropertyValueEnumExt.h"



namespace SWC {

/** @addtogroup Common
 *  @{
 */

namespace Property {


int ValueEnumExtBase::from_string(String opt) {
  int nv = get_call_from_string()(opt);
  if(nv > -1)
    set_value(nv);
  else {
    if(get() == -1)
      HT_THROWF(Error::CONFIG_GET_ERROR, 
                "Bad Value %s, no corresponding enum", opt.c_str());
    else
      HT_WARNF("Bad cfg Value %s, no corresponding enum", opt.c_str());
  }
  return get();
}

String ValueEnumExtBase::to_str() {
  return format("%s  # (%d)", get_call_repr()(get()).c_str(), get());
}

void ValueEnumExtBase::set_default_calls() {
  call_from_string = [](String opt){
      HT_THROWF(Error::CONFIG_GET_ERROR, "Bad Value %s, no from_string cb set", opt.c_str());
      return -1;
  };
  call_repr = [](int v){return "No repr cb defined!";};
}


} // namespace Property

/** @} */

} // namespace Hypertable
