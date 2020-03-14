
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_db_types_MngrState_h
#define swc_db_types_MngrState_h


namespace SWC { namespace Types { 

enum MngrState {
  NOTSET = 0,
  OFF = 1,
  STANDBY = 2,
  WANT = 3,
  NOMINATED = 4,
  ACTIVE = 5
};

}}

#endif // swc_db_types_MngrState_h