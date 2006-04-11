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

#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sstream>

#include <vpr/vpr.h>
#include <vpr/vprTypes.h>
#include <vpr/System.h>
#include <vpr/IO/Socket/SocketDatagram.h>
#include <vpr/IO/Socket/SocketStream.h>
#include <vpr/Util/Debug.h>

#include <gadget/Devices/Ascension/MotionStar/MotionStarStandalone.h>

#ifdef VPR_OS_Win32
#define M_PI   3.14159265358979323846
#endif

#define RAD2DEG(x) ((x) * 180.0 / M_PI)


namespace FLOCK
{

const unsigned char ERR_STATE           = 0x80;
const unsigned char RUNNING             = 0x40;
//const unsigned char RESERVED1           = 0x20;
//const unsigned char RESERVED2           = 0x10;
const unsigned char BUTTONS_PRESENT     = 0x08;
const unsigned char SENSOR_PRESENT      = 0x04;
const unsigned char TRANSMITTER_PRESENT = 0x02;
const unsigned char TRANSMITTER_RUNNING = 0x01;

const unsigned char SUDDEN_OUTPUT_CHANGE = 0x20;
const unsigned char XYZ_REFERENCE        = 0x10;
const unsigned char APPEND_BUTTON_DATA   = 0x08;
const unsigned char AC_NARROW_FILTER     = 0x04;
const unsigned char AC_WIDE_FILTER       = 0x02;
const unsigned char DC_FILTER            = 0x01;

// ----------------------------------------------------------------------------
// Convert the given Flock data format into a human-readable string that
// names the format.
// ----------------------------------------------------------------------------
std::string getFormatName(const data_format format)
{
   std::string name;

   switch (format)
   {
      case NO_BIRD_DATA:
      case INVALID:
         name = "NONE";
         break;
      case POSITION:
         name = "POSITION";
         break;
      case ANGLES:
         name = "ANGLES";
         break;
      case MATRIX:
         name = "MATRIX";
         break;
      case POSITION_ANGLES:
         name = "POSITION/ANGLES";
         break;
      case POSITION_MATRIX:
         name = "POSITION/MATRIX";
         break;
      case QUATERNION:
         name = "QUATERNION";
         break;
      case POSITION_QUATERNION:
         name = "POSITION/QUATERNION";
         break;
      case FEEDTHROUGH_DATA:
         name = "FEEDTHROUGH_DATA";
         break;
   }

   return name;
}

// ----------------------------------------------------------------------------
// Convert the given Flock hemisphere ID into a human-readable string that
// names the hemisphere.
// ----------------------------------------------------------------------------
std::string getHemisphereName(const hemisphere hemi)
{
   std::string name;

   switch (hemi)
   {
      case FRONT_HEMISPHERE:
         name = "Front";
         break;
      case REAR_HEMISPHERE:
         name = "Rear";
         break;
      case UPPER_HEMISPHERE:
         name = "Upper";
         break;
      case LOWER_HEMISPHERE:
         name = "Lower";
         break;
      case LEFT_HEMISPHERE:
         name = "Left";
         break;
      case RIGHT_HEMISPHERE:
         name = "Right";
         break;
   }

   return name;
}

} // End of FLOCK namespace

namespace FBB
{

const unsigned char ACCESS  = 0x80;
const unsigned char RUNNING = 0x40;
const unsigned char SENSOR  = 0x20;
const unsigned char ERC     = 0x10;
const unsigned char ERT3    = 0x08;
const unsigned char ERT2    = 0x04;
const unsigned char ERT1    = 0x02;
const unsigned char ERT0    = 0x01;

} // End of FBB namespace

namespace BIRDNET
{

const unsigned char MSG_WAKE_UP        = 10;
const unsigned char RSP_WAKE_UP        = 20;
const unsigned char MSG_SHUT_DOWN      = 11;
const unsigned char RSP_SHUT_DOWN      = 21;
const unsigned char MSG_GET_STATUS     = 101;
const unsigned char MSG_SEND_SETUP     = 102;
const unsigned char MSG_SINGLE_SHOT    = 103;
const unsigned char MSG_RUN_CONTINUOUS = 104;
const unsigned char MSG_STOP_DATA      = 105;
const unsigned char RSP_GET_STATUS     = 201;
const unsigned char RSP_SEND_SETUP     = 202;
const unsigned char RSP_RUN_CONTINUOUS = 204;
const unsigned char RSP_STOP_DATA      = 205;
const unsigned char DATA_PACKET_MULTI  = 210;
const unsigned char RSP_ILLEGAL        = 40;
const unsigned char RSP_UNKNOWN        = 50;
const unsigned char MSG_SYNC_SEQUENCE  = 30;
const unsigned char RSP_SYNC_SEQUENCE  = 31;

const unsigned char BN_PROTOCOL = 3;

} // End of BIRDNET namespace


// ============================================================================
// Public methods.
// ============================================================================

// ----------------------------------------------------------------------------
// Debugging method
/*
static std::ostream& operator<<(std::ostream& out,
                                const BIRDNET::SINGLE_BIRD_STATUS& device)
{
   out << " FBB address: " << (unsigned int) device.FBBaddress << "\n";
//   out << " Accessible: " << (m_birds[i]->accessible ? "YES" : "NO") << "\n";
//   out << " Running: " << (m_birds[i]->running ? "YES" : "NO") << "\n";
   out << " Data format: " << (unsigned int) device.dataFormat << "\n";
   out << " Hemisphere: " << (unsigned int) device.hemisphere << "\n";
   out << " Report Rate: " << (unsigned int) device.reportRate << "\n";
   out << " Report rate: " << (unsigned int) device.reportRate << "\n";
   out << " Sudden change filtering: "
       << (device.setup & FLOCK::SUDDEN_OUTPUT_CHANGE ? "ON" : "OFF") << "\n";
   out << " Button data: "
       << (device.setup & FLOCK::APPEND_BUTTON_DATA ? "ON" : "OFF") << "\n";
   out << " AC narrow filter: "
       << (device.setup & FLOCK::AC_NARROW_FILTER ? "ON" : "OFF")
       << "\n";out << " AC wide filter: "
       << (device.setup & FLOCK::AC_WIDE_FILTER ? "ON" : "OFF") << "\n";
   out << " DC filter: "
       << (device.setup & FLOCK::DC_FILTER ? "ON" : "OFF") << "\n";

   return out;
}
*/


// ============================================================================
// Public methods.
// ============================================================================

// ----------------------------------------------------------------------------
// Constructor.  This initializes member variables and determines the
// endianness of the host machine.
// ----------------------------------------------------------------------------
MotionStarStandalone::MotionStarStandalone(const std::string& address,
                                           const unsigned short port,
                                           const BIRDNET::protocol proto,
                                           const bool master,
                                           const FLOCK::hemisphere hemisphere,
                                           const FLOCK::data_format birdFormat,
                                           const BIRDNET::run_mode runMode,
                                           const unsigned char reportRate,
                                           const double measurementRate,
                                           const unsigned int birdsRequested)
   : m_active(false), m_address(), m_socket(NULL), m_proto(proto),
     m_master(master), m_seq_num(0), m_cur_mrate(0.0),
     m_measurement_rate(measurementRate), m_run_mode(runMode),
     m_hemisphere(hemisphere), m_bird_format(birdFormat),
     m_report_rate(reportRate), m_birds_requested(birdsRequested),
     m_birds_active(0), m_unit_conv(1.0)
{
   union
   {
      char   c[sizeof(short)];
      short  value;
   } endian;

   m_address.setPort(port);

   if ( address.length() > 0 )
   {
      m_address.setAddress(address);
   }

   // These are hard-coded because they are always going to be the same
   // values.
   m_xmtr_rot_scale = 180.0;
   m_xmtr_divisor   = 32767.0;

   // Set this to -1.0 so that we can test its value again later to ensure
   // that it was set before trying to do any sampling.
   m_xmtr_pos_scale = -1.0;

   // Determine the endianness of the host platform.  A value of true for
   // m_big_endian means that the host use big endian byte order.  false of
   // course means that it is little endian.  The way this works is that we
   // access the first byte of endian.value directly.  If it is 1, the host
   // treats that as the high-order byte.  Otherwise, it is the low-order
   // byte.
   endian.value = 256;
   m_big_endian = (endian.c[0] ? true : false);
}

// ----------------------------------------------------------------------------
// Destructor.
// ----------------------------------------------------------------------------
MotionStarStandalone::~MotionStarStandalone()
{
   unsigned int i;

   if ( isActive() )
   {
      stop();
   }

   for ( i = 0; i < m_erc_vec.size(); i++ )
   {
      delete m_erc_vec[i];
   }

   for ( i = 0; i < m_birds.size(); i++ )
   {
      delete m_birds[i];
   }
}

