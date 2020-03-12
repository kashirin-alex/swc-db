/* -*- c++ -*-
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * Copyright (C) 2007-2016 Hypertable, Inc.
 *
 * This file is part of Hypertable.
 *
 * Hypertable is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 3 of the
 * License, or any later version.
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

/// @file
/// Declarations for Status.
/// This file contains declarations for Status, a class to hold Nagios-style
/// status information for a running program.

#ifndef swc_core_Status_h
#define swc_core_Status_h

#include "swcdb/core/Serializable.h"

#include <mutex>
#include <string>
#include <utility>

namespace SWC {

  /// @addtogroup Common
  /// @{

  /// Holds Nagios-style program status information.
  class Status : public Serializable {

  public:

    /// Enumeration for status codes
    enum class Code: int32_t {
      /// OK
      OK = 0,
      /// Warning
      WARNING = 1,
      /// Critical
      CRITICAL = 2,
      /// Unknown
      UNKNOWN = 3
    };

    /// Status text string constants.
    class Text {
    public:
      static constexpr const char *SERVER_IS_COMING_UP = "Server is coming up";
      static constexpr const char *SERVER_IS_SHUTTING_DOWN = "Server is shutting down";
      static constexpr const char *STANDBY = "Standby";
    };

    static const char *code_to_string(Code code);
    static Code string_to_code(std::string str);

    /// Constructor.
    Status() { }

    /// Constructor with initial values.
    /// @param code %Status code
    /// @param text %Status text
    Status(Code code, const std::string &text) : m_code(code), m_text(text) {}

    /// Copy constructor.
    /// @param other Other status object from which to copy
    Status(const Status &other) {
      Code code;
      std::string text;
      other.get(&code, text);
      std::lock_guard<std::mutex> lock(m_mutex);      
      m_code = code;
      m_text.clear();
      m_text.append(text);
    }

    /// Copy assignment operator.
    /// @param rhs Right hand side object to copy
    Status &operator=(const Status &rhs) {
      Status temp(rhs);
      std::swap(m_code, temp.m_code);
      std::swap(m_text, temp.m_text);
      return *this;
    }

    /// Sets status code and text
    /// @param code %Status code
    /// @param text %Status text
    void set(Code code, const std::string &text) {
      std::lock_guard<std::mutex> lock(m_mutex);
      m_code = code;
      m_text.clear();
      m_text.append(text);
    }

    /// Gets status code and text.
    /// @param code Address of variable to hold status code
    /// @param text Reference to string variable to hold status text
    void get(Code *code, std::string &text) const {
      std::lock_guard<std::mutex> lock(m_mutex);
      *code = m_code;
      text = m_text;
    }

    /// Gets status code.
    /// @return Status code.
    Code get() {
      std::lock_guard<std::mutex> lock(m_mutex);
      return m_code;
    }

    /// Formats a Nagios-style output line.
    /// Formats and returns a Nagios-style output line in the following format:
    /// <pre>
    /// SERVICE STATUS - status text
    /// </pre>
    /// @param service Service name
    /// @return Nagios-style output line.
    std::string format_output_line(const std::string &service);

  private:

    /// Returns encoding version.
    /// @return Encoding version
    uint8_t encoding_version() const override;

    /// Returns internal serialized length.
    /// @return Internal serialized length
    /// @see encode_internal() for encoding format
    size_t encoded_length_internal() const override;

    /// Writes serialized representation of object to a buffer.
    /// @param bufp Address of destination buffer pointer (advanced by call)
    void encode_internal(uint8_t **bufp) const override;

    /// Reads serialized representation of object from a buffer.
    /// @param version Encoding version
    /// @param bufp Address of destination buffer pointer (advanced by call)
    /// @param remainp Address of integer holding amount of serialized object
    /// remaining
    /// @see encode_internal() for encoding format
    void decode_internal(uint8_t version, const uint8_t **bufp,
			 size_t *remainp) override;

    /// %Mutex for serializaing access to members
    mutable std::mutex m_mutex;

    /// Status code
    Code m_code {};

    /// Status text
    std::string m_text;

  };

  /// @}
}


#endif // swc_core_Status_h
