#include <gadget/Devices/Open/Trackd/TrackdSensorStandalone.h>
#include <assert.h>

#include <gmtl/Matrix.h>
#include <gmtl/Vec.h>
#include <gmtl/MatrixOps.h>
#include <gmtl/Generate.h>
#include <gmtl/Convert.h>


int TrackdSensorStandalone::numSensors()
{
   assert(mMem != NULL);
   assert(mShmKey != 0);
   return trackd_num_sensors(mMem);
}

// Return the position of the given sensor
gmtl::Matrix44f TrackdSensorStandalone::getSensorPos(int sensorNum)
{
   assert(mMem != NULL && "We don't have a valid trackd memory area");
   assert(sensorNum < numSensors() && "Out of bounds request for a sensor");

   CAVE_SENSOR_ST* sensor_val;
   sensor_val = trackd_sensor(mMem, sensorNum);

   // XXX: This is untested and is probably wrong. :(
   gmtl::Matrix44f ret_val;

   gmtl::setRot( ret_val, gmtl::Math::deg2Rad(sensor_val->elev),
                          gmtl::Math::deg2Rad(sensor_val->azim),
                          gmtl::Math::deg2Rad(sensor_val->roll), gmtl::XYZ );
   gmtl::setTrans( ret_val, gmtl::Vec3f( sensor_val->x, sensor_val->y, sensor_val->z) );

   return ret_val;
}

//: Attach to the memory segment with key (mShmKey)
//! POST: mMem = address of the shared memory area
void TrackdSensorStandalone::attachToMem()
{
   assert(mShmKey != 0 && "Key was not set correctly");
   mMem = trackd_attach_tracker_mem(mShmKey);
}

//: Release the memory segment of mMem
void TrackdSensorStandalone::releaseMem()
{
   assert(mMem != NULL && "Trying to release trackd memory that was NULL");
   trackd_release_tracker_mem(mMem);
}