// ----------------------------------------------------------------------------
// Initializes the driver, setting the status for each bird.
// ----------------------------------------------------------------------------
vpr::ReturnStatus MotionStarStandalone::start()
   throw(mstar::NetworkException, mstar::ConnectException,
         mstar::ScaleFactorUnknownException)
{
   vpr::ReturnStatus retval;

   vprDEBUG(vprDBG_ALL, vprDBG_STATE_LVL)
      << "\n[MotionStarStandalone] Connecting to " << m_address << " ..."
      << std::endl << vprDEBUG_FLUSH;

   // Create the socket based on the protocol in use.
   switch (m_proto)
   {
      case BIRDNET::UDP:
         m_socket = new vpr::SocketDatagram(vpr::InetAddr::AnyAddr, m_address);
         break;
      case BIRDNET::TCP:
         m_socket = new vpr::SocketStream(vpr::InetAddr::AnyAddr, m_address);
         break;
   }

   // If the socket could not be opened, we are in trouble.
   if ( ! m_socket->open().success() )
   {
      std::stringstream msg_stream;
      msg_stream << "Could not open "
                 << ((m_proto == BIRDNET::UDP) ? "datagram" : "stream")
                 << " socket";

      vprDEBUG(vprDBG_ALL, vprDBG_CRITICAL_LVL)
         << "[MotionStarStandalone] Could not open " << msg_stream.str()
         << std::endl << vprDEBUG_FLUSH;
      throw mstar::NetworkException(msg_stream.str());
   }
   // Otherwise, keep on truckin'.
   else
   {
      vprDEBUG(vprDBG_ALL, vprDBG_STATE_LVL)
         << "[MotionStarStandalone] "
         << ((m_proto == BIRDNET::UDP) ? "Datagram" : "Stream")
         << " socket open" << std::endl << vprDEBUG_FLUSH;

      // Connect to the server.  If this is a datagram socket, it will
      // set the default recipient of all future packets, so this is a
      // valid, safe operation regardless of the protocol being used.
      if ( ! m_socket->connect().success() )
      {
         // XXX: Use VPR's error message stuff for this one...
         perror("[MotionStarStandalone] Cannot connect to server");
         throw mstar::ConnectException(std::string("Cannot connect to server"));
      }
      else
      {
         vprDEBUG(vprDBG_ALL, vprDBG_STATE_LVL)
            << "[MotionStarStandalone] Connected to server" << std::endl
            << vprDEBUG_FLUSH;

         try
         {
            // Now that we are connected, send a wake-up call to the
            // server.
            sendWakeUp();

            vprDEBUG(vprDBG_ALL, vprDBG_CRITICAL_LVL)
               << "[MotionStarStandalone] The Sleeper has awakened!"
               << std::endl << vprDEBUG_FLUSH;

            BIRDNET::SYSTEM_STATUS* sys_status(NULL);

            try
            {
               // Get the general system status and save it for later.
               sys_status = getSystemStatus();

               vprDEBUG(vprDBG_ALL, vprDBG_STATE_LVL)
                  << "[MotionStarStandalone] Got system status"
                  << std::endl << vprDEBUG_FLUSH;

               // As long as a positive, non-zero measurement rate is
               // given, send it to the MotionStar chassis as a system
               // configuration parameter.
               if ( m_measurement_rate > 0.0 )
               {
                  std::string str_rate;
                  vpr::ReturnStatus status;

                  // Bounds checking on the measumrent rate.
                  if ( m_measurement_rate > 144.0 )
                  {
                     m_measurement_rate = 144.0;
                  }
                  else if ( m_measurement_rate < 20.0 )
                  {
                     m_measurement_rate = 20.0;
                  }

                  convertMeasurementRate(m_measurement_rate, str_rate);
                  status = setSystemStatus(sys_status,
                                           sys_status->transmitterNumber,
                                           str_rate.c_str());

                  if ( ! status.success() )
                  {
                     vprDEBUG(vprDBG_ALL, vprDBG_WARNING_LVL)
                        << "[MotionStarStandalone] WARNING: Failed to set system status"
                        << std::endl << vprDEBUG_FLUSH;
                  }
                  else
                  {
                     vprDEBUG(vprDBG_ALL, vprDBG_STATE_LVL)
                        << "[MotionStarStandalone] Set measurement rate to "
                        << m_measurement_rate << std::endl << vprDEBUG_FLUSH;
                  }
               }
            }
            catch (mstar::CommandException ex)
            {
               vprDEBUG(vprDBG_ALL, vprDBG_WARNING_LVL)
                  << "[MotionStarStandalone] WARNING: Failed to read system status"
                  << std::endl << vprDEBUG_FLUSH;
            }

            // Configure each of the birds.
            configureBirds();

            // If the desired run mode is BIRDNET::CONTINUOUS, we need
            // to request that the data start coming from the server.
            // If it is BIRDNET::SINGLE_SHOT, there is nothing to do.
            if ( m_run_mode == BIRDNET::CONTINUOUS )
            {
               try
               {
                  setContinuous();
                  vprDEBUG(vprDBG_ALL, vprDBG_STATE_LVL)
                     << "[MotionStarStandalone] Continuous data requested"
                     << std::endl << vprDEBUG_FLUSH;
               }
               catch (mstar::CommandException ex)
               {
                  vprDEBUG(vprDBG_ALL, vprDBG_WARNING_LVL)
                     << "[MotionStarStandalone] WARNING: Continuous data request failed!"
                     << std::endl << vprDEBUG_FLUSH;
               }
            }

            // Ensure that the position scaling factor has been set
            // by this point.  If it has not been set, we are in big,
            // big trouble since no useful data can be extracted from
            // the server's data packets.
            if ( m_xmtr_pos_scale == -1.0 )
            {
               vprDEBUG(vprDBG_ERROR, vprDBG_CRITICAL_LVL)
                  << "[MotionStarStandalone] FATAL ERROR: Position scaling factor unknown!"
                  << std::endl << vprDEBUG_FLUSH;
               throw mstar::ScaleFactorUnknownException();
            }
            else
            {
               vprDEBUG(vprDBG_ALL, vprDBG_STATE_LVL)
                  << "[MotionStarStandalone] Driver setup done\n"
                  << std::endl << vprDEBUG_FLUSH;

               if ( sys_status != NULL )
               {
                   printSystemStatus(sys_status);
                   delete sys_status;
               }

               printDeviceStatus();

               // The device setup has completed successfully, so we
               // are now considered active.  The return value is
               // left at 0 indicating successful startup.
               m_active = true;
            }
         }
         catch (mstar::CommandException ex)
         {
            vprDEBUG(vprDBG_ERROR, vprDBG_CRITICAL_LVL)
               << "[MotionStarStandalone] Could not wake up server"
               << std::endl << vprDEBUG_FLUSH;
            retval.setCode(vpr::ReturnStatus::Fail);
         }
      }
   }

   return retval;
}

// ----------------------------------------------------------------------------
// Stop the data flow (if it is in continuous mode), shut down the server and
// close the connection to it.
// ----------------------------------------------------------------------------
void MotionStarStandalone::stop() throw(mstar::CommandException)
{
   stopData();
   shutdown();

   // Close the socket.
   if ( m_socket != NULL )
   {
      m_socket->close();
      // XXX: Memory leak!!  Why is this not deleted here?  The memory is
      // allocated in start(), so it would make sense to release it here.
//       delete m_socket;
      m_socket = NULL;
   }

   // We reset the sequence number to 0 so that if the driver is restarted
   // later, the client will resync with the server starting with sequence
   // number 0.
   m_seq_num = 0;
}

// ----------------------------------------------------------------------------
// Based on the current run mode, a single sample is taken (run mode is
// BIRDNET::SINGLE_SHOT), or continuous samples are taken (run mode is
// BIRDNET::CONTINUOUS).
// ----------------------------------------------------------------------------
void MotionStarStandalone::sample()
{
   BIRDNET::DATA_PACKET recv_pkt;

   // If we are running in single-shot mode, we need to request a data
   // packet from the server.  Otherwise, all incoming packets are data
   // packets contaning the information we want.
   if ( m_run_mode == BIRDNET::SINGLE_SHOT )
   {
      BIRDNET::HEADER data_req(BIRDNET::MSG_SINGLE_SHOT);

      data_req.sequence = vpr::System::Htons(m_seq_num);
      m_seq_num++;

      try
      {
         sendMsg(&data_req, sizeof(BIRDNET::HEADER));
      }
      catch (mstar::NetworkWriteException ex)
      {
         vprDEBUG(vprDBG_ERROR, vprDBG_CRITICAL_LVL)
            << "[MotionStarStandalone]: Failed to send single-shot device request header"
            << std::endl << vprDEBUG_FLUSH;
         return;
      }
   }

   try
   {
      // First, we need to read the header for the incoming data packet so we
      // know how much data to expect.
      getRsp(&recv_pkt.header);

      if ( recv_pkt.header.error_code != 0 )
      {
         printError(recv_pkt.header.error_code);
      }
      else
      {
         // Test the type of the packet read just to be safe.
         if ( recv_pkt.header.type == BIRDNET::DATA_PACKET_MULTI )
         {
            char* base_ptr;
            BIRDNET::DATA_RECORD* rec_ptr;
            unsigned char format_code;
            unsigned int rec_data_words;
            size_t rec_data_size;

            vpr::ReturnStatus resp_status;

            // Try to read the bird data from the packet based on the number
            // of bytes the header claims are available.  If an exception that
            // derives from mstar::MotionStarException is thrown, this loop
            // will repeat.  All other exceptions are rethrown to the caller
            // of this method.
            do
            {
               try
               {
                  // Read the remainder of the waiting packet from the receive
                  // buffer.
                  resp_status = getRsp(&recv_pkt.buffer,
                                       vpr::System::Ntohs(recv_pkt.header.number_bytes));
               }
               // Catch exceptions relating to the lack of data.  If this
               // happens, something is very wrong because we were able to
               // read the header data from the packet.
               catch (mstar::NoDataReadException ex)
               {
                  resp_status.setCode(vpr::ReturnStatus::Fail);  // Retry read
                  vprDEBUG(vprDBG_ERROR, vprDBG_WARNING_LVL)
                     << "[MotionStarStandalone] WARNING: Failed to read "
                     << vpr::System::Ntohs(recv_pkt.header.number_bytes)
                     << " bytes of data--trying again ..." << std::endl
                     << vprDEBUG_FLUSH;
               }
               // Catch the general mstar::MotionStarException type and print
               // the message (if it has one).
               catch (mstar::MotionStarException ex)
               {
                  resp_status.setCode(vpr::ReturnStatus::Fail);  // Retry read
                  vprDEBUG(vprDBG_ERROR, vprDBG_CRITICAL_LVL)
                     << "[MotionStarStandalone] ERROR: MotionStarException caught: "
                     << ex.getMessage() << std::endl << vprDEBUG_FLUSH;
               }
               // Any other exceptions are rethrown to the caller.  Basically,
               // anything less specific than a mstar::MotionStarException is
               // an error from which we cannot recover.
               catch (...)
               {
                  vprDEBUG(vprDBG_ERROR, vprDBG_CRITICAL_LVL)
                     << "[MotionStarStandalone] ERROR: Generic xception occurred when reading bird data, rethrowing"
                     << std::endl << vprDEBUG_FLUSH;
                  throw;
               }
            }
            while ( ! resp_status.success() );

            // At this point, we have the bird data in recv_pkt.

            // Use a char* for doing pointer arithmetic.  It starts at the
            // beginning of the received packet's buffer field.
            base_ptr = (char*) &recv_pkt.buffer[0];

            for ( unsigned char bird = 0; bird < m_birds_active; bird++ )
            {
               // Set the record pointer to the current data record's address
               // as defined by base_ptr.
               rec_ptr = (BIRDNET::DATA_RECORD*) base_ptr;

               // The least significant four bits of the data_info field
               // contain the number of words (2 bytes) of formatted data.
               // m_birds[bird]->data_words already has this value, but we
               // read it from the packet to be safe.
               rec_data_words = (rec_ptr->data_info) & 0x0f;
               rec_data_size  = rec_data_words * 2;

               if ( rec_data_words != m_birds[bird]->data_words )
               {
                  // XXX: Figure out how to print this out neatly using vprDEBUG.
                  fprintf(stderr,
                          "[MotionStarStandalone] WARNING: Expecting %u data words from bird %u, got %u\n"
                          "                       You may have requested more birds than you\n"
                          "                       have connected, or the birds may not be\n"
                          "                       connected sequentially\n"
                          "                       Verify that your configuration is correct\n",
                          m_birds[bird]->data_words, bird, rec_data_words);
               }

               // Get the four most significant bits of the data_info field.
               // This gives the format code.  See page 134 of the Operation
               // Guide for more information.  m_birds[bird]->format already
               // has this set, but again, we read it from the current packet
               // just to be safe.  This is especially important because an
               // error may have occurred thus giving a format code of 15.
               format_code = (rec_ptr->data_info >> 4) & 0x0f;

               if ( format_code != m_birds[bird]->format )
               {
                  vprDEBUG(vprDBG_ALL, vprDBG_WARNING_LVL)
                        << "[MotionStarStandalone] WARNING: Expecting format "
                        << (unsigned int) m_birds[bird]->format << " from bird "
                        << (unsigned int) bird << ", got "
                        << (unsigned int) format_code << std::endl
                        << vprDEBUG_FLUSH;
               }

               // If the format code is 15, an error occurred in sampling the
               // data, and therefore the data block is invalid and should be
               // ignored.
               if ( format_code == 15 )
               {
                  vprDEBUG(vprDBG_ALL, vprDBG_WARNING_LVL)
                     << "[MotionStarStandalone] An error occurred in sampling"
                     << std::endl << vprDEBUG_FLUSH;
               }
               // Now that we have the size of the data block in the current
               // record, we can read it into the current bird's data block.
               // Since it uses a union, we can just read into its base
               // address.
               else
               {
                  memcpy(&m_birds[bird]->data, &rec_ptr->data, rec_data_size);
               }

               // The size of each record may vary depending on the data
               // format in use.  Increment the address of the current
               // record by the statically known sizes and by the size of
               // the current record's data block (each bird may have a
               // different sized data block).
               base_ptr += sizeof(rec_ptr->address) +
                              sizeof(rec_ptr->data_info) + rec_data_size;

               // If the most significant bit is set in address, then there
               // is also button data for this record.
               if ( rec_ptr->address & 0x80 )
               {
                  // Copy the button data into the current bird's info block.
                  m_birds[bird]->buttons[0] = rec_ptr->button_data[0];
                  m_birds[bird]->buttons[1] = rec_ptr->button_data[1];

                  // Increment the pointer to account for the button data.
                  base_ptr += sizeof(rec_ptr->button_data);
               }
            }
         }
         // It's unlikely that we will have received the wrong packet type at
         // this point, but this message will warn us if we did.
         else
         {
            vprDEBUG(vprDBG_ALL, vprDBG_WARNING_LVL)
               << "[MotionStarStandalone] WARNING: Got unexpected packet type "
               << (unsigned int) recv_pkt.header.type << " in sample()!"
               << std::endl << vprDEBUG_FLUSH;
         }
      }
   }
   catch (mstar::NetworkReadException ex)
   {
      vprDEBUG(vprDBG_ERROR, vprDBG_CRITICAL_LVL)
         << "[MotionStarStandalone] ERROR: Network read error in sample() "
         << "when requesting packet header!" << std::endl << vprDEBUG_FLUSH;
   }
}

