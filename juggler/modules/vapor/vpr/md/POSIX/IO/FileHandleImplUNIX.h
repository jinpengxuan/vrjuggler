/*************** <auto-copyright.pl BEGIN do not edit this line> **************
 *
 * VR Juggler is (C) Copyright 1998, 1999, 2000 by Iowa State University
 *
 * Original Authors:
 *   Allen Bierbaum, Christopher Just,
 *   Patrick Hartling, Kevin Meinert,
 *   Carolina Cruz-Neira, Albert Baker
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * -----------------------------------------------------------------
 * File:          $RCSfile$
 * Date modified: $Date$
 * Version:       $Revision$
 * -----------------------------------------------------------------
 *
 *************** <auto-copyright.pl END do not edit this line> ***************/

#ifndef _VPR_FILE_HANDLE_IMPL_UNIX_H_
#define _VPR_FILE_HANDLE_IMPL_UNIX_H_

#include <fcntl.h>
#include <sys/types.h>
#include <string>
#include <vector>

#include <vpr/IO/BlockIO.h>
#include <vpr/IO/IOSys.h>
#include <vpr/Util/Debug.h>


namespace vpr {

/**
 * Wrapper around UNIX file descriptors.
 *
 * @author Patrick Hartling
 */
class FileHandleImplUNIX : public BlockIO {
public:
    /**
     * Constructor.  This initializes the member variables to reasonable
     * defaults and stores the given file name for later use.
     *
     * @post All member variables are initialized including m_name that is
     *       assigned the string in file_name.
     *
     * @param file_name The name of the file to be handled.
     */
    FileHandleImplUNIX(const std::string& file_name);

    /**
     * Destructor.  If the file handle is in an open state, it is closed.
     *
     * @post If the file handle is still open, it is closed.
     */
    virtual ~FileHandleImplUNIX(void);

    // ========================================================================
    // vpr::BlockIO implementation.
    // ========================================================================

    /**
     * Opens the file handle.
     *
     * @pre The file handle is not already open.
     * @post An attempt is made to open the file.  The resulting status is
     *       returned to the caller.  If the file is opened, m_open is set to
     *       true.
     *
     * @return vpr::Status;:Success is returned if the file handle was opened
     *         successfully.  vpr::Status::Failure is returned otherwise.
     */
    Status open(void);

    /**
     * Closes the file handle.
     *
     * @pre The file handle is open.
     * @post An attempt is made to close the file.  The resulting status is
     *       returned to the caller.  If the file is closed, m_open is set to
     *       false.
     *
     * @return vpr::Status::Success is returned if the file handle was closed
     *         successfully.<br>
     *         vpr::Status::Failure is returned if the file handle could not
     *         be closed for some reason.
     */
    Status close(void);

    /**
     * Reconfigures the file handle so that it is in blocking mode.
     *
     * @pre The file handle is open.
     * @post Processes may block when accessing the file.
     *
     * @return vpr::Status;:Success is returned if the blocking mode was
     *         changed successfully; vpr::Status::Failure otherwise.
     */
    Status enableBlocking(void);

    /**
     * Reconfigures the file handle so that it is in non-blocking mode.
     *
     * @pre The file handle is open.
     * @post Processes will not block when accessing the file.
     *
     * @return vpr::Status;:Success is returned if the blocking mode was
     *         changed successfully; vpr::Status::Failure otherwise.
     */
    Status enableNonBlocking(void);

    /**
     * Returns the contained handle.
     */
    inline IOSys::Handle
    getHandle (void) {
#ifdef VPR_USE_NSPR
       vprDEBUG(vprDBG_ALL, vprDBG_CRITICAL_LVL)
           << "ERROR: Cannot get handle for UNIX file descriptor with NSPR!\n";
       return vpr::IOSys::NullHandle;
#else
       return m_fdesc;
#endif
    }

    // ========================================================================
    // vpr::FileHandle implementation.
    // ========================================================================

    /**
     * Sets the open flags so that the I/O device is opened in read-only mode.
     *
     * @pre None.
     * @post The open flags are updated so that when the device is opened, it
     *       it is opened in read-only mode.  If the device is already open,
     *       this has no effect.
     */
    void
    setOpenReadOnly (void) {
        m_open_mode = O_RDONLY;
    }

