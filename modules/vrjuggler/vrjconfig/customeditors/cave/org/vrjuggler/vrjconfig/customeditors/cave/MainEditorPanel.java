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

import java.awt.BorderLayout;
import java.awt.Component;
import java.awt.Container;
import java.awt.Dialog;
import java.awt.Dimension;
import java.awt.FlowLayout;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.Point;
import java.awt.event.*;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.util.*;
import javax.swing.*;
import javax.swing.table.*;
import javax.swing.border.TitledBorder;
import javax.swing.event.*;
import org.vrjuggler.jccl.config.*;
import org.vrjuggler.jccl.editors.PropertyEditorPanel;
//import org.vrjuggler.vrjconfig.commoneditors.TransmitterTransformPanel;
import info.clearthought.layout.*;


public class MainEditorPanel
   extends JPanel
   implements ChangeListener
{
   private WallEditorPanel mWallPanel = new WallEditorPanel();
   private List mWalls = new ArrayList();
   
   private JPanel mStereoPanel = new JPanel();
   private JRadioButton mActiveStereoBtn = new JRadioButton();
   private JRadioButton mPassiveStereoBtn = new JRadioButton();
   private JRadioButton mMonoBtn = new JRadioButton();
   private ButtonGroup mStereoGroup = new ButtonGroup();

   private JPanel mViewPanel = new JPanel();
   private ViewEditorPanel mLeftViewEditorPanel = null;
   private ViewEditorPanel mRightViewEditorPanel = null;
   
   private TitledBorder mWallBorder;
   private TitledBorder mStereoBorder;
   private TitledBorder mViewBorder;
   private CaveWall mCaveWall = null;
   private CaveModel mCaveModel = null;

   private Icon[] mRedIcons = new ImageIcon[3];
   private Icon[] mGreenIcons = new ImageIcon[3];
   
   public void setWallConfig(CaveModel cm, CaveWall in_wall)
   {
      mCaveModel = cm;
      mCaveWall = in_wall;
      
      try
      {
         jbInit();
      }
      catch (Exception ex)
      {
         ex.printStackTrace();
      }
      
      // Ensure that we remove the old wall editor so that it can be garbage collected.
      this.remove(mWallPanel);
      mWallPanel = new WallEditorPanel(mCaveWall);
      mWallPanel.setBorder(mWallBorder);
      this.add(mWallPanel, new TableLayoutConstraints(0, 0, 0, 1,
                                       TableLayout.FULL,
                                       TableLayout.FULL));

      
      mLeftViewEditorPanel = new ViewEditorPanel(mCaveModel, new Dimension(200, 200), "Left Eye");
      mViewPanel.add(mLeftViewEditorPanel, new TableLayoutConstraints(0, 0, 0, 0,
                                               TableLayout.FULL,
                                               TableLayout.FULL));
      
      mRightViewEditorPanel = new ViewEditorPanel(mCaveModel, new Dimension(200, 200), "Right Eye");
      mViewPanel.add(mRightViewEditorPanel, new TableLayoutConstraints(0, 1, 0, 1,
                                               TableLayout.FULL,
                                               TableLayout.FULL));
      
      updateStereoGUI();
      updateViewGUI();

      this.revalidate();
      this.repaint();
   }



   public MainEditorPanel()
   {
      /*
      try
      {
         jbInit();
      }
      catch(Exception ex)
      {
         ex.printStackTrace();
      }
      */
   }
   
   private Container getOwner()
   {
      Object owner = SwingUtilities.getRoot(this);
      System.out.println("owner class: " + owner.getClass());
      return (Container) owner;
   }
   
   /**
    * Positions the given Dialog object relative to this window frame.
    */
   private void positionDialog(Dialog dialog, Container parent)
   {
      Dimension dlg_size   = dialog.getPreferredSize();
      Dimension frame_size = parent.getSize();
      Point loc            = parent.getLocation();

      // Set the location of the dialog so that it is centered with respect
      // to this frame.
      dialog.setLocation((frame_size.width - dlg_size.width) / 2 + loc.x,
                         (frame_size.height - dlg_size.height) / 2 + loc.y);
   }
   
   public void stateChanged(ChangeEvent e)
   {
      /*
      JSpinner source = (JSpinner) e.getSource();

      // Using the object returned by source.getValue() seems to be safe
      // because every change in the spinner's value creates a new object.
      if ( source == mReportRateSpinner )
      {
         mElement.setProperty("report_rate", 0, source.getValue());
      }
      else if ( source == mMeasurementRateSpinner )
      {
         // NOTE: The object returned is of type Double rather than Float.
         mElement.setProperty("measurement_rate", 0, source.getValue());
      }
      */
   }
   
   void jbInit() throws Exception
   {
      String resourceBase = "org/vrjuggler/vrjconfig/customeditors/cave";
      String imageBase = resourceBase + "/images";

      ClassLoader loader = getClass().getClassLoader();
      
      try
      {
         mRedIcons[0] = new ImageIcon(loader.getResource(imageBase + "/one_red.png"));
         mRedIcons[1] = new ImageIcon(loader.getResource(imageBase + "/two_red.png"));
         mRedIcons[2] = new ImageIcon(loader.getResource(imageBase + "/three_red.png"));
         
         mGreenIcons[0] = new ImageIcon(loader.getResource(imageBase + "/one_green.png"));
         mGreenIcons[1] = new ImageIcon(loader.getResource(imageBase + "/two_green.png"));
         mGreenIcons[2] = new ImageIcon(loader.getResource(imageBase + "/three_green.png"));
         
         mWallBorder = new TitledIconBorder("Physical Characteristics", mGreenIcons[0]);
         mViewBorder = new TitledIconBorder("View", mRedIcons[2]);
         mStereoBorder = new TitledIconBorder("Stereo/Mono", mRedIcons[1]);
      }
      catch (NullPointerException ex)
      {
      }
      
      mActiveStereoBtn.setText("Active Stereo Display");
      mPassiveStereoBtn.setText("Passive Stereo Display");
      mMonoBtn.setText("Mono Display");

      mActiveStereoBtn.addActionListener(new ActionListener()
            {
               public void actionPerformed(ActionEvent e)
               {
                  mCaveWall.setStereoMode(CaveWall.ACTIVE_STEREO);
                  updateViewGUI();
               }
            });
      
      mPassiveStereoBtn.addActionListener(new ActionListener()
            {
               public void actionPerformed(ActionEvent e)
               {
                  mCaveWall.setStereoMode(CaveWall.PASSIVE_STEREO);
                  updateViewGUI();
               }
            });
      
      mMonoBtn.addActionListener(new ActionListener()
            {
               public void actionPerformed(ActionEvent e)
               {
                  mCaveWall.setStereoMode(CaveWall.MONO);
                  updateViewGUI();
               }
            });


      
      mWallPanel.setBorder(mWallBorder);
      
      double[][] main_size = {{TableLayout.PREFERRED, TableLayout.PREFERRED},
                              {TableLayout.PREFERRED, TableLayout.PREFERRED}};
      
      this.setLayout(new TableLayout(main_size));
      
      this.add(mWallPanel, new TableLayoutConstraints(0, 0, 0, 1,
                                       TableLayout.FULL,
                                       TableLayout.FULL));

      this.add(mViewPanel, new TableLayoutConstraints(1, 1, 1, 1,
                                               TableLayout.FULL,
                                               TableLayout.FULL));
      
      double[][] vp_size = {{TableLayout.PREFERRED},
                            {TableLayout.PREFERRED, TableLayout.PREFERRED}};
      mViewPanel.setLayout(new TableLayout(vp_size));
      mViewPanel.setBorder(mViewBorder);
      
      this.add(mStereoPanel, new TableLayoutConstraints(1, 0, 1, 0,
                                               TableLayout.FULL,
                                               TableLayout.FULL));

      double[][] sp_size = {{TableLayout.PREFERRED},
                            {TableLayout.PREFERRED, TableLayout.PREFERRED, TableLayout.PREFERRED}};
      mStereoPanel.setLayout(new TableLayout(sp_size));
      mStereoPanel.setBorder(mStereoBorder);
      
      mStereoGroup.add(mActiveStereoBtn);
      mStereoGroup.add(mPassiveStereoBtn);
      mStereoGroup.add(mMonoBtn);
      
      mStereoPanel.add(mActiveStereoBtn, new TableLayoutConstraints(0, 0, 0, 0,
                                         TableLayout.FULL,
                                         TableLayout.FULL));;
      mStereoPanel.add(mPassiveStereoBtn, new TableLayoutConstraints(0, 1, 0, 1,
                                          TableLayout.FULL,
                                          TableLayout.FULL));
      mStereoPanel.add(mMonoBtn, new TableLayoutConstraints(0, 2, 0, 2,
                                 TableLayout.FULL,
                                 TableLayout.FULL));
   }

   private void updateStereoGUI()
   {
      if (CaveWall.ACTIVE_STEREO == mCaveWall.getStereoMode())
      {
         mActiveStereoBtn.setSelected(true);
      }
      else if (CaveWall.PASSIVE_STEREO == mCaveWall.getStereoMode())
      {
         mPassiveStereoBtn.setSelected(true);
      }
      else
      {
         mMonoBtn.setSelected(true);
      }
   }

   private void updateViewGUI()
   {
      if (CaveWall.ACTIVE_STEREO == mCaveWall.getStereoMode())
      {
         mLeftViewEditorPanel.setConfig(mCaveWall.getStereo());
         mRightViewEditorPanel.setConfig(null);
         
         mActiveStereoBtn.setSelected(true);
         mLeftViewEditorPanel.setTitle("Stereo");
         mRightViewEditorPanel.setTitle("");
         mRightViewEditorPanel.setEnabled(false);
      }
      else if (CaveWall.PASSIVE_STEREO == mCaveWall.getStereoMode())
      {
         mLeftViewEditorPanel.setConfig(mCaveWall.getLeft());
         mRightViewEditorPanel.setConfig(mCaveWall.getRight());
         
         mLeftViewEditorPanel.setTitle("Left Eye");
         mRightViewEditorPanel.setTitle("Right Eye");
         mRightViewEditorPanel.setEnabled(true);
      }
      else
      {
         mLeftViewEditorPanel.setConfig(mCaveWall.getLeft());
         mRightViewEditorPanel.setConfig(null);
         
         mLeftViewEditorPanel.setTitle("Mono");
         mRightViewEditorPanel.setTitle("");
         mRightViewEditorPanel.setEnabled(false);
      }
      
      this.revalidate();
      this.repaint();
   }
   
   class MyCellRenderer extends JLabel implements ListCellRenderer
   {
      // This is the only method defined by ListCellRenderer.
      // We just reconfigure the JLabel each time we're called.

      public Component getListCellRendererComponent(JList list,
                                                    Object value,            // value to display
                                                    int index,               // cell index
                                                    boolean isSelected,      // is the cell selected
                                                    boolean cellHasFocus)    // the list and the cell have the focus
      {

         Component cmp = (Component)value;
         if (isSelected)
         {
            cmp.setBackground(list.getSelectionBackground());
            cmp.setForeground(list.getSelectionForeground());
         }
         else
         {
            cmp.setBackground(list.getBackground());
            cmp.setForeground(list.getForeground());
         }
         //cmp.setEnabled(list.isEnabled());
         //cmp.setFont(list.getFont());
         ((JComponent)cmp).setOpaque(true);
         ((JComponent)cmp).setMinimumSize(new Dimension(200, 200));
         ((JComponent)cmp).setPreferredSize(new Dimension(200, 200));
         ((JComponent)cmp).setMaximumSize(new Dimension(200, 200));

         return cmp;
      }
   }
}
