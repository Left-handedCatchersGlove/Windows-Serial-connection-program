#include <stdio.h>
#include <Windows.h>
#include <string>
#include <tchar.h>
#include <conio.h>
#include <process.h>

/*-------- Global parameters --------*/
std::string SCI_pBufferRecieved;   // 受信した文字のバッファ
HANDLE hComPort;                   // シリアルポートのハンドル
OVERLAPPED sendOverlapped, recieveOverlapped;

int main(void)
{
  ZeroMemory(&sendOverlapped, sizeof(OVERLAPPED));
  ZeroMemory(&recieveOverlapped, sizeof(OVERLAPPED));

  hComPort = CreateFile(_T("COM3")
    , GENERIC_READ | GENERIC_WRITE , 0 , NULL
    , OPEN_EXISTING , FILE_FLAG_OVERLAPPED, NULL
    );
  if (hComPort == INVALID_HANDLE_VALUE)
  {
    printf("Failed open com port");
    return -1;
  }
  SetupComm(hComPort, 1024, 1024);

  DCB dcb;
  dcb.BaudRate = 9600;
  dcb.ByteSize = 8;
  dcb.Parity = NOPARITY;
  dcb.StopBits = ONESTOPBIT;
  dcb.fOutxCtsFlow = RTS_CONTROL_DISABLE;
  dcb.fRtsControl = RTS_CONTROL_DISABLE;
  dcb.EvtChar = NULL;
  SetCommState(hComPort, &dcb);

  std::string SCI_pBufferSend;
  SCI_pBufferSend = "2\0";

}