// ----------------------------------------------------------------------------
// Stops the data flow if it is in continuous mode.
// ----------------------------------------------------------------------------
vpr::ReturnStatus MotionStarStandalone::stopData() throw(mstar::CommandException)
{
   vpr::ReturnStatus status;

   // If and only if the server is sending continuous data, we need to stop
   // it.
   if ( m_run_mode == BIRDNET::CONTINUOUS )
   {
      try
      {
         BIRDNET::HEADER msg(BIRDNET::MSG_STOP_DATA), rsp;

         // Send the MSG_STOP_DATA packet.
         status = sendMsg(&msg);

         try
         {
            // Get the server's response.
            status = getRsp(&rsp);

            vprDEBUG(vprDBG_ALL, vprDBG_STATE_LVL)
               << "[MotionStarStandalone] Continuous data stopped"
               << std::endl << vprDEBUG_FLUSH;
         }
         // If getRsp() threw an exception, print a warning message stating
         // that the data flow could not be stopped.
         catch (mstar::NetworkReadException ex)
         {
            vprDEBUG(vprDBG_ALL, vprDBG_WARNING_LVL)
               << "[MotionStarStandalone] WARNING: Could not stop continuous data"
               << std::endl << vprDEBUG_FLUSH;
            throw mstar::CommandException(ex.getMessage());
         }
      }
      catch (mstar::NetworkWriteException ex)
      {
         vprDEBUG(vprDBG_ALL, vprDBG_WARNING_LVL)
            << "[MotionStarStandalone] WARNING: Could not send message to stop "
            << "continuous data" << std::endl << vprDEBUG_FLUSH;
         throw mstar::CommandException(ex.getMessage());
      }
   }

   return status;
}

// ----------------------------------------------------------------------------
// Shut down the server chassis.
// ----------------------------------------------------------------------------
vpr::ReturnStatus MotionStarStandalone::shutdown() throw(mstar::CommandException)
{
   vpr::ReturnStatus status;

   try
   {
      BIRDNET::HEADER msg(BIRDNET::MSG_SHUT_DOWN), rsp;

      // Send the MSG_SHUT_DOWN packet.
      status = sendMsg(&msg);

      try
      {
         // Get the server's response to the MSG_SHUT_DOWN packet.
         status = getRsp(&rsp);

         vprDEBUG(vprDBG_ALL, vprDBG_STATE_LVL)
            << "[MotionStarStandalone] Server chassis shut down" << std::endl
            << vprDEBUG_FLUSH;
         m_active = false;
      }
      // If one could not be read, print a warning message.
      catch (mstar::NetworkReadException ex)
      {
         vprDEBUG(vprDBG_ALL, vprDBG_WARNING_LVL)
            << "[MotionStarStandalone] WARNING: Could not shutdown server chassis"
            << std::endl << vprDEBUG_FLUSH;
         throw mstar::CommandException(ex.getMessage());
      }
   }
   catch (mstar::NetworkWriteException ex)
   {
      vprDEBUG(vprDBG_ALL, vprDBG_WARNING_LVL)
         << "[MotionStarStandalone] WARNING: Could not send shutdown request"
         << std::endl << vprDEBUG_FLUSH;
      throw mstar::CommandException(ex.getMessage());
   }

   return status;
}

// ----------------------------------------------------------------------------
// Get the current server address for the device.
// ----------------------------------------------------------------------------
void MotionStarStandalone::setRunMode(const BIRDNET::run_mode mode)
   throw(mstar::CommandException)
{
   // If the driver is already active, we may need to do some communication
   // with the server before changing the run mode.
   if ( m_active )
   {
      // If the old mode is BIRDNET::CONTINUOUS and the new mode is
      // BIRDNET::SINGLE_SHOT, we need to stop the incoming data first.
      if ( m_run_mode == BIRDNET::CONTINUOUS &&
           mode == BIRDNET::SINGLE_SHOT )
      {
         try
         {
            BIRDNET::HEADER msg(BIRDNET::MSG_STOP_DATA), rsp;

            sendMsg(&msg);
            vprDEBUG(vprDBG_ALL, vprDBG_STATE_LVL)
               << "[MotionStarStandalone] Continuous data stopped" << std::endl
               << vprDEBUG_FLUSH;

            try
            {
               getRsp(&rsp);

               if ( rsp.error_code != 0 )
               {
                  printError(rsp.error_code);
               }
            }
            catch (...)
            {
               /* We don't care about exceptions in this case. */ ;
            }
         }
         catch (mstar::NetworkWriteException ex)
         {
            vprDEBUG(vprDBG_ALL, vprDBG_WARNING_LVL)
               << "[MotionStarStandalone] WARNING: Could not stop continuous "
               << "data before changing run modes!" << std::endl
               << vprDEBUG_FLUSH;
            throw mstar::CommandException(ex.getMessage());
         }
      }
      // If the run mode was BIRDNET::SINGLE_SHOT and the new mode is
      // BIRDNET::CONTINUOUS, we need to request continuous data from the
      // server.
      else if ( m_run_mode == BIRDNET::SINGLE_SHOT &&
                mode == BIRDNET::CONTINUOUS )
      {
         try
         {
            BIRDNET::HEADER msg(BIRDNET::MSG_RUN_CONTINUOUS), rsp;

            sendMsg(&msg);
            vprDEBUG(vprDBG_ALL, vprDBG_STATE_LVL)
               << "[MotionStarStandalone] Continuous data requested"
               << std::endl << vprDEBUG_FLUSH;

            try
            {
               getRsp(&rsp);

               if ( rsp.error_code != 0 )
               {
                  printError(rsp.error_code);
               }
            }
            catch (...)
            {
               /* We don't care about exceptions in this case. */ ;
            }
         }
         catch (mstar::NetworkWriteException ex)
         {
            vprDEBUG(vprDBG_ALL, vprDBG_WARNING_LVL)
               << "[MotionStarStandalone] WARNING: Could not request "
               << "continuous data flow!" << std::endl << vprDEBUG_FLUSH;
            throw mstar::CommandException(ex.getMessage());
         }
      }
   }

   m_run_mode = mode;
}

// ----------------------------------------------------------------------------
// Get the x position of the i'th bird.
// ----------------------------------------------------------------------------
float MotionStarStandalone::getXPos(const unsigned int i) const
{
   float x_pos;
   FLOCK::data_format format;

   format = m_birds[i]->format;

   if ( format == FLOCK::POSITION || format == FLOCK::POSITION_ANGLES ||
        format == FLOCK::POSITION_MATRIX ||
        format == FLOCK::POSITION_QUATERNION )
   {
      // Ahh, the beauty of a union is is that this statement works for
      // any of these data formats since the memory address is the same in
      // all cases.
      x_pos = toXPos((unsigned char*) &m_birds[i]->data) / m_unit_conv;
   }
   else
   {
      vprDEBUG(vprDBG_ALL, vprDBG_CRITICAL_LVL)
         << "[MotionStarStandalone] getXPos() for bird " << i
         << " does not support data format " << FLOCK::getFormatName(format)
         << std::endl << vprDEBUG_FLUSH;
      x_pos = 0.0;
   }

   return x_pos;
}

