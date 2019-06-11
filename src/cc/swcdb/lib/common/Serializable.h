/* -*- c++ -*-
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

/// @file
/// Declarations for Serializable.
/// This file contains declarations for Serializable, a mixin class that
/// provides a serialization interface.

#ifndef Hypertable_Common_Serializable_h
#define Hypertable_Common_Serializable_h

namespace Hypertable {

  /// @addtogroup Common
  /// @{

  /// Mixin class that provides a standard serialization interface.
  /// This class is designed to provide a standard serialization interface to
  /// any class.  It allows the serialization of a class to be extended in such
  /// a way to allow for both <i>backward</i> and <i>forward</i> compatibility.
  /// It accomplishes this by encoding a header at the beginning of the
  /// serialization that includes a version number and the length of the
  /// remaining encoding.  An encoding version can be extended in a forward
  /// compatible way by adding additional encoded state at the end of the
  /// encoding.  This additional encoding will be skipped by older decoders and
  /// newer decoders can decode older encodings (of the same version) by
  /// returning once the end of the serialization is reached.  For example,
  /// let's say that you create an encoding that initially just encodes a 32-bit
  /// integer and then is later extended to included an additional string.
  /// To handle the decoding in a backward and forward compatible way, the
  /// decode_internal() function should be implemented as follows:
  /// <pre>
  /// void decode_internal(uint8_t version, const uint8_t **bufp, size_t *remainp) {
  ///   if (version == 1) {
  ///     m_num = Serialization::decode_i32(bufp, remainp);
  ///     // Check to see if we're at the end of the record and if so return.
  ///     if (*remainp == 0)
  ///       return;
  ///     m_str = Serialization::decode_vstr(bufp, remainp);
  ///   }
  /// }
  /// </pre>
  /// A change in the encoding version number means that the encoding has
  /// changed in a forward incompatible way.  The decode() function will throw
  /// an Exception with code Error::PROTOCOL_ERROR if the version being decoded
  /// is greater than the version returned by encoding_version().
  class Serializable {

  public:

    /// Returns serialized object length.
    /// Returns the serialized length of the object as encoded by encode().
    /// @see encode() for encoding format
    /// @return Overall serialized object length
    virtual size_t encoded_length() const;

    /// Writes serialized representation of object to a buffer.
    /// This function encodes a serialized representation of the object,
    /// starting with a header that includes the encoding version and the
    /// serialized length of the object.  After the header, the object per-se is
    /// encoded with encode_internal().
    /// @param bufp Address of destination buffer pointer (advanced by call)
    virtual void encode(uint8_t **bufp) const;

    /// Reads serialized representation of object from a buffer.
    /// @param bufp Address of destination buffer pointer (advanced by call)
    /// @param remainp Address of integer holding amount of remaining buffer
    /// @see encode() for encoding format
    /// @throws Exception with code Error::PROTOCOL_ERROR if version being
    /// decoded is greater than that returned by encoding_version().
    virtual void decode(const uint8_t **bufp, size_t *remainp);

  protected:

    /// Returns encoding version.
    /// @return Encoding version
    virtual uint8_t encoding_version() const = 0;

    /// Returns internal serialized length.
    /// This function is to be overridden by derived classes and should return
    /// the length of the the serialized object per-se.
    /// @return Internal serialized length
    /// @see encode_internal() for encoding format
    virtual size_t encoded_length_internal() const = 0;

    /// Writes serialized representation of object to a buffer.
    /// This function is to be overridden by derived classes and should encode
    /// the object per-se.
    /// @param bufp Address of destination buffer pointer (advanced by call)
    virtual void encode_internal(uint8_t **bufp) const = 0;

    /// Reads serialized representation of object from a buffer.
    /// This function is to be overridden by derived classes and should decode
    /// the object per-se as encoded with encode_internal().
    /// @param version Encoding version
    /// @param bufp Address of destination buffer pointer (advanced by call)
    /// @param remainp Address of integer holding amount of serialized encoding remaining
    /// @see encode_internal() for encoding format
    virtual void decode_internal(uint8_t version, const uint8_t **bufp, size_t *remainp) = 0;

  };

  /// @}

}

#endif // Hypertable_Common_Serializable_h
