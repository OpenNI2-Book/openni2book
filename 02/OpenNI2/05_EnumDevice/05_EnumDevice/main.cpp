#include <iostream>

#include <OpenNI.h>

int main(int argc, const char * argv[])
{
  try {
    // OpenNI を初期化する
    openni::OpenNI::initialize();
    
    // 接続されているデバイス一覧を取得する
    openni::Array<openni::DeviceInfo> deviceInfoList;
    openni::OpenNI::enumerateDevices( &deviceInfoList );
    
    // 接続されているデバイス一覧を表示する
    std::cout << "Connected devive count : " << deviceInfoList.getSize() << std::endl;
    for ( int i = 0; i < deviceInfoList.getSize(); ++i ) {
      std::cout << deviceInfoList[i].getName() << ", "
                << deviceInfoList[i].getVendor() << ", "
                << deviceInfoList[i].getUri() << std::endl;
    }
  }
  catch ( std::exception& ) {
    std::cout << openni::OpenNI::getExtendedError() << std::endl;
  }
  
  return 0;
}

