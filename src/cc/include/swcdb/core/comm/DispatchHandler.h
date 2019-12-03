/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 * Copyright (C) 2007-2016 Hypertable, Inc.
 *
 * This file is part of Hypertable.
 *
 * Hypertable is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or any later version.
 *
 * Hypertable is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */


#ifndef swc_core_comm_DispatchHandler_h
#define swc_core_comm_DispatchHandler_h


#include <memory>
#include "swcdb/core/comm/Event.h"
#include "swcdb/core/comm/AppContext.h"

namespace SWC {

/** Abstract base class for application dispatch handlers registered with
* AsyncComm.  Dispatch handlers are the mechanism by which an application
* is notified of communication events.
*/

class DispatchHandler : public std::enable_shared_from_this<DispatchHandler> {
  public:

  typedef std::shared_ptr<DispatchHandler> Ptr;

  virtual void handle(ConnHandlerPtr conn, Event::Ptr& ev);
    
  virtual bool run(uint32_t timeout=0);
  
};


} // namespace SWC



#ifdef SWC_IMPL_SOURCE
#include "../../../../lib/swcdb/core/comm/DispatchHandler.cc"
#endif 

#endif // swc_core_comm_DispatchHandler_h
