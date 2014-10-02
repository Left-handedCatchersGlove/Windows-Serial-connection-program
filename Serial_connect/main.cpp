#include <stdio.h>
#include <windows.h>
#include <conio.h>
#include <stdlib.h>
#include "serial.h"

#define DATA_SIZE 10

int main (void)
{
  int i = 0;
  // シリアル通信の作成
  serial_t obj = serial_create("COM4",9600);
  // 受け取る配列
  char buf[DATA_SIZE];
  // 受け取ったバイト数
  char len;

  if ( obj == NULL ) {
    fprintf(stderr,"オブジェクト生成に失敗\n");
    return EXIT_FAILURE;
  }

  while (1)
  {
    // バイト数を受け取るとともに、データを受け取る
    len = serial_recv(obj,buf,sizeof(buf));

    //if (len) serial_send(obj,(unsigned char *)buf,len);
    for (i = 0; i < DATA_SIZE; i++)
    {
      printf("but[i] : %d\n", i, buf[i]);
    }
    printf("len : %d\n", len);
    Sleep(100);
    if ( kbhit() )  break;
  }


  serial_delete(obj);

  return EXIT_SUCCESS;
}