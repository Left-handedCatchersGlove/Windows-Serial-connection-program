#ifndef SERIAL_H_
#define SERIAL_H_

#include <Windows.h>

typedef struct _TAG_SERIAL * serial_t;

serial_t serial_create(char * com_port, unsigned int baudrate);

#endif /* SERIAL_H_ */