    /**
     * Sets the open flags so that the I/O device is opened in write-only mode.
     *
     * @pre None.
     * @post The open flags are updated so that when the device is opened, it
     *       is opened in write-only mode.  If the device is already open,
     *       this has no effect.
     */
    void
    setOpenWriteOnly (void) {
        m_open_mode = O_WRONLY;
    }

    /**
     * Sets the open flags so that the I/O device is opened in read/write mode.
     *
     * @pre None.
     * @post The open flags are updated so that when the device is opened, it
     *       is opened in read/write mode.  If the device is already open,
     *       this has no effect.
     */
    void
    setOpenReadWrite (void) {
        m_open_mode = O_RDWR;
    }

    /**
     * Reconfigures the file handle to be in append mode.
     *
     * @pre The file handle is open.
     * @post The file handle's write mode is set to append.
     *
     * @return vpr::Status::Success is returned if the write mode was changed
     *         successfully.<br>
     *         vpr::Status::Failure is returned if the write mode could not be
     *         changed for some reason.
     */
    Status enableAppend(void);

    /**
     * Reconfigures the file handle so that it is not in append mode.
     *
     * @pre The file handle is open.
     * @post The file handle's write mode is set so that writes are appended.
     *
     * @return vpr::Status::Success is returned if the write mode was changed
     *         successfully.<br>
     *         vpr::Status::Failure is returned if the write mode could not be
     *         changed for some reason.
     */
    Status disableAppend(void);

    /**
     * Reconfigures the file handle so that writes are synchronous.
     *
     * @pre The file handle is open.
     * @post Writes are performed synchronously.
     *
     * @return vpr::Status::Success is returned if the write mode was changed
     *         successfully.<br>
     *         vpr::Status::Failure is returned if the write mode could not be
     *         changed for some reason.
     */
    Status enableSynchronousWrite(void);

    /**
     * Reconfigure the file handle so that writes are asynchronous.
     *
     * @pre The file handle is open.
     * @post Writes are performed asynchronously.
     *
     * @return vpr::Status::Success is returned if the write mode was changed
     *         successfully.<br>
     *         vpr::Status::Failure is returned if the write mode could not be
     *         changed for some reason.
     */
    Status enableAsynchronousWrite(void);

    /**
     * Tests if the I/O device is read-only.
     *
     * @pre The I/O device is open.
     * @post The access mode is tested for read-only mode, and the result is
     *       returned to the caller.
     *
     * @return <code>true</code> is returned if the device is in read-only
     *         mode; <code>false</code> otherwise.
     */
    inline bool
    isReadOnly (void) {
        return (m_open_mode == O_RDONLY);
    }

    /**
     * Tests if the I/O device is write-only.
     *
     * @pre The I/O device is open.
     * @post The access mode is tested for write-only mode, and the result is
     *       returned to the caller.
     *
     * @return <code>true</code> is returned if the device is in write-only
     *         mode; <code>false</code> otherwise.
     */
    inline bool
    isWriteOnly (void) {
        return (m_open_mode == O_WRONLY);
    }

    /**
     * Tests if the I/O device is read/write.
     *
     * @pre The I/O device is open.
     * @post The access mode is tested for read/write mode, and the result is
     *       returned to the caller.
     *
     * @return <code>true</code> is returned if the device is in read/write
     *         mode; <code>false</code> otherwise.
     */
    inline bool
    isReadWrite (void) {
        return (m_open_mode == O_RDWR);
    }

    /**
     * Implementation of the <code>read</code> template method.  This reads at
     * most the specified number of bytes from the file into the given buffer.
     *
     * @pre The device is open for reading, and the buffer is at least
     *      <code>length</code> bytes long.
     * @post The given buffer has length bytes copied into it from the device,
     *       and the number of bytes read successfully is returned to the
     *       caller via the <code>bytes_read</code> parameter.
     *
     * @param buffer     A pointer to the buffer where the device's buffer
     *                   contents are to be stored.
     * @param length     The number of bytes to be read.
     * @param bytes_read The number of bytes read into the buffer.
     * @param timeout    The maximum amount of time to wait for data to be
     *                   available for reading.  This argument is optional and
     *                   defaults to vpr::Interval::NoTimeout.
     *
     * @return vpr::Status::Success is returned if the read operation
     *         completed successfully.<br>
     *         vpr::Status::InProgress if the device is in non-blocking mode,
     *         and the read operation is in progress.<br>
     *         vpr::Status::Timeout is returned if the read could not begin
     *         within the timeout interval.<br>
     *         vpr::Status::Failure is returned if the read operation failed.
     */
    virtual Status read_i(void* buffer, const size_t length,
                          ssize_t& bytes_read,
                          const vpr::Interval timeout = vpr::Interval::NoTimeout);

