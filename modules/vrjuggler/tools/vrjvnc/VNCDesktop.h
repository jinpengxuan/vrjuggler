/*************** <auto-copyright.pl BEGIN do not edit this line> **************
 *
 * VRJ VNC is (C) Copyright 2003 by Iowa State University
 *
 * Original Author:
 *   Patrick Hartling
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this application; if not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 *
 * -----------------------------------------------------------------
 * File:          $RCSfile$
 * Date modified: $Date$
 * Version:       $Revision$
 * -----------------------------------------------------------------
 *
 *************** <auto-copyright.pl END do not edit this line> ***************/

#ifndef _VRJVNC_VNC_DESKTOP_H_
#define _VRJVNC_VNC_DESKTOP_H_

#include <stdlib.h>
#include <string>
#include <gmtl/Matrix.h>
#include <gmtl/AABox.h>
#include <gmtl/Point.h>
#include <gmtl/Vec.h>
#include <gmtl/Ray.h>
#include <vpr/vpr.h>
#include <gadget/Type/PositionInterface.h>
#include <gadget/Type/DigitalInterface.h>
#include <gadget/Type/KeyboardInterface.h>
#include <vector>

#include <VNCInterface.h>


namespace vrjvnc
{

class VNCDesktop
{
public:
   enum Focus
   {
      IN_FOCUS,     /**< */
      NOT_IN_FOCUS  /**< */
   };

   VNCDesktop(const std::string& hostname, const vpr::Uint16& port,
              const std::string& password, const float& desktopSideLength);

   ~VNCDesktop();

   void init(const std::string& wandName, const std::string& leftButtonName,
             const std::string& middleButtonName,
             const std::string& rightButtonName,
             const std::string& keyboardName = std::string(""));

   Focus update(const gmtl::Matrix44f& navMatrix);

   /**
    * Renders the desktop.
    *
    * @pre The navigation matrix is on the stack.
    */
   void draw();

protected:     // Helpers
   /** Current state of selection */
   enum Selection
   {
      Nothing,
      LRCornerSelect, URCornerSelect, LLCornerSelect, ULCornerSelect,
      TopBorderSelect, BottomBorderSelect, LeftBorderSelect, RightBorderSelect,
      GrabBegin,
      LRCornerGrab, URCornerGrab, LLCornerGrab, ULCornerGrab,
      TopBorderGrab, BottomBorderGrab, LeftBorderGrab, RightBorderGrab,
      GrabEnd
   };

   /** Updates the desktop parameters
   * @pre mDesktopWidth, mDesktopHeight, and pos matrix are update
   * @post Everything else is updated correspondingly
   */
   void updateDesktopParameters();


   void drawSphere(float radius, gmtl::Point3f offset, int parts=4);

   void drawBox(const gmtl::AABoxf& box);

   inline void setColorIfState(gmtl::Vec3f true_color, gmtl::Vec3f false_color, Selection state1, Selection state2)
   {
      if((mSelectState == state1) || (mSelectState == state2))
         glColor3fv( true_color.mData );
      else
         glColor3fv( false_color.mData);
   }


private:
   VNCInterface                         mVncIf;
   vpr::ThreadRunFunctor<VNCInterface>* mVncThreadFunctor;
   vpr::Thread*                         mVncThread;

   gadget::PositionInterface mWand;
   gadget::DigitalInterface  mLeftButton;
   gadget::DigitalInterface  mMiddleButton;
   gadget::DigitalInterface  mRightButton;
   gadget::KeyboardInterface mKeyboard;

   bool mHaveKeyboard;

private:

   /** @name Desktop parameters
   * The desktop window (ie. the texture of the desktop) is assumed to be
   * centered on 0,0,0 in the local coordinate frame with
   */
   //@{
   float mDesktopWidth, mDesktopHeight;      /**< Width and height of the virtual desktop. VRJ units */
   float mMaxSize, mMinSize, mIncSize;
   int   mTexWidth, mTexHeight;             /**< Width and height of the texture in pixels */
   int   mVncWidth, mVncHeight;              /**< The width and height of the vnc/X desktop */

   /** Scale value for converting from virt desktop size to VNC coords
   *    ex. vnc.x = desktoppoly.x * mDesktopToVncWidthScale;
   */
   float          mDesktopToVncWidthScale;
   float          mDesktopToVncHeightScale;

   Selection        mSelectState;      /**< State of selection */

   /** Transform matrix of the desktop.
   * This is from world to desktop.
   * In only captures the local transformations (not any app nav matrix stuff)
   */
   gmtl::Matrix44f  m_world_M_desktop;

   gmtl::AABoxf     mDesktopBox;
   gmtl::Rayf       mWandRay;          /**< The wand ray to draw (clipped to length) */
   gmtl::Point3f    mIsectPoint;       /**< The point of intersection on the desktop plane */

   // Corners
   gmtl::AABoxf     mLLCorner;         /**< LL of the desktop */
   gmtl::AABoxf     mLRCorner;         /**< LR of the desktop */
   gmtl::AABoxf     mURCorner;         /**< UR of the desktop */
   gmtl::AABoxf     mULCorner;         /**< UL of the desktop */
   // Borders
   gmtl::AABoxf     mRightBorder;      /**< The right border of the desktop */
   gmtl::AABoxf     mLeftBorder;       /**< The left border of the desktop */
   gmtl::AABoxf     mTopBorder;        /**< The top border of the desktop */
   gmtl::AABoxf     mBottomBorder;     /**< The bottom border of the desktop */
   //@}

   bool mDesktopWandIsect;
   bool mDesktopGrabbed;

   char* mTextureData;

   // Debugging stuff
   GLUquadric*   mSphereQuad;       /**< Sphere rep for draw "points" */
};

}


#endif /*_VRJVNC_VNC_DESKTOP_H_ */
