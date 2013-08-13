#include <iostream>
#include <stdexcept>

#include <OpenNI.h>
#include <opencv2/opencv.hpp>

class DepthSensor
{
public:
  
  void initialize()
  {
    // デバイスを取得する
    openni::Status ret = device.open( openni::ANY_DEVICE );
    if ( ret != openni::STATUS_OK ) {
      throw std::runtime_error( "openni::Device::open() failed." );
    }
    
    // カラーストリームを有効にする
    colorStream.create( device, openni::SENSOR_COLOR );
    changeResolution( colorStream );
    colorStream.start();
  }
  
  // フレームの更新処理
  void update()
  {
    openni::VideoFrameRef colorFrame;
    
    // 更新されたフレームを取得する
    colorStream.readFrame( &colorFrame );
    
    // フレームのデータを表示できる形に変換する
    colorImage = showColorStream( colorFrame );
    
    // フレームのデータを表示する
    cv::imshow( "Color Stream", colorImage );
  }
  
private:
  
  void changeResolution( openni::VideoStream& stream )
  {
    openni::VideoMode mode = stream.getVideoMode();
    mode.setResolution( 640, 480 );
    mode.setFps( 30 );
    stream.setVideoMode( mode );
  }
  
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
  
private:
  
  openni::Device device;            // 使用するデバイス
  openni::VideoStream colorStream;  // カラーストリーム
  
  cv::Mat colorImage;               // 表示用データ
};

int main(int argc, const char * argv[])
{

  try {
    // OpenNI を初期化する
    openni::OpenNI::initialize();
    
    // センサーを初期化する
    DepthSensor sensor;
    sensor.initialize();
    
    // メインループ
    while ( 1 ) {
      sensor.update();
      
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

