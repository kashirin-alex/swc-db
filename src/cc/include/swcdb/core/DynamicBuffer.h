/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
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
 * A dynamic, resizable memory buffer.
 */

#ifndef swc_core_DynamicBuffer_h
#define swc_core_DynamicBuffer_h

#include <memory>

namespace SWC {

  /**
   * A dynamic, resizable and reference counted memory buffer
   */
  class DynamicBuffer final {

  public:

    typedef std::shared_ptr<DynamicBuffer> Ptr;

    /**
     * Constructor
     *
     * @param initial_size Initial size of the buffer
     * @param own_buffer If true, then this object takes ownership of the
     *      buffer and releases it when going out of scope
     */
    explicit DynamicBuffer(size_t initial_size = 0, bool own_buffer = true);

    DynamicBuffer(const DynamicBuffer&) = delete;

    DynamicBuffer(const DynamicBuffer&&) = delete;
    
    DynamicBuffer& operator=(const DynamicBuffer&) = delete;
    
    /** Destructor; releases the buffer if it "owns" it */
    ~DynamicBuffer();

    void take_ownership(DynamicBuffer& other);

    /** Returns the size of the unused portion */
    size_t remaining() const;

    /** Returns the size of the used portion */
    size_t fill() const;

    /** Returns true if the buffer is empty */
    bool empty() const;

    /**
     * Ensure space for additional data
     * Will grow the space to 1.5 of the needed space with existing data
     * unchanged.
     *
     * @param len Additional bytes to grow
     */
    void ensure(size_t len);

    /**
     * Reserve space for additional data
     * Will grow the space to exactly what's needed. Existing data is NOT
     * preserved by default
     *
     * @param len Size of the reserved space
     * @param nocopy If true then the existing data is not preserved
     */
    void reserve(size_t len, bool nocopy = false);

    /** Adds additional data without boundary checks
     *
     * @param data A pointer to the new data
     * @param len The size of the new data
     * @return A pointer to the added data
     */
    uint8_t *add_unchecked(const void *data, size_t len);

    /** Adds more data WITH boundary checks; if required the buffer is resized
     * and existing data is preserved
     *
     * @param data A pointer to the new data
     * @param len The size of the new data
     * @return A pointer to the added data
     */
    uint8_t *add(const void *data, size_t len);

    /** Overwrites the existing data
     *
     * @param data A pointer to the new data
     * @param len The size of the new data
     */
    void set(const void *data, size_t len);

    /** Clears the buffer */
    void clear();

    /** Sets the mark; the mark can be used by the caller just like a
     * bookmark */
    void set_mark();

    /** Frees resources */
    void free();

    /** Moves ownership of the buffer to the caller
     *
     * @param lenp If not null then the length of the buffer is stored
     * @return A pointer to the data
     */
    uint8_t *release(size_t *lenp = 0);

    /** Grows the buffer and copies the data unless nocopy is true
     *
     * @param new_size The new buffer size
     * @param nocopy If true then the data will not be preserved
     */
    void grow(size_t new_size, bool nocopy = false);

    /** The size of the allocated memory buffer (@ref base) */
    uint32_t size;
    
    /** If true then the buffer (@ref base) will be released when going out of
     * scope; if false then the caller has to release it */
    bool own;

    /** Pointer to the allocated memory buffer */
    uint8_t *base;

    /** Pointer to the end of the used part of the buffer */
    uint8_t *ptr;

    /** A "bookmark", can be set by the caller */
    uint8_t *mark;


  };


}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/DynamicBuffer.cc"
#endif 


#endif // Common_DynamicBuffer_h
