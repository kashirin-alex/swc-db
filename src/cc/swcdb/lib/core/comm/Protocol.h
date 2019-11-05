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
 * Declarations for Protocol.
 * This file contains type declarations for Protocl, an abstract base class
 * from which different server protocols are derived.
 */


#ifndef swc_core_comm_Protocol_h
#define swc_core_comm_Protocol_h

#include "CommBuf.h"
#include "Event.h"
#include "../Serialization.h"

namespace SWC { namespace Protocol {


static int32_t response_code(const Event *event) {
  if (event->type == Event::ERROR)
    return event->error;

  const uint8_t *msg = event->data.base;
  size_t remaining = event->data.size;

  try { return Serialization::decode_i32(&msg, &remaining); }
  catch (Exception &e) { return e.code(); }
}

static int32_t response_code(const Event::Ptr &event) {
  return response_code(event.get());
}

    /** Creates a standard error message response.  This method creates a
     * standard error message response encoded in the following format:
     * <pre>
     *   [int32] error code
     *   [int16] error message length
     *   [chars] error message
     * </pre>
     * @param header Reference to header to use in response buffer
     * @param error %Error code
     * @param msg %Error message
     * @return Pointer to Commbuf message holding standard error response
     */
static CommBuf::Ptr 
create_error_message(CommHeader &header, int error, const char *msg){
  auto cbp = CommBuf::make(header, 4 + Serialization::encoded_length_str16(msg));
  cbp->append_i32(error);
  cbp->append_str16(msg);
  return cbp;
}

    /** Returns error message decoded from standard error MESSAGE generated
     * in response to a request message.  When a request to a service method
     * results in an error, the error code an message are typically returned
     * in a response message encoded in the following format:
     * <pre>
     *   [int32] error code
     *   [int16] error message length
     *   [chars] error message
     * </pre>
     * This method extracts and returns the error message from a response
     * MESSAGE event.
     * @note The semantics of this method are wacky, it should be re-written
     * @param event Pointer to MESSAGE event received in response to a request
     * @return %Error message
     */

static String string_format_message(const Event *event) {
  int error = Error::OK;

  if (event == 0)
    return String("NULL event");

  const uint8_t *msg = event->data.base;
  size_t remaining = event->data.size;

  if (event->type != Event::MESSAGE)
    return event->to_str();

  try {
    error = Serialization::decode_i32(&msg, &remaining);

    if (error == Error::OK)
      return Error::get_text(error);

    uint16_t len;
    const char *str = Serialization::decode_str16(&msg, &remaining, &len);

    return String(str, len > 150 ? 150 : len);
  }
  catch (Exception &e) {
    return format("%s - %s", e.what(), Error::get_text(e.code()));
  }
}

static String string_format_message(const Event::Ptr &event) {
  return string_format_message(event.get());
}


}}

#endif // swc_core_comm_Protocol_h
