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

#include <Environment/vjNetCommunicator.h>
//#include <iostream.h>
//#include <string>

//class vjConnect;

//: Constructor
vjNetCommunicator::vjNetCommunicator () {
    connection = NULL;
}

//: Destructor
/*virtual*/ vjNetCommunicator::~vjNetCommunicator () {
    ;
}

//: Called when a new connection is established.
//  Used to send any initial messages on a new connection.
//  Useful for any initial data queries that need to be sent
//  when a new connection is opened.  Frex, requesting the
//  current configuration of whatever's on the other side of
//  the connection.
//! PRE: _connection is open & valid for writing to; connection
//+      is NULL.
//! POST: true.
/*virtual*/ void vjNetCommunicator::initConnection(vjConnect* _connection) {
    connection = _connection;
}


//: Called when the connection is shut down.
//  Useful for doing things like removing the Active chunkdb/descdb
//  (at least on vjcontrol's side).
//! PRE: connection != NULL.
//! POST: connection = NULL.
/*virtual*/ void vjNetCommunicator::shutdownConnection() {
    connection = 0;
}


//: True if the identifier represents a stream we can read.
//  The id is the token in the handler attribute of a
//  protocol tag in the EM/VjC command stream.  A given 
//  NetCommunicator can theoretically serve several handlers;
//  this is useful for backwards compatibility.
//! RETURNS: True - if self knows how to parse this stream.
//! RETURNS: False - otherwise.
/*virtual*/ bool vjNetCommunicator::acceptsStreamIdentifier (const std::string& id) {
    return false;
}


//: Reads data from a communications stream.
//  This should only be called by the vjConnect object self is
//  owned by.
//  The Communicator should read data until it reaches the end of
//  the protocol stream (signified by the character string
//  "</protocol>".  readStream should read that string and
//  absolutely no further (as this can confuse parsing of later
//  protocol streams on the same connection).
//! PRE: connection != NULL;
//! RETURNS: true - if reading the protocol stream was succesful.
//! RETURNS: false - if EOF or a fatal error occurs.  This will
//+                  kill the vjConnect.
/*virtual*/ bool vjNetCommunicator::readStream (std::istream& instream, const std::string& id) {
    return false;
}
