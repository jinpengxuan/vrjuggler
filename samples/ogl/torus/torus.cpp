//----------------------------------------
// Test OpenGL VR Juggler program
//
//  Test lighting across gl contexts.
//
// torus.cpp
//----------------------------------------
#include <torusApp.h>

      // --- Lib Stuff --- //
#include <Kernel/vjKernel.h>
#include <Kernel/vjProjection.h>
//#include <unistd.h>

#ifndef WIN32
#include <sched.h>
#endif


int main(int argc, char* argv[])
{
   vjProjection::setNearFar(0.01f, 10000.0f);
   
   vjKernel* kernel = vjKernel::instance();        // Get the kernel
   torusApp* application = new torusApp(kernel);   // Declare an instance of the app

   
   // Load any config files specified on the command line
   for(int i=1;i<argc;i++)
      kernel->loadConfigFile(argv[i]);

   kernel->start();
      //- Kernel load global config  -- Environment variable
      //- App KernelConfig function
      //- Load user local config

   kernel->setApplication(application);         // Set application

   while(1)
   {   
#ifdef WIN32
      Sleep(0);
#else
      /*
      sleep(2);
      kernel->stopApplication();
      sleep(2);
      kernel->setApplication(application);
      */
      //usleep (250000);
      sched_yield();
#endif
   }

   return 1;
}
