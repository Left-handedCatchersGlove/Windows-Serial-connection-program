#include <stdio.h>
#include <windows.h>
#include <conio.h>
#include <stdlib.h>
#include "serial.h"
int main (void)
{
  // シリアル通信の作成
  serial_t obj = serial_create("COM4",9600);

  // 受け取る配列
  char buf[2];
  // 受け取ったバイト数
  char len;

  if ( obj == NULL ) {
    fprintf(stderr,"オブジェクト生成に失敗");
    return EXIT_FAILURE;
  }

  while (1)
  {
    // バイト数を受け取るとともに、データを受け取る
    len = serial_recv(obj,buf,sizeof(buf));

    //if (len) serial_send(obj,(unsigned char *)buf,len);
    printf("but[0] : %d\n",buf[0]);
    printf("buf[1] : %d\n",buf[1]);
    //printf("buf[2] : %d\n\n",buf[2]);
    printf("len : %d\n", len);
    Sleep(100);
    if ( kbhit() )  break;
  }


  serial_delete(obj);

  return EXIT_SUCCESS;
}