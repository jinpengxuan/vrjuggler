#ifndef _GADGET_INPUT_HANDLER_H_
#define _GADGET_INPUT_HANDLER_H_

#include <gadget/gadgetConfig.h>

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/version.hpp>

#if BOOST_VERSION >= 103600
#  include <boost/unordered_map.hpp>
#else
#  include <map>
#endif

#include <vpr/vprDomain.h>
#include <vpr/Sync/Mutex.h>
#include <vpr/Util/Interval.h>

#include <gadget/Event/EventGeneratorPtr.h>
#include <gadget/InputHandlerPtr.h>


namespace gadget
{

class AbstractEventInterface;

/** \class InputHandler InputHandler.h gadget/InputHandler.h
 *
 * @since 2.1.2
 */
class GADGET_CLASS_API InputHandler
   : public boost::enable_shared_from_this<InputHandler>
   , private boost::noncopyable
{
private:
   InputHandler();

public:
   static InputHandlerPtr create();

   ~InputHandler();

   /**
    * Starts the thread for periodic event emission.
    *
    * @pre If \c mRunning is false, then \c mThread is NULL.
    * @post \c mThread is not NULL.
    *
    * @param interval The wait interval for the periodic event emission loop.
    *                 This must represent a number of milliseconds greater
    *                 than 0.
    *
    * @throw vpr::IllegalArgumentException Thrown if \p interval is 0.
    *
    * @since 2.1.22
    */
   void start(const vpr::Interval& interval);

   /**
    * Stops the thread for periodic event emission.
    *
    * @pre If \c mRunning is true, then \c mThread is not NULL.
    * @post \c mRunning is false, and \c mThread is NULL.
    *
    * @since 2.1.22
    */
   void stop();

   /**
    * @since 2.1.22
    */
   bool isRunning() const
   {
      return mRunning;
   }

   /** @name Interface Registration */
   //@{
   /**
    * Registers the given event interface so that its even tgenerator will
    * emit events while the thread managed by this object executes.
    *
    * @pre \p iface will return a valid event generator when asked to do so.
    *
    * @param iface The abstract event interface to register.
    *
    * @see AbstractEventInterface::getEventGenerator()
    */
   void registerPeriodicInterface(AbstractEventInterface* iface);

   void unregisterPeriodicInterface(AbstractEventInterface* iface);

   /**
    * Registers the given event interface so that its even tgenerator will
    * emit events once per frame when the Input Manager invokes the sync()
    * method.
    *
    * @pre \p iface will return a valid event generator when asked to do so.
    *
    * @param iface The abstract event interface to register.
    *
    * @see AbstractEventInterface::getEventGenerator()
    * @see InputManager::updateAllDevices()
    * @see sync()
    */
   void registerSynchronizedInterface(AbstractEventInterface* iface);

   void unregisterSynchronizedInterface(AbstractEventInterface* iface);
   //@}

   /**
    * Causes all registered synchronized event generators to emit their
    * pending events.
    *
    * @since 2.1.3
    */
   void sync();

private:
   /**
    * @since 2.1.22
    */
   void run();

   /**
    *
    */
   //@{
   bool          mRunning;
   vpr::Thread*  mThread;
   vpr::Uint32   mWaitInterval;
   //@}

#if BOOST_VERSION >= 103600
   typedef boost::unordered_map<AbstractEventInterface*, EventGeneratorPtr>
      iface_map_type;
#else
   typedef std::map<AbstractEventInterface*, EventGeneratorPtr> iface_map_type;
#endif

   iface_map_type mPeriodicIfaces;
   iface_map_type mSyncIfaces;

   vpr::Mutex mPeriodicIfacesLock;
   vpr::Mutex mSyncIfacesLock;
};

}


#endif /* _GADGET_INPUT_HANDLER_H_ */