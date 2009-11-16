/****************** <SNX heading BEGIN do not edit this line> *****************
 *
 * sonix
 *
 * Original Authors:
 *   Kevin Meinert, Carolina Cruz-Neira
 *
 ****************** <SNX heading END do not edit this line> ******************/

/*************** <auto-copyright.pl BEGIN do not edit this line> **************
 *
 * VR Juggler is (C) Copyright 1998-2009 by Iowa State University
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

/* Generated by Together */

#ifndef SONIX___H
#define SONIX___H

#include <snx/snxConfig.h>

#include <stdlib.h>
#include <string>
#include <gmtl/Matrix.h>
#include <vpr/Util/Singleton.h>

#include "snx/SoundInfo.h"
#include "snx/SoundImplementation.h"
#include "snx/SoundAPIInfo.h"


namespace snx
{

/** \class sonix sonix.h snx/sonix.h
 *
 * Singelton entry point to the Sonix API.
 */
class SNX_CLASS_API sonix
{
protected:
   /** Default constructor. */
   sonix() : mImplementation( NULL )
   {
   }

   /** Virtual destructor. */
   ~sonix();

public:

   /**
    * Triggers a sound.
    *
    * @pre alias does not have to be associated with a loaded sound.
    * @post If alias is associated with a loaded sound, then the loaded sound
    *       is triggered.  If it isn't, then nothing happens.
    *
    * @param alias  Alias of the sound to trigger.
    * @param repeat The number of times to play.  Use -1 to repeat forever.
    *              -1 to repeat infinately, 1 (single shot) is default.
    */
   void trigger(const std::string& alias, const int repeat = 1);

   /**
    * Is the named sound currently playing?
    *
    * @param alias The alias of the sound to query.
    */
   bool isPlaying(const std::string& alias);

   /**
    * Specifies whether the named sound retriggers from beginning when
    * triggered while playing.  In other words, when the named sound is already
    * playing and trigger() is called, does the sound restart from the
    * beginning?
    *
    * @param alias The alias of the sound to change.
    * @param onOff A Boolean value enabling or disabling retriggering.
    */
   void setRetriggerable(const std::string& alias, bool onOff);

   /**
    * Is the named sound retriggerable?
    *
    * @param alias The alias of the sound to query.
    */
   bool isRetriggerable(const std::string& alias);

   /**
    * Stops the named sound.
    *
    * @param alias Alias of the sound to be stopped.
    */
   void stop(const std::string& alias);

   /**
    * Pauses the named sound.  Use unpause() to return playback from where the
    * sound was paused.
    *
    * @param alias The alias of the sound to pause.
    */
   void pause(const std::string& alias);

   /**
    * Resumes playback of the named sound from a paused state.  This does
    * nothing if sound was not paused.
    *
    * @param alias The alias of the sound to unpause.
    */
   void unpause(const std::string& alias);

   /**
    * If the named sound is paused, then return true.
    *
    * @param alias The alias of the sound to query.
    */
   bool isPaused(const std::string& alias);

   /**
    * Sets the named sound as either ambient or positional depending on the
    * value of ambient.  If the sound is ambient, it is attached to the
    * listener, and its volume does not change when the listener moves.  If
    * the sound is positional, the volume changes when the listener moves.
    *
    * @param ambient A flag identifying whether this sound is ambient (true)
    *                or positional (false).  This parameter is optional,
    *                and it defaults to false (the sound is positional).
    */
   void setAmbient(const std::string& alias, const bool ambient = false);

   /**
    * Is the named sound ambient?
    *
    * @param alias The alias of the sound to query.
    */
   bool isAmbient(const std::string& alias);

   /**
    * Alters the frequency of the named sound.
    *
    * @param alias  The alias of the sound to change.
    * @param amount The value that determines the pitch bend.  1.0 means
    *               that there is no change.  A value less than 1.0 is low;
    *               a value greather than 1.0 is high.
    */
   void setPitchBend(const std::string& alias, float amount);

   /**
    * Sets the effect volume of the named sound.  The value must be in the
    * range [0,1].
    *
    * @param alias  The alias of the sound to change.
    * @param amount The value of the volume.  It must be between 0.0 and
    *               1.0 inclusive.
    */
   void setVolume(const std::string& alias, float amount);