// ----------------------------------------------------------------------------
// Get the y position of the i'th bird.
// ----------------------------------------------------------------------------
float MotionStarStandalone::getYPos(const unsigned int i) const
{
   float y_pos;
   FLOCK::data_format format;

   format = m_birds[i]->format;

   if ( format == FLOCK::POSITION || format == FLOCK::POSITION_ANGLES ||
        format == FLOCK::POSITION_MATRIX ||
        format == FLOCK::POSITION_QUATERNION )
   {
      // Ahh, the beauty of a union is is that this statement works for
      // any of these data formats since the memory address is the same in
      // all cases.
      y_pos = toYPos((unsigned char*) &m_birds[i]->data) / m_unit_conv;
   }
   else
   {
      vprDEBUG(vprDBG_ALL, vprDBG_CRITICAL_LVL)
         << "[MotionStarStandalone] getYPos() for bird " << i
         << " does not support data format " << FLOCK::getFormatName(format)
         << std::endl << vprDEBUG_FLUSH;
      y_pos = 0.0;
   }

   return y_pos;
}

// ----------------------------------------------------------------------------
// Get the z position of the i'th bird.
// ----------------------------------------------------------------------------
float MotionStarStandalone::getZPos(const unsigned int i) const
{
   float z_pos;
   FLOCK::data_format format;

   format = m_birds[i]->format;

   if ( format == FLOCK::POSITION || format == FLOCK::POSITION_ANGLES ||
        format == FLOCK::POSITION_MATRIX ||
        format == FLOCK::POSITION_QUATERNION )
   {
      // Ahh, the beauty of a union is is that this statement works for
      // any of these data formats since the memory address is the same in
      // all cases.
      z_pos = toZPos((unsigned char*) &m_birds[i]->data) / m_unit_conv;
   }
   else
   {
      vprDEBUG(vprDBG_ALL, vprDBG_CRITICAL_LVL)
         << "[MotionStarStandalone] getZPos() for bird " << i
         << " does not support data format " << FLOCK::getFormatName(format)
         << std::endl << vprDEBUG_FLUSH;
      z_pos = 0.0;
   }

   return z_pos;
}

// ----------------------------------------------------------------------------
// Get the z rotation of the i'th bird.
// ----------------------------------------------------------------------------
float MotionStarStandalone::getZRot(const unsigned int i) const
{
   float z_rot;
   FLOCK::data_format format;

   format = m_birds[i]->format;

   if ( format == FLOCK::ANGLES )
   {
      z_rot = toZRot(&(m_birds[i]->data.ANGLES[0]));
   }
   else if ( format == FLOCK::POSITION_ANGLES )
   {
      z_rot = toZRot(&(m_birds[i]->data.POSITION_ANGLES[6]));
   }
   else
   {
      vprDEBUG(vprDBG_ALL, vprDBG_CRITICAL_LVL)
         << "[MotionStarStandalone] getZRot() for bird " << i
         << " does not support data format " << FLOCK::getFormatName(format)
         << std::endl << vprDEBUG_FLUSH;
      z_rot = 0.0;
   }

   return z_rot;
}

// ----------------------------------------------------------------------------
// Get the y rotation of the i'th bird.
// ----------------------------------------------------------------------------
float MotionStarStandalone::getYRot(const unsigned int i) const
{
   float y_rot;
   FLOCK::data_format format;

   format = m_birds[i]->format;

   if ( format == FLOCK::ANGLES )
   {
      y_rot = toYRot(&(m_birds[i]->data.ANGLES[0]));
   }
   else if ( format == FLOCK::POSITION_ANGLES )
   {
      y_rot = toYRot(&(m_birds[i]->data.POSITION_ANGLES[6]));
   }
   else
   {
      vprDEBUG(vprDBG_ALL, vprDBG_CRITICAL_LVL)
         << "[MotionStarStandalone] getYRot() for bird " << i
         << " does not support data format " << FLOCK::getFormatName(format)
         << std::endl << vprDEBUG_FLUSH;
      y_rot = 0.0;
   }

   return y_rot;
}

// ----------------------------------------------------------------------------
// Get the x rotation of the i'th bird.
// ----------------------------------------------------------------------------
float MotionStarStandalone::getXRot(const unsigned int i) const
{
   float x_rot;
   FLOCK::data_format format;

   format = m_birds[i]->format;

   if ( format == FLOCK::ANGLES )
   {
      x_rot = toXRot(&(m_birds[i]->data.ANGLES[0]));
   }
   else if ( format == FLOCK::POSITION_ANGLES )
   {
      x_rot = toXRot(&(m_birds[i]->data.POSITION_ANGLES[6]));
   }
   else
   {
      vprDEBUG(vprDBG_ALL, vprDBG_CRITICAL_LVL)
         << "[MotionStarStandalone] getXRot() for bird " << i
         << " does not support data format " << FLOCK::getFormatName(format)
         << std::endl << vprDEBUG_FLUSH;
      x_rot = 0.0;
   }

   return x_rot;
}

// ----------------------------------------------------------------------------
// Get the rotation angles of the requested bird as matrix elements that are
// then stored in the given array.
// ----------------------------------------------------------------------------
void MotionStarStandalone::getMatrixAngles(const unsigned int bird,
                                           float angles[3])
   const
{
   FLOCK::data_format format;

   format = m_birds[bird]->format;

   if ( format == FLOCK::MATRIX || format == FLOCK::POSITION_MATRIX )
   {
      float x_val, y_val, z_val, radians;
      float y_cos;

      // Extract the values from the angle matrix that will be used to get
      // the actual rotation values.  The X value comes from element (2, 3);
      // the Y value comes from element (1, 3); and the Z value comes from
      // element (1, 2).  Refer to page 84 of the MotionStar Installation
      // and Operation Guide for the data format.  Data read is in network
      // byte order (MS byte, LS byte).

      // MATRIX format.
      if ( format == FLOCK::MATRIX )
      {
         // Element (2, 3).
         x_val = toFloat(m_birds[bird]->data.MATRIX[14],
                         m_birds[bird]->data.MATRIX[15]);

         // Element (1, 3).
         y_val = toFloat(m_birds[bird]->data.MATRIX[12],
                         m_birds[bird]->data.MATRIX[13]);

         // Element (1, 2).
         z_val = toFloat(m_birds[bird]->data.MATRIX[6],
                         m_birds[bird]->data.MATRIX[7]);
      }
      // POSITION/MATRIX format.
      else
      {
         // Element (2, 3).
         x_val = toFloat(m_birds[bird]->data.POSITION_MATRIX[20],
                         m_birds[bird]->data.POSITION_MATRIX[21]);

         // Element (1, 3).
         y_val = toFloat(m_birds[bird]->data.POSITION_MATRIX[18],
                         m_birds[bird]->data.POSITION_MATRIX[19]);

         // Element (1, 2).
         z_val = toFloat(m_birds[bird]->data.POSITION_MATRIX[12],
                         m_birds[bird]->data.POSITION_MATRIX[13]);
      }

      // Get the y-axis rotation first.  It is the opposite of the arcsine
      // of element (1, 3) in the matrix.
      radians   = asin(-y_val);
      y_cos     = cos(radians);
      angles[1] = RAD2DEG(radians);

      // Get the x-axis rotation.  It is the arcsine of element (2, 3)
      // divided by the cosine of y.
      radians   = asin(x_val / y_cos);
      angles[0] = RAD2DEG(radians);

      // Get the z-axis rotation.  It is the arcsine of element (1, 2)
      // divided by the cosine of y.
      radians   = asin(z_val / y_cos);
      angles[2] = RAD2DEG(radians);
   }
   else
   {
      vprDEBUG(vprDBG_ALL, vprDBG_CRITICAL_LVL)
         << "[MotionStarStandalone] getMatrixAngles() for bird " << bird
         << " does not support data format " << FLOCK::getFormatName(format)
         << std::endl << vprDEBUG_FLUSH;
      angles[0] = angles[1] = angles[2] = 0.0;
   }
}

// ----------------------------------------------------------------------------
// Get the rotation angles of the i'th bird as quaternion parameters that are
// then stored in the given array.
// ----------------------------------------------------------------------------
void MotionStarStandalone::getQuaternion(const unsigned int bird, float quat[4])
   const
{
   FLOCK::data_format format;

   format = m_birds[bird]->format;

   // Read the quaternion parameters from the data block.  Refer to page 92
   // of the MotionStar Installation and Operation Guide for details on the
   // data format.  Data is read in network byte order (MS byte, LS byte).

   // QUATERNION format.
   if ( format == FLOCK::QUATERNION )
   {
      for ( int i = 0, word = 0; word < 8; i++, word += 2 )
      {
         quat[i] = toFloat(m_birds[bird]->data.QUATERNION[word],
                           m_birds[bird]->data.QUATERNION[word + 1]);
      }
   }
   // POSITION/QUATERNION format.
   else if ( format == FLOCK::POSITION_QUATERNION )
   {
      for ( int i = 0, word = 6; word < 14; i++, word += 2 )
      {
         quat[i] = toFloat(m_birds[bird]->data.POSITION_QUATERNION[word],
                           m_birds[bird]->data.POSITION_QUATERNION[word + 1]);
      }
   }
   else
   {
      vprDEBUG(vprDBG_ALL, vprDBG_CRITICAL_LVL)
         << "[MotionStarStandalone] getQuaternion() for bird " << bird
         << " does not support data format " << FLOCK::getFormatName(format)
         << std::endl << vprDEBUG_FLUSH;
      quat[0] = quat[1] = quat[2] = quat[3] = 0.0;
   }
}

// ============================================================================
// Private methods.
// ============================================================================

