/* -*- c++ -*-
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
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

/** @file
 * Abstract base class for a filesystem.
 * All commands have synchronous and asynchronous versions. Commands that
 * operate on the same file descriptor are serialized by the underlying
 * filesystem. In other words, if you issue three asynchronous commands,
 * they will get carried out and their responses will come back in the
 * same order in which they were issued. Unless otherwise mentioned, the
 * methods could throw Exception.
 */

#ifndef swc_core_fs_Filesystem_h
#define swc_core_fs_Filesystem_h


#include "swcdb/lib/core/fs/Settings.h"
#include "swcdb/lib/core/comm/AppContext.h"

#include "swcdb/lib/core/Serializable.h"
#include "swcdb/lib/core/StaticBuffer.h"

#include "swcdb/lib/core/Status.h"
#include "swcdb/lib/core/Timer.h"



#include <memory>
#include <string>
#include <vector>

using namespace std;

#define HT_DIRECT_IO_ALIGNMENT      512

#define HT_IO_ALIGNED(size)                     \
    (((size) % HT_DIRECT_IO_ALIGNMENT) == 0)

#define HT_IO_ALIGNMENT_PADDING(size)           \
    (HT_DIRECT_IO_ALIGNMENT - ((size) % HT_DIRECT_IO_ALIGNMENT))

namespace SWC { namespace FS {
  
  class Filesystem {
  public:


    /// Enumeration type for append flags
    enum class Flags : uint8_t {
      /// None
      NONE=0,
      /// Flush
      FLUSH=1,
      /// Sync
      SYNC=2
    };

    enum OpenFlags {
      OPEN_FLAG_DIRECTIO = 0x00000001,
      OPEN_FLAG_OVERWRITE = 0x00000002,
      OPEN_FLAG_VERIFY_CHECKSUM = 0x00000004
    };


    virtual ~Filesystem() { }

    /** Opens a file asynchronously.  Issues an open file request.  The caller
     * will get notified of successful completion or error via the given
     * dispatch handler. It is up to the caller to deserialize the returned
     * file descriptor from the MESSAGE event object.
     *
     * @param name Absolute path name of file to open
     * @param flags Open flags (OPEN_FLAG_DIRECTIO or 0)
     * @param handler The dispatch handler which will handle the reply
     */
    virtual void open(const String &name, uint32_t flags,
      DispatchHandler *handler) = 0;

    /** Opens a file.  Issues an open file request and waits for it to complete.
     *
     * @param name Absolute path name of file to open
     * @param flags Open flags (OPEN_FLAG_DIRECTIO or 0)
     * @return The new file handle
     */
    virtual int open(const String &name, uint32_t flags) = 0;

    /** Opens a file in buffered (readahead) mode.  Issues an open file request
     * and waits for it to complete. Turns on readahead mode so that data is
     * prefetched.
     *
     * @param name Absolute path name of file to open
     * @param flags Open flags (OPEN_FLAG_DIRECTIO or 0)
     * @param buf_size Read ahead buffer size
     * @param outstanding Maximum number of outstanding reads
     * @param start_offset Starting read offset
     * @param end_offset Ending read offset
     * @return The new file handle
     */
    virtual int open_buffered(const String &name, uint32_t flags,
      uint32_t buf_size, uint32_t outstanding, uint64_t start_offset = 0,
      uint64_t end_offset = 0) = 0;

    /// Decodes the response from an open request.
    /// @param event reference to response event
    /// @param fd Address of variable to hold file descriptor
    virtual void decode_response_open(EventPtr &event, int32_t *fd) = 0;

    /** Creates a file asynchronously.  Issues a create file request with
     * various create mode parameters. The caller will get notified of
     * successful completion or error via the given dispatch handler.  It is
     * up to the caller to deserialize the returned file descriptor from the
     * MESSAGE event object.
     *
     * @param name Absolute path name of file to open
     * @param flags Open flags (OPEN_FLAG_DIRECTIO or OPEN_FLAG_OVERWRITE)
     * @param bufsz Buffer size to use for the underlying FS
     * @param replication Replication factor to use for this file
     * @param blksz Block size to use for the underlying FS
     * @param handler The dispatch handler which will handle the reply
     */
    virtual void create(const String &name, uint32_t flags, int32_t bufsz,
      int32_t replication, int64_t blksz, DispatchHandler *handler) = 0;

