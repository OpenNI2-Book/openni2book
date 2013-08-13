#include <iostream>

#include <OpenNI.h>

class DeviceConnectListener : public openni::OpenNI::DeviceConnectedListener
                            , public openni::OpenNI::DeviceDisconnectedListener
{
public:
  
  // デバイスが接続されたときに呼ばれる
  virtual void onDeviceConnected( const openni::DeviceInfo* device )
  {
    std::cout << "onDeviceConnected : " << device->getName() << std::endl;
  }
  
  // デバイスが切断されたときに呼ばれる
  virtual void onDeviceDisconnected( const openni::DeviceInfo* device )
  {
    std::cout << "onDeviceDisconnected : " << device->getName() << std::endl;
  }
};

int main(int argc, const char * argv[])
{
  try {
    openni::OpenNI::initialize();
    
    // 接続および切断通知の登録を行う
    DeviceConnectListener listener;
    openni::OpenNI::addDeviceConnectedListener( &listener );
    openni::OpenNI::addDeviceDisconnectedListener( &listener );
    
    while ( 1 ) {
#ifdef WIN32
      ::Sleep( 1 );
#else
      ::sleep( 1 );
#endif
    }
  }
  catch ( std::exception& ) {
    std::cout << openni::OpenNI::getExtendedError() << std::endl;
  }
  
  return 0;
}

