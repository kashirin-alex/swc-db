/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_client_Settings_h
#define swcdb_db_client_Settings_h

#include "swcdb/core/config/Settings.h"
#include "swcdb/core/comm/Settings.h"

namespace SWC{ namespace Config {

void init_client_options(Settings* settings);

}} // namespace SWC::Config


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/client/Settings.cc"
#endif

#endif // swcdb_db_client_Settings_h