// ----------------------------------------------------------------------------
// Send a wake-up call to the MotionStar server.
// ----------------------------------------------------------------------------
vpr::ReturnStatus MotionStarStandalone::sendWakeUp()
   throw(mstar::CommandException)
{
   vpr::ReturnStatus status;

   try
   {
      BIRDNET::HEADER msg(BIRDNET::MSG_WAKE_UP), rsp;

      status = sendMsg(&msg);

      try
      {
         getRsp(&rsp);

         // If we got a response but there was an error from the server, shut
         // it down and send the wake-up message again.  This is the
         // recommended procedure documented in the operation guide.
         if ( rsp.error_code != 0 )
         {
            BIRDNET::HEADER shutdown_msg(BIRDNET::MSG_SHUT_DOWN);

            printError(rsp.error_code);
            vprDEBUG(vprDBG_ALL, vprDBG_STATE_LVL)
               << "[MotionStarStandalone] Reinitializing server and sending "
               << "wake-up call again" << std::endl << vprDEBUG_FLUSH;

            try
            {
               status = sendMsg(&shutdown_msg);

               try
               {
                  // This reuses the rsp object declared above.
                  status = getRsp(&rsp);

                  if ( rsp.error_code != 0 )
                  {
                     printError(rsp.error_code);
                  }
               }
               catch (...)
               {
                  /* We don't care about exceptions in this case. */ ;
               }
            }
            catch (mstar::NetworkWriteException write_ex)
            {
               vprDEBUG(vprDBG_ERROR, vprDBG_CRITICAL_LVL)
                  << "[MotionStarStandalone] ERROR: Failed to reinitialize "
                  << "server using second wake-up call" << std::endl
                  << vprDEBUG_FLUSH;
               throw mstar::CommandException(write_ex.getMessage());
            }
         }
      }
      catch (mstar::NetworkReadException ex)
      {
         vprDEBUG(vprDBG_ALL, vprDBG_WARNING_LVL)
            << "[MotionStarStandalone] WARNING: Failed to read response "
            << "after sending wake-up call" << std::endl << vprDEBUG_FLUSH;
      }
   }
   // If the wake-up packet could not be sent to the server, print a
   // warning message.
   catch (mstar::NetworkWriteException ex)
   {
      vprDEBUG(vprDBG_ERROR, vprDBG_CRITICAL_LVL)
         << "[MotionStarStandalone] ERROR: Could not send wake-up message "
         << "to server!" << std::endl << vprDEBUG_FLUSH;
      throw mstar::CommandException(ex.getMessage());
   }

   return status;
}

// ----------------------------------------------------------------------------
// Get the system status.
// ----------------------------------------------------------------------------
BIRDNET::SYSTEM_STATUS* MotionStarStandalone::getSystemStatus()
   throw(mstar::CommandException)
{
   BIRDNET::SYSTEM_STATUS* status_info;

   try
   {
      BIRDNET::DATA_PACKET* sys_status;
      sys_status = getDeviceStatus(0);

      unsigned char flock_number;

      // Set the status info block to the address of the system status
      // packet's data buffer.  This will be returned to the caller.
      status_info = (BIRDNET::SYSTEM_STATUS*) &(sys_status->buffer[0]);

      // Save these values.
      // NOTE: Most of them are not currently being used, but they may prove
      // useful at a later time.
      flock_number        = status_info->flockNumber;
      m_chassis_id        = status_info->chassisNumber;
      m_chassis_dev_count = status_info->chassisDevices;
      m_cur_mrate         = convertMeasurementRate(status_info->measurementRate);

      // If this is the master chassis, get the extra system information
      // about all of the devices connected to the chassis.  This will tell
      // us how many birds there are in the system and fill the m_birds
      // vector.
//      if ( m_master )
//      {
         unsigned char* fbb_devices;
         FBB::Device* cur_dev;

         // Get the start address for the "array" of devices.  Element 0
         // of this "array" is the first byte after the end of the system
         // status block.
         fbb_devices = &sys_status->buffer[sizeof(BIRDNET::SYSTEM_STATUS)];

         for ( unsigned char i = 0; i < flock_number; i++ )
         {
            // Create a new FBB Device object.
            cur_dev = new FBB::Device();

            // The ERC device is not put into the m_birds vector.  We do
            // not want to try reading data from it.
            if ( fbb_devices[i] & FBB::ERC )
            {
               m_erc_vec.push_back(cur_dev);

               cur_dev->ert3_present = fbb_devices[i] & FBB::ERT3;
               cur_dev->ert2_present = fbb_devices[i] & FBB::ERT2;
               cur_dev->ert1_present = fbb_devices[i] & FBB::ERT1;
               cur_dev->ert0_present = fbb_devices[i] & FBB::ERT0;
            }
            // Put all non-ERC devices in the m_birds vector.
            else
            {
               m_birds.push_back(cur_dev);
            }

            cur_dev->accessible = fbb_devices[i] & FBB::ACCESS;
            cur_dev->running    = fbb_devices[i] & FBB::RUNNING;
            cur_dev->has_sensor = fbb_devices[i] & FBB::SENSOR;
            cur_dev->is_erc     = fbb_devices[i] & FBB::ERC;

            // This is how we handle remembering which device this is
            // after this point.
            cur_dev->addr = i + 1;
         }

         // If the number of birds connected to the server is less than
         // the number of birds we want to sample, we have a problem.
         if ( flock_number < m_birds_requested )
         {
            vprDEBUG(vprDBG_ALL, vprDBG_WARNING_LVL)
               << "[MotionStarStandalone] WARNING: Needed "
               << (unsigned int) m_birds_requested
               << " birds, but the server only has "
               << (unsigned int) flock_number << std::endl << vprDEBUG_FLUSH;
         }
//      }
   }
   // If nothing was read into the system status block, set status_info to
   // NULL to inform the caller that something went wrong.
   catch (mstar::NoDeviceStatusException ex)
   {
      throw mstar::CommandException(ex.getMessage());
   }

   return status_info;
}

// ----------------------------------------------------------------------------
// Set the system status.
// ----------------------------------------------------------------------------
vpr::ReturnStatus MotionStarStandalone::setSystemStatus(BIRDNET::SYSTEM_STATUS* sysStatus,
                                                        const unsigned char xmtrNum,
                                                        const char dataRate[6])
{
   if ( sysStatus->transmitterNumber != xmtrNum )
   {
      sysStatus->transmitterNumber = xmtrNum;
      vprDEBUG(vprDBG_ALL, vprDBG_STATE_LVL)
         << "[MotionStarStandalone] Settting active transmitter to "
         << (unsigned int) xmtrNum << std::endl << vprDEBUG_FLUSH;
   }

   // Copy the contents of data_rate into the measuermentRate block.
   // XXX: This may be a little bit of overkill.  strncpy(3) is probably
   // sufficient.
   for ( int i = 0; i < 6; i++ )
   {
      sysStatus->measurementRate[i] = dataRate[i];
   }

   // Set the system status by setting the status for device 0.
   return setDeviceStatus(0, (char*) sysStatus,
                          sizeof(BIRDNET::SYSTEM_STATUS));
}

// ----------------------------------------------------------------------------
// Read the configurations of all the birds and send our configuration data
// to them.
// ----------------------------------------------------------------------------
unsigned int MotionStarStandalone::configureBirds()
{
   BIRDNET::BIRD_STATUS* bird_status;
   unsigned int bird_count;
   bool values_set;

   bird_count = 0;
   values_set = false;

   // Configure each of the birds on the server chassis.
   for ( unsigned int bird = 0; bird < m_birds.size(); bird++ )
   {
      // First, read the current configuration for the bird in question.
      bird_status = getBirdStatus(bird);

      if ( bird_status == NULL )
      {
         vprDEBUG(vprDBG_ALL, vprDBG_WARNING_LVL)
            << "[MotionStarStandalone] Could not get status for bird "
            << bird << std::endl << vprDEBUG_FLUSH;
      }
      // If we got the status for bird, parse it and send a new
      // configuration based on the bird's current configuration and our
      // own desired configuration.
      else
      {
         // Read the position scaling factor from the first available
         // device.
         if ( ! values_set )
         {
            getUnitInfo(bird, bird_status);

            // We have the values we need, and we don't want to set them again.
            values_set = true;
         }

         // If we are still reading configuration information for
         // requested birds, handle the setup steps.
         if ( bird < m_birds_requested )
         {
            unsigned char format;

            // XXX: Eventually, we would like to have all birds get their
            // format value set individually.
            switch (m_bird_format)
            {
               // 0 words, mode 0.
               case FLOCK::NO_BIRD_DATA:
                  format = 0x00;
                  m_birds[bird]->data_words = 0;
                  break;
               // 3 words, mode 1.
               case FLOCK::POSITION:
                  format = 0x31;
                  m_birds[bird]->data_words = 3;
                  break;
               // 3 words, mode 2.
               case FLOCK::ANGLES:
                  format = 0x32;
                  m_birds[bird]->data_words = 3;
                  break;
               // 9 words, mode 3.
               case FLOCK::MATRIX:
                  format = 0x93;
                  m_birds[bird]->data_words = 9;
                  break;
               // 6 words, mode 4.
               case FLOCK::POSITION_ANGLES:
                  format = 0x64;
                  m_birds[bird]->data_words = 6;
                  break;
               // 12 words (hex c), mode 5.
               case FLOCK::POSITION_MATRIX:
                  format = 0xc5;
                  m_birds[bird]->data_words = 12;
                  break;
               // 0 words, mode 6.
               case FLOCK::INVALID:
                  format = 0x06;
                  m_birds[bird]->data_words = 0;
                  break;
               // 4 words, mode 7.
               case FLOCK::QUATERNION:
                  format = 0x47;
                  m_birds[bird]->data_words = 4;
                  break;
               // 7 words, mode 8.
               case FLOCK::POSITION_QUATERNION:
                  format = 0x78;
                  m_birds[bird]->data_words = 7;
                  break;
            }

            // Set the current bird's format to the value in
            // m_bird_format.  Also set the report rate to the value in
            // m_report_rate and the hemisphere to the value in
            // m_hemisphere.
            m_birds[bird]->format      = m_bird_format;
            m_birds[bird]->hemisphere  = m_hemisphere;
            m_birds[bird]->report_rate = bird_status->status.reportRate;
            m_birds[bird]->setup       = bird_status->status.setup;

            // Fill in the bird_status struct.
            bird_status->status.setup      |= FLOCK::APPEND_BUTTON_DATA;
            bird_status->status.dataFormat = format;
            bird_status->status.reportRate = m_report_rate;
            bird_status->status.hemisphere = m_hemisphere;

            // XXX: The second clause in the conditional is here so that
            // we can use the 0-based m_birds_active in the sample()
            // method and use the < operator for comparison.
            if ( m_birds[bird]->data_words > 0 && bird < m_birds_requested )
            {
               m_birds_active++;
            }
         }
         // Otherwise, disable any birds we do not request.
         else
         {
            // Disable the current bird in the m_birds vector.
            m_birds[bird]->data_words  = 0;
            m_birds[bird]->format      = FLOCK::NO_BIRD_DATA;
            m_birds[bird]->report_rate = 1;
            m_birds[bird]->hemisphere  = m_hemisphere;

            // Fill in the bird_status struct with disabling values.
            bird_status->status.dataFormat = 0x00;
            bird_status->status.reportRate = 1;
         }

         // Finally, send the new configuration to the current bird.
         setBirdStatus(bird, bird_status);

         bird_count++;
         delete bird_status;
      }
   }

   vprDEBUG(vprDBG_ALL, vprDBG_CRITICAL_LVL)
      << "[MotionStarStandalone] Configured " << bird_count << " birds ("
      << (unsigned int) m_birds_requested << " requested, "
      << (unsigned int) m_birds_active << " active, "
      << (unsigned int) bird_count - m_birds_active << " disabled)"
      << std::endl << vprDEBUG_FLUSH;

   return bird_count;
}

