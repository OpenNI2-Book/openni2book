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
    
    // Depth ストリームを有効にする
    depthStream.create( device, openni::SENSOR_DEPTH );
    changeResolution( depthStream );
    depthStream.start();
  }
  
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
    cv::imshow( "Color Stream", colorImage );
    cv::imshow( "Depth Stream", depthImage );
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
  
  // Depth ストリームを表示できる形に変換する
  cv::Mat showDepthStream( const openni::VideoFrameRef& depthFrame )
  {
    // 距離データを画像化する(16bit)
    cv::Mat depthImage = cv::Mat( depthFrame.getHeight(),
                                 depthFrame.getWidth(),
                                 CV_16UC1, (unsigned short*)depthFrame.getData() );
    
    // 0-10000mmまでのデータを0-255(8bit)にする
    depthImage.convertTo( depthImage, CV_8U, 255.0 / 1000 );
    
    // 中心点の距離を表示する
    showCenterDistance( depthImage, depthFrame );

    return depthImage;
  }

  // 中心点の距離を表示する
  void showCenterDistance( cv::Mat& depthImage, const openni::VideoFrameRef& depthFrame)
  {
    // 中心点の距離を表示する
	  openni::VideoMode videoMode = depthStream.getVideoMode();

    int centerX = videoMode.getResolutionX() / 2;
    int centerY = videoMode.getResolutionY() / 2;
    int centerIndex = (centerY * videoMode.getResolutionX()) + centerX;

    unsigned short* depth = (unsigned short*)depthFrame.getData();

    std::stringstream ss;
    ss << "Center Point :" << depth[centerIndex];
    cv::putText( depthImage, ss.str(), cv::Point( 0, 50 ),
                cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar( 255 ) );
  }
  
private:
  
  openni::Device device;            // 使用するデバイス
  openni::VideoStream colorStream;  // カラーストリーム
  openni::VideoStream depthStream;  // Depth ストリーム
  
  cv::Mat colorImage;               // 表示用データ
  cv::Mat depthImage;               // Depth 表示用データ
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

