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

package org.vrjuggler.vrjconfig.customeditors.cave;

import java.awt.Dimension;
import java.awt.geom.Point2D;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Vector;
import org.vrjuggler.jccl.config.*;
import org.vrjuggler.jccl.config.event.*;
import org.vrjuggler.vrjconfig.commoneditors.EditorConstants;

public class CaveModel
{
   private List mWalls = new ArrayList();
   private ConfigContext mConfigContext = null;
   private BrokerChangeListener mBrokerChangeListener = null;
   private Vector mScreens = new Vector();
   private Map mScreenToNodeMap = new HashMap();
   private Map mViewToScreenMap = new HashMap();

   public Vector getScreens()
   {
      return mScreens;
   }

   public Map getScreenToNodeMap()
   {
      return mScreenToNodeMap;
   }
   
   public Map getViewToScreenMap()
   {
      return mViewToScreenMap;
   }
   
   public CaveModel(ConfigContext ctx)
   {
      mConfigContext = ctx;

      ConfigBrokerProxy broker = new ConfigBrokerProxy();
      List elements = broker.getElements(mConfigContext);
      
      List display_windows = ConfigUtilities.getElementsWithDefinition(elements, "display_window");
      List cluster_nodes = ConfigUtilities.getElementsWithDefinition(elements, "cluster_node");
      
      //System.out.println("Display Windows: " + display_windows);
      //System.out.println("Cluster Nodes: " + cluster_nodes);
      
      List walls = new ArrayList();

      // Add all surface viewports for the local machine.
      for (Iterator itr = display_windows.iterator() ; itr.hasNext(); )
      {
         ConfigElement win = (ConfigElement)itr.next();

         // Add Screen
         mScreens.addElement(win);
         mScreenToNodeMap.put(win, null);
      
         List views = win.getPropertyValues("surface_viewports");
         
         for (Iterator v_itr = views.iterator() ; v_itr.hasNext() ; )
         {
            ConfigElement view = (ConfigElement)v_itr.next();
            walls.add(view);
            mViewToScreenMap.put(view, win);
         }
      }
      
      // Add all surface viewports for each node.
      for (Iterator node_itr = cluster_nodes.iterator() ; node_itr.hasNext(); )
      {
         ConfigElement node = (ConfigElement)node_itr.next();
         List windows = node.getPropertyValues("display_windows");
         
         for (Iterator winds = windows.iterator() ; winds.hasNext(); )
         {
            ConfigElement win = (ConfigElement)winds.next();
            
            // Add Screen
            mScreens.addElement(win);
            mScreenToNodeMap.put(win, node);
            
            List views = win.getPropertyValues("surface_viewports");
         
            for (Iterator v_itr = views.iterator() ; v_itr.hasNext() ; )
            {
               ConfigElement view = (ConfigElement)v_itr.next();
               walls.add(view);
               mViewToScreenMap.put(view, win);
            }
         }
      }
      
      MultiMap sorter = new MultiMap();
      
      // Sort all view-screen pairs depending on their corner values.
      for (Iterator itr = walls.iterator() ; itr.hasNext() ; )
      {
         ConfigElement view = (ConfigElement)itr.next();
         // Grab the corner values out of the view.
         Corners c = new Corners(view);
         sorter.addValue(c, view);
      }
      
      // Create new CaveWall structure for each surface.
      for (Iterator itr = sorter.getKeys().iterator() ; itr.hasNext() ; )
      {
         Corners new_corners = (Corners)itr.next();
         CaveWall new_wall = new CaveWall(this, new_corners, (List)sorter.getValues(new_corners));
         mWalls.add(new_wall);
      }

      mBrokerChangeListener = new BrokerChangeListener(this);
      broker.addConfigListener( mBrokerChangeListener );
   }

   public List getWalls()
   {
      return mWalls;
   }

   public ConfigContext getConfigContext()
   {
      return mConfigContext;
   }

   public void changeScreenForView(ConfigElement view, ConfigElement new_screen)
   {
      ConfigElement old_screen = (ConfigElement)mViewToScreenMap.remove(view);
      mViewToScreenMap.put(view, new_screen);
      
      int result = old_screen.removeProperty(EditorConstants.surface_viewports_prop,
                                             view, mConfigContext);
      if (-1 == result)
      {
         System.out.println("ERROR: Trying to move a view from a screen that does not contain it.");
      }
      else
      {
         new_screen.addProperty(EditorConstants.surface_viewports_prop, view, mConfigContext);
      }
   }
   
   public void setViewPosX(ConfigElement view, int pos_x)
   {
      if (pos_x < 0)
      {
         pos_x = 0;
      }

      ConfigElement screen = (ConfigElement)mViewToScreenMap.get( view );
      updateScreenSize(screen);
   }
   
