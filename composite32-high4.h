// コンポジットカラー信号出力プログラム 高解像度版4（正方画素）　PIC32MX150F128B用ヘッダーファイル　by K.Tanaka
//　4ビット単位VRAM、16色

#define X_RES	256 // 横方向解像度
#define Y_RES	224 // 縦方向解像度
#define H_WORD	(X_RES/4) // 1行当りのワード数(16bit単位)

// 入力ボタンのポート、ビット定義
#define KEYPORT PORTB
#define KEYUP 0x0400
#define KEYDOWN 0x0080
#define KEYLEFT 0x0100
#define KEYRIGHT 0x0200
#define KEYSTART 0x0800
#define KEYFIRE 0x4000

extern volatile char drawing;		//　表示期間中は-1
extern volatile unsigned short drawcount;		//　1画面表示終了ごとに1足す。アプリ側で0にする。
							// 最低1回は画面表示したことのチェックと、アプリの処理が何画面期間必要かの確認に利用。

extern unsigned short VRAM[];
extern void start_composite(void); //カラーコンポジット出力開始
extern void stop_composite(void); //カラーコンポジット出力停止
extern void init_composite(void); //カラーコンポジット出力初期化
extern void clearscreen(void); //画面クリア
extern void set_palette(unsigned char n,unsigned char b,unsigned char r,unsigned char g); //カラーパレット設定
