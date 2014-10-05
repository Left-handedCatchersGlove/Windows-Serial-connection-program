#include <stdio.h>
#include <windows.h>
#include <conio.h>
#include <stdlib.h>
#include "serial.h"

#define DATA_SIZE 10

int main (void)
{
  int i = 0;
  // �V���A���ʐM�̍쐬
  serial_t obj = serial_create("COM3",9600);
  // �󂯎��z��
  char rbuf[DATA_SIZE];
  char *sbuf = "test";
  unsigned char testSbuf = 0x0F;
  // �󂯎�����o�C�g��
  char len;

  if ( obj == NULL ) {
    fprintf(stderr,"�I�u�W�F�N�g�����Ɏ��s\n");
    return EXIT_FAILURE;
  }

  while (1)
  {
    /*------------- ��M���� -------------*/
    // �o�C�g�����󂯎��ƂƂ��ɁA�f�[�^���󂯎��
    len = serial_recv(obj, rbuf, sizeof(rbuf));

    for (i = 0; i < len; i++)
    {
      printf("but[i] : %d\n", i, rbuf[i]);
    }
    printf("len : %d\n", len);

    /*------------- ���M���� -------------*/
    //serial_send(obj, (unsigned char *)sbuf, sizeof(sbuf));
    serial_send(obj, &testSbuf, sizeof(sbuf));
    Sleep(100);

    if ( kbhit() )  break;
  }


  serial_delete(obj);

  return EXIT_SUCCESS;
}