    /** Creates a file.  Issues a create file request and waits for completion
     *
     * @param name Absolute path name of file to open
     * @param flags Open flags (OPEN_FLAG_DIRECTIO or OPEN_FLAG_OVERWRITE)
     * @param bufsz Buffer size to use for the underlying FS
     * @param replication Replication factor to use for this file
     * @param blksz Block size to use for the underlying FS
     * @return The new file handle
     */
    virtual int create(const String &name, uint32_t flags, int32_t bufsz,
      int32_t replication, int64_t blksz) = 0;

    /// Decodes the response from a create request.
    /// @param event reference to response event
    /// @param fd Address of variable to hold file descriptor
    virtual void decode_response_create(EventPtr &event, int32_t *fd) = 0;

    /** Closes a file asynchronously.  Issues a close file request.
     * The caller will get notified of successful completion or error via
     * the given dispatch handler.  This command will get serialized along
     * with other commands issued with the same file descriptor.
     *
     * @param fd The open file descriptor
     * @param handler The dispatch handler
     */
    virtual void close(int fd, DispatchHandler *handler) = 0;

    /** Closes a file.  Issues a close command and waits for it
     * to complete.
     * This command will get serialized along with other commands
     * issued with the same file descriptor.
     *
     * @param fd The open file descriptor
     */
    virtual void close(int fd) = 0;

    /** Reads data from a file at the current position asynchronously.  Issues
     * a read request.  The caller will get notified of successful completion or
     * error via the given dispatch handler.  It's up to the caller to
     * deserialize the returned data in the MESSAGE event object.  EOF is
     * indicated by a short read. This command will get serialized along with
     * other commands issued with the same file descriptor.
     *
     * @param fd The open file descriptor
     * @param len Amount of data to read
     * @param handler The dispatch handler
     */
    virtual void read(int fd, size_t len, DispatchHandler *handler) = 0;

    /** Reads data from a file at the current position.  Issues a read
     * request and waits for it to complete, returning the read data.
     * EOF is indicated by a short read.
     * This command will get serialized along with other commands
     * issued with the same file descriptor.
     *
     * @param fd The open file descriptor
     * @param dst The destination buffer for read data
     * @param len The amount of data to read
     * @return The amount read (in bytes)
     */
    virtual size_t read(int fd, void *dst, size_t len) = 0;

    /// Decodes the response from a read request.
    /// @param event A reference to the response event
    /// @param buffer Address of buffer pointer
    /// @param offset Address of offset variable
    /// @param length Address of length variable
    virtual void decode_response_read(EventPtr &event, const void **buffer,
              uint64_t *offset, uint32_t *length) = 0;

    /** Appends data to a file asynchronously.  Issues an append request.
     * The caller will get notified of successful completion or error via the
     * given dispatch handler.  This command will get serialized along with
     * other commands issued with the same file descriptor.
     *
     * @param fd The open file descriptor
     * @param buffer The buffer to append
     * @param flags Flags for this operation: O_FLUSH or 0
     * @param handler The dispatch handler
     */
    virtual void append(int fd, StaticBuffer &buffer, Flags flags,
      DispatchHandler *handler) = 0;

    /**
     * Appends data to a file.  Issues an append request and waits for it to
     * complete.
     * This command will get serialized along with other commands
     * issued with the same file descriptor.
     *
     * @param fd The open file descriptor
     * @param buffer The buffer to append
     * @param flags Flags for this operation: O_FLUSH or 0
     */
    virtual size_t append(int fd, StaticBuffer &buffer, Flags flags = Flags::NONE) = 0;

    /** Decodes the response from an append request
     *
     * @param event A reference to the response event
     * @param offset Address of offset variable
     * @param length Address of length variable
     */
    virtual void decode_response_append(EventPtr &event, uint64_t *offset,
          uint32_t *length) = 0;

    /** Seeks current file position asynchronously.  Issues a seek request.
     * The caller will get notified of successful completion or error via the
     * given dispatch handler.  This command will get serialized along with
     * other commands issued with the same file descriptor.
     *
     * @param fd The open file descriptor
     * @param offset The absolute offset to seek to
     * @param handler The dispatch handler
     */
    virtual void seek(int fd, uint64_t offset, DispatchHandler *handler) = 0;

    /** Seeks current file position.  Issues a seek request and waits for it to
     * complete.
     * This command will get serialized along with other commands
     * issued with the same file descriptor.
     *
     * @param fd The open file descriptor
     * @param offset The absolute offset to seek to
     */
    virtual void seek(int fd, uint64_t offset) = 0;

