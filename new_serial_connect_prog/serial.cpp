#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>

#include "serial.h"

struct _TAG_SERIAL
{
  HANDLE h_comport;            // Serial port handle
  HANDLE h_receive_thread;     // Global variables's reception thread
  DCB dcb;
  OVERLAPPED * sendOverLapped;
  OVERLAPPED * receiveOverLapped;
  unsigned int receive_thread_id;
};

serial_t serial_create(char * com_port, unsigned int baudrate)
{
  serial_t obj;
  obj = (serial_t)malloc(sizeof(struct _TAG_SERIAL));
  if (obj == NULL)
  {
    printf("Failed create obj!!\n");
    return NULL;
  }
  ZeroMemory(obj, sizeof(struct _TAG_SERIAL));
  ZeroMemory(obj->sendOverLapped, sizeof(OVERLAPPED));
  ZeroMemory(obj->receiveOverLapped, sizeof(OVERLAPPED));

  /*-------------------------------------
    Setting COM port
  -------------------------------------*/
  obj->h_comport = com_port;
  obj->h_comport = CreateFile( com_port
    , GENERIC_READ | GENERIC_WRITE
    , 0
    , NULL
    , OPEN_EXISTING
    , FILE_FLAG_OVERLAPPED
    , NULL
    );
  if (obj->h_comport == INVALID_HANDLE_VALUE)
  {
    free(obj);
    printf("Failed set com port!!\n");
    return NULL;
  }
}