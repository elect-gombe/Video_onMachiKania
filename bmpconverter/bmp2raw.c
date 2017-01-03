#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>


typedef struct{
  unsigned int width;
  unsigned int height;
  unsigned int bdepth;
} bmp_infohead_t;

typedef struct{
  uint8_t blue;
  uint8_t green;
  uint8_t red;
  uint8_t reserved;
} bmp_palette_t;

bmp_palette_t palette[256];

#define TOUINT32(x) (uint32_t)((x)[3]<<24|(x)[2]<<16|(x)[1]<<8|(x)[0]<<0)
#define TOINT32(x) (int32_t)((x)[3]<<24|(x)[2]<<16|(x)[1]<<8|(x)[0]<<0)

#define TOUINT16(x) (uint16_t)((x)[1]<<8|(x)[0]<<0)
#define TOINT16(x) (int16_t)((x)[1]<<8|(x)[0]<<0)

#define TOUINT8(x) (uint8_t)((x)[0]<<0)
#define TOINT8(x) (int8_t)((x)[0]<<0)
#define ERROROUT(fmt,...) _msg( __FUNCTION__, __LINE__,fmt, ## __VA_ARGS__)

void _msg(const char *func,int line,const char *fmt,...){
  fprintf(stderr,"%-10s %-3d:", func, line);
  {
    va_list arp;
    va_start(arp, fmt);
    vfprintf(stderr,fmt,arp);
    va_end(arp);
  }
  fprintf(stderr,"\n");
}

int readhead(FILE *fp,bmp_infohead_t *bh);
void displaybmpinfo(bmp_infohead_t *bh);
void writecolor(uint8_t color,bmp_palette_t *palv);
int readbmp_onaline(FILE *fp,bmp_infohead_t *bh,uint8_t *data);
void getpalette(FILE *fp,const bmp_infohead_t *bh,bmp_palette_t *palv);
int writepalette(bmp_infohead_t *bh,bmp_palette_t *palv);
void printbmp(uint8_t *data,unsigned int size);

int main(int argc,char **argv){
  char *filename;
  FILE *fp;
  bmp_infohead_t bh;
  uint8_t *data;
  int i;

  while(--argc){
    filename = *++argv;
    fprintf(stderr,"%s\n",filename);
    fp = fopen(filename,"r");
    if(fp==NULL){
      ERROROUT("<%s> file cannot open\n",filename);
      return EXIT_FAILURE;
    }
    if(readhead(fp,&bh)!=EXIT_SUCCESS)continue;
    displaybmpinfo(&bh);
    getpalette(fp, &bh, palette);

    writepalette(&bh,palette);

    unsigned int size;
    if(bh.bdepth==4){
      size = bh.width * bh.height;
    }else{
      ERROROUT("bd != 4");
    }

    data = malloc(size);
    if(data==NULL){
      ERROROUT("cannot allocate (%d)byte",size);
    }
    
    for(i=bh.height-1;i>=0;i--){
      readbmp_onaline(fp, &bh, i*bh.width+data);
    }
    printbmp(data, size);
    free(data);
    
    fclose(fp);
  }
}

void printbmp(uint8_t *data,unsigned int size){
  while(size){
    printf("%c",((data[3])<<4)|((data[2])));
    printf("%c",((data[1])<<4)|((data[0])));
    data += 4;
    size-=4;
  }
}

int writepalette(bmp_infohead_t *bh,bmp_palette_t *palv){
  int i;
  if(bh->bdepth > 8){
    ERROROUT("no palette");
    return EXIT_FAILURE;
  }
  
  for(i=0;i<(1<<bh->bdepth);i++){
    fprintf(stderr,"%d,",palv[i].red);
    fprintf(stderr,"%d,",palv[i].green);
    fprintf(stderr,"%d\n",palv[i].blue);
    printf("%c",palv[i].red);
    printf("%c",palv[i].green);
    printf("%c",palv[i].blue);
  }
  return EXIT_SUCCESS;
}

int readhead(FILE *fp,bmp_infohead_t *bh){
  unsigned char head[40];
  int readnum;

  if(fseek(fp, 14, SEEK_SET)==14){
    ERROROUT("seek faild\n");
  }
  readnum=fread(head, 4, 1, fp);

  if(readnum == 1&&TOUINT32(head)!=40){
    ERROROUT("read:%d\n",readnum);
    ERROROUT("err not surport filetype.\nsize:%d NOT MS type\n",head[0]);
    return EXIT_FAILURE;
  }else{
    readnum = fread(head+4,1,36,fp);
    if(readnum == 36){
      bh->width = TOUINT32(head+4);
      bh->height = TOUINT32(head+8);
      bh->bdepth = TOUINT16(head+14);
    }else{
      ERROROUT("err not surport filetype.\n");
      return EXIT_FAILURE;
    }
  }
  return EXIT_SUCCESS;
}

void displaybmpinfo(bmp_infohead_t *bh){
  fprintf(stderr ,"width:%d\n",bh->width);
  fprintf(stderr ,"height:%d\n",bh->height);
  fprintf(stderr ,"bpdepth:%d\n",bh->bdepth);
}

void getpalette(FILE *fp,const bmp_infohead_t *bh,bmp_palette_t *palv){
  int i;
  unsigned char palette[4];
  if(bh->bdepth < 8){
    for(i=0;i<(0x01<<bh->bdepth);i++){
      if(fread(palette,1,4,fp)==4){
	palv[i].blue = palette[0];
	palv[i].green = palette[1];
	palv[i].red = palette[2];
	palv[i].reserved = palette[3];
      }else{
	ERROROUT("file error");
      }
    }
  }else{
    ERROROUT("no palette");
  }
}

int readbmp_onaline(FILE *fp,bmp_infohead_t *bh,uint8_t *data){
  int i;
  uint8_t *buff;
  uint8_t *bp;

  if(bh->bdepth==4){
    buff = malloc(bh->width);
    if(buff == NULL){
      ERROROUT("buff is NULL.\nmalloc failed");
      return EXIT_FAILURE;
    }
    fread(buff, 1, bh->width / 2,fp);
    bp = buff;
    for(i=0;i<(int)bh->width;){
      data[i++] = *bp >> 4;
      data[i++] = *bp++ & 0xF;
    }
  }
  free(buff);
  return EXIT_SUCCESS;
}

void setBackColor(int r,int g,int b);
void writecolor(uint8_t color,bmp_palette_t *palv){
  setBackColor(palv[color].red,palv[color].green,palv[color].blue);
  printf("  ");
}

void setBackColor(int r,int g,int b){
  int n;

  r/=43;
  g/=43;
  b/=43;
  
  n=r;
  n*=6;
  n+=g;
  n*=6;
  n+=b;
  n+=16;
  printf("\033[48;05;%dm",n);
}
