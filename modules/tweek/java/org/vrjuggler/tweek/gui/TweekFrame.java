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

package org.vrjuggler.tweek;

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
public class TweekFrame extends JFrame implements TreeModelRefreshListener,
                                                  BeanFocusChangeListener,
                                                  MessageAdditionListener
{
   public TweekFrame ()
   {
      enableEvents(AWTEvent.WINDOW_EVENT_MASK);
   }

   public void setBeanViewer (String viewer)
   {
      BeanRegistry registry = BeanRegistry.instance();
      if ( viewer != null )
      {
         ViewerBean bean = (ViewerBean)registry.getBean( viewer );

         if ( bean != null )
         {
            BeanModelViewer bv = bean.getViewer();
            m_bean_container.replaceViewer(bv);
         }
         else
         {
            System.out.println("WARNING: Unknown viewer type: '" + viewer + "'");
            ViewerBean defaultBean = (ViewerBean)registry.getBeansOfType( ViewerBean.class.getName() ).get( 0 );
            m_bean_container.replaceViewer((BeanModelViewer)defaultBean.getViewer() );
         }
      }
      else
      {
         ViewerBean defaultBean = (ViewerBean)registry.getBeansOfType( ViewerBean.class.getName() ).get( 0 );
         m_bean_container.replaceViewer((BeanModelViewer)defaultBean.getViewer() );
      }
   }

   /**
    * Initializes the GUI.
    *
    * @pre The JavaBean search has been performed.
    */
   public void initGUI (BeanTreeModel data_model)
   {
      data_model.addTreeModelRefreshListener(this);
      MessagePanel.instance().addMessageAdditionListener(this);

      try
      {
         GlobalPreferencesService prefs =
            (GlobalPreferencesService)BeanRegistry.instance().getBean( "GlobalPreferences" );
         UIManager.setLookAndFeel( prefs.getLookAndFeel() );
         jbInit();
      }
      catch (Exception e)
      {
         e.printStackTrace();
      }

      // Validate frames that have preset sizes.
      // Pack frames that have useful preferred size info, e.g. from their
      // layout.
      if ( packFrame )
      {
         this.pack();
      }
      else
      {
         this.validate();
      }

      this.setVisible(true);
   }

   public void treeModelRefresh (TreeModelRefreshEvent e)
   {
      BeanTreeModel data_model = TweekCore.instance().getTreeModel();
      List viewers = BeanRegistry.instance().getBeansOfType( ViewerBean.class.getName() );

      for ( Iterator itr = viewers.iterator(); itr.hasNext(); )
      {
         BeanModelViewer bv = ((ViewerBean)itr.next()).getViewer();
         bv.refreshDataModel(data_model);
      }
   }

   public void beanFocusChanged (BeanFocusChangeEvent e)
   {
      if ( e.getFocusType() == BeanFocusChangeEvent.BEAN_FOCUSED )
      {
         System.out.println("Bean " + e.getBean() + " focused!");
      }
      else if ( e.getFocusType() == BeanFocusChangeEvent.BEAN_UNFOCUSED )
      {
         System.out.println("Bean " + e.getBean() + " unfocused!");
      }
   }

   public void messageAdded (MessageAdditionEvent e)
   {
      if ( m_bulb_on_icon != null )
      {
         m_status_msg_button.setIcon(m_bulb_on_icon);
      }

      // If the user's skill level is below intermediate, give them a hint that
      // there is a message printed in the message panel.  They may not have
      // noticed the icon change made above.
      GlobalPreferencesService prefs =
         (GlobalPreferencesService)BeanRegistry.instance().getBean( "GlobalPreferences" );
      if ( prefs.getUserLevel() <= 5 )
      {
         m_status_msg_label.setText("New message in message panel ");
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

      m_content_pane = (JPanel) this.getContentPane();
      m_content_pane.setLayout(m_content_pane_layout);
      this.setSize(new Dimension(1024, 768));
      this.setTitle("Tweek JavaBean Loader");

      // Define the Connect option in the Network menu.
      m_menu_net_connect.setMnemonic('C');
      m_menu_net_connect.setText("Connect to ORB ...");
      m_menu_net_connect.setAccelerator(javax.swing.KeyStroke.getKeyStroke(67, KeyEvent.CTRL_MASK | KeyEvent.SHIFT_MASK, false));
      m_menu_net_connect.addActionListener(new ActionListener()
         {
            public void actionPerformed (ActionEvent e)
            {
               networkConnectAction(e);
            }
         });

      // Define the Disconnect option in the Network menu.
      m_menu_net_disconnect.setEnabled(false);
      m_menu_net_disconnect.setMnemonic('D');
      m_menu_net_disconnect.setText("Disconnect from ORB ...");
      m_menu_net_disconnect.addActionListener(new ActionListener()
         {
            public void actionPerformed (ActionEvent e)
            {
               networkDisconnectAction(e);
            }
         });

      // Define the Edit Global option in the Preferences menu.
      m_menu_prefs_gedit.setMnemonic('G');
      m_menu_prefs_gedit.setText("Edit Global ...");
      m_menu_prefs_gedit.addActionListener(new ActionListener()
         {
            public void actionPerformed (ActionEvent e)
            {
               prefsEditGlobal(e);
            }
         });

      // Define the Edit Local option in the Preferences menu.
      m_menu_prefs_ledit.setMnemonic('L');
      m_menu_prefs_ledit.setText("Edit Local ...");
      m_menu_prefs_ledit.setEnabled(false);

      m_menu_beans_load.setMnemonic('L');
      m_menu_beans_load.setText("Load Beans ...");
      m_menu_beans_load.setAccelerator(javax.swing.KeyStroke.getKeyStroke(66, KeyEvent.CTRL_MASK, false));
      m_menu_beans_load.addActionListener(new ActionListener ()
         {
            public void actionPerformed (ActionEvent e)
            {
               beansLoadAction(e);
            }
         });

      // Define the About option in the Help menu.
      m_menu_help_about.setMnemonic('A');
      m_menu_help_about.setText("About ...");
      m_menu_help_about.addActionListener(new ActionListener ()
         {
            public void actionPerformed(ActionEvent e)
            {
               helpAboutAction(e);
            }
         });

      m_menu_file_open.setText("Open ...");
      m_menu_file_open.setMnemonic('O');
      m_menu_file_open.setAccelerator(javax.swing.KeyStroke.getKeyStroke(79, KeyEvent.CTRL_MASK, false));
/*
      m_menu_file_open.addActionListener(new ActionListener ()
         {
            public void actionPerformed(ActionEvent e)
            {
               fileOpenAction(e);
            }
         });
*/

      // Define the Quit option in the File menu.
      m_menu_file_quit.setText("Quit");
      m_menu_file_quit.setMnemonic('Q');
      m_menu_file_quit.setAccelerator(javax.swing.KeyStroke.getKeyStroke(81, java.awt.event.KeyEvent.CTRL_MASK, false));
      m_menu_file_quit.addActionListener(new ActionListener ()
         {
            public void actionPerformed(ActionEvent e)
            {
               fileQuitAction(e);
            }
         });

      // Set up the File menu.  This adds the Open option, a separator, and
      // the Quit option.
      m_menu_file.setText("File");
      m_menu_file.setMnemonic('F');
      m_menu_file.add(m_menu_file_open);
      m_menu_file.addSeparator();
      m_menu_file.add(m_menu_file_quit);

      // Set up the Network menu.
      m_menu_network.setText("Network");
      m_menu_network.setMnemonic('N');
      m_menu_network.add(m_menu_net_connect);
      m_menu_network.add(m_menu_net_disconnect);

      // Set up the Preferences menu.
      m_menu_prefs.setText("Preferences");
      m_menu_prefs.setMnemonic('P');
      m_menu_prefs.add(m_menu_prefs_gedit);
      m_menu_prefs.add(m_menu_prefs_ledit);

      // Set up the Beans menu.
      m_menu_beans.setText("Beans");
      m_menu_beans.setMnemonic('B');
      m_menu_beans.add(m_menu_beans_load);

      // Add the About option to the Help menu.
      m_menu_help.setText("Help");
      m_menu_help.setMnemonic('H');
      m_menu_help.add(m_menu_help_about);

      // Add the menus to the menu bar.
      m_menu_bar.add(m_menu_file);
      m_menu_bar.add(m_menu_network);
      m_menu_bar.add(m_menu_prefs);
      m_menu_bar.add(m_menu_beans);
      m_menu_bar.add(m_menu_help);

      // Finally, set the menu bar to what we have just defined.
      this.setJMenuBar(m_menu_bar);

      m_main_panel.setTopComponent(m_bean_container);
      m_main_panel.setBottomComponent(null);
      m_main_panel.setOrientation(JSplitPane.VERTICAL_SPLIT);
      m_main_panel.setDividerSize(1);

      m_status_bar.setLayout(m_status_bar_layout);
      m_status_bar.setBorder(BorderFactory.createLoweredBevelBorder());
      m_progress_bar.setMinimumSize(new Dimension(100, 14));
      m_status_msg_button.setMinimumSize(new Dimension(24, 24));
      m_status_msg_button.setToolTipText("Expand or collapse the message panel");

      String bulb_on_icon_name  = "org/vrjuggler/tweek/bulb-on-small.gif";
      String bulb_off_icon_name = "org/vrjuggler/tweek/bulb-off-small.gif";

      try
      {
         m_bulb_on_icon = new ImageIcon(BeanJarClassLoader.instance().getResource(bulb_on_icon_name));
      }
      catch (NullPointerException e)
      {
         System.err.println("WARNING: Failed to load icon " + bulb_on_icon_name);
      }

      try
      {
         m_bulb_off_icon = new ImageIcon(BeanJarClassLoader.instance().getResource(bulb_off_icon_name));
      }
      catch (NullPointerException e)
      {
         System.err.println("WARNING: Failed to load icon " + bulb_off_icon_name);
      }

      if ( m_bulb_off_icon != null )
      {
         m_status_msg_button.setPreferredSize(new Dimension(24, 24));
         m_status_msg_button.setIcon(m_bulb_off_icon);
      }
      else
      {
         m_status_msg_button.setText("Expand");
      }

      m_status_msg_button.addActionListener(new ActionListener()
         {
            public void actionPerformed (ActionEvent e)
            {
               statusMessageExpandAction(e);
            }
         });

      // Add the various components to their respective containers.
      m_content_pane.add(m_main_panel, BorderLayout.CENTER);
      m_content_pane.add(m_status_bar,  BorderLayout.SOUTH);
      m_status_bar.add(m_status_msg_button,  BorderLayout.EAST);
      m_status_bar.add(m_status_msg_label,  BorderLayout.CENTER);
      m_status_bar.add(m_progress_bar, BorderLayout.WEST);
      m_main_panel.setDividerLocation(500);
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
    * File | Quit action performed.
    */
   private void fileQuitAction (ActionEvent e)
   {
      if ( m_bean_container != null )
      {
         m_bean_container.fireFrameClosed();
      }

      for ( int i = 0; i < m_orbs.size(); i++ )
      {
         ((CorbaService) m_orbs.elementAt(i)).shutdown(true);
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
         if ( dialog.getNameServiceHost() != null )
         {
            CorbaService new_orb = new CorbaService(dialog.getNameServiceHost(),
                                                    dialog.getNameServicePort(),
                                                    dialog.getNamingSubcontext());

            TweekBean service = BeanRegistry.instance().getBean( "Environment" );

            try
            {
               if ( service != null )
               {
                  EnvironmentService env_service = (EnvironmentService) service;
                  new_orb.init(env_service.getCommandLineArgs());
               }
               else
               {
                  new_orb.init(null);
               }

               m_bean_container.fireConnectionEvent(new_orb);
               m_orbs.add(new_orb);
               m_menu_net_disconnect.setEnabled(true);
            }
            catch (org.omg.CORBA.SystemException sys_ex)
            {
               sys_ex.getMessage();
               sys_ex.printStackTrace();
            }
         }
      }
   }

   /**
    * Network | Disconnect action performed.
    */
   private void networkDisconnectAction (ActionEvent e)
   {
      m_menu_net_disconnect.setEnabled(false);
/*
      try
      {
         m_plex_if.disconnect();
         m_bean_container.fireDisconnectionEvent();
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
         m_bean_container.replaceViewer( bean.getViewer() );

         String new_laf = prefs.getLookAndFeel();
         String old_laf = UIManager.getCrossPlatformLookAndFeelClassName();

         if ( ! old_laf.equals(new_laf) )
         {
            try
            {
               UIManager.setLookAndFeel(new_laf);
               SwingUtilities.updateComponentTreeUI(this);
               m_content_pane.updateUI();
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
            m_bean_container.fireUserLevelChange(old_level,
                                                 prefs.getUserLevel());
         }
      }
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
            TweekCore.instance().getTreeModel().fireTreeModelRefreshEvent();
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

               TweekCore.instance().getTreeModel().fireTreeModelRefreshEvent();
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
      if ( m_msg_panel_expanded )
      {
         m_main_panel.setBottomComponent(null);
         m_main_panel.resetToPreferredSizes();
         MessagePanel.instance().clear();
         m_msg_panel_expanded = false;

         // The message panel has been cleared now, so we can clear any message
         // that may have been printed in the status bar regarding that panel.
         m_status_msg_label.setText("");

         if ( m_status_msg_button.getIcon() == null )
         {
            m_status_msg_button.setText("Expand");
         }
      }
      else
      {
         m_main_panel.setBottomComponent(MessagePanel.instance().getPanel());
         m_main_panel.setDividerLocation(0.85);
         m_msg_panel_expanded = true;

         if ( m_bulb_off_icon != null )
         {
            m_status_msg_button.setIcon(m_bulb_off_icon);
         }
         else
         {
            m_status_msg_button.setText("Collapse");
         }
      }
   }

   // ========================================================================
   // Private data members.
   // ========================================================================

   private boolean packFrame = false;

   // GUI objects.
   private JPanel        m_content_pane        = null;    /**< Top-level container */
   private BorderLayout  m_content_pane_layout = new BorderLayout();
   private JSplitPane    m_main_panel          = new JSplitPane();
   private BeanContainer m_bean_container      = new BeanContainer();

   // Status bar stuff.
   private JPanel        m_status_bar         = new JPanel();
   private JProgressBar  m_progress_bar       = new JProgressBar();
   private JLabel        m_status_msg_label   = new JLabel();
   private JButton       m_status_msg_button  = new JButton();
   private BorderLayout  m_status_bar_layout  = new BorderLayout();
   private ImageIcon     m_bulb_on_icon       = null;
   private ImageIcon     m_bulb_off_icon      = null;
   private boolean       m_msg_panel_expanded = false;

   // Menu bar objects.
   private JMenuBar m_menu_bar             = new JMenuBar();
   private JMenu m_menu_file               = new JMenu();
   private JMenuItem m_menu_file_open      = new JMenuItem();
   private JMenuItem m_menu_file_quit      = new JMenuItem();
   private JMenu m_menu_network            = new JMenu();
   private JMenuItem m_menu_net_connect    = new JMenuItem();
   private JMenuItem m_menu_net_disconnect = new JMenuItem();
   private JMenu m_menu_prefs              = new JMenu();
   private JMenuItem m_menu_prefs_gedit    = new JMenuItem();
   private JMenuItem m_menu_prefs_ledit    = new JMenuItem();
   private JMenu m_menu_beans              = new JMenu();
   private JMenuItem m_menu_beans_load     = new JMenuItem();
   private JMenu m_menu_help               = new JMenu();
   private JMenuItem m_menu_help_about     = new JMenuItem();

   // Networking stuff.
   private Vector m_orbs = new Vector();
}