// ----------------------------------------------------------------------------
// Get the status of an individual bird.
// ----------------------------------------------------------------------------
BIRDNET::BIRD_STATUS* MotionStarStandalone::getBirdStatus(const unsigned char bird)
{
   BIRDNET::BIRD_STATUS* bird_status;

   try
   {
      BIRDNET::DATA_PACKET* status;

      // The value in bird is the index into the m_birds vector.  Using that
      // element from the vector, we get the actual FBB address.
      status = getDeviceStatus(m_birds[bird]->addr);

      // The requested bird's status descrpition begins at the returned
      // packet's data buffer.  This is what will be returned to the caller.
      bird_status = (BIRDNET::BIRD_STATUS*) &(status->buffer[0]);

      // The dataFormat field contains the number of words (2 bytes) in
      // this bird's formatted data packet.  It is in the most significant
      // four bits.  See page 127 of the MotionStar Operation Guide.
      m_birds[bird]->data_words = (bird_status->status.dataFormat >> 4) & 0x0f;

      // The least significant four bits of dataFormat contain the format
      // of the data this bird will send by default.  Again, refer to page
      // 127 of the MotionStar Operation Guide.
      switch (bird_status->status.dataFormat & 0x0f)
      {
         case 0:
            m_birds[bird]->format = FLOCK::NO_BIRD_DATA;
            break;
         case 1:
            m_birds[bird]->format = FLOCK::POSITION;
            break;
         case 2:
            m_birds[bird]->format = FLOCK::ANGLES;
            break;
         case 3:
            m_birds[bird]->format = FLOCK::MATRIX;
            break;
         case 4:
            m_birds[bird]->format = FLOCK::POSITION_ANGLES;
            break;
         case 5:
            m_birds[bird]->format = FLOCK::POSITION_MATRIX;
            break;
         case 6:
            vprDEBUG(vprDBG_ALL, vprDBG_WARNING_LVL)
               << "[MotionStarStandalone] WARNING: Got INVALID data format "
               << "for bird " << (unsigned int) bird << std::endl
               << vprDEBUG_FLUSH;
            m_birds[bird]->format = FLOCK::INVALID;
            break;
         case 7:
            m_birds[bird]->format = FLOCK::QUATERNION;
            break;
         case 8:
            m_birds[bird]->format = FLOCK::POSITION_QUATERNION;
            break;
         default:
            vprDEBUG(vprDBG_ALL, vprDBG_WARNING_LVL)
               << "[MotionStarStandalone] WARNING: Got unknown data format "
               << (unsigned int) (bird_status->status.dataFormat & 0x0f)
               << " for bird " << (unsigned int) bird << std::endl
               << vprDEBUG_FLUSH;
            break;
      }

      switch (bird_status->status.hemisphere)
      {
         case 0:
            m_birds[bird]->hemisphere = FLOCK::FRONT_HEMISPHERE;
            break;
         case 1:
            m_birds[bird]->hemisphere = FLOCK::REAR_HEMISPHERE;
            break;
         case 2:
            m_birds[bird]->hemisphere = FLOCK::UPPER_HEMISPHERE;
            break;
         case 3:
            m_birds[bird]->hemisphere = FLOCK::LOWER_HEMISPHERE;
            break;
         case 4:
            m_birds[bird]->hemisphere = FLOCK::LEFT_HEMISPHERE;
            break;
         case 5:
            m_birds[bird]->hemisphere = FLOCK::RIGHT_HEMISPHERE;
            break;
         default:
            vprDEBUG(vprDBG_ALL, vprDBG_WARNING_LVL)
               << "[MotionStarStandalone] WARNING: Got unknown hemisphere "
               << (unsigned int) bird_status->status.hemisphere << " for bird "
               << (unsigned int) bird << std::endl << vprDEBUG_FLUSH;
            break;
      }

      m_birds[bird]->report_rate = bird_status->status.reportRate;
      m_birds[bird]->address     = bird_status->status.FBBaddress;
   }
   // If nothing was read, nothing can be returned.
   catch (mstar::NoDeviceStatusException ex)
   {
      bird_status = NULL;
   }

   return bird_status;
}

// ----------------------------------------------------------------------------
// Set the status of an individual bird.
// ----------------------------------------------------------------------------
vpr::ReturnStatus MotionStarStandalone::setBirdStatus(const unsigned char bird,
                                                      BIRDNET::BIRD_STATUS* status)
{
   // The value in bird is the index into the m_birds vector.  Using that
   // entry, we get the actual FBB address.
   return setDeviceStatus(m_birds[bird]->addr, (char*) status,
                          sizeof(BIRDNET::BIRD_STATUS));
}

// ----------------------------------------------------------------------------
// Get the status of the requested FBB device.  The device number must be that
// of the device's address on the Fast Bird Bus.  Thus, a value of 0 is
// interpreted as a request for the overall system status.  The birds are
// addressed from 1 through 120.
// ----------------------------------------------------------------------------
BIRDNET::DATA_PACKET* MotionStarStandalone::getDeviceStatus(const unsigned char device)
   throw(mstar::NoDeviceStatusException)
{
   vpr::ReturnStatus status;
   BIRDNET::HEADER msg(BIRDNET::MSG_GET_STATUS);
   BIRDNET::DATA_PACKET* rsp(NULL);

   // Set the xtype field to the given device number.
   msg.xtype = device;

   try
   {
      // Send the status request packet to the server.
      status = sendMsg(&msg);

      // Allocate the new data packet object that will be returned to the
      // caller.
      rsp = new BIRDNET::DATA_PACKET();

      try
      {
         // First read the header of the response.  It will contain the size
         // of the data part of the packet.
         getRsp(&(rsp->header));

         // If that succeeded (i.e., didn't throw an exception), read the rest
         // of the response using the size in the header's number_bytes field.
         try
         {
            getRsp((void*) &(rsp->buffer),
                   vpr::System::Ntohs(rsp->header.number_bytes));

            if ( rsp->header.error_code != 0 )
            {
               printError(rsp->header.error_code);
            }
         }
         catch (mstar::NetworkReadException ex)
         {
            vprDEBUG(vprDBG_ERROR, vprDBG_CRITICAL_LVL)
               << "[MotionStarStandalone] ERROR: Could not read status data buffer for device "
               << (unsigned int) device << " from server" << std::endl
               << vprDEBUG_FLUSH;
         }
      }
      catch (mstar::NetworkReadException ex)
      {
         std::stringstream msg_stream;
         msg_stream << "Could not read status header for device"
                    << (unsigned int) device << " from server";
         std::string msg(msg_stream.str());

         vprDEBUG(vprDBG_ERROR, vprDBG_CRITICAL_LVL)
            << "[MotionStarStandalone] ERROR: " << msg << std::endl
            << vprDEBUG_FLUSH;

         // There is nothing valid in rsp, so we can free its memory.
         delete rsp;
         rsp = NULL;

         throw mstar::NoDeviceStatusException(device, msg);
      }
   }
   catch (mstar::NetworkWriteException ex)
   {
      std::stringstream msg_stream;
      msg_stream << "Could not request status for device"
                 << (unsigned int) device;
      std::string msg(msg_stream.str());
      vprDEBUG(vprDBG_ERROR, vprDBG_CRITICAL_LVL)
         << "[MotionStarStandalone] ERROR: " << msg << std::endl
         << vprDEBUG_FLUSH;
      throw mstar::NoDeviceStatusException(device, msg);
   }

   return rsp;
}

// ----------------------------------------------------------------------------
// Set the status of the requested FBB device using the given configuration
// block.  The device number must be that of the device's address on the Fast
// Bird Bus.  Thus, a value of 0 is interpreted as a configuation block for
// the overall system.  The birds are addressed from 1 through 120.
// ----------------------------------------------------------------------------
vpr::ReturnStatus MotionStarStandalone::setDeviceStatus(const unsigned char device,
                                                        const char* buffer,
                                                        const unsigned short bufferSize)
{
   BIRDNET::DATA_PACKET msg;
   size_t total_size;
   vpr::ReturnStatus status;

   // Get the total size of the packet that we construct based on the size
   // of the header and the size of the passed buffer.
   total_size = sizeof(BIRDNET::HEADER) + bufferSize;

   // Fill in the header bits.
   // Note that we do not increment m_seq_num.  It should only be
   // incremented when a response is received from the MotionStar chassis.
   msg.header.sequence     = vpr::System::Htons(m_seq_num);
   msg.header.type         = BIRDNET::MSG_SEND_SETUP;
   msg.header.xtype        = device;
   msg.header.number_bytes = vpr::System::Htons(bufferSize);

   // Copy the given buffer into the packet's data buffer.
   memcpy((void*) &msg.buffer[0], (void*) buffer, bufferSize);

   try
   {
      // Send the constructed packet to the server.
      status = sendMsg(&msg, total_size);

      try
      {
         BIRDNET::HEADER rsp;
         status = getRsp(&rsp);

         if ( rsp.error_code != 0 )
         {
            printError(rsp.error_code);
         }
      }
      catch (mstar::NetworkReadException ex)
      {
         vprDEBUG(vprDBG_ALL, vprDBG_WARNING_LVL)
            << "[MotionStarStandalone] WARNING: Could not read server response to device "
            << (unsigned int) device << " setup" << std::endl
            << vprDEBUG_FLUSH;
      }
   }
   catch (mstar::NetworkWriteException ex)
   {
      vprDEBUG(vprDBG_ALL, vprDBG_WARNING_LVL)
         << "[MotionStarStandalone] WARNING: Could not set device status for device "
         << (unsigned int) device << std::endl << vprDEBUG_FLUSH;
   }

   return status;
}

