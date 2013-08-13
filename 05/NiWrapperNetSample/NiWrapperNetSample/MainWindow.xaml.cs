using System;
using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using OpenNIWrapper;

namespace NiWrapperNetSample
{
    /// <summary>
    /// MainWindow.xaml の相互作用ロジック
    /// </summary>
    public partial class MainWindow : Window
    {
        // OpenNI 関連
        Device device;
        VideoStream color;
        VideoStream depth;

        // Color Stream のビットマップ
        System.Drawing.Bitmap colorBitmap;

        // 表示用のDepthデータ
        byte[] depthPixel;
        int depthWidth;
        int depthHeight;

        /// <summary>
        /// コンストラクタ
        /// </summary>
        public MainWindow()
        {
            InitializeComponent();

            Loaded += Window_Loaded;
        }

        /// <summary>
        /// ウィンドウがロードされるときのイベント
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void Window_Loaded( object sender, RoutedEventArgs e )
        {
            try {
                // バージョンを表示する
                TextVersion.Text = OpenNI.Version.ToString();

                // OpenNIを初期化する
                var ret = OpenNI.Initialize();
                if ( ret != OpenNI.Status.OK ) {
                    throw new Exception( ret.ToString() );
                }

                // デバイスを開く
                device = Device.Open( Device.ANY_DEVICE );
                if ( device == null ) {
                    throw new Exception( "failed device open." );
                }

                // Color Stream を有効にする
                color = VideoStream.Create( device, Device.SensorType.COLOR );
                if ( color == null ) {
                    throw new Exception( "failed color stream." );
                }

                color.onNewFrame += color_onNewFrame;
                color.Start();

                // Depth Stream を有効にする
                depth = VideoStream.Create( device, Device.SensorType.DEPTH );
                if ( depth == null ) {
                    throw new Exception( "failed depth stream." );
                }

                depth.onNewFrame += depth_onNewFrame;
                depth.Start();
            }
            catch ( Exception ex ) {
                MessageBox.Show( ex.Message );
            }
        }

        /// <summary>
        /// Color Stream のフレーム更新イベント
        /// </summary>
        /// <param name="vStream"></param>
        void color_onNewFrame( VideoStream vStream )
        {
            using ( var colorFrame = vStream.readFrame() ) {
                if ( colorFrame != null ) {
                    colorBitmap = colorFrame.toBitmap();

                    Dispatcher.BeginInvoke( new Action( () =>
                    {
                        ImageColor.Source = colorBitmap.ToBitmapFrame();
                    } ) );
                }
            }
        }

        /// <summary>
        /// Depth Stream のフレーム更新イベント
        /// </summary>
        /// <param name="vStream"></param>
        void depth_onNewFrame( VideoStream vStream )
        {
            using ( var depthFrame = vStream.readFrame() ) {
                if ( depthFrame != null ) {
                    // フレームのサイズを取得する
                    depthWidth = depthFrame.FrameSize.Width;
                    depthHeight = depthFrame.FrameSize.Height;

                    // Depth のデータを取得する
                    var depth = new short[depthWidth * depthHeight];
                    Marshal.Copy( depthFrame.Data, depth, 0, depth.Length );

                    // 8bitのデータに変換する
                    depthPixel = Array.ConvertAll( depth, d => (byte)(d * (255.0 / 10000)) );

                    // UIスレッドに戻して表示する
                    Dispatcher.BeginInvoke( new Action( () =>
                    {
                        ImageDepth.Source = BitmapSource.Create( depthWidth, depthHeight,
                            96, 96, PixelFormats.Gray8, null, depthPixel, depthWidth );
                    } ) );
                }
            }
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
            using ( System.IO.Stream st = new System.IO.MemoryStream() ) {
                bitmap.Save( st, System.Drawing.Imaging.ImageFormat.Bmp );
                st.Seek( 0, System.IO.SeekOrigin.Begin );
                return BitmapFrame.Create( st, BitmapCreateOptions.None,
                                           BitmapCacheOption.OnLoad );
            }
        }
    }
}
