#include <gadget/Type/NetPosition.h>
#include <vrj/Util/Debug.h>

namespace gadget{

// constructor for a transmitting network device
NetPosition::NetPosition(const std::string& src_device_name, Input* input_ptr, VJ_NETID_TYPE local_device_id, VJ_NETID_TYPE rmt_device_id) : NetInput(src_device_name, input_ptr){

   vprDEBUG(vrjDBG_INPUT_MGR, vprDBG_CONFIG_LVL) << "NetPosition: local_device_id = " << local_device_id << std::endl << vprDEBUG_FLUSH;

   mLocalId = local_device_id;
   mRemoteId = rmt_device_id;
   mDeviceName = src_device_name;

   if(src_device_name.length() > 0){   // pointing to another device/proxy for data source
      mLocalSource.init(mDeviceName);
      mSendBuffer.resize(3 + 16 * sizeof(float));  // 2 bytes of code/id, 16 bytes of float data, 1 byte for semicolon
      vprDEBUG(vrjDBG_INPUT_MGR, vprDBG_HEX_LVL) << "NetPosition: SendBuffer size: " << (3 + 16 * sizeof(float)) << std::endl << vprDEBUG_FLUSH; 
      mNetworkMatrices.resize(1);  // space for one data item
   }
   else{                              // use ourself as source of data
      // find the number of devices

      int num_elements = this->getPositionDataBuffer().size();
      if (num_elements <= 0)
         num_elements = 1;

      mNetworkMatrices.resize(num_elements);

      // allocate space for data
      mSendBuffer.resize(3 + 16 * sizeof(float) * num_elements);  // 2 bytes of code/id, 16*4*n bytes of data, 1 byte for semicolon
      vprDEBUG(vrjDBG_INPUT_MGR, vprDBG_HEX_LVL) << "NetPosition: SendBuffer size: " << (3 + 16 * sizeof(float) * num_elements) << std::endl << vprDEBUG_FLUSH;
   }
}

NetPosition::NetPosition(const std::string& src_device_name, Proxy* proxy_ptr, VJ_NETID_TYPE local_device_id, VJ_NETID_TYPE rmt_device_id) : NetInput(src_device_name, proxy_ptr){
   mLocalId = local_device_id;
   mRemoteId = rmt_device_id;
   mDeviceName = src_device_name;
   mLocalSource.init(mDeviceName);
   mSendBuffer.resize(3 + 16 * sizeof(float)); // 2 bytes of code/id, 16*4 bytes of data, 1 byte for semicolon
   vprDEBUG(vrjDBG_INPUT_MGR, vprDBG_HEX_LVL) << "NetPosition: SendBuffer size: " << (3 + 16 * sizeof(float) ) << std::endl << vprDEBUG_FLUSH;
   mNetworkMatrices.resize(1);  // a proxy only points to a single data item
}

// constructor for a receiving NetInput
NetPosition::NetPosition(jccl::ConfigChunkPtr chunk, VJ_NETID_TYPE local_device_id) : NetInput(chunk){
   mLocalId = local_device_id;
   mRemoteId = 0;
   mNetworkMatrices.resize(1);  // a proxy only points to a single data item
   mSendBuffer.resize(3 + 16 * sizeof(float) * mNetworkMatrices.size()); // buffer not used when receiving, but its size is currently used to determine how much data to pull from the recv buffer
}

// When updating, the data shouldn't really be accessed
void NetPosition::updateFromLocalSource(){
   vprDEBUG(vrjDBG_INPUT_MGR, vprDBG_HEX_LVL) << "NetPosition: mSendBuffer.size() = " << mSendBuffer.size() << std::endl << vprDEBUG_FLUSH;
   vprASSERT(mNetworkMatrices.size() == 1);  // only single position can be transmitted by current code

   std::vector<PositionData>  pos_data_sample(mNetworkMatrices.size());

   // next 3 lines should only be executed when using mLocalSource (remote Proxies, not devices)
   //   (i.e. source of data is not contained in the Position portion of this object, but in another object/device).
   //   the lines are not crucial to send the net msg, but keep this object sync just in case it's used.
   if(mLocalSource->getPositionPtr() != NULL){
      pos_data_sample[0] = ( *(mLocalSource->getPosition()) );  // proxies only have one input
      mPosSamples.lock();
      mPosSamples.addSample(pos_data_sample);
      mPosSamples.unlock();
      mPosSamples.swapBuffers();
   }
   else{
      for(unsigned int i = 0; i < mNetworkMatrices.size(); i++)
         pos_data_sample[i] = this->getPositionData(i);
   }

   for(unsigned int m = 0; m < mNetworkMatrices.size(); m++){
      vprDEBUG(vrjDBG_INPUT_MGR, vprDBG_VERB_LVL) << "NetPosition: Sending : __________________________ : " << std::endl << *( pos_data_sample[m].getPosition() )  << std::endl << vprDEBUG_FLUSH;
      // convert local data to network data
      for(int f = 0; f < 16; f++){
         mNetworkMatrices[m].getFloatPtr()[f] = vj_htonf( pos_data_sample[m].getPosition()->getFloatPtr()[f] ); // change it to network byte order
      }
      vprDEBUG(vrjDBG_INPUT_MGR, vprDBG_VERB_LVL) << "NetPosition: Sending : ntwk byte order --------------------------: " << std::endl <<  mNetworkMatrices[m] << std::endl << vprDEBUG_FLUSH;
   }

   ushortTo2Bytes((char*)(&(mSendBuffer[0])), htons(mRemoteId));  // put the 2 byte id in the buffer

   // copy the number byte by byte into the buffer
   for(unsigned int n = 0; n < mNetworkMatrices.size();  n++){
      for(unsigned int j = 0; j < 16 * sizeof(float); j++)
         mSendBuffer[2 + 16*n*sizeof(float) + j ] = ( (char*)(mNetworkMatrices[n].getFloatPtr()) )[j];
   }

   mSendBuffer[2 + 16 * sizeof(float) * mNetworkMatrices.size()] = ';';
}

void NetPosition::updateFromRemoteSource(char* recv_buffer, int recv_buff_len){
   vprDEBUG(vrjDBG_INPUT_MGR, vprDBG_HEX_LVL) << "NetPosition: mSendBuffer.size() = " << mSendBuffer.size() << std::endl << vprDEBUG_FLUSH;

   vprASSERT(mNetworkMatrices.size() == 1);  // only single position can be transmitted by current code

   std::vector<PositionData> pos_data_sample(mNetworkMatrices.size());

   if(recv_buff_len < (int)getDataByteLength()){  // make sure the data length is long enough
      vprDEBUG(vrjDBG_INPUT_MGR, vprDBG_VERB_LVL) << "ERROR NetPosition: recv_buff_len is not big enough to receive message." << std::endl << vprDEBUG_FLUSH;
      vprASSERT(0);
   }

   // copy the number byte by byte out of the buffer
   for(unsigned int k = 0; k < mNetworkMatrices.size(); k++){

      vprDEBUG(vrjDBG_INPUT_MGR, vprDBG_VERB_LVL) << "NetPosition: PreReceiving : ntwk byte order --------------------------: " << std::endl << mNetworkMatrices[k] << std::endl << vprDEBUG_FLUSH;

      for(unsigned int j = 0; j < 16 * sizeof(float); j++)
         ( (char*)( mNetworkMatrices[k].getFloatPtr() ) )[j] = recv_buffer[2 + k*16*sizeof(float) + j];

      vprDEBUG(vrjDBG_INPUT_MGR, vprDBG_VERB_LVL) << "NetPosition: Receiving : ntwk byte order --------------------------: " << std::endl << mNetworkMatrices[k] << std::endl << vprDEBUG_FLUSH;


      // convert network data to local data
      for(int i = 0; i < 16; i++)
         pos_data_sample[k].getPosition()->getFloatPtr()[i] = vj_ntohf( mNetworkMatrices[k].getFloatPtr()[i] ); // change to host byte order
      vprDEBUG(vrjDBG_INPUT_MGR, vprDBG_VERB_LVL) << "NetPosition: Receiving : __________________________ : " << std::endl << *(pos_data_sample[k].getPosition()) << std::endl << vprDEBUG_FLUSH;
   }

   mPosSamples.lock();
   mPosSamples.addSample(pos_data_sample);
   mPosSamples.unlock();

   mPosSamples.swapBuffers();
   vprDEBUG(vrjDBG_INPUT_MGR, vprDBG_VERB_LVL) << "NetPosition: New Data available: -------------------------- : " << *( this->getPositionData(0).getPosition() ) << std::endl << vprDEBUG_FLUSH;
}


}  // end namespace gadget

