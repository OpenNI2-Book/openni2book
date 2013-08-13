#include <iostream>
#include <list>

#include <NiTE.h>
#include <opencv2/opencv.hpp>

class GestureApp
{
public:
  
  void initialize()
  {
    // ハンドトラッカーを作成する
    handTracker.create();
    
    // 認識させるジェスチャーを登録する
    handTracker.startGestureDetection( nite::GESTURE_WAVE );
    handTracker.startGestureDetection( nite::GESTURE_CLICK );
  }
  
  void update()
  {
    // 手の追跡フレームを取得する
    nite::HandTrackerFrameRef handTrackerFrame;
    handTracker.readFrame( &handTrackerFrame );
    
    // Depth データを可視化する
    depthImage = showDepthStream( handTrackerFrame.getDepthFrame() );
    
    // 検出したジェスチャーを表示する
    showGesture( depthImage, handTrackerFrame );
    
    // 手の追跡を表示する
    showHandTracker( depthImage, handTrackerFrame );
    
    cv::imshow( "Hand Tracker", depthImage );
  }
  
private:
  
  // Depth ストリームを表示できる形に変換する
  cv::Mat showDepthStream( const openni::VideoFrameRef& depthFrame )
  {
    cv::Mat depthImage = cv::Mat( depthFrame.getHeight(),
                                 depthFrame.getWidth(),
                                 CV_8UC4 );
    
    openni::DepthPixel* depth = (openni::DepthPixel*)depthFrame.getData();
    for ( int i = 0; i < (depthFrame.getDataSize()/sizeof(openni::DepthPixel)); ++i ) {
      // 画像インデックスを生成
      int index = i * 4;
      
      // 距離データを画像化する
      uchar* data = &depthImage.data[index];
      
      // 0-255のグレーデータを作成する
      // distance : 10000 = gray : 255
      int gray = ~((depth[i] * 255) / 10000);
      data[0] = gray;
      data[1] = gray;
      data[2] = gray;
    }
    
    return depthImage;
  }
  
  // 検出したジェスチャーを表示する
  void showGesture( cv::Mat depthImage, const nite::HandTrackerFrameRef& handTrackerFrame )
  {
    // 認識したジェスチャー名を表示する
    const nite::Array<nite::GestureData>& gestures = handTrackerFrame.getGestures();
    for ( int i = 0; i < gestures.getSize(); ++i ) {
      if ( gestures[i].isComplete() ) {
        // ジェスチャーを認識した手の位置から、手の追跡を開始する
        nite::HandId newId;
        handTracker.startHandTracking( gestures[i].getCurrentPosition(), &newId );
        
        if ( gestures[i].getType() == nite::GESTURE_WAVE ) {
          detectGesture = "wave";
        }
        else if (gestures[i].getType() == nite::GESTURE_CLICK){
          detectGesture = "click";
        }
      }
    }
    
    // 検出したジェスチャーを表示する
    cv::putText( depthImage, detectGesture.c_str(), cv::Point( 0, 50 ),
                cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar( 255, 0, 0 ), 1 );
  }
  
  // 手の追跡を表示する
  void showHandTracker( cv::Mat depthImage, const nite::HandTrackerFrameRef& handTrackerFrame )
  {
    // 手を追跡していたら、その点を記録する(30点を保持する)
    const nite::Array<nite::HandData>& hands = handTrackerFrame.getHands();
    for (int i = 0; i < hands.getSize(); ++i) {
      if ( hands[i].isTracking() ) {
        handPoints.push_back( hands[i].getPosition() );
        if ( handPoints.size() > 30 ) {
          handPoints.pop_front();
        }
      }
    }
    
    // 手の軌跡を表示する
    for ( std::list<nite::Point3f>::iterator it = handPoints.begin(); it != handPoints.end();  ) {
      // 線の開始位置を取得する
      cv::Point start = convertHandCoordinatesToDepth( *it );
      if ( ++it == handPoints.end() ) {
        // 最後の点であれば終了
        break;
      }
      
      // 終点を取得して線を引く
      cv::Point end = convertHandCoordinatesToDepth( *it );
      cv::line( depthImage, start, end, cv::Scalar( 0, 255, 0 ), 3 );
    }
  }
  
  // 3次元の座標を2次元に変換する
  cv::Point convertHandCoordinatesToDepth( nite::Point3f& position )
  {
    float x = 0, y = 0;
    handTracker.convertHandCoordinatesToDepth( position.x, position.y, position.z, &x, &y );
    return cv::Point( x, y );
  };
  
private:
  
  nite::HandTracker handTracker;        // 手の追跡
  
  cv::Mat depthImage;                   // Depth データを可視化したもの
  std::string detectGesture;            // 検出したジェスチャー
  
  std::list<nite::Point3f> handPoints;  // 手の軌跡
};

int main(int argc, const char * argv[])
{
  try {
    // NiTE を初期化する
    nite::NiTE::initialize();
    
    // アプリケーションの初期化
    GestureApp app;
    app.initialize();
    
    // メインループ
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

