#define _SUPPRESS_PLIB_WARNING
// TETRIS�i�L�����N�^�[�x�[�X�j Rev.1.0 2013/8/22
// �@with �e�L�X�gVRAM�R���|�W�b�g�o�̓V�X�e�� for PIC32MX1xx/2xx by K.Tanaka
// rev.2 PS/2�L�[�{�[�h�V�X�e���ɑΉ�

#include <plib.h>
#include <stdlib.h>
#include "composite32-high4.h"

#include "ff.h"
#include <stdint.h>

// ���̓{�^���̃|�[�g�A�r�b�g��`
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


//�O�t���N���X�^�� with PLL (15�{)
#pragma config PMDL1WAY = OFF, IOL1WAY = OFF
#pragma config FPLLIDIV = DIV_1, FPLLMUL = MUL_15, FPLLODIV = DIV_1
#pragma config FNOSC = PRIPLL, FSOSCEN = OFF, POSCMOD = XT, OSCIOFNC = OFF
#pragma config FPBDIV = DIV_1, FWDTEN = OFF, JTAGEN = OFF, ICESEL = ICS_PGx1

typedef unsigned int uint;

void audiotask(void);

unsigned char sounddata[SIZEOFSOUNDBF] = {0};

unsigned char *cursor; //�J�[�\���ʒu
unsigned char cursorc; //�J�[�\���F

FIL fhandle;
FATFS fatfs;

void main(void) {

    int i;
    static volatile FRESULT res;
    uint8_t buff[32 * 96 * 2];
    FIL video;

    OSCConfig(OSC_POSC_PLL, OSC_PLL_MULT_15, OSC_PLL_POST_1, 0);

    // ���Ӌ@�\�s�����蓖��
    SDI2R = 2; //RPA4:SDI2
    RPB5R = 4; //RPB5:SDO2

    //�|�[�g�̏����ݒ�
    TRISA = 0x0010; // RA4�͓���
    CNPUA = 0x0010; // RA4���v���A�b�v
    ANSELA = 0x0000; // �S�ăf�W�^��
    TRISB = KEYSTART | KEYFIRE | KEYUP | KEYDOWN | KEYLEFT | KEYRIGHT; // �{�^���ڑ��|�[�g���͐ݒ�
    CNPUBSET = KEYSTART | KEYFIRE | KEYUP | KEYDOWN | KEYLEFT | KEYRIGHT; // �v���A�b�v�ݒ�
    ANSELB = 0x0000; // �S�ăf�W�^��
    LATACLR = 2; // RA1=0�i�{�^�����[�h�j

    RPB13R = 5; //RPB13�s����OC4�����蓖��
    OC4R = 0;
    OC4CON = 0x000e; // Timer3�x�[�X�APWM���[�h
    OC4CONSET = 0x8000; //OC4�X�^�[�g
    T3CON = 0x0000; // �v���X�P�[��1:1
    PR3 = 256;
    T3CONSET = 0x8000; // �^�C�}3�X�^�[�g

    T4CONbits.SIDL = 0;
    T4CONbits.TCKPS = 3;
    T4CONbits.T32 = 0;
    T4CONbits.TCS = 0;
    TMR4 = 0; /*�Ƃ肠�����a�����Đ����܂����B�\�[�X�����J���܂��B�n�[�h�͓����ő������v�ł����S�z�Ȃ烍�[�p�X�t�B���^�[�����Ă��������B���荞�݂Ȃǂ͈�؎g���Ă��܂���B
	      �R�[�h�̓��t�@�N�^�����O���Ă����܂��B*/
    PR4 = CLOCK_FREQ / 8 / SAMPLING_FREQ;
    T4CONbits.ON = 1;

    DmaChnOpen(0, 0, DMA_OPEN_AUTO);

    DmaChnSetEventControl(0, DMA_EV_START_IRQ(_TIMER_4_IRQ));

    DmaChnSetTxfer(0, sounddata, (void*) &OC4RS, sizeof (sounddata), 1, 1);

    DmaChnEnable(0);

    init_composite(); // �r�f�I�o�̓V�X�e���̏�����

    for (i = 0; i < 16; i++) {
        set_palette(i, i * 17, i * 17, i * 17);
    }

    int curr = 2;
#define FILENAME "music.raw"
    printstr(3, curr++*10, 13, -1, "SD INIT...");
    if (disk_initialize(0) != 0) {
        printstr(3, curr++*10, 13, -1, "SD INIT ERR");
        while (1) asm("wait");
    }
    if (res = f_mount(&fatfs, "", 0) != FR_OK) {
        printstr(3, curr++*10, 13, -1, "SD INIT ERR");
        while (1) asm("wait");
    } else {
        res = f_open(&fhandle, FILENAME, FA_READ);
        if (res != FR_OK) {
            printstr(3, curr++*10, 13, -1, "FILE <"FILENAME"> NOT FOUND");
            while (1) asm("wait");
        }
#define VIDEOFILE "video"   
        printstr(3, curr++*10, 13, -1, "FILE <"VIDEOFILE"> FOUND");
        res = f_open(&video,VIDEOFILE,FA_READ);
        if (res!=FR_OK) {
            printstr(3, curr++*10, 13, -1, "FILE <"VIDEOFILE"> NOT FOUND");
            while (1) asm("wait");
        }
        printstr(3, curr++*10, 13, -1, "FILE <"VIDEOFILE"> FOUND");
    }
    line(0, 0, 255, 224, 3);
    //    SPI2BRG = 0;
    int read;
    f_read(&video,VRAM,14,&read);
    int prevcount = 0;
    while (1) {
//        musicTask();
        prevcount = drawcount;
        f_read(&video, VRAM, 256*192/2, &read);

//        f_read(&fhandle, buff, SIZEOFSOUNDBF / 2, &time);

        printnum(100,200,13,0,drawcount);
        printnum(100,210,13,0,drawcount-prevcount);
        
        while(drawcount-prevcount<2){
            musicTask();
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