/***************** <Tweek heading BEGIN do not edit this line> ****************
 * Tweek
 *
 * -----------------------------------------------------------------
 * File:          $RCSfile$
 * Date modified: $Date$
 * Version:       $Revision$
 * -----------------------------------------------------------------
 ***************** <Tweek heading END do not edit this line> *****************/

/*************** <auto-copyright.pl BEGIN do not edit this line> **************
 *
 * VR Juggler is (C) Copyright 1998, 1999, 2000, 2001 by Iowa State University
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
 *************** <auto-copyright.pl END do not edit this line> ***************/

package org.vrjuggler.tweek.gui;

import java.awt.*;
import java.awt.event.*;
import java.net.InetAddress;
import java.util.Iterator;
import java.util.List;
import java.util.Vector;
import javax.swing.*;
import javax.swing.filechooser.FileFilter;
import javax.swing.tree.DefaultTreeModel;
import javax.swing.tree.DefaultMutableTreeNode;
import org.vrjuggler.tweek.TweekCore;
import org.vrjuggler.tweek.beans.*;
import org.vrjuggler.tweek.beans.loader.BeanJarClassLoader;
import org.vrjuggler.tweek.net.CommunicationEvent;
import org.vrjuggler.tweek.net.CommunicationListener;
import org.vrjuggler.tweek.net.corba.*;
import org.vrjuggler.tweek.services.*;


/**
 * The mediator for the whole GUI.  This holds an instance of
 * org.vrjuggler.tweek.BeanContainer which in turn holds the JavaBeans loaded
 * at runtime.  Events generated by this frame are dispatched to the Beans
 * through the org.vrjuggler.tweek.BeanContainer instance.
 *
 * @version $Revision$
 *
 * @see org.vrjuggler.tweek.BeanContainer
 */
