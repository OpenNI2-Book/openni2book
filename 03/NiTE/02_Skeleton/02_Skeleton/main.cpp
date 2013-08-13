#include <iostream>

#include <NiTE.h>
#include <opencv2/opencv.hpp>

class NiteApp
{
public:
  
  // 初期化
  void initialize()
  {
    // UserTracker を作成する
    userTracker.create();
  }
  
  // フレーム更新処理
  void update()
  {
    // ユーザーフレームを取得する
    nite::UserTrackerFrameRef userFrame;
    userTracker.readFrame( &userFrame );
    
    // ユーザー位置を描画する
    depthImage = showUser( userFrame );
    
    // 検出したユーザーを取得する
    const nite::Array<nite::UserData>& users = userFrame.getUsers();
    for ( int i = 0; i < users.getSize(); ++i ) {
      const nite::UserData& user = users[i];
      
      // 新しく検出したユーザーの場合は、スケルトントラッキングを開始する
      if ( user.isNew() ) {
        userTracker.startSkeletonTracking( user.getId() );
      }
      // すでに検出したユーザーで、消失していない場合は、スケルトンの位置を表示する
      else if ( !user.isLost() ) {
        showSkeleton( depthImage, userTracker, user );
      }
    }
    
    cv::imshow( "Skeleton", depthImage );
  }
  
private:
  
  // ユーザーの検出
  cv::Mat showUser( nite::UserTrackerFrameRef& userFrame )
  {
    // ユーザーにつける色
    static const cv::Scalar colors[] = {
      cv::Scalar( 0, 0, 1 ),
      cv::Scalar( 1, 0, 0 ),
      cv::Scalar( 0, 1, 0 ),
      cv::Scalar( 1, 1, 0 ),
      cv::Scalar( 1, 0, 1 ),
      cv::Scalar( 0, 1, 1 ),
      cv::Scalar( 0.5, 0, 0 ),
      cv::Scalar( 0, 0.5, 0 ),
      cv::Scalar( 0, 0, 0.5 ),
      cv::Scalar( 0.5, 0.5, 0 ),
    };
    
    cv::Mat depthImage;
    
    // Depth フレームを取得する
    openni::VideoFrameRef depthFrame = userFrame.getDepthFrame();
    if ( depthFrame.isValid() ) {
      depthImage = cv::Mat( depthFrame.getHeight(),
                           depthFrame.getWidth(),
                           CV_8UC4 );
      
      // Depth データおよびユーザーインデックスを取得する
      openni::DepthPixel* depth = (openni::DepthPixel*)depthFrame.getData();
      const nite::UserId* pLabels = userFrame.getUserMap().getPixels();
      
      // 1ピクセルずつ調べる
      for ( int i = 0; i < (depthFrame.getDataSize() / sizeof(openni::DepthPixel)); ++i ) {
        // カラー画像インデックスを生成
        int index = i * 4;
        
        // 距離データを画像化する
        uchar* data = &depthImage.data[index];
        if ( pLabels[i] != 0 ) {
          // 人を検出したピクセルにはユーザー番号で色を付ける
          data[0] *= colors[pLabels[i]][0];
          data[1] *= colors[pLabels[i]][1];
          data[2] *= colors[pLabels[i]][2];
        }
        else {
          // 人を検出しなかったピクセルは Depth データを書きこむ
          // 0-255のグレーデータを作成する
          // distance : 10000 = gray : 255
          int gray = ~((depth[i] * 255) / 10000);
          data[0] = gray;
          data[1] = gray;
          data[2] = gray;
        }
      }
    }
    
    return depthImage;
  }
  
  // スケルトンの検出
  void showSkeleton( cv::Mat& depthImage, nite::UserTracker& userTracker, const nite::UserData& user )
  {
    // スケルトンを取得し、追跡状態を確認する
    const nite::Skeleton& skeelton = user.getSkeleton();
    if ( skeelton.getState() != nite::SKELETON_TRACKED ) {
      return;
    }
    
    // すべての関節を描画する
    for ( int j = 0; j <= 14; ++j ) {
      // 関節情報を取得し、信頼度の数値が一定以上の場所のみ表示する
      const nite::SkeletonJoint& joint = skeelton.getJoint( (nite::JointType)j );
      if ( joint.getPositionConfidence() < 0.7f ) {
        continue;
      }
      
      // 3次元の座標を2次元の座標に変換する
      const nite::Point3f& position = joint.getPosition();
      float x = 0, y = 0;
      userTracker.convertJointCoordinatesToDepth(
                                                 position.x, position.y, position.z, &x, &y );
      
      // 円を表示する
      cv::circle( depthImage, cvPoint( (int)x, (int)y ),
                 5, cv::Scalar( 0, 0, 255 ), -1 );
    }
  }
  
private:
  
  nite::UserTracker userTracker;  // ユーザー検出
  
  cv::Mat depthImage;             // 可視化した Depth データ
};

int main(int argc, const char * argv[])
{
  try {
    // NiTE を初期化する
    nite::NiTE::initialize();
    
    // アプリケーションの初期化
    NiteApp app;
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

