#include <stdio.h>
#include <Windows.h>
#include <string>
#include <tchar.h>
#include <conio.h>
#include <process.h>

//#define SEND_DEBUG
//#define RECV_DEBUG

void initDCB(DCB * dcb);
void initTimeOut(COMMTIMEOUTS * comtime);
BOOL receive(void);
void send(void);

/*-------- Global parameters --------*/
std::string SCI_pBufferRecieved;   // 受信した文字のバッファ
HANDLE hComPort;                   // シリアルポートのハンドル
OVERLAPPED sendOverlapped, recieveOverlapped;

BOOL ret;
BYTE recv_buf[1];
DWORD recv_len;

// Send param
std::string SCI_pBufferSend;
DWORD SCI_LengthOfPutOrRecieved;

inline DWORD rcv(unsigned char buf[], int reqlen)
{
  DWORD rcved_len;
  ret = ReadFile(hComPort, buf, reqlen, &rcved_len, NULL);
  return rcved_len;
}

int main(void)
{
  DCB dcb;

  // 受信したデータを読み込む
  DWORD numberOfPut;
  //受信データ数を調べる
  DWORD errors;
  COMSTAT comStat;

  // time out param
  COMMTIMEOUTS timeout;

  ZeroMemory(&sendOverlapped, sizeof(OVERLAPPED));
  ZeroMemory(&recieveOverlapped, sizeof(OVERLAPPED));

  /*------ Set com port -----*/
  hComPort = CreateFile(_T("COM3")
    , GENERIC_READ | GENERIC_WRITE , 0 , NULL
    , OPEN_EXISTING,
    /*FILE_ATTRIBUTE_NORMAL*/FILE_FLAG_WRITE_THROUGH,
    NULL
    );
  if (hComPort == INVALID_HANDLE_VALUE)
  {
    printf("Failed open com port");
    return -1;
  }
  SetupComm(hComPort, 1024, 1024);
  //PurgeComm(hComPort, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);

  initDCB(&dcb);
  initTimeOut(&timeout);

  unsigned char buf[256];
  DWORD rd;

  SCI_pBufferSend = 0x31;

  while (1)
  {
    while((rd = rcv(buf, 1)) == 0){};

#ifdef RECV_DEBUG
    //ClearCommError(hComPort, &errors, &comStat);
    int i;
    rd = rcv(buf, 1);
    if (rd > 0) {
      printf("len = %d ", (int)rd);
      for (i = 0; i < rd; i++) printf(" 0x%2x", buf[i]);
      printf("\n");
    }
#endif

    //Sleep(500);
    //while (!receive()){};
    send();
  }

  // バッファの消去
  PurgeComm(hComPort, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
  CloseHandle(hComPort);
}

void initDCB(DCB * dcb)
{
  GetCommState(hComPort, dcb);
  dcb->BaudRate = 9600;
  dcb->ByteSize = 8;
  dcb->fParity = FALSE;
  dcb->Parity = NOPARITY;
  dcb->StopBits = ONESTOPBIT;
  dcb->fOutxCtsFlow = RTS_CONTROL_DISABLE; // CTSフロー制御なし
  dcb->fRtsControl = RTS_CONTROL_DISABLE;  // RTSライン無効
  dcb->EvtChar = NULL;
  if (SetCommState(hComPort, dcb) == FALSE)
  {
    printf("Failed set dcb\n");
    free(hComPort);
    exit(-1);
  }
}

void initTimeOut(COMMTIMEOUTS * comtime)
{
  ZeroMemory(comtime, sizeof(COMMTIMEOUTS));
  GetCommTimeouts(hComPort, comtime);
  comtime->ReadIntervalTimeout = 0;
  comtime->ReadTotalTimeoutMultiplier = 0;
  if (SetCommTimeouts(hComPort, comtime) == FALSE)
  {
    printf("Failed set timeouts\n");
    free(hComPort);
    exit(-1);
  }
}

BOOL receive(void)
{
  ret = ReadFile(hComPort, recv_buf, sizeof(recv_buf), &recv_len, NULL);
  if (ret == FALSE)
  {
    printf("ReadFile failed.\n");
  }
  else
  {
    printf("Data : 0x%x\n", recv_buf[0]);
  }

  return ret;
}

void send(void)
{
  //ret = WriteFile(hComPort, (LPCVOID)test, sizeof(test)+1, &SCI_LengthOfPutOrRecieved, &sendOverlapped);
  ret = WriteFile(hComPort, SCI_pBufferSend.c_str(), SCI_pBufferSend.length() + 1, &SCI_LengthOfPutOrRecieved, &sendOverlapped);
#ifdef SEND_DEBUG
  if (ret == FALSE)
  {
    printf("SendFile failed\n");
  }
#endif
}