   public void updateScreenSize(ConfigElement screen)
   {
      Map v_low_map = new HashMap();
      Map v_high_map = new HashMap();

      Point2D.Float min = new Point2D.Float(Float.POSITIVE_INFINITY, Float.POSITIVE_INFINITY);
      Point2D.Float max = new Point2D.Float(Float.NEGATIVE_INFINITY, Float.NEGATIVE_INFINITY);
      
      List views = screen.getPropertyValues("surface_viewports");
      int s_origin_x  = ((Integer) screen.getProperty("origin", 0)).intValue();
      int s_origin_y  = ((Integer) screen.getProperty("origin", 1)).intValue();
      int s_size_x  = ((Integer) screen.getProperty("size", 0)).intValue();
      int s_size_y  = ((Integer) screen.getProperty("size", 1)).intValue();
      for (Iterator v_itr = views.iterator() ; v_itr.hasNext() ; )
      {
         ConfigElement view = (ConfigElement)v_itr.next();
         float v_origin_x  = ((Float) view.getProperty("origin", 0)).floatValue();
         float v_origin_y  = ((Float) view.getProperty("origin", 1)).floatValue();
         float v_size_x  = ((Float) view.getProperty("size", 0)).floatValue();
         float v_size_y  = ((Float) view.getProperty("size", 1)).floatValue();
         
         Point2D.Float lower = new Point2D.Float();
         Point2D.Float upper = new Point2D.Float();
         lower.x = s_origin_x + (s_size_x * v_origin_x);
         lower.y = s_origin_y + (s_size_y * v_origin_y);
         
         upper.x = lower.x + (s_size_x * v_size_x);
         upper.y = lower.y + (s_size_y * v_size_y);

         System.out.println("lower: " + lower);
         System.out.println("upper: " + upper);
         
         v_low_map.put(view, lower);
         v_high_map.put(view, upper);

         if (lower.x < min.x)
         {
            min.x = lower.x;
         }
         if (lower.y < min.y)
         {
            min.y = lower.y;
         }
         if (upper.x > max.x)
         {
            max.x = upper.x;
         }
         if (upper.y > max.y)
         {
            max.y = upper.y;
         }
      }
      
      System.out.println("min: " + min);
      System.out.println("max: " + max);
      
      screen.setProperty("origin", 0, new Integer((int)min.x));
      screen.setProperty("origin", 1, new Integer((int)min.y));
      screen.setProperty("size", 0, new Integer((int)(max.x-min.x)));
      screen.setProperty("size", 1, new Integer((int)(max.y-min.y)));
      
      for (Iterator v_itr = views.iterator() ; v_itr.hasNext() ; )
      {
         ConfigElement view = (ConfigElement)v_itr.next();
         
         Point2D.Float low = (Point2D.Float)v_low_map.get(view);
         Point2D.Float high = (Point2D.Float)v_high_map.get(view);
         
         System.out.println("low: " + low);
         System.out.println("upp: " + high);

         float v_origin_x = (low.x - min.x)/(max.x - min.x);
         float v_origin_y = (low.y - min.y)/(max.y - min.y);

         float v_size_x = (high.x - low.x)/(max.x - min.x);
         float v_size_y = (high.y - low.y)/(max.y - min.y);
         
         screen.setProperty("origin", 0, new Float(v_origin_x));
         screen.setProperty("origin", 1, new Float(v_origin_y));
         screen.setProperty("size", 0, new Float(v_size_x));
         screen.setProperty("size", 1, new Float(v_size_y));
      }
   }
   public void setViewPosY(ConfigElement view, int pos_y)
   {
   }
   
   public void setViewWidth(ConfigElement view, int width)
   {
   }

   public void setViewHeight(ConfigElement view, int height)
   {
   }
   
   /**
    * Custom listener for changes to the config broker.
    */
   private class BrokerChangeListener
      implements ConfigListener
   {
      private CaveModel mCaveModel = null;

      public BrokerChangeListener(CaveModel cm)
      {
         mCaveModel = cm;
      }

      public void configElementAdded(ConfigEvent evt)
      {
         /*
         if (mConfigContext.contains(evt.getResource()))
         {
            ConfigElement elm = evt.getElement();
            List walls = new ArrayList();

            if ( elm.getDefinition().getToken().equals(EditorConstants.display_window_type) )
            {


               // Add all surface viewports for the local machine.
               List views = elm.getPropertyValues("surface_viewports");
               
               for (Iterator v_itr = views.iterator() ; v_itr.hasNext() ; )
               {
                  ConfigElement view = (ConfigElement)v_itr.next();
                  walls.add(view);
                  mViewToScreenMap.put(view, elm);
               }
            }
            else if ( elm.getDefinition().getToken().equals(EditorConstants.cluster_node_type) )
            {
               
               // Add all surface viewports for each node.
               List windows = elm.getPropertyValues("display_windows");
               
               for (Iterator winds = windows.iterator() ; winds.hasNext(); )
               {
                  ConfigElement win = (ConfigElement)winds.next();
                  List views = win.getPropertyValues("surface_viewports");
               
                  for (Iterator v_itr = views.iterator() ; v_itr.hasNext() ; )
                  {
                     ConfigElement view = (ConfigElement)v_itr.next();
                     walls.add(view, win);
                     mViewToScreenMap.put(view, win);
                  }
               }
            }

            // XXX: Do something with this.
         }
         */
      }

      public void configElementRemoved(ConfigEvent evt)
      {
         if (mConfigContext.contains(evt.getResource()))
         {
            /* XXX: remove
            ConfigElement elm = evt.getElement();
            if ( elm.getDefinition().getToken().equals(EditorConstants.display_window_type) )
            {
               Object obj = mScreens.remove( elm );
               if ( null != obj )
               {
                  mScreenDisplay.remove( (Component)obj );
                  mScreenDisplay.revalidate();
                  mScreenDisplay.repaint();
               }
            }
            */
         }
      }
   }
}
