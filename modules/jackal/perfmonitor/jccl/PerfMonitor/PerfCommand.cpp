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



#include <jccl/JackalServer/Command.h>
#include <jccl/Config/ConfigChunkDB.h>
#include <jccl/Config/ChunkDescDB.h>
#include <jccl/Config/ConfigIO.h>
#include <jccl/Performance/PerfDataBuffer.h>

namespace jccl {


    // generic Command

    Command::Command () {
        ;
    }


    // Periodic Command

    PeriodicCommand::PeriodicCommand (float _rt): refresh_time (_rt) {
        next_fire_time = 0.0f;
    }

    void PeriodicCommand::resetFireTime (TimeStamp& ts) {
        next_fire_time = ts.usecs()/1000 + refresh_time;
    }


    int PeriodicCommand::operator < (const PeriodicCommand& cmd2) const {
        // used in priority queue in connects.
        // true if self should be called _after_ cmd2
        return (next_fire_time < cmd2.next_fire_time);
    }
    



    // CommandRefresh

    /*static*/ const std::string CommandRefresh::protocol_name ("xml_config");


    CommandRefresh::CommandRefresh() {
        ;
    }

    
    /*virtual*/ void CommandRefresh::call (std::ostream& out) const {
        out << "<refresh_all/>\n";
    }


    /*virtual*/ const std::string& CommandRefresh::getProtocolName () const {
        return protocol_name;
    }



    // CommandSendChunkDB

    /*static*/ const std::string CommandSendChunkDB::protocol_name ("xml_config");


    CommandSendChunkDB::CommandSendChunkDB (ConfigChunkDB* _db, bool _all) {
        db = _db;
        all = _all;
    }


    /*virtual*/ void CommandSendChunkDB::call (std::ostream& out) const {
        if (all)
            out << "<apply_chunks all=\"true\">\n";
        else
            out << "<apply_chunks>\n";
        ConfigIO::instance()->writeConfigChunkDB (out, *db, "xml_config");
        out << "</apply_chunks>\n";
    }


    /*virtual*/ const std::string& CommandSendChunkDB::getProtocolName () const {
        return protocol_name;
    }



    //CommandSendDescDB
    
    /*static*/ const std::string CommandSendDescDB::protocol_name ("xml_config");


    CommandSendDescDB::CommandSendDescDB (ChunkDescDB* _db, bool _all) {
        db = _db;
        all = _all;
    }

    
    /*virtual*/ void CommandSendDescDB::call (std::ostream& out) const {
        if (all)
            out << "<apply_descs all=\"true\">\n";
        else
            out << "<apply_descs>\n";
        ConfigIO::instance()->writeChunkDescDB (out, *db, "xml_config");
        out << "</apply_descs>\n";
    }


    /*virtual*/ const std::string& CommandSendDescDB::getProtocolName () const {
        return protocol_name;
    }



    // CommandWritePerfData

    /*static*/ const std::string CommandWritePerfData::protocol_name ("vjc_performance");
    

    CommandWritePerfData::CommandWritePerfData (PerfDataBuffer* _perf_buffer, 
                                                float _refresh_time) :PeriodicCommand (_refresh_time) {
        perf_buffer = _perf_buffer;
    }

    
    /*virtual*/ void CommandWritePerfData::call (std::ostream& out) const {
    perf_buffer->write (out);
    }


    /*virtual*/ const std::string& CommandWritePerfData::getProtocolName () const {
        return protocol_name;
    }
    
};