public class TweekFrame extends JFrame implements BeanFocusChangeListener,
                                                  MessageAdditionListener,
                                                  BeanInstantiationListener
{
   public TweekFrame ()
   {
      enableEvents(AWTEvent.WINDOW_EVENT_MASK);
      BeanInstantiationCommunicator.instance().addBeanInstantiationListener(this);
   }

   public void setBeanViewer (String viewer)
   {
      BeanRegistry registry = BeanRegistry.instance();
      if ( viewer != null )
      {
         mBeanViewer = (ViewerBean)registry.getBean( viewer );

         if ( mBeanViewer != null )
         {
            BeanModelViewer bv = mBeanViewer.getViewer();
            mBeanContainer.replaceViewer(bv);
         }
         else
         {
            MessagePanel.instance().printWarning("WARNING: Unknown viewer type: '" + viewer + "'\n");
            List viewers = registry.getBeansOfType(ViewerBean.class.getName());

            if ( viewers.size() > 0 )
            {
               mBeanViewer = (ViewerBean) viewers.get(0);
               mBeanContainer.replaceViewer((BeanModelViewer)mBeanViewer.getViewer() );
            }
            else
            {
               MessagePanel.instance().printWarning("WARNING: No Viewer Beans loaded");
               mBeanViewer = null;
            }
         }
      }
      else
      {
         mBeanViewer = (ViewerBean)registry.getBeansOfType(ViewerBean.class.getName()).get(0);
         mBeanContainer.replaceViewer((BeanModelViewer)mBeanViewer.getViewer());
      }
   }

   public ViewerBean getBeanViewer()
   {
      return mBeanViewer;
   }

   /**
    * Initializes the GUI.
    *
    * @pre The JavaBean search has been performed.
    */
   public void initGUI ()
   {
      // This needs to be done as early as possible so that we receive events
      // that happen during initialization.
      MessagePanel.instance().addMessageAdditionListener(this);

      try
      {
         GlobalPreferencesService prefs =
            (GlobalPreferencesService)BeanRegistry.instance().getBean( "GlobalPreferences" );
         UIManager.setLookAndFeel( prefs.getLookAndFeel() );
         jbInit();
         mBeanPrefsDialog =
            new BeanPrefsDialog(this, "Bean-Specific Preferences Editor");
      }
      catch (Exception e)
      {
         e.printStackTrace();
      }

      // Validate frames that have preset sizes.
      // Pack frames that have useful preferred size info, e.g. from their
      // layout.
      if ( mPackFrame )
      {
         this.pack();
      }
      else
      {
         this.validate();
      }

      this.setVisible(true);
   }

   /**
    * Handles a BeanFocusChangeEvent.  The change in Bean focus gives us the
    * opportunity to manipulate context-specific (Bean-specific) information.
    * This includes context-specific file loading.  When a new Bean is focuesd,
    * it is queried to determine if it implements
    * org.vrjuggler.tweek.beans.FileLoader.  If so, the Open item in the File
    * menu is enabled, and the newly focused Bean is set up to receive open
    * requests from the GUI.
    */
   public void beanFocusChanged (BeanFocusChangeEvent e)
   {
      if ( e.getFocusType() == BeanFocusChangeEvent.BEAN_FOCUSED )
      {
         MessagePanel.instance().printStatus("Bean " + e.getBean() +
                                             " focused!\n");

         Object panel_bean = e.getBean().getBean();

         // If the Panel Bean supports file opening, enable mMenuFileOpen and
         // set the Bean as the current file loader.  If and when the user
         // selects the Open item in the File menu, panel_bean will be informed
         // of the open request via the FileLoader interface.
         if ( panel_bean instanceof FileLoader )
         {
            FileLoader loader = (FileLoader) panel_bean;
            mMenuFileOpen.setText("Open " + loader.getFileType() + " ...");
            mMenuFileClose.setText("Close " + loader.getFileType() + " ...");
            mMenuFileOpen.setEnabled(true);
            mMenuFileSave.setEnabled(loader.canSave());

            if ( loader.getOpenFileCount() > 0 )
            {
               mMenuFileClose.setEnabled(true);
            }

            mFileLoader = loader;
         }
         // This new Bean can't open files, we we make sure to disable the
         // Open menu item.
         else
         {
            disableFileHandlingItems();
         }
      }
      else if ( e.getFocusType() == BeanFocusChangeEvent.BEAN_UNFOCUSED )
      {
         MessagePanel.instance().printStatus("Bean " + e.getBean() +
                                             " unfocused!\n");

         // Disable the Open menu item in the File menu just to be safe.
         disableFileHandlingItems();
      }
   }

   public void messageAdded (MessageAdditionEvent e)
   {
      if ( mBulbOnIcon != null )
      {
         mStatusMsgButton.setIcon(mBulbOnIcon);
      }
      else
      {
         mStatusMsgButton.setForeground(Color.red);
      }

      // If the user's skill level is below intermediate, give them a hint that
      // there is a message printed in the message panel.  They may not have
      // noticed the icon change made above.
      GlobalPreferencesService prefs =
         (GlobalPreferencesService)BeanRegistry.instance().getBean( "GlobalPreferences" );
      if ( prefs.getUserLevel() <= 5 )
      {
         mStatusMsgLabel.setText("New message in message panel ");
      }
   }

   /**
    * @post If the instantiated Bean implements the BeanPreferences
    *       interface, its preferences are loaded, and its editor is added to
    *       the Bean-specified editor dialog.
    */
   public void beanInstantiated (BeanInstantiationEvent e)
   {
      Object new_bean = ((TweekBean) e.getBean()).getBean();

      if ( new_bean instanceof BeanPreferences )
      {
         try
         {
            ((BeanPreferences) new_bean).load();
         }
         catch (java.io.IOException io_ex)
         {
            MessagePanel.instance().printWarning("Failed to load preferences for " +
                                                 ((TweekBean) e.getBean()).getName() +
                                                 ": " + io_ex.getMessage());
         }

         mBeanPrefsDialog.addPrefsBean((BeanPreferences) new_bean);
         mMenuPrefsBeanEdit.setEnabled(true);
      }
   }

   // =========================================================================
   // Private methods.
   // =========================================================================

   /**
    * Performes the real GUI intialization.  This method is needed for JBuilder
    * to be happy.
    */
   private void jbInit () throws Exception
   {
      GlobalPreferencesService prefs =
         (GlobalPreferencesService)BeanRegistry.instance().getBean( "GlobalPreferences" );
      setBeanViewer( prefs.getBeanViewer() );

      mContentPane = (JPanel) this.getContentPane();
      mContentPane.setLayout(mContentPaneLayout);
      this.setSize(new Dimension(1024, 768));
      this.setTitle("Tweek JavaBean Loader");

      // Define the Connect option in the Network menu.
      mMenuNetConnect.setMnemonic('C');
      mMenuNetConnect.setText("Connect to ORB ...");
      mMenuNetConnect.setAccelerator(javax.swing.KeyStroke.getKeyStroke(67, KeyEvent.CTRL_MASK | KeyEvent.SHIFT_MASK, false));
      mMenuNetConnect.addActionListener(new ActionListener()
         {
            public void actionPerformed (ActionEvent e)
            {
               networkConnectAction(e);
            }
         });

      // Define the Disconnect option in the Network menu.
      mMenuNetDisconnect.setEnabled(false);
      mMenuNetDisconnect.setMnemonic('D');
      mMenuNetDisconnect.setText("Disconnect from ORB ...");
      mMenuNetDisconnect.addActionListener(new ActionListener()
         {
            public void actionPerformed (ActionEvent e)
            {
               networkDisconnectAction(e);
            }
         });

      // Define the Edit Global option in the Preferences menu.
      mMenuPrefsGlobalEdit.setMnemonic('G');
      mMenuPrefsGlobalEdit.setText("Edit Global ...");
      mMenuPrefsGlobalEdit.addActionListener(new ActionListener()
         {
            public void actionPerformed (ActionEvent e)
            {
               prefsEditGlobal(e);
            }
         });

      // Define the Edit Local option in the Preferences menu.
      mMenuPrefsBeanEdit.setMnemonic('B');
      mMenuPrefsBeanEdit.setText("Edit Bean-Specific ...");
      mMenuPrefsBeanEdit.setEnabled(false);
      mMenuPrefsBeanEdit.addActionListener(new ActionListener()
         {
            public void actionPerformed (ActionEvent e)
            {
               prefsEditBean(e);
            }
         });

      mMenuBeansLoad.setMnemonic('L');
      mMenuBeansLoad.setText("Load Beans ...");
      mMenuBeansLoad.setAccelerator(javax.swing.KeyStroke.getKeyStroke(66, KeyEvent.CTRL_MASK, false));
      mMenuBeansLoad.addActionListener(new ActionListener ()
         {
            public void actionPerformed (ActionEvent e)
            {
               beansLoadAction(e);
            }
         });

      // Define the About option in the Help menu.
      mMenuHelpAbout.setMnemonic('A');
      mMenuHelpAbout.setText("About ...");
      mMenuHelpAbout.addActionListener(new ActionListener ()
         {
            public void actionPerformed(ActionEvent e)
            {
               helpAboutAction(e);
            }
         });

      mMenuFileOpen.setText("Open ...");
      mMenuFileOpen.setEnabled(false);
      mMenuFileOpen.setMnemonic('O');
      mMenuFileOpen.setAccelerator(javax.swing.KeyStroke.getKeyStroke(79, java.awt.event.KeyEvent.CTRL_MASK, false));
      mMenuFileOpen.addActionListener(new java.awt.event.ActionListener()
      {
         public void actionPerformed(ActionEvent e)
         {
            fileOpenAction(e);
         }
      });

      mMenuFileClose.setEnabled(false);
      mMenuFileClose.setMnemonic('C');
      mMenuFileClose.setText("Close ...");
      mMenuFileClose.setAccelerator(javax.swing.KeyStroke.getKeyStroke(87, java.awt.event.KeyEvent.CTRL_MASK, false));
      mMenuFileClose.addActionListener(new java.awt.event.ActionListener()
      {
         public void actionPerformed(ActionEvent e)
         {
            fileCloseAction(e);
         }
      });

      // Define the Quit option in the File menu.
      mMenuFileQuit.setText("Quit");
      mMenuFileQuit.setMnemonic('Q');
      mMenuFileQuit.setAccelerator(javax.swing.KeyStroke.getKeyStroke(81, java.awt.event.KeyEvent.CTRL_MASK, false));
      mMenuFileQuit.addActionListener(new ActionListener ()
         {
            public void actionPerformed(ActionEvent e)
            {
               fileQuitAction(e);
            }
         });

      // Set up the File menu.  This adds the Open option, a separator, and
      // the Quit option.
      mMenuFile.setText("File");
      mMenuFile.setMnemonic('F');
      mMenuFileSave.setEnabled(false);
      mMenuFileSave.setMnemonic('S');
      mMenuFileSave.setText("Save ...");
      mMenuFileSave.setAccelerator(javax.swing.KeyStroke.getKeyStroke(83, java.awt.event.KeyEvent.CTRL_MASK, false));
      mMenuFileSave.addActionListener(new java.awt.event.ActionListener()
      {
         public void actionPerformed(ActionEvent e)
         {
            fileSaveAction(e);
         }
      });
      mMenuFile.add(mMenuFileOpen);
      mMenuFile.add(mMenuFileSave);
      mMenuFile.add(mMenuFileClose);
      mMenuFile.addSeparator();
      mMenuFile.add(mMenuFileQuit);

      // Set up the Network menu.
      mMenuNetwork.setText("Network");
      mMenuNetwork.setMnemonic('N');
      mMenuNetwork.add(mMenuNetConnect);
      mMenuNetwork.add(mMenuNetDisconnect);

      // Set up the Preferences menu.
      mMenuPrefs.setText("Preferences");
      mMenuPrefs.setMnemonic('P');
      mMenuPrefs.add(mMenuPrefsGlobalEdit);
      mMenuPrefs.add(mMenuPrefsBeanEdit);

      // Set up the Beans menu.
      mMenuBeans.setText("Beans");
      mMenuBeans.setMnemonic('B');
      mMenuBeans.add(mMenuBeansLoad);

      // Add the About option to the Help menu.
      mMenuHelp.setText("Help");
      mMenuHelp.setMnemonic('H');
      mMenuHelp.add(mMenuHelpAbout);

      // Add the menus to the menu bar.
      mMenuBar.add(mMenuFile);
      mMenuBar.add(mMenuNetwork);
      mMenuBar.add(mMenuPrefs);
      mMenuBar.add(mMenuBeans);
      mMenuBar.add(mMenuHelp);

      // Finally, set the menu bar to what we have just defined.
      this.setJMenuBar(mMenuBar);

      mMainPanel.setTopComponent(mBeanContainer);
      mMainPanel.setBottomComponent(null);
      mMainPanel.setOrientation(JSplitPane.VERTICAL_SPLIT);
      mMainPanel.setDividerSize(1);

      mStatusBar.setLayout(mStatusBarLayout);
      mStatusBar.setBorder(BorderFactory.createLoweredBevelBorder());
      mProgressBar.setMinimumSize(new Dimension(100, 14));
      mStatusMsgButton.setMinimumSize(new Dimension(24, 24));
      mStatusMsgButton.setToolTipText("Expand or collapse the message panel");

      String bulb_on_icon_name  = "org/vrjuggler/tweek/gui/bulb-on-small.gif";
      String bulb_off_icon_name = "org/vrjuggler/tweek/gui/bulb-off-small.gif";

      try
      {
         mBulbOnIcon = new ImageIcon(BeanJarClassLoader.instance().getResource(bulb_on_icon_name));
      }
      catch (NullPointerException e)
      {
         MessagePanel.instance().printWarning("WARNING: Failed to load icon " +
                                              bulb_on_icon_name + "\n");
      }

      try
      {
         mBulbOffIcon = new ImageIcon(BeanJarClassLoader.instance().getResource(bulb_off_icon_name));
      }
      catch (NullPointerException e)
      {
         MessagePanel.instance().printWarning("WARNING: Failed to load icon " +
                                              bulb_off_icon_name + "\n");
      }

      if ( mBulbOffIcon != null )
      {
         mStatusMsgButton.setPreferredSize(new Dimension(24, 24));
         mStatusMsgButton.setIcon(mBulbOffIcon);
      }
      else
      {
         mStatusMsgButton.setText("Expand");
      }

      mStatusMsgButton.addActionListener(new ActionListener()
         {
            public void actionPerformed (ActionEvent e)
            {
               statusMessageExpandAction(e);
            }
         });

      // Add the various components to their respective containers.
      mContentPane.add(mMainPanel, BorderLayout.CENTER);
      mContentPane.add(mStatusBar,  BorderLayout.SOUTH);
      mStatusBar.add(mStatusMsgButton,  BorderLayout.EAST);
      mStatusBar.add(mStatusMsgLabel,  BorderLayout.CENTER);
      mStatusBar.add(mProgressBar, BorderLayout.WEST);
      mMainPanel.setDividerLocation(500);
   }

   // =========================================================================
   // Protected methods.
   // =========================================================================

   /**
    * Overridden so we can exit when window is closed.
    */
   protected void processWindowEvent (WindowEvent e)
   {
      super.processWindowEvent(e);

      if (e.getID() == WindowEvent.WINDOW_CLOSING)
      {
         fileQuitAction(null);
      }
   }

   // ========================================================================
   // Private methods.
   // ========================================================================

   /**
    * File | Open action performed.
    */
   private void fileOpenAction (ActionEvent e)
   {
      if ( mFileLoader.openRequested() )
      {
         mMenuFileClose.setEnabled(true);

         if ( ! mFileLoader.canOpenMultiple() )
         {
            mMenuFileOpen.setEnabled(false);
         }
      }
   }

   /**
    * File | Save action performed.
    */
   private void fileSaveAction (ActionEvent e)
   {
      mFileLoader.saveRequested();
   }

   /**
    * File | Close action performed.
    */
   private void fileCloseAction (ActionEvent e)
   {
      if ( mFileLoader.closeRequested() )
      {
         mMenuFileOpen.setEnabled(true);

         if ( mFileLoader.getOpenFileCount() == 0 )
         {
            mMenuFileClose.setEnabled(false);
         }
      }
   }

   /**
    * File | Quit action performed.
    */
   private void fileQuitAction (ActionEvent e)
   {
      if ( mBeanContainer != null )
      {
         mBeanContainer.fireFrameClosed();
      }

      for ( int i = 0; i < mORBs.size(); i++ )
      {
         ((CorbaService) mORBs.elementAt(i)).shutdown(true);
      }

      System.exit(0);
   }

   /**
    * Network | Connect action performed.
    */
   private void networkConnectAction (ActionEvent e)
   {
      ConnectionDialog dialog = new ConnectionDialog(this, "ORB Connections");
      dialog.display();

      if ( dialog.getStatus() == ConnectionDialog.OK_OPTION )
      {
         CorbaService corba_service = dialog.getCorbaService();

         if ( null != corba_service &&
              null != corba_service.getSubjectManager() )
         {
            mORBs.add(corba_service);
            mBeanContainer.fireConnectionEvent(corba_service);
            mMenuNetDisconnect.setEnabled(true);
         }
      }
   }

   /**
    * Network | Disconnect action performed.
    */
   private void networkDisconnectAction (ActionEvent e)
   {
      mMenuNetDisconnect.setEnabled(false);
/*
      try
      {
         m_plex_if.disconnect();
         mBeanContainer.fireDisconnectionEvent();
      }
      catch (org.vrjuggler.tweek.net.CommException ex)
      {
         ex.printStackTrace();
      }
*/
   }

   /**
    * Opens a dialog box used for editing the user's global preferences
    * visually.
    */
   private void prefsEditGlobal (ActionEvent e)
   {
      GlobalPreferencesService prefs =
         (GlobalPreferencesService)BeanRegistry.instance().getBean( "GlobalPreferences" );

      // Save this for later.
      int old_level = prefs.getUserLevel();

      PrefsDialog dialog = new PrefsDialog(this, "Global Preferences", prefs);
      dialog.display();

      if ( dialog.getStatus() == PrefsDialog.OK_OPTION )
      {
         String viewer = prefs.getBeanViewer();

         ViewerBean bean = (ViewerBean)BeanRegistry.instance().getBean( viewer );
         mBeanContainer.replaceViewer( bean.getViewer() );

         String new_laf = prefs.getLookAndFeel();
         String old_laf = UIManager.getCrossPlatformLookAndFeelClassName();

         if ( ! old_laf.equals(new_laf) )
         {
            try
            {
               UIManager.setLookAndFeel(new_laf);
               SwingUtilities.updateComponentTreeUI(this);
               mContentPane.updateUI();
            }
            catch (Exception laf_e)
            {
               prefs.setLookAndFeel(old_laf);
               JOptionPane.showMessageDialog(null, "Invalid look and feel '" +
                                             new_laf + "'",
                                             "Bad Look and Feel Setting",
                                             JOptionPane.ERROR_MESSAGE);
            }
         }

         // If the user level changed, fire an event saying as much.
         if ( old_level != prefs.getUserLevel() )
         {
            mBeanContainer.fireUserLevelChange(old_level,
                                                 prefs.getUserLevel());
         }
      }
   }

   private void prefsEditBean (ActionEvent e)
   {
      mBeanPrefsDialog.display();
   }

   private void beansLoadAction (ActionEvent e)
   {
      GlobalPreferencesService prefs =
         (GlobalPreferencesService)BeanRegistry.instance().getBean( "GlobalPreferences" );

      if ( prefs.getUserLevel() > 5 )
      {
         String path =
            JOptionPane.showInputDialog(null,
               "Please provide the path to a Bean XML file or directory",
               "Tweek Bean Finder", JOptionPane.QUESTION_MESSAGE);

         if ( path != null )
         {
            String exp_path = EnvironmentService.expandEnvVars(path);
            loadBeansFromPath(exp_path);
         }
      }
      else
      {
         JFileChooser chooser = new JFileChooser();
         chooser.setMultiSelectionEnabled(true);
         chooser.setDialogTitle("Tweek Bean Finder");
         chooser.setFileSelectionMode(JFileChooser.FILES_AND_DIRECTORIES);

         ExtensionFileFilter filter = new ExtensionFileFilter("Bean XML Files");
         filter.addExtension("xml");
         chooser.addChoosableFileFilter(filter);

         int status = chooser.showOpenDialog(this);

         if ( status == JFileChooser.APPROVE_OPTION )
         {
            java.io.File[] files = chooser.getSelectedFiles();

            if ( files.length > 0 )
            {
               for ( int i = 0; i < files.length; i++ )
               {
                  String path = files[i].getAbsolutePath();
                  loadBeansFromPath(path);
               }
            }
         }
      }
   }

   private void loadBeansFromPath (String path)
   {
      try
      {
         TweekCore.instance().findAndLoadBeans( path );
      }
      catch (BeanPathException path_ex)
      {
         JOptionPane.showMessageDialog(null, "Invalid Bean XML path '" + path + "'",
                                       "Bad Bean Path",
                                       JOptionPane.ERROR_MESSAGE);
      }
   }

   /**
    * Help | About action performed.
    */
   private void helpAboutAction(ActionEvent e)
   {
      AboutBox dlg      = new AboutBox(this);
      Dimension dlgSize = dlg.getPreferredSize();
      Dimension frmSize = getSize();
      Point loc = getLocation();
      dlg.setLocation((frmSize.width - dlgSize.width) / 2 + loc.x,
                      (frmSize.height - dlgSize.height) / 2 + loc.y);
      dlg.setModal(true);
      dlg.show();
   }

   private void statusMessageExpandAction (ActionEvent e)
   {
      if ( mMsgPanelExpanded )
      {
         mMainPanel.setBottomComponent(null);
         mMainPanel.resetToPreferredSizes();
         mMsgPanelExpanded = false;

         mStatusMsgLabel.setText("");

         if ( mStatusMsgButton.getIcon() == null )
         {
            mStatusMsgButton.setText("Expand");
         }
      }
      else
      {
         mMainPanel.setBottomComponent(MessagePanel.instance().getPanel());
         mMainPanel.setDividerLocation(0.85);
         mMsgPanelExpanded = true;

         if ( mBulbOffIcon != null )
         {
            mStatusMsgButton.setIcon(mBulbOffIcon);
         }
         else
         {
            mStatusMsgButton.setText("Collapse");
            mStatusMsgButton.setForeground(Color.black);
         }
      }
   }

   /**
    * Disables the menu items in the File menu related to handling of data
    * files.  This effectively resets the menu items to their state before any
    * Beans were loaded in the GUI.
    */
   private void disableFileHandlingItems ()
   {
      mFileLoader = null;
      mMenuFileOpen.setText("Open ...");
      mMenuFileOpen.setEnabled(false);
      mMenuFileSave.setEnabled(false);
      mMenuFileClose.setText("Close ...");
      mMenuFileClose.setEnabled(false);
   }

   // ========================================================================
   // Private data members.
   // ========================================================================

   private boolean mPackFrame = false;

   // GUI objects.
   private JPanel        mContentPane       = null;    /**< Top-level container */
   private BorderLayout  mContentPaneLayout = new BorderLayout();
   private JSplitPane    mMainPanel         = new JSplitPane();
   private BeanContainer mBeanContainer     = new BeanContainer();
   private ViewerBean    mBeanViewer        = null;

   // Status bar stuff.
   private JPanel        mStatusBar        = new JPanel();
   private JProgressBar  mProgressBar      = new JProgressBar();
   private JLabel        mStatusMsgLabel   = new JLabel();
   private JButton       mStatusMsgButton  = new JButton();
   private BorderLayout  mStatusBarLayout  = new BorderLayout();
   private ImageIcon     mBulbOnIcon       = null;
   private ImageIcon     mBulbOffIcon      = null;
   private boolean       mMsgPanelExpanded = false;

   // Menu bar objects.
   private JMenuBar mMenuBar              = new JMenuBar();
   private JMenu mMenuFile                = new JMenu();
   private JMenuItem mMenuFileOpen        = new JMenuItem();
   private JMenuItem mMenuFileSave        = new JMenuItem();
   private JMenuItem mMenuFileClose       = new JMenuItem();
   private JMenuItem mMenuFileQuit        = new JMenuItem();
   private JMenu mMenuNetwork             = new JMenu();
   private JMenuItem mMenuNetConnect      = new JMenuItem();
   private JMenuItem mMenuNetDisconnect   = new JMenuItem();
   private JMenu mMenuPrefs               = new JMenu();
   private JMenuItem mMenuPrefsGlobalEdit = new JMenuItem();
   private JMenuItem mMenuPrefsBeanEdit   = new JMenuItem();
   private JMenu mMenuBeans               = new JMenu();
   private JMenuItem mMenuBeansLoad       = new JMenuItem();
   private JMenu mMenuHelp                = new JMenu();
   private JMenuItem mMenuHelpAbout       = new JMenuItem();

   // Networking stuff.
   private Vector mORBs = new Vector();

   private FileLoader      mFileLoader      = null;
   private BeanPrefsDialog mBeanPrefsDialog = null;
}
