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

#ifndef _VRJ_WALL_PROJECTION_H_
#define _VRJ_WALL_PROJECTION_H_

#include <vrj/vrjConfig.h>
#include <vrj/Display/Projection.h>
#include <gmtl/MatrixOps.h>
#include <gmtl/Point.h>

namespace vrj
{

class Matrix;

/**
 * Surface specific class for projection definitions.
 *
 * Responsible for storing and computing projection information of a surface
 * specified.
 */
class SurfaceProjection : public Projection
{
public:
   /**
   * @param surfaceRot - The rotation of the surface relative to the base world
   *                     base_M_surface
   */
   SurfaceProjection(gmtl::Point3f llCorner, gmtl::Point3f lrCorner,
                    gmtl::Point3f urCorner, gmtl::Point3f ulCorner)
   {
      mLLCorner=llCorner;
      mLRCorner=lrCorner;
      mURCorner=urCorner;
      mULCorner=ulCorner;

      calculateOffsets();
   }

   /** Configures the projection using the element given. */
   virtual void config(jccl::ConfigElementPtr element);

   /**
    * Recalculates the projection matrix.
    * @pre WallRotation matrix must be set correctly.
    *      mOrigin*'s must all be set correctly.
    * @pre eyePos is scaled by position factor.
    * @pre scaleFactor is the scale current used
    * @post frustum has been recomputed for given eyePos.
    */
   virtual void calcViewMatrix(gmtl::Matrix44f& eyePos, const float scaleFactor);

   /**
    * Calculates the frustum needed for the view matrix.
    * @note This function is called as part of calcViewMatrix.
    */
   virtual void calcViewFrustum(gmtl::Matrix44f& eyePos, const float scaleFactor);

   std::ostream& outStream(std::ostream& out,
                           const unsigned int indentLevel = 0);

protected:
   /** Checks the pts to make sure they form a legal surface. */
   void assertPtsLegal()
   {
      gmtl::Vec3f norm1, norm2;
      gmtl::Vec3f bot_side = mLRCorner-mLLCorner;
      gmtl::Vec3f diag = mULCorner-mLRCorner;
      gmtl::Vec3f right_side = mURCorner-mLRCorner;
      gmtl::cross(norm1, bot_side, diag);
      gmtl::cross(norm2, bot_side, right_side);
      gmtl::normalize( norm1 ); gmtl::normalize(norm2);
      if(gmtl::isEqual(norm1,norm2,1e-4f)==false){
         vprDEBUG(vprDBG_ERROR, vprDBG_CRITICAL_LVL) << "ERROR: Invalid surface corners.\n" << vprDEBUG_FLUSH;
      }
   }

   /** These calculate mOriginToScreen, etc, from the screen corners.
   * calculateOffsets requires that mLLCorner to mULCorner be set correctly, and it will
   * handle calling calculateSurfaceRotation & calculateCornersInBaseFrame
   */
   void calculateOffsets();
   void calculateSurfaceRotation();
   void calculateCornersInBaseFrame();

   /* Coordinate system descriptions
   * Base - B - Base coordinate system of the real physical world
   * Surface - S - Cordinate frame that is aligned with the surface.  This is the coordinates that
   *                the surface and frustum (projection) parameters are stored in/relative to.
   * Surface Transform - ST - Coordinate frame that offsets the Surface in space.  Only used for tracked surfaces.
   * Eye - E - Coordinate system that is aligned with the eye.  This is the frame that the frustum
   *              is drawn in for final projection.  This is where the "view" starts from.
   */

   gmtl::Matrix44f mSurfaceRotation;	/** Same as m_base_M_surface */

   /** Rotation of the surface
   * Xfrom from the Base to the surface
   */
   gmtl::Matrix44f   m_base_M_surface;
   gmtl::Matrix44f   m_surface_M_base;    /** Stored inverse for performance */

   /** Screen configuration (in Surface coordinate frame )
   */
   gmtl::Point3f mLLCorner, mLRCorner, mURCorner, mULCorner;
   gmtl::Point3f  mxLLCorner, mxLRCorner, mxURCorner, mxULCorner;    /**< The corners transformed onto an x,y plane */
   float mOriginToScreen, mOriginToRight, mOriginToLeft, mOriginToTop, mOriginToBottom;
};

}

#endif