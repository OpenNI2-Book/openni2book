#include <OpenNI.h>
#include <opencv2\opencv.hpp>
#include <vector>


class DepthSensor
{
public:

  void initialize()
  {
    // OpenNI ������������
    openni::OpenNI::initialize();

    // �f�o�C�X���擾����
    openni::Status ret = device.open( openni::ANY_DEVICE );
    if ( ret != openni::STATUS_OK ) {
      throw std::runtime_error( "openni::Device::open() failed." );
    }

    // �J���[�X�g���[����L���ɂ���
    colorStream.create( device, openni::SensorType::SENSOR_COLOR );
    colorStream.start();

    // Depth �X�g���[����L���ɂ���
    depthStream.create( device, openni::SensorType::SENSOR_DEPTH );
    depthStream.start();

    // �X�g���[���̏���\������
    std::cout << "Color Stream" << std::endl;
    showStreamParameter( colorStream );

    std::cout << "Depth Stream" << std::endl;
    showStreamParameter( depthStream );
  }

  void update()
  {
    openni::VideoFrameRef colorFrame;
    openni::VideoFrameRef depthFrame;

    // �X�V���ꂽ�t���[�����擾����
    colorStream.readFrame( &colorFrame );
    depthStream.readFrame( &depthFrame );

    // �t���[���̃f�[�^��\���ł���`�ɕϊ�����
    colorImage = showColorStream( colorFrame );
    depthImage = showDepthStream( depthFrame );

    // �t���[���̃f�[�^��\������
    cv::imshow( "Color Stream", colorImage );
    cv::imshow( "Depth Stream", depthImage );
  }

  // �~���[���[�h��ύX����
  void changeMirrorMode()
  {
    colorStream.setMirroringEnabled( !colorStream.getMirroringEnabled() );
    depthStream.setMirroringEnabled( !depthStream.getMirroringEnabled() );
  }

  // Cropping�̐ݒ��ύX����
  void changeCropping()
  {
    int x, y, w, h;
    bool enable = colorStream.getCropping( &x, &y, &w, &h );
    if ( enable ) {
      // enable == ture �̎��́A x,y,w,h �ɗL���Ȓl������
      std::cout << "Cropping x : " << x << " y : " << y
                << " width : " << w << " height : " << h << std::endl;

      // Cropping �𖳌��ɂ���
      colorStream.resetCropping();
    }
    else {
      // ���S�� Cropping ����
      colorStream.setCropping( 160, 120, 320, 240 );
    }
  }

private:

  // �J���[�X�g���[����\���ł���`�ɕϊ�����
  cv::Mat showColorStream( const openni::VideoFrameRef& colorFrame )
  {
    // OpenCV �̌`�ɕϊ�����
    cv::Mat colorImage = cv::Mat( colorFrame.getHeight(),
                                  colorFrame.getWidth(),
                                  CV_8UC3, (char*)colorFrame.getData() );

    // BGR �̕��т� RGB �ɕϊ�����
    cv::cvtColor( colorImage, colorImage, CV_BGR2RGB );

    return colorImage;
  }

  // Depth �X�g���[����\���ł���`�ɕϊ�����
  cv::Mat showDepthStream( const openni::VideoFrameRef& depthFrame )
  {
    // �����f�[�^���摜������(16bit)
    cv::Mat depthImage = cv::Mat( depthFrame.getHeight(),
                                  depthFrame.getWidth(),
                                  CV_16U, (char*)depthFrame.getData() );

    // 0-10000mm�܂ł̃f�[�^��0-255(8bit)�ɂ���
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

  openni::Device device;            // �g�p����f�o�C�X
  openni::VideoStream colorStream;  // �J���[�X�g���[��
  openni::VideoStream depthStream;  // Depth �X�g���[��

  cv::Mat colorImage;               // �\���p�f�[�^
  cv::Mat depthImage;               // Depth �\���p�f�[�^
};

int main(int argc, const char * argv[])
{
  try {
    // OpenNI ������������
    openni::OpenNI::initialize();

    // �Z���T�[������������
    DepthSensor sensor;
    sensor.initialize();

    // ���C�����[�v
    while ( 1 ) {
      sensor.update();

      int key = cv::waitKey( 10 );
      if ( key == 'q' ) {
        break;
      }
      // Mirroring ��ύX����
      else if ( key == 'm' ) {
        sensor.changeMirrorMode();
      }
      // Cropping ��ύX����
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


