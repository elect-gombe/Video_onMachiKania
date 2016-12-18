#define WIDTH_X	30 // 横方向文字数
#define WIDTH_Y	27 // 縦方向文字数
#define ATTROFFSET	(WIDTH_X*WIDTH_Y) // VRAM上のカラーパレット格納位置

extern volatile char drawing;		//　表示期間中は-1
extern volatile unsigned short drawcount;		//　1画面表示終了ごとに1足す。アプリ側で0にする。
							// 最低1回は画面表示したことのチェックと、アプリの処理が何画面期間必要かの確認に利用。
extern unsigned char TVRAM[]; //テキストビデオメモリ
extern void start_composite(void); //カラーコンポジット出力開始
extern void stop_composite(void); //カラーコンポジット出力停止
extern void init_composite(void); //カラーコンポジット出力初期化
extern void clearscreen(void); //画面クリア
extern void set_palette(unsigned char n,unsigned char b,unsigned char r,unsigned char g); //パレット設定
extern void set_bgcolor(unsigned char b,unsigned char r,unsigned char g); //バックグランドカラー設定

extern const unsigned char FontData[]; //フォントパターン定義