    /** Removes a file asynchronously.  Issues a remove request.
     * The caller will get notified of successful completion or error via the
     * given dispatch handler.
     *
     * @param name The absolute pathname of file to delete
     * @param handler The dispatch handler
     */
    virtual void remove(const String &name, DispatchHandler *handler) = 0;

    /** Removes a file.  Issues a remove request and waits for it to
     * complete.
     *
     * @param name The absolute pathname of file to delete
     * @param force If true then ignore non-existence error
     */
    virtual void remove(const String &name, bool force = true) = 0;

    /** Gets the length of a file asynchronously.  Issues a length request.
     * The caller will get notified of successful completion or error via the
     * given dispatch handler.
     *
     * @param name The absolute pathname of file
     * @param accurate Whether the accurate or an estimated file length
     *      is required (an hdfs performance optimization)
     * @param handler The dispatch handler
     */
    virtual void length(const String &name, bool accurate,
      DispatchHandler *handler) = 0;

    /** Gets the length of a file.  Issues a length request and waits for it
     * to complete.
     *
     * @param name The absolute pathname of file
     * @param accurate Whether the accurate or an estimated file length
     *      is required (an hdfs performance optimization)
     */
    virtual int64_t length(const String &name, bool accurate = true) = 0;

    /// Decodes the response from a length request.
    /// @param event Reference to response event
    /// @return length of the file, in bytes
    virtual int64_t decode_response_length(EventPtr &event) = 0;

    /** Reads data from a file at the specified position asynchronously.
     * Issues a pread request.  The caller will get notified of successful
     * completion or error via the given dispatch handler.  It's up to the
     * caller to deserialize the returned data in the MESSAGE event object.
     * EOF is indicated by a short read.
     *
     * @param fd The open file descriptor
     * @param offset The starting offset of read
     * @param amount The amount of data to read (in bytes)
     * @param verify_checksum Tells filesystem to perform checksum verification
     * @param handler The dispatch handler
     */
    virtual void pread(int fd, size_t amount, uint64_t offset,
           bool verify_checksum, DispatchHandler *handler) = 0;

    /** Reads data from a file at the specified position.  Issues a pread
     * request and waits for it to complete, returning the read data.
     * EOF is indicated by a short read.
     * This command will get serialized along with other commands
     * issued with the same file descriptor.
     *
     * @param fd The open file descriptor
     * @param dst The destination buffer for read data
     * @param len The amount of data to read
     * @param offset starting The offset of read
     * @param verify_checksum Tells filesystem to perform checksum verification
     * @return The amount of data read (in bytes)
     */
    virtual size_t pread(int fd, void *dst, size_t len, uint64_t offset,
      bool verify_checksum = true) = 0;

    /// Decodes the response from a pread request.
    /// @param event A reference to the response event
    /// @param buffer Address of buffer pointer
    /// @param offset Address of offset variable
    /// @param length Address of length variable
    virtual void decode_response_pread(EventPtr &event, const void **buffer,
               uint64_t *offset, uint32_t *length) = 0;

    /** Creates a directory asynchronously.  Issues a mkdirs request which
     * creates a directory, including all its missing parents.  The caller
     * will get notified of successful completion or error via the given
     * dispatch handler.
     *
     * @param name The absolute pathname of directory to create
     * @param handler The dispatch handler
     */
    virtual void mkdirs(const String &name, DispatchHandler *handler) = 0;

    /** Creates a directory.  Issues a mkdirs request which creates a directory,
     * including all its missing parents, and waits for it to complete.
     *
     * @param name The absolute pathname of the directory to create
     */
    virtual void mkdirs(const String &name) = 0;

    /** Recursively removes a directory asynchronously.  Issues a rmdir request.
     * The caller will get notified of successful completion or error via the
     * given dispatch handler.
     *
     * @param name The absolute pathname of directory to remove
     * @param handler The dispatch handler
     */
    virtual void rmdir(const String &name, DispatchHandler *handler) = 0;

    /** Recursively removes a directory.  Issues a rmdir request and waits for
     * it to complete.
     *
     * @param name The absolute pathname of directory to remove
     * @param force If true then don't throw an error if file does not exist
     */
    virtual void rmdir(const String &name, bool force = true) = 0;

    /** Obtains a listing of all files in a directory asynchronously.  Issues a
     * readdir request.  The caller will get notified of successful completion
     * or error via the given dispatch handler.
     *
     * @param name The absolute pathname of directory
     * @param handler The dispatch handler
     */
    virtual void readdir(const String &name, DispatchHandler *handler) = 0;

