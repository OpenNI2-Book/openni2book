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
    colorStream.create( device, openni::SENSOR_IR );
    colorStream.start();
    
    // Depth ストリームを有効にする
    depthStream.create( device, openni::SENSOR_DEPTH );
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
  
  // カラーストリームを表示できる形に変換する
  cv::Mat showColorStream( const openni::VideoFrameRef& colorFrame )
  {
    cv::Mat colorImage;
    
    // Color ストリーム
    if ( colorFrame.getVideoMode().getPixelFormat() ==
        openni::PIXEL_FORMAT_RGB888 ) {
      // OpenCV の形に変換する
      colorImage = cv::Mat( colorFrame.getHeight(),
                           colorFrame.getWidth(),
                           CV_8UC3, (unsigned char*)colorFrame.getData() );
      
      // BGR の並びを RGB に変換する
      cv::cvtColor( colorImage, colorImage, CV_RGB2BGR );
    }
    // Xtion IR ストリーム
    else if ( colorFrame.getVideoMode().getPixelFormat() ==
             openni::PIXEL_FORMAT_GRAY16 ) {
      // XitonのIRのフォーマットは16bitグレースケール
      // 実際は255諧調らしく、CV_8Uに落とさないと見えない
      colorImage = cv::Mat( colorFrame.getHeight(),
                           colorFrame.getWidth(),
                           CV_16UC1, (unsigned short*)colorFrame.getData() );
      colorImage.convertTo( colorImage, CV_8U );
    }
    // Kinect for Windows IR ストリーム
    else {
      // KinectのIRのフォーマットは8bitグレースケール
      // Kinect SDKは16bitグレースケール
      colorImage = cv::Mat( colorFrame.getHeight(),
                           colorFrame.getWidth(),
                           CV_8UC1, (unsigned char*)colorFrame.getData() );
    }
    
    return colorImage;
  }
  
  // Depth ストリームを表示できる形に変換する
  cv::Mat showDepthStream( const openni::VideoFrameRef& depthFrame )
  {
    // 距離データを画像化する(16bit)
    cv::Mat depthImage = cv::Mat( depthFrame.getHeight(),
                                 depthFrame.getWidth(),
                                 CV_16U, (char*)depthFrame.getData() );
    
    // 0-10000mmまでのデータを0-255(8bit)にする
    depthImage.convertTo( depthImage, CV_8U, 255.0 / 10000 );
    
    return depthImage;
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

