#define _SUPPRESS_PLIB_WARNING
// TETRIS（キャラクターベース） Rev.1.0 2013/8/22
// 　with テキストVRAMコンポジット出力システム for PIC32MX1xx/2xx by K.Tanaka
// rev.2 PS/2キーボードシステムに対応

#include <plib.h>
#include <stdlib.h>
#include "videoout.h"

#include "ff.h"
#include <stdint.h>

// 入力ボタンのポート、ビット定義
#define KEYPORT PORTB
#define KEYUP 0x0400
#define KEYDOWN 0x0080
#define KEYLEFT 0x0100
#define KEYRIGHT 0x0200
#define KEYSTART 0x0800
#define KEYFIRE 0x4000



#define SAMPLING_FREQ 32000
#define OUTPUT_FREQ 100000
#define CLOCK_FREQ (3.58*1000000*15)


#define SIZEOFSOUNDBF 2048


//外付けクリスタル with PLL (15倍)
#pragma config PMDL1WAY = OFF, IOL1WAY = OFF
#pragma config FPLLIDIV = DIV_1, FPLLMUL = MUL_15, FPLLODIV = DIV_1
#pragma config FNOSC = PRIPLL, FSOSCEN = OFF, POSCMOD = XT, OSCIOFNC = OFF
#pragma config FPBDIV = DIV_1, FWDTEN = OFF, JTAGEN = OFF, ICESEL = ICS_PGx1

uint16_t VRAMA[256*224/4+256*220/4];
uint16_t *VRAM = VRAMA;

typedef unsigned int uint;

void audiotask(void);

unsigned char sounddata[SIZEOFSOUNDBF] = {0};

FIL fhandle;
FATFS fatfs;

//uint16_t VRAM[X_RES*Y_RES/4];

void main(void) {

    int i;
    static volatile FRESULT res;
    FIL video;

    OSCConfig(OSC_POSC_PLL, OSC_PLL_MULT_15, OSC_PLL_POST_1, 0);

    // 周辺機能ピン割り当て
    SDI2R = 2; //RPA4:SDI2
    RPB5R = 4; //RPB5:SDO2

    //ポートの初期設定
    TRISA = 0x0010; // RA4は入力
    CNPUA = 0x0010; // RA4をプルアップ
    ANSELA = 0x0000; // 全てデジタル
    TRISB = KEYSTART | KEYFIRE | KEYUP | KEYDOWN | KEYLEFT | KEYRIGHT; // ボタン接続ポート入力設定
    CNPUBSET = KEYSTART | KEYFIRE | KEYUP | KEYDOWN | KEYLEFT | KEYRIGHT; // プルアップ設定
    ANSELB = 0x0000; // 全てデジタル
    LATACLR = 2; // RA1=0（ボタンモード）

    RPB13R = 5; //RPB13ピンにOC4を割り当て
    OC4R = 0;
    OC4CON = 0x000e; // Timer3ベース、PWMモード
    OC4CONSET = 0x8000; //OC4スタート
    T3CON = 0x0000; // プリスケーラ1:1
    PR3 = 256;
    T3CONSET = 0x8000; // タイマ3スタート

    T4CONbits.SIDL = 0;
    T4CONbits.TCKPS = 3;
    T4CONbits.T32 = 0;
    T4CONbits.TCS = 0;
    TMR4 = 0; /*とりあえず和音を再生しました。ソースを公開します。ハードは同じで多分大丈夫ですが心配ならローパスフィルターを入れてください。割り込みなどは一切使っていません。
	      コードはリファクタリングしておきます。*/
    PR4 = CLOCK_FREQ / 8 / SAMPLING_FREQ;
    T4CONbits.ON = 1;

    DmaChnOpen(0, 0, DMA_OPEN_AUTO);

    DmaChnSetEventControl(0, DMA_EV_START_IRQ(_TIMER_4_IRQ));

    DmaChnSetTxfer(0, sounddata, (void*) &OC4RS, sizeof (sounddata), 1, 1);

    DmaChnEnable(0);

    init_composite(); // ビデオ出力システムの初期化
    init_graphic(VRAMA);
    set_graphmode(1);


    int curr = 2;
#define FILENAME "music.raw"
    printstr("SD INIT...");
    if (disk_initialize(0) != 0) {
        printstr("SD INIT ERR");
        while (1) asm("wait");
    }
    if (res = f_mount(&fatfs, "", 0) != FR_OK) {
        printstr("SD INIT ERR");
        while (1) asm("wait");
    } else {
        res = f_open(&fhandle, FILENAME, FA_READ);
        if (res != FR_OK) {
            printstr("FILE <"FILENAME"> NOT FOUND");
            while (1) asm("wait");
        }
#define VIDEOFILE "video"   
        printstr("FILE <"VIDEOFILE"> FOUND");
        res = f_open(&video,VIDEOFILE,FA_READ);
        if (res!=FR_OK) {
            printstr("FILE <"VIDEOFILE"> NOT FOUND");
            while (1) asm("wait");
        }
        printstr("FILE <"VIDEOFILE"> FOUND");
    }
    //    SPI2BRG = 0;
    int read;
    int prevcount = 0;
    uint8_t palettebuff[16*3];
    
    while (1) {
        f_read(&video,palettebuff,16*3,&read);

        //        musicTask();
//        VRAM = VRAMA;
        prevcount = drawcount;
  
        f_read(&video, VRAM, 256*130/2, &read);
        for (i = 0; i < 16; i++) {
            g_set_palette(i, palettebuff[i*3+2],palettebuff[i*3+0],palettebuff[i*3+1]);
        }
        if(VRAM==VRAMA){
            VRAM = VRAMA + 256*220/4;
            gVRAM = VRAMA;
        }else{
            VRAM = VRAMA;            
            gVRAM = VRAMA + 256*220/4;
        }
  
//        f_read(&fhandle, buff, SIZEOFSOUNDBF / 2, &time);

        while(drawcount-prevcount<1){
            asm("wait");
        }
//        if(time==0){
//            while(1);
//        }
        
    }
}

void musicTask(void) {
    audiotask();
}

void audiotask(void) {
    static uint prevtrans = 1;
    uint8_t *buff;
    UINT read;

#ifdef SIMMODE
    buff = &sounddata[0];
#else
    buff = NULL; //&sounddata[0];
#endif
    if (DmaChnGetEvFlags(0) & DMA_EV_SRC_HALF) {
        DmaChnClrEvFlags(0, DMA_EV_SRC_HALF);
        if (prevtrans == 2) {
            prevtrans = 1;
            buff = &sounddata[0];
        }
    } else if (DmaChnGetEvFlags(0) & DMA_EV_SRC_FULL) {
        DmaChnClrEvFlags(0, DMA_EV_SRC_FULL);
        if (prevtrans == 1) {
            prevtrans = 2;
            buff = &sounddata[SIZEOFSOUNDBF / 2];
        }
    }

    if (buff) {
        int i;
        for (i = 0; i < SIZEOFSOUNDBF / 2; i++) {
            buff[i] = 128;
        }
        //soundTask(buff);
        f_read(&fhandle, buff, SIZEOFSOUNDBF / 2, &read);
    }
}