   /**
    * Sets the effect cutoff of the named soudn.  The value must be in the
    * range [0,1].
    *
    * @param alias  The alias of the sound to change.
    * @param amount The value of the cutoff.  1.0 is no change; 0.0 is
    *               total cutoff.
    */
   void setCutoff(const std::string& alias, float amount);

   /**
    * Set sound's 3D position.
    *
    * @param alias A name that has been associated with some sound data.
    * @param x     The X coordinate of the sound in 3D OpenGL coordinates.
    * @param y     The Y coordinate of the sound in 3D OpenGL coordinates.
    * @param z     The Z coordinate of the sound in 3D OpenGL coordinates.
    */
   void setPosition(const std::string& alias, const float x, const float y,
                    const float z);

   /**
    * Get sound's 3D position
    *
    * @param alias A name that has been associated with some sound data.
    * @param x     Storage for the X coordinate of the sound in 3D OpenGL
    *              coordinates.
    * @param y     Storage for the Y coordinate of the sound in 3D OpenGL
    *              coordinates.
    * @param z     Storage for the Z coordinate of the sound in 3D OpenGL
    *              coordinates.
    */
   void getPosition(const std::string& alias, float& x, float& y, float& z);

   /**
    * Sets the position of the listener.
    *
    * @param mat A transformation matrix representing the position of the
    *            listener.
    */
   void setListenerPosition(const gmtl::Matrix44f& mat);

   /**
    * Gets the position of the listener.
    *
    * @param mat Storage for returning the position of the listener.
    */
   void getListenerPosition(gmtl::Matrix44f& mat);

   /**
    * Changes the underlying sound API to something else.  This function is
    * safe.  It always changes to a valid implementation.  It runs in O(1)
    * (constant) time.
    *
    * @pre The named sound implementation should be registered.
    * @post The underlying API is changed to the requested API name.   If
    *       apiName's implementation is not registered, then the underlying API
    *       is changed to the stub version.
    *
    * @param apiName Usually a name of a valid, registered sound API
    *                implementation.
    */
   void changeAPI(const std::string& apiName);

   /**
    * If there is a sound API currently active, its sounds are unbound, and
    * it is told to shut itself down.
    *
    * @post \c mImplementation is NULL.
    *
    * @since 1.3.6
    */
   void shutdownAPI();

   /**
    * Configures/reconfigures the sound API global settings.
    *
    * @param sai A description of the settings for this sound API.
    */
   void configure(const snx::SoundAPIInfo& sai);

   /**
    * Configures/reconfigures the named sound by associating sound data with
    * the named sound.  Afterwards, the alias can be used to operate on sound
    * data.
    *
    * Configure: associate a name (alias) to the description if not already
    * done.
    *
    * Reconfigure: change properties of the sound to the description
    * provided.
    *
    * @pre Provide a SoundInfo which describes the sound.
    * @post This handle will point to loaded sound data.
    *
    * @param description An object that describes the sound for which this
    *                    object will be a handle.
    */
   void configure(const std::string& alias,
                  const snx::SoundInfo& description);

   /**
    * Removes a configured sound.  Any future reference to the alias will not
    * cause an error, but it will not result in a rendered sound.
    *
    * @param alias The alias of the sound to be removed.
    */
   void remove(const std::string alias);

   /**
    * Call once per sound frame (which does not have to be same as the graphics
    * frame).
    *
    * @param timeElapsed Time elapsed since the last sound frame.
    */
   void step(const float timeElapsed);

protected:
   snx::ISoundImplementation& impl();

private:
   /** @link dependency */
   /*#  snx::SoundFactory lnkSoundFactory; */

   /** @link aggregation
    * @clientCardinality 1
    * @supplierCardinality 1*/
   snx::ISoundImplementation* mImplementation;

   /** sonix API includes objects of this type
    * @link dependency */
   /*#  snx::SoundInfo lnkSoundInfo; */

   /** @link dependency */
   /*#  snx::SoundAPIInfo lnksnx::SoundAPIInfo; */

   vprSingletonHeader(sonix);
};

} // namespace snx

#endif //SONIX___H
