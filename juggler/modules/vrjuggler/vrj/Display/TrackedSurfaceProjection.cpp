/*************** <auto-copyright.pl BEGIN do not edit this line> **************
 *
 * VR Juggler is (C) Copyright 1998-2005 by Iowa State University
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

#include <vrj/vrjConfig.h>

#include <iomanip>
#include <gmtl/Vec.h>
#include <gmtl/Matrix.h>
#include <gmtl/MatrixOps.h>
#include <gmtl/Generate.h>
#include <gmtl/Output.h>
#include <gmtl/Xforms.h>

#include <gadget/Type/Position/PositionUnitConversion.h>
#include <vrj/Display/TrackedSurfaceProjection.h>


namespace vrj
{

void TrackedSurfaceProjection::calcViewMatrix(gmtl::Matrix44f& eyePos, const float scaleFactor)
{
   updateSurfaceParams(scaleFactor);

   SurfaceProjection::calcViewMatrix(eyePos,scaleFactor);
}

void TrackedSurfaceProjection::updateSurfaceParams(const float scaleFactor)
{
   /* Here we rotate the original screen corners, and then have SurfaceProjection::calculateOffsets
   *  handle the calculation of mOriginToScreen, etc.
   */

   // Get the tracking data for the surface offset
   gmtl::Matrix44f trackerMatrix = mTracker->getData(scaleFactor);

   gmtl::Vec3f trans=gmtl::makeTrans<gmtl::Vec3f>(trackerMatrix);
   trans=trans/scaleFactor;
   gmtl::setTrans(trackerMatrix,trans);

   mLLCorner=trackerMatrix * mOrigionalLLCorner;
   mLRCorner=trackerMatrix * mOrigionalLRCorner;
   mURCorner=trackerMatrix * mOrigionalURCorner;
   mULCorner=trackerMatrix * mOrigionalULCorner;

   calculateOffsets();
}

std::ostream& TrackedSurfaceProjection::outStream(std::ostream& out,
                                                  const unsigned int indentLevel)
{
   const int pad_width_dot(20 - indentLevel);
   out.setf(std::ios::left);

   const std::string indent_text(indentLevel, ' ');

   out << indent_text << "vrj::TrackedSurfaceProjection:\n";
   out << indent_text << std::setw(pad_width_dot)
       << "  Pos Proxy " << " " << mTracker.getProxyName() << std::endl;

   return SurfaceProjection::outStream(out, indentLevel);
}

}