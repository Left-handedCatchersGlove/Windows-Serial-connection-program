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
  serial_t obj = serial_create("COM4",9600);
  // �󂯎��z��
  char buf[DATA_SIZE];
  // �󂯎�����o�C�g��
  char len;

  if ( obj == NULL ) {
    fprintf(stderr,"�I�u�W�F�N�g�����Ɏ��s\n");
    return EXIT_FAILURE;
  }

  while (1)
  {
    // �o�C�g�����󂯎��ƂƂ��ɁA�f�[�^���󂯎��
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