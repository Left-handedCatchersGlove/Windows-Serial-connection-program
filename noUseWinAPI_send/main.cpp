#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>

HANDLE h;

int main()
{
  /*------------------------------------------------
    ファイルのクリエイト、オープン
  ------------------------------------------------*/
  h = CreateFile("COM3", GENERIC_READ | GENERIC_WRITE,
    0, 0, OPEN_EXISTING, 0, 0);
  if (h == INVALID_HANDLE_VALUE)
  {
    printf("Open error\n");
    exit(1);
  }
}