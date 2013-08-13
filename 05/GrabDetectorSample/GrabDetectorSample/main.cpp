#include <iostream>
#include <list>

#include <OpenNI.h>
#include <NiTE.h>
#include "GrabDetector.h"

#include <opencv2\opencv.hpp>

class GrabDetectorSample : PSLabs::IGrabEventListener
{
public:

  // ������
  void initialize()
  {
    initOpenNI();
    initNite();
    initGrabDetector();
  }

  // OpenNI �̏�����
  void initOpenNI()
  {
    auto version = openni::OpenNI::getVersion();
    std::cout << "OpenNI " << version.major << "." << version.minor << "." <<
                              version.maintenance << "." << version.build << std::endl;

    auto ret = openni::OpenNI::initialize();
    if ( ret != openni::DEVICE_STATE_OK ) {
      throw std::runtime_error( "openni::OpenNI::initialize" );
    }

    ret = device.open( openni::ANY_DEVICE );
    if ( ret != openni::DEVICE_STATE_OK ) {
      throw std::runtime_error( "device.open" );
    }

    // Grab Detector �̃h�L�������g���
    // Depth �� Color �̉𑜓x�� VGA(640x480)�Ƃ��ADepth �� Color �̃t���[���𓯊������邱��
    // 
    // Requires VGA depth & color input, with both depth-color registration and time sync 
    // enabled.  See included sample for setting this up. 

    // Color �X�g���[�����쐬����
    ret = colorStream.create( device, openni::SensorType::SENSOR_COLOR );
    if ( ret != openni::STATUS_OK ) {
      throw std::runtime_error( "colorStream.create" );
    }

    initStream( colorStream );
    ret = colorStream.start();
    if ( ret != openni::STATUS_OK ) {
      throw std::runtime_error( "colorStream.start" );
    }

    // Depth �X�g���[�����쐬����
    ret = depthStream.create( device, openni::SensorType::SENSOR_DEPTH );
    if ( ret != openni::STATUS_OK ) {
      throw std::runtime_error( "depthStream.create" );
    }

    initStream( depthStream );
    ret = depthStream.start();
    if ( ret != openni::STATUS_OK ) {
      throw std::runtime_error( "depthStream.start" );
    }

    // Depth �� Color �̃t���[���𓯊�������
    device.setDepthColorSyncEnabled( true );

    // Depth �� Color �̍��W�ɍ��킹��
    device.setImageRegistrationMode( openni::IMAGE_REGISTRATION_DEPTH_TO_COLOR );

    // �X�g���[�����܂Ƃ߂�
    streams.push_back( &colorStream );
    streams.push_back( &depthStream );
  }

  // OpenNI �X�g���[���̏�����
  void initStream( openni::VideoStream& stream )
  {
    auto videoMode = stream.getVideoMode();
    videoMode.setFps( 30 );
    videoMode.setResolution( 640, 480 );
    auto ret = stream.setVideoMode( videoMode );
    if ( ret != openni::STATUS_OK ) {
      throw std::runtime_error( "stream.setVideoMode" );
    }

    stream.setMirroringEnabled(true);
  }

  // NiTE �̏�����
  void initNite()
  {
    auto version = nite::NiTE::getVersion();
    std::cout << "NiTE   " << version.major << "." << version.minor << "." <<
                              version.maintenance << "." << version.build << std::endl;

    auto ret = nite::NiTE::initialize();

    // HandTracker ���쐬����
    ret = handTracker.create( &device );
    if ( ret != nite::STATUS_OK ) {
      throw std::runtime_error( "handTracker.create" );
    }

    // �W�F�X�`���[�����o����
    handTracker.startGestureDetection( nite::GestureType::GESTURE_WAVE );
  }

  // Grab Detector �̏�����
  void initGrabDetector()
  {
    grabDetector = PSLabs::CreateGrabDetector( device );
    if ( grabDetector == 0 ) {
      throw std::runtime_error( "PSLabs::CreateGrabDetector" );
    }

    grabDetector->AddListener( this );
  }

