/*************** <auto-copyright.pl BEGIN do not edit this line> **************
 *
 * VR Juggler is (C) Copyright 1998-2003 by Iowa State University
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

#ifndef _JCCL_PARAM_H_
#define _JCCL_PARAM_H_


/*
 * Define to the 9-digit JCCL version integer.  This is the "internal version
 * number" that is intended for internal feature checks by JCCL itself.
 */
#define __JCCL_version 14000

/*
 * This is the "human-readable" JCCL version _string_.  It is of the form
 * <major>.<minor>.<days since last major release>.
 */
namespace jccl
{
   const char* JCCL_VERSION  = "v0.14.0-0 'Bride of Chucky' (@SUBSYSTEM@) HEAD Nov 15, 2003 12:44:46";
}


#endif	/* _JCCL_PARAM_H_ */
