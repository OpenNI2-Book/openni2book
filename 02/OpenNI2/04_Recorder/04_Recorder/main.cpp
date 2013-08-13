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
    // ストリームデータを再生する
    //openni::Status ret = device.open( "record.oni" );
    if ( ret != openni::STATUS_OK ) {
      throw std::runtime_error( "openni::Device::open() failed." );
    }
    
    // カラーストリームを有効にする
    colorStream.create( device, openni::SENSOR_COLOR );
    colorStream.start();
    
    // Depth ストリームを有効にする
    depthStream.create( device, openni::SENSOR_DEPTH );
    depthStream.start();
    
    // ストリームデータを記録する
    //recorder.create( "record.oni" );
    //recorder.attach( colorStream );
    //recorder.attach( depthStream );
    //recorder.start();
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
    // OpenCV の形に変換する
    cv::Mat colorImage = cv::Mat( colorFrame.getHeight(),
                                 colorFrame.getWidth(),
                                 CV_8UC3, (unsigned char*)colorFrame.getData() );
    
    // BGR の並びを RGB に変換する
    cv::cvtColor( colorImage, colorImage, CV_BGR2RGB );
    
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
  openni::Recorder recorder;
  
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

