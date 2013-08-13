#include <OpenNI.h>
#include <opencv2\opencv.hpp>
#include <vector>


class DepthSensor
{
public:

  void initialize()
  {
    // OpenNI を初期化する
    openni::OpenNI::initialize();

    // デバイスを取得する
    openni::Status ret = device.open( openni::ANY_DEVICE );
    if ( ret != openni::STATUS_OK ) {
      throw std::runtime_error( "openni::Device::open() failed." );
    }

    // カラーストリームを有効にする
    colorStream.create( device, openni::SensorType::SENSOR_COLOR );
    colorStream.start();

    // Depth ストリームを有効にする
    depthStream.create( device, openni::SensorType::SENSOR_DEPTH );
    depthStream.start();

    // ストリームの情報を表示する
    std::cout << "Color Stream" << std::endl;
    showStreamParameter( colorStream );

    std::cout << "Depth Stream" << std::endl;
    showStreamParameter( depthStream );
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

  // ミラーモードを変更する
  void changeMirrorMode()
  {
    colorStream.setMirroringEnabled( !colorStream.getMirroringEnabled() );
    depthStream.setMirroringEnabled( !depthStream.getMirroringEnabled() );
  }

  // Croppingの設定を変更する
  void changeCropping()
  {
    int x, y, w, h;
    bool enable = colorStream.getCropping( &x, &y, &w, &h );
    if ( enable ) {
      // enable == ture の時は、 x,y,w,h に有効な値が入る
      std::cout << "Cropping x : " << x << " y : " << y
                << " width : " << w << " height : " << h << std::endl;

      // Cropping を無効にする
      colorStream.resetCropping();
    }
    else {
      // 中心を Cropping する
      colorStream.setCropping( 160, 120, 320, 240 );
    }
  }

private:

  // カラーストリームを表示できる形に変換する
  cv::Mat showColorStream( const openni::VideoFrameRef& colorFrame )
  {
    // OpenCV の形に変換する
    cv::Mat colorImage = cv::Mat( colorFrame.getHeight(),
                                  colorFrame.getWidth(),
                                  CV_8UC3, (char*)colorFrame.getData() );

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

  void showStreamParameter( openni::VideoStream& stream )
  {
    std::cout << "Cropping support : " << stream.isCroppingSupported() << std::endl;
    std::cout << "Horizontal FOV   : " << stream.getHorizontalFieldOfView() << std::endl;
    std::cout << "Vertical  FOV    : " << stream.getVerticalFieldOfView() << std::endl;

    std::cout << "Max Pixel Value  : " << stream.getMaxPixelValue() << std::endl;
    std::cout << "Min Pixel Value  : " << stream.getMinPixelValue() << std::endl;

    const openni::SensorInfo& sensorInfo = stream.getSensorInfo();
    std::cout << "Sensor Type      : " << getSensorTypeToString( sensorInfo.getSensorType() ) << std::endl;

    std::cout << std::endl;
    std::cout << "Supported VideoMode : " << std::endl;
    const openni::Array<openni::VideoMode>& videoModes = sensorInfo.getSupportedVideoModes();
    for ( int i = 0; i < videoModes.getSize(); ++i ) {
      std::cout << " ResolutionX  : " << videoModes[i].getResolutionX() << std::endl;
      std::cout << " ResolutionY  : " << videoModes[i].getResolutionY() << std::endl;
      std::cout << " FPS          : " << videoModes[i].getFps() << std::endl;
      std::cout << " PixelFormat  : " << getPixelFormatToString( videoModes[i].getPixelFormat() ) << std::endl;
      std::cout << std::endl;
    }

    openni::CameraSettings* cameraSettings = stream.getCameraSettings();
    if ( cameraSettings != 0 ) {
      std::cout << "CameraSettings" << std::endl;
      std::cout << " Auto Exposure Enabled      : " << cameraSettings->getAutoExposureEnabled() << std::endl;
      std::cout << " Auto WhiteBalance Enabled  : " << cameraSettings->getAutoWhiteBalanceEnabled() << std::endl;
      std::cout << " Exposure                   : " << cameraSettings->getExposure() << std::endl;
      std::cout << " Gain                       : " << cameraSettings->getGain() << std::endl;
    }

    std::cout << std::endl;
  }

  const char* getSensorTypeToString( openni::SensorType sensorType )
  {
    if ( sensorType == openni::SensorType::SENSOR_COLOR ) {
      return "SENSOR_COLOR";
    }
    else if ( sensorType == openni::SensorType::SENSOR_DEPTH ) {
      return "SENSOR_DEPTH";
    }
    else if ( sensorType == openni::SensorType::SENSOR_IR ) {
      return "SENSOR_IR";
    }

    return "Unknown SensorType.";
  }

  const char* getPixelFormatToString( openni::PixelFormat pixelFormat )
  {
	  // Depth
    if ( pixelFormat == openni::PixelFormat::PIXEL_FORMAT_DEPTH_1_MM ) {
      return "PIXEL_FORMAT_DEPTH_1_MM";
    }
    else if ( pixelFormat == openni::PixelFormat::PIXEL_FORMAT_DEPTH_100_UM ) {
      return "PIXEL_FORMAT_DEPTH_100_UM";
    }
    else if ( pixelFormat == openni::PixelFormat::PIXEL_FORMAT_SHIFT_9_2 ) {
      return "PIXEL_FORMAT_SHIFT_9_2";
    }
    else if ( pixelFormat == openni::PixelFormat::PIXEL_FORMAT_SHIFT_9_3 ) {
      return "PIXEL_FORMAT_SHIFT_9_3";
    }
	  // Color
    else if ( pixelFormat == openni::PixelFormat::PIXEL_FORMAT_RGB888 ) {
      return "PIXEL_FORMAT_RGB888";
    }
    else if ( pixelFormat == openni::PixelFormat::PIXEL_FORMAT_YUV422 ) {
      return "PIXEL_FORMAT_YUV422";
    }
    else if ( pixelFormat == openni::PixelFormat::PIXEL_FORMAT_GRAY8 ) {
      return "PIXEL_FORMAT_GRAY8";
    }
    else if ( pixelFormat == openni::PixelFormat::PIXEL_FORMAT_GRAY16 ) {
      return "PIXEL_FORMAT_GRAY16";
    }
    else if ( pixelFormat == openni::PixelFormat::PIXEL_FORMAT_JPEG ) {
      return "PIXEL_FORMAT_JPEG";
    }
    else if ( pixelFormat == openni::PixelFormat::PIXEL_FORMAT_YUYV ) {
      return "PIXEL_FORMAT_YUYV";
    }

    return "Unknown PixelFormat.";
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
      // Mirroring を変更する
      else if ( key == 'm' ) {
        sensor.changeMirrorMode();
      }
      // Cropping を変更する
      else if ( key == 'c' ) {
        sensor.changeCropping();
      }
    }
  }
  catch ( std::exception& ex ) {
    std::cout << ex.what() << std::endl;
  }

  return 0;
}