// ----------------------------------------------------------------------------
// Tell the MotionStar server to sample continuously.
// ----------------------------------------------------------------------------
vpr::ReturnStatus MotionStarStandalone::setContinuous()
   throw(mstar::CommandException)
{
   vpr::ReturnStatus status;

   try
   {
      BIRDNET::HEADER msg(BIRDNET::MSG_RUN_CONTINUOUS), rsp;

      status = sendMsg(&msg);

      try
      {
         status = getRsp(&rsp);

         if ( rsp.error_code != 0 )
         {
            printError(rsp.error_code);
         }
      }
      catch (mstar::NetworkReadException ex)
      {
         ;
      }
   }
   catch (mstar::NetworkWriteException ex)
   {
      std::string msg("Failed to put chassis in continuous data mode");
      vprDEBUG(vprDBG_ERROR, vprDBG_CRITICAL_LVL)
         << "[MotionStarStandalone] ERROR: " << msg << std::endl
         << vprDEBUG_FLUSH;
      throw mstar::CommandException(msg);
   }

   return status;
}

// ----------------------------------------------------------------------------
// Combine the two given bytes (passed as high byte and low byte respectively)
// into a single word in host byte order.  This is used for reading bytes from
// the packets and converting them into usable values.
// ----------------------------------------------------------------------------
short MotionStarStandalone::toShort(const char highByte, const char lowByte)
   const
{
   union
   {
      char  c[sizeof(short)];
      short value;
   } result;

   // If this is a big-endian host, put the high byte in the first byte of
   // result.value and the low byte in the second byte.
   if ( m_big_endian )
   {
      result.c[0] = highByte;
      result.c[1] = lowByte;
   }
   // Otherwise, put the low byte in the first byte and the high byte in the
   // second byte.
   else
   {
      result.c[0] = lowByte;
      result.c[1] = highByte;
   }

   return result.value;
}

// ----------------------------------------------------------------------------
// Convert the given two bytes into a single-precision floating-point value
// that falls within a defined range.  This is documented in the MotionStar
// Operation and Installation Guide.
// ----------------------------------------------------------------------------
float MotionStarStandalone::toFloat(const unsigned char highByte,
                                    const unsigned char lowByte)
   const
{
   unsigned short input;
   float result;

   // Construct the input from the given bytes.
   input = toShort(highByte, lowByte);

   // Use the values documented on page 92 of the MotionStar Installation and
   // Operation Guide.
   const unsigned short min_base = 0x8000;
   const float min               = -1.0f;
   const unsigned short max_base = 0x7fff;
   const float max               = 0.99996f;

   // If the input is 0, the result is 0.
   if ( input == 0 )
   {
      result = 0.0;
   }
   // If the input is greater than or equal to 0x8000, the result will be
   // negative and should be multiplied by min and divided by min_base.
   else if ( input >= min_base )
   {
      result = input * min / min_base;
   }
   // Otherwise, the result is positive and should be multiplied by max and
   // divided by max_base.
   else
   {
      result = input * max / max_base;
   }

   return result;
}

// ----------------------------------------------------------------------------
// Convert the given 6-byte array of characters to a double-precision
// floating-point number representing the data rate.
// ----------------------------------------------------------------------------
double MotionStarStandalone::convertMeasurementRate(const unsigned char rate[6])
{
   double data_rate;
   char data_rate_a[7];

   // Copy the bytes in status_info->measurementRate into data_rate_a.  I
   // know this is probably being overly cautious, but getting the byte
   // ordering right is important.
   for ( int i = 0; i < 6; i++ )
   {
      data_rate_a[i] = rate[i];
   }

   // Terminate the measurement rate string.
   data_rate_a[6] = '\0';

   // This conversion is described in the measurementRate row of the
   // General System Status table on page 121 of the MotionStar Operation and
   // Installation Guide.
   data_rate = atof(data_rate_a) / 1000.0;

   return data_rate;
}

// ----------------------------------------------------------------------------
// Convert the given double-precision floating-point number to a 6-byte array
// of characters representing the data rate.
// ----------------------------------------------------------------------------
void MotionStarStandalone::convertMeasurementRate(const double rate,
                                                  std::string& strRate)
{
   char rate_a[7];

#ifdef HAVE_SNPRINTF
   snprintf(rate_a, 7, "%06.0f", rate * 1000.0);
#else
   sprintf(rate_a, "%06.0f", rate * 1000.0);
#endif
   strRate = rate_a;
}

// ----------------------------------------------------------------------------
// Extract the information regarding the measurement units (e.g., inches) and
// the position scaling factor.
// ----------------------------------------------------------------------------
void MotionStarStandalone::getUnitInfo(const unsigned int bird,
                                       const BIRDNET::BIRD_STATUS* birdStatus)
{
   unsigned char high_byte, low_byte, units;

   high_byte = birdStatus->status.scaling[0];
   low_byte  = birdStatus->status.scaling[1];

   // The highest four bits of the high byte tell the measurement system
   // being used.
   units = high_byte >> 4;

   // Determine the measurement units from the value in units.
   if ( units == 0 )
   {
      m_units     = BIRDNET::INCHES;
      m_unit_conv = 12.0;
   }
   else {
      vprDEBUG(vprDBG_ALL, vprDBG_WARNING_LVL)
         << "[MotionStarStandalone] WARNING: Unknown units value "
         << (unsigned int) units << " read from scaling factor on bird "
         << bird << std::endl << vprDEBUG_FLUSH;
   }

   // Mask out the most significant four bits of the high byte because they
   // are used only to tell the measurement system.
   high_byte &= 0x0f;

   m_xmtr_pos_scale = (float) toShort(high_byte, low_byte);
}

// ----------------------------------------------------------------------------
// Send the given message to the server.
// ----------------------------------------------------------------------------
vpr::ReturnStatus MotionStarStandalone::sendMsg(const void* packet,
                                                const size_t packetSize)
   throw(mstar::NetworkWriteException, mstar::NoDataWrittenException)
{
   vpr::Uint32 bytes;
   vpr::ReturnStatus status;

   // Send the packet to the server.
   status = m_socket->send(packet, packetSize, bytes);

   // An error occurred while trying to send the packet.
   if ( status.failure() )
   {
      std::string msg("Could not send message to ");
      msg += m_address.getAddressString();

      vprDEBUG(vprDBG_ERROR, vprDBG_CRITICAL_LVL)
         << "[MotionStarStandalone] " << msg << std::endl << vprDEBUG_FLUSH;
      throw mstar::NetworkWriteException(msg);
   }
   // Nothing was sent.
   else if ( bytes == 0 )
   {
      std::string msg("Sent 0 bytes to ");
      msg += m_address.getAddressString();

      vprDEBUG(vprDBG_ERROR, vprDBG_CRITICAL_LVL)
         << "[MotionStarStandalone] " << msg << std::endl << vprDEBUG_FLUSH;
      throw mstar::NoDataWrittenException(msg);
   }

   return status;
}

vpr::ReturnStatus MotionStarStandalone::sendMsg(BIRDNET::HEADER* packet)
   throw(mstar::NetworkWriteException, mstar::NoDataWrittenException)
{
   vprASSERT(packet->sequence == 0 && "Sequence number already set");

   // NOTE: We do not increment m_seq_num here.  The response we get from the
   // server based on this message will have the same sequence number as what
   // is being assigned here.  Therefore, the sequence number should only be
   // incremented after a response is received.
   packet->sequence = vpr::System::Htons(m_seq_num);

   // If sendMsg() throws an exception, we will let it propagate up to the
   // caller.
   return sendMsg((void*) packet, sizeof(BIRDNET::HEADER));
}

// ----------------------------------------------------------------------------
// Get the server's response to a sent message.
// ----------------------------------------------------------------------------
vpr::ReturnStatus MotionStarStandalone::getRsp(void* packet,
                                               const size_t packetSize)
   throw(mstar::NetworkReadException, mstar::NoDataReadException)
{
   vpr::Uint32 bytes;
   vpr::ReturnStatus status;

   // Get the packet from the server.
   status = m_socket->recvn(packet, packetSize, bytes);

   // An error occurred while trying to receive the packet.
   if ( status.failure() )
   {
      std::string msg("Could not read message from ");
      msg += m_address.getAddressString();
      vprDEBUG(vprDBG_ERROR, vprDBG_CRITICAL_LVL)
         << "[MotionStarStandalone] " << msg  << std::endl << vprDEBUG_FLUSH;
      throw mstar::NetworkReadException(msg);
   }
   // Nothing was read.
   else if ( bytes == 0 )
   {
      std::string msg("Read 0 bytes from ");
      msg += m_address.getAddressString();
      vprDEBUG(vprDBG_ERROR, vprDBG_CRITICAL_LVL)
         << "[MotionStarStandalone] " << msg << std::endl << vprDEBUG_FLUSH;
      throw mstar::NoDataReadException(msg);
   }

   return status;
}

vpr::ReturnStatus MotionStarStandalone::getRsp(BIRDNET::HEADER* packet)
   throw(mstar::NetworkReadException, mstar::NoDataReadException)
{
   vpr::ReturnStatus status;

   // If getRsp() throws an exception, we will let it propagate up to the
   // caller.
   status = getRsp((void*) packet, sizeof(BIRDNET::HEADER));

   if ( status.success() )
   {
      const unsigned short seq_num(vpr::System::Ntohs(packet->sequence));

      // Verify that we got the message we were expecting based on its
      // sequence number.
      if ( seq_num != m_seq_num )
      {
         vprDEBUG(vprDBG_ALL, vprDBG_WARNING_LVL)
            << "[MotionStarStandalone]: "
            << clrOutBOLD(clrYELLOW, "WARNING") << ": Expected message #"
            << m_seq_num << " but received #" << seq_num << std::endl
            << vprDEBUG_FLUSH;

         // Resynchronize with the server's sequence number.
         m_seq_num = seq_num;
      }
      // Increment the sequence number for the next message we will send.
      else
      {
         m_seq_num++;
      }
   }

   return status;
}

