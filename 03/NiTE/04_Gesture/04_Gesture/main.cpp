#include <iostream>

#include <opencv2/opencv.hpp>
#include <NiTE.h>

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
    
    cv::imshow( "Gesture", depthImage );
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
  
private:
  
  nite::HandTracker handTracker;  // 手の追跡
  
  cv::Mat depthImage;             // Depth データを可視化したもの
  std::string detectGesture;      // 検出したジェスチャー
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

