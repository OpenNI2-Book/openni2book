#include <iostream>
#include <list>

#include <OpenNI.h>
#include <NiTE.h>
#include "GrabDetector.h"

#include <opencv2\opencv.hpp>

class GrabDetectorSample : PSLabs::IGrabEventListener
{
public:

  // 初期化
  void initialize()
  {
    initOpenNI();
    initNite();
    initGrabDetector();
  }

  // OpenNI の初期化
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

    // Grab Detector のドキュメントより
    // Depth と Color の解像度は VGA(640x480)とし、Depth と Color のフレームを同期させること
    // 
    // Requires VGA depth & color input, with both depth-color registration and time sync 
    // enabled.  See included sample for setting this up. 

    // Color ストリームを作成する
    ret = colorStream.create( device, openni::SensorType::SENSOR_COLOR );
    if ( ret != openni::STATUS_OK ) {
      throw std::runtime_error( "colorStream.create" );
    }

    initStream( colorStream );
    ret = colorStream.start();
    if ( ret != openni::STATUS_OK ) {
      throw std::runtime_error( "colorStream.start" );
    }

    // Depth ストリームを作成する
    ret = depthStream.create( device, openni::SensorType::SENSOR_DEPTH );
    if ( ret != openni::STATUS_OK ) {
      throw std::runtime_error( "depthStream.create" );
    }

    initStream( depthStream );
    ret = depthStream.start();
    if ( ret != openni::STATUS_OK ) {
      throw std::runtime_error( "depthStream.start" );
    }

    // Depth と Color のフレームを同期させる
    device.setDepthColorSyncEnabled( true );

    // Depth を Color の座標に合わせる
    device.setImageRegistrationMode( openni::IMAGE_REGISTRATION_DEPTH_TO_COLOR );

    // ストリームをまとめる
    streams.push_back( &colorStream );
    streams.push_back( &depthStream );
  }

  // OpenNI ストリームの初期化
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

  // NiTE の初期化
  void initNite()
  {
    auto version = nite::NiTE::getVersion();
    std::cout << "NiTE   " << version.major << "." << version.minor << "." <<
                              version.maintenance << "." << version.build << std::endl;

    auto ret = nite::NiTE::initialize();

    // HandTracker を作成する
    ret = handTracker.create( &device );
    if ( ret != nite::STATUS_OK ) {
      throw std::runtime_error( "handTracker.create" );
    }

    // ジェスチャーを検出する
    handTracker.startGestureDetection( nite::GestureType::GESTURE_WAVE );
  }

  // Grab Detector の初期化
  void initGrabDetector()
  {
    grabDetector = PSLabs::CreateGrabDetector( device );
    if ( grabDetector == 0 ) {
      throw std::runtime_error( "PSLabs::CreateGrabDetector" );
    }

    grabDetector->AddListener( this );
  }

  // メインループ
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

      // depth および color フレームを取得する
      openni::VideoFrameRef depthFrame;
      depthStream.readFrame( &depthFrame );
      depthImage = convertDepthToColor( depthFrame );

      openni::VideoFrameRef colorFrame;
      colorStream.readFrame( &colorFrame );

      // 手の追跡フレームを取得する
      nite::HandTrackerFrameRef handTrackerFrame;
      handTracker.readFrame( &handTrackerFrame );

      // ジェスチャーを認識したら、その点から手を追跡する
      const nite::Array<nite::GestureData>& gestures =
        handTrackerFrame.getGestures();
      for ( int i = 0; i < gestures.getSize(); ++i ) {
        if ( gestures[i].isComplete() ) {
          nite::HandId newId;
          handTracker.startHandTracking( gestures[i].getCurrentPosition(), &newId );
        }
      }

      // 手を追跡していたら、GrabDetectorにフレームのデータを渡す
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

  // 手の3次元位置を、Depthの2次元位置に変換する
  cv::Point convertHandCoordinatesToDepth( const nite::Point3f& position )
  {
    float x = 0, y = 0;
    handTracker.convertHandCoordinatesToDepth(
      position.x, position.y, position.z, &x, &y );
    return cv::Point( x, y );
  };

  // Depth データをカラー画像に変換する
  cv::Mat convertDepthToColor( openni::VideoFrameRef& depthFrame )
  {
    cv::Mat depthImage = cv::Mat( depthFrame.getVideoMode().getResolutionY(),
      depthFrame.getVideoMode().getResolutionX(),
      CV_8UC4 );

    openni::DepthPixel* depth = (openni::DepthPixel*)depthFrame.getData();
    for ( int i = 0; i < (depthFrame.getDataSize()/sizeof(openni::DepthPixel)); ++i ) {
      // 画像インデックスを生成
      int index = i * 4;

      // 距離データを画像化する
      UCHAR* data = &depthImage.data[index];
      // 0-255のグレーデータを作成する
      // distance : 10000 = gray : 255
      int gray = ~((depth[i] * 255) / 10000);
      data[0] = gray;
      data[1] = gray;
      data[2] = gray;
    }

    return depthImage;
  }

  // GrabEvent の通知(PSLabs::IGrabEventListener::ProcessGrabEvent をオーバーライド)
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