// ----------------------------------------------------------------------------
// Print the system status as read from the server.
// ----------------------------------------------------------------------------
void MotionStarStandalone::printSystemStatus(const BIRDNET::SYSTEM_STATUS* status)
{
   unsigned char erc_addr, xmtr_num;
   unsigned int pad_width_full, pad_width_dot;
   char rev_str[8];
   float rev_num;

   // Set the padding for the dots that go between the description and the
   // corresponding value and for the full line width.  In this case, we are
   // using 8 more characters for the full line width to make it just wide
   // enough to accommodate the output data.
   pad_width_dot  = 52;
   pad_width_full = pad_width_dot + 8;

   // Refer to page 122 of the MotionStar Installation and Operation Guide
   // for information about the format of the transmitterNumber field.
   erc_addr = status->transmitterNumber >> 4;
   xmtr_num = status->transmitterNumber & 0x03;

   // Set left justification so that everything will be formatted properly.
   std::cout.setf(std::ios::left);

   // XXX: This should use vprDEBUG somehow...
   // Print a short heading.
   std::cout << std::setw(pad_width_full) << std::setfill('=') << "" << "\n";
   std::cout << "Ascension MotionStar General System Status ("
             << (m_master ? "master" : "slave") << ")\n";
   std::cout << std::setw(pad_width_full) << std::setfill('-') << "" << "\n";

   std::cout << std::setw(pad_width_dot) << std::setfill('.')
             << "* Number of Fast Bird Bus devices in system " << " "
             << (unsigned int) status->flockNumber << "\n";
   std::cout << std::setw(pad_width_dot) << std::setfill('.')
             << "* Number of servers in system " << " "
             << (unsigned int) status->serverNumber << "\n";
   std::cout << std::setw(pad_width_dot) << std::setfill('.')
             << "* Server chassis ID number " << " "
             << (unsigned int) status->chassisNumber << "\n";
   std::cout << std::setw(pad_width_dot) << std::setfill('.')
             << "* Number of devices within chassis " << " "
             << (unsigned int) status->chassisDevices << "\n";
   std::cout << std::setw(pad_width_dot) << std::setfill('.')
             << "* FBB address of first device in chassis " << " "
             << (unsigned int) status->firstAddress << "\n";
   std::cout << std::setw(pad_width_dot) << std::setfill('.')
             << "* Active transmitter FBB address " << " "
             << (unsigned int) erc_addr << "\n";
   std::cout << std::setw(pad_width_dot) << std::setfill('.')
             << "* Active transmitter number " << " "
             << (unsigned int) xmtr_num << "\n";
   std::cout << std::setw(pad_width_dot) << std::setfill('.')
             << "* Measurement rate " << " "
             << convertMeasurementRate(status->measurementRate) << "\n";
   sprintf(rev_str, "%u.%u", status->softwareRevision[0],
           status->softwareRevision[1]);
   rev_num = atof(rev_str);
   std::cout << std::setw(pad_width_dot) << std::setfill('.')
             << "* Server Software Revision " << " " << rev_num << "\n";

   // Finish off with another line of = signs.
   std::cout << std::setw(pad_width_full) << std::setfill('=') << "" << "\n";
   std::cout << std::flush;
}

// ----------------------------------------------------------------------------
// Print the status for all the devices that have not been disabled.
// ----------------------------------------------------------------------------
void MotionStarStandalone::printDeviceStatus()
{
   unsigned int pad_width_full, pad_width_dot;

   // Set the padding for the dots that go between the description and the
   // corresponding value and for the full line width.  In this case, we are
   // using 21 more characters for the full line width to make it just wide
   // enough to accommodate the output data.
   pad_width_dot  = 35;
   pad_width_full = pad_width_dot + 21;

   // Set left justification so that everything will be formatted properly.
   std::cout.setf(std::ios::left);

   // XXX: This should use vprDEBUG somehow...
   // Print a short heading.
   std::cout << std::endl;
   std::cout << std::setw(pad_width_full) << std::setfill('=') << "" << "\n";
   std::cout << "Ascension MotionStar Device Status\n";
   std::cout << std::setw(pad_width_full) << std::setfill('-') << "" << "\n";

   std::cout << std::setw(pad_width_dot) << std::setfill('.')
             << "* Position scaling factor " << " " << m_xmtr_pos_scale << "\n";
   std::cout << std::setw(pad_width_dot) << std::setfill('.')
             << "* Rotation scaling factor " << " " << m_xmtr_rot_scale << "\n";

   // Loop over the birds that we are actually using and print their status.
   for ( unsigned int i = 0; i < m_birds_active; i++ )
   {
      std::cout << "* Bird " << i + 1 << " (unit " << i << "):\n";
      std::cout << std::setw(pad_width_dot) << std::setfill('.')
                << "*     FBB address " << " "
                << (unsigned int) m_birds[i]->address << "\n";
      std::cout << std::setw(pad_width_dot) << std::setfill('.')
                << "*     Accessible " << " "
                << (m_birds[i]->accessible ? "YES" : "NO") << "\n";
      std::cout << std::setw(pad_width_dot) << std::setfill('.')
                << "*     Running " << " "
                << (m_birds[i]->running ? "YES" : "NO") << "\n";
      std::cout << std::setw(pad_width_dot) << std::setfill('.')
                << "*     Data format " << " "
                << FLOCK::getFormatName(m_birds[i]->format) << "\n";
      std::cout << std::setw(pad_width_dot) << std::setfill('.')
                << "*     Hemisphere " << " "
                << FLOCK::getHemisphereName(m_birds[i]->hemisphere)
                << "\n";
      std::cout << std::setw(pad_width_dot) << std::setfill('.')
                << "*     Report rate " << " "
                << (unsigned int) m_birds[i]->report_rate << "\n";
      std::cout << std::setw(pad_width_dot) << std::setfill('.')
                << "*     Sudden change filtering " << " "
                << (m_birds[i]->setup & FLOCK::SUDDEN_OUTPUT_CHANGE ? "ON" :
                                                                      "OFF")
                << "\n";
      std::cout << std::setw(pad_width_dot) << std::setfill('.')
                << "*     Button data " << " "
                << (m_birds[i]->setup & FLOCK::APPEND_BUTTON_DATA ? "ON" :
                                                                    "OFF")
                << "\n";
      std::cout << std::setw(pad_width_dot) << std::setfill('.')
                << "*     AC narrow filter " << " "
                << (m_birds[i]->setup & FLOCK::AC_NARROW_FILTER ? "ON" :
                                                                  "OFF")
                << "\n";
      std::cout << std::setw(pad_width_dot) << std::setfill('.')
                << "*     AC wide filter " << " "
                << (m_birds[i]->setup & FLOCK::AC_WIDE_FILTER ? "ON" : "OFF")
                << "\n";
      std::cout << std::setw(pad_width_dot) << std::setfill('.')
                << "*     DC filter " << " "
                << (m_birds[i]->setup & FLOCK::DC_FILTER ? "ON" : "OFF")
                << "\n";
   }

   // Finish off with another line of = signs.
   std::cout << std::setw(pad_width_full) << std::setfill('=') << "" << "\n";
   std::cout << std::flush;
}

// ----------------------------------------------------------------------------
// Print the error message that corresponds to the given error code.  The
// message is based on the table on page 140 of the MotionStar manual.
// ----------------------------------------------------------------------------
void MotionStarStandalone::printError(const unsigned char errCode)
{
   // Map the error code to a human-readable string.  These messages are
   // based on the table on page 140 of the MotionStar manual.
   switch (errCode)
   {
      case 1:
         vprDEBUG(vprDBG_ALL, vprDBG_WARNING_LVL)
            << clrOutBOLD(clrYELLOW, "WARNING:")
            << " Single packet missing (error 1)" << std::endl
            << vprDEBUG_FLUSH;
         break;
      case 2:
         vprDEBUG(vprDBG_ALL, vprDBG_WARNING_LVL)
            << clrOutBOLD(clrYELLOW, "WARNING:")
            << " Two or more packets missing (error 2)" << std::endl
            << vprDEBUG_FLUSH;
         break;
      case 3:
         vprDEBUG(vprDBG_ALL, vprDBG_WARNING_LVL)
            << clrOutBOLD(clrYELLOW, "WARNING:")
            << " Many, many packets missing (error 3)" << std::endl
            << vprDEBUG_FLUSH;
         break;
      case 4:
         vprDEBUG(vprDBG_ALL, vprDBG_WARNING_LVL)
            << clrOutBOLD(clrYELLOW, "WARNING:")
            << " Packet sequence number repeated (error 4)" << std::endl
            << vprDEBUG_FLUSH;
         break;
      case 6:
         vprDEBUG(vprDBG_ALL, vprDBG_WARNING_LVL)
            << clrOutBOLD(clrYELLOW, "WARNING:")
            << " Unexpected packet type received (error 6)\n"
            << "Re-syncing may be necessary!" << std::endl << vprDEBUG_FLUSH;
//         fprintf(stderr, "         Illegal packet sequence number: %hu\n",
//                 error_code_extension);
         break;
      case 7:
         vprDEBUG(vprDBG_ALL, vprDBG_WARNING_LVL)
            << clrOutBOLD(clrYELLOW, "WARNING:")
            << " Packet contents could not be determined (error 7)"
            << std::endl << vprDEBUG_FLUSH;
         break;
      case 8:
         vprDEBUG(vprDBG_ALL, vprDBG_WARNING_LVL)
            << clrOutBOLD(clrYELLOW, "WARNING:")
            << " Status requested for non-existent FBB device (error 8)"
            << std::endl << vprDEBUG_FLUSH;
         break;
      case 9:
         vprDEBUG(vprDBG_ALL, vprDBG_WARNING_LVL)
            << clrOutBOLD(clrYELLOW, "WARNING:")
            << " Illegal setup data sent (error 9)" << std::endl
            << vprDEBUG_FLUSH;
         break;
      case 100:
         vprDEBUG(vprDBG_ALL, vprDBG_WARNING_LVL)
            << clrOutBOLD(clrYELLOW, "WARNING:")
            << " System not ready (error 100)" << std::endl << vprDEBUG_FLUSH;
         break;
   }
}