    /** Obtains a listing of all files in a directory.  Issues a readdir request
     * and waits for it to complete.
     *
     * @param name Absolute pathname of directory
     * @param listing Reference to output vector of Dirent objects for each entry
     */
    virtual void readdir(const String &name, std::vector<Dirent> &listing) = 0;

    /// Decodes the response from a readdir request.
    /// @param event A reference to the response event
    /// @param listing Reference to output vector of Dirent objects
    virtual void decode_response_readdir(EventPtr &event,
           std::vector<Dirent> &listing) = 0;

    /** Flushes a file asynchronously.  Isues a flush command which causes all
     * buffered writes to get persisted to disk.  The caller will get notified
     * of successful completion or error via the given dispatch handler.  This
     * command will get serialized along with other commands issued with the
     * same file descriptor.
     *
     * @param fd The open file descriptor
     * @param handler The dispatch handler
     */
    virtual void flush(int fd, DispatchHandler *handler) = 0;

    /** Flushes a file.  Issues a <i>flush</i> command which causes all buffered
     * writes to get flushed to the underlying filesystem.  For "normal"
     * filesystems, such as the local filesystem, this command translates into
     * an fsync() system call.  However, for some distributed filesystems such
     * as HDFS, this command causes the filesystem broker to flush buffered
     * writes into the memory of all of the replica servers, but doesn't
     * necessarily push the writes all the way down to the physical storage.
     * This command will get serialized along with other commands issued
     * with the same file descriptor.
     *
     * @param fd The open file descriptor
     */
    virtual void flush(int fd) = 0;

    /** Syncs a file.  Issues a <i>sync</i> command which causes the filesystem
     * to persist all buffered updates to the physical storage.  It is
     * equivalent to the fsync() Unix system call.  This command will get
     * serialized along with other commands issued with the same file descriptor.
     *
     * @param fd The open file descriptor
     */
    virtual void sync(int fd) = 0;

    /** Determines if a file exists asynchronously.  Issues an exists request.
     * The caller will get notified of successful completion or error via the
     * given dispatch handler.
     *
     * @param name The absolute pathname of file
     * @param handler The dispatch handler
     */
    virtual void exists(const String &name, DispatchHandler *handler) = 0;

    /** Determines if a file exists.
     *
     * @param name The absolute pathname of the file
     * @return true if the file exists, otherwise false
     */
    virtual bool exists(const String &name) = 0;

    /// Decodes the response from an exists request.
    /// @param event A reference to the response event
    /// @return <i>true</i> if the file exists, <i>false</i> otherwise
    virtual bool decode_response_exists(EventPtr &event) = 0;

    /** Rename a path asynchronously.
     * @param src The source path
     * @param dst The destination path
     * @param handler The dispatch/callback handler
     */
    virtual void rename(const String &src, const String &dst,
      DispatchHandler *handler) = 0;

    /** Rename a path
     *
     * @param src The source path
     * @param dst The destination path
     */
    virtual void rename(const String &src, const String &dst) = 0;

    /// Check status of filesystem
    /// @param status %Status output
    /// @param timer Deadline timer
    /// @return Nagios-style status code
    virtual void status(Status &status, Timer *timer=0) = 0;

    /// Decodes the response from an status request.
    /// @param event Reference to response event
    /// @param status Reference to status information output parameter
    virtual void decode_response_status(EventPtr &event, Status &status) = 0;

    /** Decodes the response from an request that only returns an error code
     *
     * @param event A reference to the response event
     * @return The error code
     */
    static int decode_response(EventPtr &event);

    /** Invokes debug request asynchronously
     *
     * @param command debug command identifier
     * @param serialized_parameters command specific serialized parameters
     */
    virtual void debug(int32_t command,
      StaticBuffer &serialized_parameters) = 0;

    /** Invokes debug request
     *
     * @param command The debug command identifier
     * @param serialized_parameters The command specific serialized parameters
     * @param handler The dispatch/callback handler
     */
    virtual void debug(int32_t command, StaticBuffer &serialized_parameters,
      DispatchHandler *handler) = 0;

    /** 
     * A posix-compliant dirname() which strips the last component from a
     * file name
     *
     *     /usr/bin/ -> /usr
     *     stdio.h   -> .
     *
     * @param name The directory name
     * @param separator The path separator
     * @return The stripped directory
     */
    static String dirname(String name, char separator = '/');

    /** 
     * A posix-compliant basename() which strips directory names from a
     * filename
     *
     *    /usr/bin/sort -> sort
     *
     * @param name The directory name
     * @param separator The path separator
     * @return The basename
     */
    static String basename(String name, char separator = '/');


