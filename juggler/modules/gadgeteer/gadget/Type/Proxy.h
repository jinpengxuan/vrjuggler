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


#ifndef _VJ_PROXY_H_
#define _VJ_PROXY_H_

#include <vjConfig.h>
#include <Input/vjInput/vjInput.h>
#include <Config/vjConfigChunk.h>
class vjInput;

//: Base class for all input proxies
// TODO rename to vjInputProxy
//
//!PUBLIC_API:
class vjProxy
{
public:
   vjProxy()
   { mStupified = true;}

   virtual ~vjProxy() {;}

   //: Configure the proxy
   //! POST:
   //+  Proxy is configured (it is not registered yet though)
   //! RETURNS: success
   virtual bool config(vjConfigChunk* chunk)
   {
      mName = (std::string)chunk->getProperty("name");
      return true;
   }

   virtual void updateData()
   {;}

   //: Return a pointer to the base class of the devices being proxied
   virtual vjInput* getProxiedInputDevice() = 0;

   //: Returns the string rep of the chunk type used to config this device
   // Used by input manager to find chunks that construct devices
   static std::string getChunkType() { return "Undefined"; }

   std::string getName()
   { return mName; }

   void stupify(bool newState = true)
   { mStupified = newState; }

protected:
   std::string mName;         // The name of the proxy
   bool        mStupified;    // Is the proxy current stupified (returns default data)
};

#endif
