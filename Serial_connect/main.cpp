#include <stdio.h>
#include <windows.h>
#include <conio.h>
#include <stdlib.h>
#include "serial.h"

#define DATA_SIZE 10

#define OK true
int main (void)
{
  int i = 0;
  // シリアル通信の作成
  serial_t obj = serial_create("COM3",9600);
  // 受け取る配列
  char rbuf[DATA_SIZE];
  char *sbuf = "0";
  unsigned char testSbuf = 0x0F;
  // 受け取ったバイト数
  char len;

  if ( obj == NULL ) {
    fprintf(stderr,"オブジェクト生成に失敗\n");
    return EXIT_FAILURE;
  }

  while (1)
  {
    /*------------- 受信部分 -------------*/
    // バイト数を受け取るとともに、データを受け取る
    //    len = serial_recv(obj, rbuf, sizeof(rbuf));
    while (0 == (len = serial_recv(obj, rbuf, 20))) {};

#ifdef OK
    printf("len : %d\n", len);
    for (i = 0; i < len; i++)
    {
      printf("but[%d] : 0x%2x\n", i, rbuf[i]);
    }
#endif
     // serial_send(obj, (unsigned char *)sbuf, sizeof(sbuf));
    

    /*------------- 送信部分 -------------*/
    //serial_send(obj, (unsigned char *)sbuf, sizeof(sbuf));
    //serial_send(obj, &testSbuf, sizeof(sbuf));
   Sleep(500);

//    if ( kbhit() )  break;
  }


  serial_delete(obj);

  return EXIT_SUCCESS;
}