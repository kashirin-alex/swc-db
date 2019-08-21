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

/** @file
 * Declarations for DispatchHandler.
 * This file contains type declarations for DispatchHandler, an abstract
 * base class from which are derived handlers for delivering communication
 * events to an application.
 */

#ifndef swc_core_comm_DispatchHandler_h
#define swc_core_comm_DispatchHandler_h

#include "Event.h"


#include <memory>

namespace SWC {

  /** @addtogroup AsyncComm
   *  @{
   */

  /** Abstract base class for application dispatch handlers registered with
   * AsyncComm.  Dispatch handlers are the mechanism by which an application
   * is notified of communication events.
   */
  class DispatchHandler : public std::enable_shared_from_this<DispatchHandler> {
  public:

    /** Destructor
     */
    virtual ~DispatchHandler() { return; }

    /** Callback method.  When the Comm layer needs to deliver an event to the
     * application, this method is called to do so.  The set of event types
     * include, CONNECTION_ESTABLISHED, DISCONNECT, MESSAGE, ERROR, and TIMER.
     *
     * @param event_ptr smart pointer to Event object
     */
    virtual void handle(ConnHandlerPtr conn, EventPtr &ev) { 
      HT_DEBUGF("handle(virtual): %s", ev->to_str().c_str());
      // Not a pure method,
      // instead of wait and keep a handler method for outstanding events in parent
      // do a discard
      return;
    }
    
    virtual bool run(uint32_t timeout=0) { return false; }
    
    virtual void run(ConnHandlerPtr conn) {}
  
    void run_within(IOCtxPtr io_ctx, uint32_t t_ms = 1000) {
      (new asio::high_resolution_timer(
        *io_ctx.get(), std::chrono::milliseconds(t_ms)))
      ->async_wait(
          [ptr=shared_from_this()](const asio::error_code ec) {
            if (ec != asio::error::operation_aborted){
              ptr->run();
            }
          });
    }

  };

  /// Smart pointer to DispatchHandler
  typedef std::shared_ptr<DispatchHandler> DispatchHandlerPtr;
  /** @}*/
}

#endif // swc_core_comm_DispatchHandler_h
