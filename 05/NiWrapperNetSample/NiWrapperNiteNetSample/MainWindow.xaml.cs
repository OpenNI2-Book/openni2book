using System;
using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using NiTEWrapper;

namespace NiWrapperNiteNetSample
{
    /// <summary>
    /// MainWindow.xaml の相互作用ロジック
    /// </summary>
    public partial class MainWindow : Window
    {
        UserTracker user;

        short[] userMap;

        WriteableBitmap bitmap;

        byte[] colorPixels;
        Int32Rect rect = new Int32Rect( 0, 0, 640, 480 );
        int stride = 640 * 4;

        public MainWindow()
        {
            InitializeComponent();

            Loaded += Window_Loaded;
        }

        /// <summary>
        /// ウィンドウがロードされるときに呼ばれるイベント
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void Window_Loaded( object sender, RoutedEventArgs e )
        {
            try {
                TextVersion.Text = NiTE.Version.ToString();

                var ret = NiTE.Initialize();
                if ( ret != NiTE.Status.OK ) {
                    throw new Exception( ret.ToString() );
                }

                user = UserTracker.Create();
                user.onNewData += user_onNewData;

                colorPixels = new byte[stride * rect.Height];
                bitmap = new WriteableBitmap( rect.Width, rect.Height, 96, 96, PixelFormats.Pbgra32, null );
                ImageColor.Source = bitmap;
            }
            catch ( Exception ex ) {
                MessageBox.Show( ex.Message );
                Close();
            }
        }

        /// <summary>
        /// ユーザーごとの色
        /// </summary>
        Color[] userColors = new Color[] {
            Colors.Black,
            Colors.Red,
            Colors.Blue,
            Colors.Green,
            Colors.Yellow,
            Colors.Pink,
            };

        /// <summary>
        /// ユーザーフレームの更新イベント
        /// </summary>
        /// <param name="uTracker"></param>
        void user_onNewData( UserTracker uTracker )
        {
            using ( var userFrame = uTracker.readFrame() ) {
                if ( !userFrame.isValid ) {
                    return;
                }

                ShowUserMap( userFrame );

                // DrawingVisualに描画
                var dv = new DrawingVisual();
                using ( var dc = dv.RenderOpen() ) {
                    dc.DrawImage( BitmapSource.Create( 640, 480, 96, 96, PixelFormats.Pbgra32,
                                  null, colorPixels, stride ),
                                  new System.Windows.Rect( 0, 0, 640, 480 ) );

                    Array.ForEach( userFrame.Users, u =>
                    {
                        ProcessUserFrame( uTracker, dc, u );
                    } );
                }

                // Bitmapの作成
                lock ( this ) {
                    var bitmap = new RenderTargetBitmap( 640, 480, 96, 96, PixelFormats.Pbgra32 );
                    bitmap.Render( dv );
                    bitmap.CopyPixels( colorPixels, stride, 0 );
                }

                // RGBカメラの表示
                Dispatcher.BeginInvoke( new Action( () =>
                {
                    lock ( this ) {
                        bitmap.WritePixels( rect, colorPixels, stride, 0 );
                    }
                } ) );

            }
        }

        /// <summary>
        /// ユーザーフレームの処理
        /// </summary>
        /// <param name="uTracker"></param>
        /// <param name="dc"></param>
        /// <param name="u"></param>
        private void ProcessUserFrame( UserTracker uTracker, DrawingContext dc, UserData u )
        {
            const int R = 10;

            // 新しいユーザー
            if ( u.isNew ) {
                // スケルトントラッキングを開始する
                uTracker.StartSkeletonTracking( u.UserId );
            }
            // ユーザーを認識し続けている状態
            else if ( !u.isLost ) {
                var skeleton = u.Skeleton;
                if ( skeleton.State != Skeleton.SkeletonState.TRACKED ) {
                    return;
                }

                // スケルトンの位置を描画する
                foreach ( SkeletonJoint.JointType jointType in
                          Enum.GetValues( typeof( SkeletonJoint.JointType ) ) ) {
                    var joint = skeleton.getJoint( jointType );
                    if ( joint.PositionConfidence >= 0.7 ) {
                        var point = uTracker.ConvertJointCoordinatesToDepth( joint.Position );
                        dc.DrawEllipse( Brushes.White, null,
                                        new System.Windows.Point( point.X, point.Y ), R, R );
                    }
                }
            }
        }

        /// <summary>
        /// ユーザー位置を描画する
        /// </summary>
        /// <param name="userFrame"></param>
        private void ShowUserMap( UserTrackerFrameRef userFrame )
        {
            var um = userFrame.UserMap;
            if ( userMap == null ) {
                userMap = new short[um.FrameSize.Width * um.FrameSize.Height];
            }

            Marshal.Copy( um.Pixels, userMap, 0, userMap.Length );
            for ( int i = 0; i < userMap.Length; i++ ) {
                int colorIndex = i * 4;
                var color = userColors[userMap[i] % userColors.Length];

                colorPixels[colorIndex + 0] = color.B;
                colorPixels[colorIndex + 1] = color.G;
                colorPixels[colorIndex + 2] = color.R;
                colorPixels[colorIndex + 3] = 255;
            }

            Dispatcher.BeginInvoke( new Action( () =>
            {
            } ) );
        }
    }

    /// <summary>
    /// System.Drawing.Bitmap を
    /// System.Windows.Media.Imaging.BitmapFrame に変換する拡張メソッド
    /// </summary>
    public static class BitmapExtentions
    {
        // http://hpcgi2.nifty.com/natupaji/bbs/patio.cgi?mode=view&no=2798
        public static BitmapFrame ToBitmapFrame( this System.Drawing.Bitmap bitmap )
        {
            using ( var st = new System.IO.MemoryStream() ) {
                bitmap.Save( st, System.Drawing.Imaging.ImageFormat.Bmp );
                st.Seek( 0, System.IO.SeekOrigin.Begin );
                return BitmapFrame.Create( st, BitmapCreateOptions.None, BitmapCacheOption.OnLoad );
            }
        }
    }
}
