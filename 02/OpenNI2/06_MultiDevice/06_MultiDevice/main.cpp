#include <iostream>
#include <stdexcept>
#include <vector>

#include <OpenNI.h>
#include <opencv2/opencv.hpp>

class DepthSensor
{
public:
  
  void initialize( const char* uri = openni::ANY_DEVICE )
  {
    // デバイスを取得する
    openni::Status ret = device.open( uri );
    if ( ret != openni::STATUS_OK ) {
      throw std::runtime_error( "openni::Device::open() failed." );
    }
    
    // カラーストリームを有効にする
    colorStream.create( device, openni::SENSOR_COLOR );
    colorStream.start();
    
    depthStream.create( device, openni::SENSOR_DEPTH );
    depthStream.start();
    
    // URIを保存しておく
    this->uri = uri;
  }
  
  // フレームの更新処理
  void update()
  {
    openni::VideoFrameRef colorFrame;
    openni::VideoFrameRef depthFrame;
    
    // 更新されたフレームを取得する
    colorStream.readFrame( &colorFrame );
    depthStream.readFrame( &depthFrame );
    
    // フレームのデータを表示できる形に変換する
    colorImage = showColorStream( colorFrame );
    depthImage = showDepthStream( depthFrame );
    
    // フレームのデータを表示する
    cv::imshow( "Color Stream " + getUri(), colorImage );
    cv::imshow( "Depth Stream " + getUri(), depthImage );
  }
  
  const std::string& getUri() const
  {
    return uri;
  }
  
private:
  
  // カラーストリームを表示できる形に変換する
  cv::Mat showColorStream( const openni::VideoFrameRef& colorFrame )
  {
    // OpenCV の形に変換する
    cv::Mat colorImage = cv::Mat( colorFrame.getHeight(),
                                 colorFrame.getWidth(),
                                 CV_8UC3, (unsigned char*)colorFrame.getData() );
    
    // BGR の並びを RGB に変換する
    cv::cvtColor( colorImage, colorImage, CV_RGB2BGR );
    
    return colorImage;
  }
  
  cv::Mat showDepthStream( const openni::VideoFrameRef& depthFrame )
  {
    // 距離データを画像化する(16bit)
    cv::Mat depthImage = cv::Mat( depthFrame.getHeight(),
                                 depthFrame.getWidth(),
                                 CV_16UC1, (unsigned short*)depthFrame.getData() );
    
    // 0-10000mmまでのデータを0-255(8bit)にする
    depthImage.convertTo( depthImage, CV_8U, 255.0 / 10000 );
    
    return depthImage;
  }
  
private:
  
  openni::Device device;
  openni::VideoStream colorStream;
  openni::VideoStream depthStream;
  
  cv::Mat colorImage;
  cv::Mat depthImage;
  
  std::string uri;
};

class SampleApp
{
public:
  
  void initialize()
  {
    // 接続されているデバイスの一覧を取得する
    openni::Array<openni::DeviceInfo> deviceInfoList;
		openni::OpenNI::enumerateDevices( &deviceInfoList );
    
		std::cout << "Connected device counnt : " << deviceInfoList.getSize() << std::endl;
		for ( int i = 0; i < deviceInfoList.getSize(); ++i ) {
			std::cout << deviceInfoList[i].getName() << ", "
                << deviceInfoList[i].getVendor() << ", "
                << deviceInfoList[i].getUri() << std::endl;
      
      openDevice( deviceInfoList[i].getUri() );
		}
  }
  
  void update()
  {
    for ( std::vector<DepthSensor*>::iterator it = sensors.begin();
      it != sensors.end(); ++it ) {
        (*it)->update();
    }
  }
  
private:
  
  void openDevice( const char* uri )
  {
    DepthSensor* sensor = new DepthSensor();
    sensor->initialize( uri );
    sensors.push_back( sensor );
  }
  
private:
  
  std::vector<DepthSensor*> sensors;
};

int main(int argc, const char * argv[])
{
  try {
    // OpenNI を初期化する
    openni::OpenNI::initialize();
    
    SampleApp app;
    app.initialize();
    while ( 1 ) {
      app.update();
      
      int key = cv::waitKey( 10 );
      if ( key == 'q' ) {
        break;
      }
    }
  }
  catch ( std::exception& ) {
    std::cout << openni::OpenNI::getExtendedError() << std::endl;
  }
  
  return 0;
}

