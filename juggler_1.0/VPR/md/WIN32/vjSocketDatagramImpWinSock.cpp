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

#include <stdio.h>
#include <string.h>

#include <VPR/md/WIN32/vjSocketDatagramImpWinSock.h>


// ============================================================================
// External global variables.
// ============================================================================
extern int errno;


// ============================================================================
// Public methods.
// ============================================================================

// ----------------------------------------------------------------------------
// Default constructor.  This does nothing.
// ----------------------------------------------------------------------------
vjSocketDatagramImpWinSock::vjSocketDatagramImpWinSock (void)
    : vjSocketImpWinSock(), vjSocketDatagramImp()
{
fprintf(stderr, "vjSocketDatagramImpWinSock default constructor\n");
    /* Do nothing. */ ;
}

// ----------------------------------------------------------------------------
// Constructor.  This takes the address (either hostname or IP address) of a
// remote site and a port and stores the values for later use in the member
// variables of the object.
// ----------------------------------------------------------------------------
vjSocketDatagramImpWinSock::vjSocketDatagramImpWinSock (const std::string& address,
                                                        const unsigned short port,
                                                        const vjSocketTypes::Domain domain)
    : vjSocketImpWinSock(address, port, domain, vjSocketTypes::DATAGRAM),
      vjSocketDatagramImp(address, port, domain)
{
    m_name = address;
    m_addr.setPort(port);
    m_addr.setFamily(domain);
    m_type = vjSocketTypes::DATAGRAM;
fprintf(stderr, "vjSocketDatagramImpWinSock(address, port, domain) constructor\n");
fprintf(stderr, "    Address: %s -> %s\n", address.c_str(), m_name.c_str());
fprintf(stderr, "    Port: %hu -> %hu\n", port, m_addr.getPort());
fprintf(stderr, "    Domain: %d -> %d\n", domain, m_addr.getFamily());
fprintf(stderr, "    Type: %d\n", m_type);
}

// ----------------------------------------------------------------------------
// Destructor.  This currently does nothing.
// ----------------------------------------------------------------------------
vjSocketDatagramImpWinSock::~vjSocketDatagramImpWinSock () {
    /* Do nothing. */ ;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
ssize_t
vjSocketDatagramImpWinSock::recvfrom (void* msg, const size_t len,
                                      const int flags, vjInetAddr& from)
{
    int fromlen;
    ssize_t bytes;

    fromlen = from.size();
    bytes   = ::recvfrom(m_sockfd, (char*) msg, len, flags,
                         (struct sockaddr*) &from.m_addr, &fromlen);

    if ( bytes == -1 ) {
        fprintf(stderr,
                "[vjSocketDatagramImpWinSock] ERROR: Could not read from socket (%s:%hu): %s\n",
                m_addr.getAddressString().c_str(), m_addr.getPort(),
                strerror(errno));
    }

    return bytes;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
ssize_t
vjSocketDatagramImpWinSock::recvfrom (unsigned char* msg, const size_t len,
                                      const int flags, vjInetAddr& from)
{
    return recvfrom((void*) msg, len, flags, from);
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
ssize_t
vjSocketDatagramImpWinSock::recvfrom (char* msg, const size_t len,
                                      const int flags, vjInetAddr& from)
{
    return recvfrom((void*) msg, len, flags, from);
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
ssize_t
vjSocketDatagramImpWinSock::recvfrom (std::vector<char>& msg, const int flags,
                                      vjInetAddr& from)
{
    return recvfrom(msg, msg.size(), flags, from);
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
ssize_t
vjSocketDatagramImpWinSock::recvfrom (std::vector<char>& msg, const size_t len,
                                      const int flags, vjInetAddr& from)
{
    ssize_t bytes;
    char* temp_buf;

    temp_buf = (char*) malloc(len);
    bytes    = recvfrom(temp_buf, len, flags, from);

    // If anything was read into temp_buf, copy it into msg.
    if ( bytes > -1 ) {
        for ( ssize_t i = 0; i < bytes; i++ ) {
            msg[i] = temp_buf[i];
        }
    }

    free(temp_buf);

    return bytes;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
ssize_t
vjSocketDatagramImpWinSock::sendto (const void* msg, const size_t len,
                                    const int flags, const vjInetAddr& to)
{
    ssize_t bytes;

    bytes = ::sendto(m_sockfd, (char*) msg, len, flags,
                     (struct sockaddr*) &to.m_addr, to.size());

    if ( bytes == -1 ) {
        fprintf(stderr,
                "[vjSocketDatagramImpWinSock] ERROR: Could not send to %s:%hu on socket (%s:%hu): %s\n",
                to.getAddressString().c_str(), to.getPort(),
                m_addr.getAddressString().c_str(), m_addr.getPort(),
                strerror(errno));
    }

    return bytes;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
ssize_t
vjSocketDatagramImpWinSock::sendto (const unsigned char* msg, const size_t len,
                                    const int flags, const vjInetAddr& to)
{
    return sendto((void*) msg, len, flags, to);
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
ssize_t
vjSocketDatagramImpWinSock::sendto (const char* msg, const size_t len,
                                    const int flags, const vjInetAddr& to)
{
    return sendto((void*) msg, len, flags, to);
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
ssize_t
vjSocketDatagramImpWinSock::sendto (const std::vector<char>& msg,
                                    const int flags, const vjInetAddr& to)
{
    return sendto(msg, msg.size(), flags, to);
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
ssize_t
vjSocketDatagramImpWinSock::sendto (const std::vector<char>& msg,
                                    const size_t len, const int flags,
                                    const vjInetAddr& to)
{
    ssize_t bytes;
    char* temp_buf;

    temp_buf = (char*) malloc(len);

    // Copy the contents of msg into temp_buf.
    for ( size_t i = 0; i < len; i++ ) {
        temp_buf[i] = msg[i];
    }

    // Write temp_buf to the file handle.
    bytes = sendto(temp_buf, len, flags, to);

    free(temp_buf);

    return bytes;
}