    /**
     * Implementation of the <code>readn</code> template method.  This reads
     * exactly the specified number of bytes from the file into the given
     * buffer.
     *
     * @pre The device is open for reading, and the buffer is at least
     *      <code>length</code> bytes long.
     * @post The given buffer has <code>length</code> bytes copied into
     *       it from the device, and the number of bytes read successfully
     *       is returned to the caller via the <code>bytes_read</code>
     *       parameter.
     *
     * @param buffer     A pointer to the buffer where the device's buffer
     *                   contents are to be stored.
     * @param length     The number of bytes to be read.
     * @param bytes_read The number of bytes read into the buffer.
     * @param timeout    The maximum amount of time to wait for data to be
     *                   available for reading.  This argument is optional and
     *                   defaults to <code>vpr::Interval::NoTimeout</code>
     *
     * @return vpr::Status::Success is returned if the read operation
     *         completed successfully.<br>
     *         vpr::Status::InProgress if the device is in non-blocking mode,
     *         and the read operation is in progress.<br>
     *         vpr::Status::Timeout is returned if the read could not begin
     *         within the timeout interval.<br>
     */
    virtual Status readn_i(void* buffer, const size_t length,
                           ssize_t& bytes_read,
                           const vpr::Interval timeout = vpr::Interval::NoTimeout);

    /**
     * Implementation of the <code>write</code> template method.  This writes
     * the buffer to the file.
     *
     * @pre The device is open for writing.
     * @post The given buffer is written to the I/O device, and the number
     *       of bytes written successfully is returned to the caller via the
     *       <code>bytes_written</code> parameter.
     *
     * @param buffer        A pointer to the buffer to be written.
     * @param length        The length of the buffer.
     * @param bytes_written The number of bytes written to the device.
     * @param timeout       The maximum amount of time to wait for data to be
     *                      available for writing.  This argument is optional
     *                      and defaults to
     *                      <code>vpr::Interval::NoTimeout</code>.
     *
     * @return vpr::Status::Success is returned if the write operation
     *         completed successfully.<br>
     *         vpr::Status::InProgress is returned if the handle is in
     *         non-blocking mode, and the write operation is in progress.<br>
     *         vpr::Status::Timeout is returned if the write could not begin
     *         within the timeout interval.<br>
     *         vpr::Status::Failure is returned if the write operation failed.
     */
    virtual Status write_i(const void* buffer, const size_t length,
                           ssize_t& bytes_written,
                           const vpr::Interval timeout = vpr::Interval::NoTimeout);

protected:
    // Friends.
    friend class SerialPortImplTermios;
    friend class SocketDatagramImplBSD;
    friend class SocketImplBSD;
    friend class SocketStreamImplBSD;

    /**
     * Gets the current file handle flags.
     *
     * @pre The file handle is open.
     * @post The current flags for the handle are returned to the caller.
     *
     * @return A value larger than -1 is returned giving the current flags for
     *         the file handle.<br>
     *         -1 is returned if the current flags could not be requested.
     */
    int getFlags(void);

    /**
     * Overwrites the current file handle flags with the given value.
     *
     * @pre The file handle is open.
     * @post The flags for the file handle are overwritten with the new
     *       value.
     *
     * @return 0 is returned if the flags were updated successfully.<br>
     *         -1 is returned if the current flags could not be overwritten.
     */
    int setFlags(const int flags);

    /**
     * Tests if the file handle is ready for reading within the timeout
     * period.
     */
    Status isReadable(const vpr::Interval timeout);

    /**
     * Tests if the file handle is ready for writing within the timeout
     * period.
     */
    Status isWriteable(const vpr::Interval timeout);

    int m_fdesc;      /**< File descriptor */
    int m_open_mode;  /**< The open mode of the device */
};

}; // End of vpr namespace


#endif	/* _VPR_FILE_HANDLE_IMPL_UNIX_H_ */