    // Methods based on SmartFd

    virtual	void open(SmartFdPtr fd_obj, DispatchHandler *handler) = 0;
    virtual	void open(SmartFdPtr fd_obj) = 0;
    virtual	void open_buffered(SmartFdPtr &fd_obj, 
      uint32_t buf_size, uint32_t outstanding,
      uint64_t start_offset = 0, uint64_t end_offset = 0) = 0;
    virtual	void decode_response_open(SmartFdPtr fd_obj, EventPtr &event) = 0;
        
    virtual	void create(SmartFdPtr fd_obj, int32_t bufsz, 
      int32_t replication, int64_t blksz, DispatchHandler *handler) = 0;
    virtual	void create(SmartFdPtr fd_obj, int32_t bufsz, 
      int32_t replication, int64_t blksz) = 0;
    virtual	void decode_response_create(SmartFdPtr fd_obj, 
      EventPtr &event) = 0;

    virtual	void close(SmartFdPtr fd_obj, DispatchHandler *handler) = 0;
    virtual	void close(SmartFdPtr fd_obj) = 0;

    virtual	void read(SmartFdPtr fd_obj, size_t amount, 
      DispatchHandler *handler) = 0;
    virtual	size_t read(SmartFdPtr fd_obj, void *dst,	size_t amount) = 0;
    virtual	void decode_response_read(SmartFdPtr fd_obj, 
      EventPtr &event, const void **buffer, 
      uint64_t *offset, uint32_t *length) = 0;

    virtual	void pread(SmartFdPtr fd_obj, size_t len, 
      uint64_t offset,bool verify_checksum, DispatchHandler *handler) = 0;
    virtual	size_t pread(SmartFdPtr fd_obj, void *dst, 
      size_t len, uint64_t offset, bool verify_checksum) = 0;
    virtual	void decode_response_pread(SmartFdPtr fd_obj, 
      EventPtr &event, const void **buffer, uint64_t *offset, 
      uint32_t *length) = 0;

    virtual	void append(SmartFdPtr fd_obj, 
      StaticBuffer &buffer,	Flags flags, DispatchHandler *handler) = 0;
    virtual	size_t append(SmartFdPtr fd_obj, StaticBuffer &buffer, 
                          Flags flags = Flags::NONE) = 0;
    virtual	void decode_response_append(SmartFdPtr fd_obj, 
      EventPtr &event, uint64_t *offset, uint32_t *length) = 0;

    virtual	void seek(SmartFdPtr fd_obj, uint64_t offset, 
      DispatchHandler *handler) = 0;
    virtual	void seek(SmartFdPtr fd_obj, uint64_t offset) = 0;

    virtual	void flush(SmartFdPtr fd_obj, DispatchHandler *handler) = 0;
    virtual	void flush(SmartFdPtr fd_obj) = 0;

    virtual	void sync(SmartFdPtr fd_obj) = 0;

  
    virtual	SmartFdPtr create_local_temp(const String &for_filename) = 0;
    virtual	void append_to_temp(SmartFdPtr smartfd_ptr, 
      StaticBuffer &buffer) = 0;
    virtual	void commit_temp(SmartFdPtr &smartfd_ptr, 
      SmartFdPtr to_smartfd_ptr, int32_t replication)  = 0;

    /** Determines if it is OK to retry write.
     *
     * @param smartfd_ptr the SmartFdPtr 
     * @param e_code Exception code
     * @param tries_count pointer to write_count
     * @return true if OK, otherwise false
     */
    virtual bool retry_write_ok(SmartFdPtr smartfd_ptr, 
			int32_t e_code, int32_t *tries_count, bool del_old=true) = 0;

    virtual	int32_t get_retry_write_limit() = 0;

  };

  /// Smart pointer to Filesystem
  typedef std::shared_ptr<Filesystem> FilesystemPtr;

  inline bool operator< (const Filesystem::Dirent& lhs,
       const Filesystem::Dirent& rhs) {
    return lhs.name.compare(rhs.name) < 0;
  }

  /// Converts string mnemonic to corresponding Filesystem::Flags value.
  /// @param str String mnemonic for append flag ("NONE", "FLUSH", or "SYNC")  
  /// @return Append flag corresponding to string mnemonic
  /// @throws Exception with code equal to Error::INVALID_ARGUMENT if string
  /// mnemonic is not recognized
  extern Filesystem::Flags convert(std::string str);


}} // namespace SWC

#endif // swc_core_fs_Filesystem_h

