#include <vjConfig.h>

#include <assert.h>
#include <stdio.h>
#include <string>
#include <aw.h> //audio works
#include <Sound/Sound.h>

#include <Sound/aw/AwSound.h> // my header

//specify the sound's name in the adf file...
AwSound::AwSound( SoundEngine& engine ) : Sound( engine ), mSoundPlayer( NULL ), mSound( NULL )
{
}

// loads the sound from an alias as specified in the audioworks .adf file
bool AwSound::load( const char* const alias )
{
   // default functionality...
   Sound::load( alias );

   mSoundName = alias;

   // first look for a sound-player in the .adf file.
   // they support position
   mSoundPlayer = awFindPlyr( mSoundName.data() );
   if (mSoundPlayer == NULL)
   {
      cout<<"[aw] Loading \""<<mSoundName.data()<<"\" as an awSound (non-positional type)\n"<<flush;
   }
   else
      cout<<"[aw] Loading \""<<mSoundName.data()<<"\" as an awPlayer (positional type)\n"<<flush;
   
   // find the sound with the same name.
   mSound = awFindSnd( mSoundName.data() );
      
   if (mSound == NULL)
   {
      char buffer[256];
      sprintf( buffer, " Unable to locate sound %s\n", mSoundName.data() );
      awNotify( AW_FATAL, AW_APP, buffer );
      return false;
   }
  
   // allow user to trigger sound by default
   // user must call play and stop to control sound
   if (mSoundPlayer != NULL)
   {
      // always on.
      awProp( mSoundPlayer, AWPLYR_STATE, AWSND_ON );   
   }
   
   awProp( mSound, AWSND_RETRIGGER, AWSND_ON );
   
   // get the observer tht the player is reletive to.
   awObserver* obs = awFindObs( "you" );
   if (obs == NULL)
   {
      cout<<"[aw] \n"
          <<"[aw] !!! WARNING !!!: could not find in the .adf file the \"observer\" named \"you\".  \n"
          <<"[aw] !!!         !!!: This *has* to be there, or else none of the sound localization \n"
          <<"[aw] !!!         !!!:   functions will work (like setPosition). \n"
          <<"[aw] \n"<<flush;
      // make the sound absolute (reletive to 0,0,0?)
      awProp( mSoundPlayer, AWPLYR_CSREF, AWPLYR_ABSOLUTE );
      return true; // observer failed, sound did not, if it's a player it will still be reletive to absolute (i think?)
   }
   
   else
   {
      // set the player reletive to the observer.
      if (mSoundPlayer != NULL)
      {
         // make the sound reletive to an observer (this should be default, so isn't nessesary i think)
         awPlyrObsRef( mSoundPlayer, obs );
         awProp( mSoundPlayer, AWPLYR_CSREF, AWPLYR_OBS );
      }
   }
      
   return true;
}   

AwSound::~AwSound() {}

void AwSound::pitchBend( float amount )
{
   assert( mSound != NULL );
   // turn on the sound by default
   cout<<"[aw] awProp( mSoundPlayer, AWSND_PBEND, "<<amount<<" )\n"<<flush;
   
   //if (mSoundPlayer != NULL)
   //   awProp( mSoundPlayer, AWSND_PBEND, amount );
   awProp( mSound, AWSND_PBEND, amount );
}   

// allows a user to enable or disable a sound without worring about its state set by start or stop.
// NOTE: to hear a sound enable() and play() must be active.
//       default is ON
void AwSound::enable( Sound::BinaryState state )
{
   assert( mSound != NULL );
   switch (state)
   {
      case Sound::ON:
         cout<<"[aw] awProp( mSoundPlayer, AWSND_ENABLED, "<<AWSND_ON<<" )\n"<<flush;
         if (mSoundPlayer != NULL)
            awProp( mSoundPlayer, AWSND_ENABLED, AWSND_ON );
         awProp( mSound, AWSND_ENABLED, AWSND_ON );
         break;
      case Sound::OFF:
         cout<<"[aw] awProp( mSoundPlayer, AWSND_ENABLED, "<<AWSND_OFF<<" )\n"<<flush;
         if (mSoundPlayer != NULL)
            awProp( mSoundPlayer, AWSND_ENABLED, AWSND_OFF );
         awProp( mSound, AWSND_ENABLED, AWSND_OFF );
         break;
   }
}

void AwSound::trigger()
{
   assert( mSound != NULL );
   // make sure that the state is observed     
   
   switch (mLooping)
   {
      case 0:
         cout<<"[aw] Playing audio "<<mSoundName.data()<<"\n"<<flush;
         if (mSoundPlayer != NULL)
            awProp( mSoundPlayer, AWPLYR_STATE, AWSND_ON );
         awProp( mSound, AWSND_STATE, AWSND_ON );
         //awProp( mSound, AWSND_RETRIGGER, AWSND_ON );
         break;
         
      default:
      case -1:
         cout<<"[aw] Looping audio "<<mSoundName.data()<<"\n"<<flush;
         if (mSoundPlayer != NULL)
            awProp( mSoundPlayer, AWPLYR_STATE, AWSND_ON );
         awProp( mSound, AWSND_NLOOPS, mLooping );
         awProp( mSound, AWSND_STATE, AWSND_ON );
         //awProp( mSound, AWSND_RETRIGGER, AWSND_ON );
         break;
   }
}

// stop the sound.
// useful when a sound has been set to LOOP, this will stop it
void AwSound::stop()
{
   assert( mSound != NULL );

   if (mSoundPlayer != NULL)
     awProp( mSoundPlayer, AWPLYR_STATE, AWSND_OFF );
   awProp( mSound, AWSND_STATE, AWSND_OFF );
}

// change the position of the sound.
void AwSound::setPosition( float x, float y, float z )
{
   assert( mSound != NULL );
   // set the base functionality
   Sound::setPosition( x, y, z );
   
   // you can set position for a awPlayer object only...
   float xyz[3] = {x, y, z};
   float hpr[3] = {0.0f, 0.0f, 0.0f};

   if (mSoundPlayer != NULL)
   {
      cout<<"[aw] Setting player position to "<<xyz[0]<<" "<<xyz[1]<<" "<<xyz[2]<<"\n"<<flush;
      awXYZHPR( mSoundPlayer, xyz, hpr);
   }
   // doesn't work... second awSound man page says it should...
   //awXYZHPR( mSound, xyz, hpr );
}

// output to stdout, useful for debugging.
void AwSound::print()
{
   assert( mSound != NULL );
   
   if (mSoundPlayer != NULL)
   {
      cout<<"[aw] Printing the Player: \n"<<flush;
      awPrint( mSoundPlayer );
   }
   
   cout<<"[aw] Printing the Sound: \n"<<flush;
   awPrint( mSound );
}