  // ���C�����[�v
  void run()
  {
    cv::Mat depthImage;

    while ( 1 ) {
      int changedIndex;
      auto ret = openni::OpenNI::waitForAnyStream( &streams[0],
                                                   streams.size(), &changedIndex );
      if ( ret != openni::STATUS_OK ) {
        continue;
      }

      // depth ����� color �t���[�����擾����
      openni::VideoFrameRef depthFrame;
      depthStream.readFrame( &depthFrame );
      depthImage = convertDepthToColor( depthFrame );

      openni::VideoFrameRef colorFrame;
      colorStream.readFrame( &colorFrame );

      // ��̒ǐՃt���[�����擾����
      nite::HandTrackerFrameRef handTrackerFrame;
      handTracker.readFrame( &handTrackerFrame );

      // �W�F�X�`���[��F��������A���̓_������ǐՂ���
      const nite::Array<nite::GestureData>& gestures =
        handTrackerFrame.getGestures();
      for ( int i = 0; i < gestures.getSize(); ++i ) {
        if ( gestures[i].isComplete() ) {
          nite::HandId newId;
          handTracker.startHandTracking( gestures[i].getCurrentPosition(), &newId );
        }
      }

      // ���ǐՂ��Ă�����AGrabDetector�Ƀt���[���̃f�[�^��n��
      const nite::Array<nite::HandData>& hands = handTrackerFrame.getHands();
      for (int i = 0; i < hands.getSize(); ++i) {
        if ( hands[i].isTracking() ) {
          auto position = hands[i].getPosition();
          cv::circle( depthImage, convertHandCoordinatesToDepth( position ),
            2, cv::Scalar( 0, 255, 0 ), 3 );

          grabDetector->SetHandPosition( position.x, position.y, position.z );
          grabDetector->UpdateFrame( depthFrame, colorFrame );
        }
      }

      cv::imshow( "Grab Detector Sample", depthImage );

      int key = cv::waitKey( 10 );
      if ( key == 'q' ) {
        break;
      }
    }
  }

  // ���3�����ʒu���ADepth��2�����ʒu�ɕϊ�����
  cv::Point convertHandCoordinatesToDepth( const nite::Point3f& position )
  {
    float x = 0, y = 0;
    handTracker.convertHandCoordinatesToDepth(
      position.x, position.y, position.z, &x, &y );
    return cv::Point( x, y );
  };

  // Depth �f�[�^���J���[�摜�ɕϊ�����
  cv::Mat convertDepthToColor( openni::VideoFrameRef& depthFrame )
  {
    cv::Mat depthImage = cv::Mat( depthFrame.getVideoMode().getResolutionY(),
      depthFrame.getVideoMode().getResolutionX(),
      CV_8UC4 );

    openni::DepthPixel* depth = (openni::DepthPixel*)depthFrame.getData();
    for ( int i = 0; i < (depthFrame.getDataSize()/sizeof(openni::DepthPixel)); ++i ) {
      // �摜�C���f�b�N�X�𐶐�
      int index = i * 4;

      // �����f�[�^���摜������
      UCHAR* data = &depthImage.data[index];
      // 0-255�̃O���[�f�[�^���쐬����
      // distance : 10000 = gray : 255
      int gray = ~((depth[i] * 255) / 10000);
      data[0] = gray;
      data[1] = gray;
      data[2] = gray;
    }

    return depthImage;
  }

  // GrabEvent �̒ʒm(PSLabs::IGrabEventListener::ProcessGrabEvent ���I�[�o�[���C�h)
  void DLL_CALL ProcessGrabEvent( const EventParams& params )
  {
    if ( params.Type == PSLabs::IGrabEventListener::GRAB_EVENT ) {
      std::cout << "Grab" << std::endl;
    }
    else if ( params.Type == PSLabs::IGrabEventListener::RELEASE_EVENT ) {
      std::cout << "Release" << std::endl;
    }
  }

private:

  openni::Device device;

  openni::VideoStream colorStream;
  openni::VideoStream depthStream;
  std::vector<openni::VideoStream*> streams;

  nite::HandTracker handTracker;

  PSLabs::IGrabDetector* grabDetector;
};

void main()
{
  try {
    GrabDetectorSample app;
    app.initialize();
    app.run();
  }
  catch ( std::exception& ) {
    std::cout << openni::OpenNI::getExtendedError() << std::endl;
  }
}

