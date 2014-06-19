#include <stdio.h>
#include <windows.h>
#include <conio.h>
#include <stdlib.h>
#include "serial.h"
int main (void)
{
  // �V���A���ʐM�̍쐬
  serial_t obj = serial_create("COM4",9600);

  // �󂯎��z��
  char buf[2];
  // �󂯎�����o�C�g��
  char len;

  if ( obj == NULL ) {
    fprintf(stderr,"�I�u�W�F�N�g�����Ɏ��s");
    return EXIT_FAILURE;
  }

  while (1)
  {
    // �o�C�g�����󂯎��ƂƂ��ɁA�f�[�^���󂯎��
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