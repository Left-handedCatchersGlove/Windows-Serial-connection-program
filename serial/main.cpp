#include <stdio.h>
#include <Windows.h>
#include <string>
#include <tchar.h>

//#define RECV_DEBUG
//#define SEND_DEBUG

void initDCB(DCB * dcb);
void initTimeOut(COMMTIMEOUTS * comtime);

/*-------- Global parameters --------*/
std::string SCI_pBufferRecieved;   // 受信した文字のバッファ
HANDLE hComPort;                   // シリアルポートのハンドル
OVERLAPPED sendOverlapped, recieveOverlapped;

// Recv param
BOOL ret;
DWORD recv_len;

// Send param
std::string SCI_pBufferSend;
DWORD SCI_LengthOfPutOrRecieved;
char c = 0x31;

inline void send(void)
{
  //ret = WriteFile(hComPort, SCI_pBufferSend.c_str(), SCI_pBufferSend.length() + 1, &SCI_LengthOfPutOrRecieved, &sendOverlapped);
  ret = WriteFile(hComPort, &c, sizeof(c), &SCI_LengthOfPutOrRecieved, &sendOverlapped);
#ifdef SEND_DEBUG
  printf("Send data = %c 0x%2x, %d\n", c, c, sizeof(c));
#endif
}

inline BOOL recv(unsigned char buf[], int reqlen)
{
  ret = ReadFile(hComPort, buf, reqlen, &recv_len, NULL);
#ifdef RECV_DEBUG
  if (ret == FALSE)
  {
    printf("ReadFile failed\n");
  }
  else
  {
    if (recv_len > 0)
    {
      int i;
      printf("len = %d\n", (int)recv_len);
      printf("DATA = ");
      for (i = 0; i < recv_len; i++) printf("No.%d 0x%2x", i, buf[i]);
    }
  }
  printf("\n");
#endif
  if (buf[0] == 0x31)
  {
    //printf("0x%2x\n", buf[0]);
    send();
    return true;
  }
  else{
    return false;
  }
  return ret;
}

int main(void)
{
  DCB dcb;

  // time out param
  COMMTIMEOUTS timeout;

  ZeroMemory(&sendOverlapped, sizeof(OVERLAPPED));
  ZeroMemory(&recieveOverlapped, sizeof(OVERLAPPED));

  /*------ Set com port -----*/
  hComPort = CreateFile(_T("COM3")
    , GENERIC_READ | GENERIC_WRITE, 0, NULL
    , OPEN_EXISTING
    , /*FILE_ATTRIBUTE_NORMAL*/0
    , NULL
    );
  if (hComPort == INVALID_HANDLE_VALUE)
  {
    printf("Failed open com port");
    return -1;
  }
  // 通信バッファーの設定
  SetupComm(hComPort, 128, 128);
  PurgeComm(hComPort, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);

  initDCB(&dcb);
  initTimeOut(&timeout);

  SCI_pBufferSend = 0x31;

  unsigned char buf[128];

  while (1)
  {
    //  while (!recv(buf, 1)){};
    //  send();
    recv(buf, 1);
    ZeroMemory(buf, sizeof(buf));
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
  dcb->fOutxCtsFlow = FALSE;               // CTSフロー制御なし(送信時に、CTSを監視)
  dcb->fOutxDsrFlow = FALSE;               // DSRフロー制御なし(送信時に、DSRを監視)
  dcb->fDtrControl = DTR_CONTROL_DISABLE;  // DTRライン無効
  dcb->fRtsControl = RTS_CONTROL_DISABLE;  // RTSライン無効
  dcb->fDsrSensitivity = FALSE;   // DSR制御なし
  dcb->fOutX = FALSE;             // XONなし
  dcb->fInX = FALSE;              // XOFFなし
  //dcb->EvtChar = NULL;
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

  /*
  送受信時のタイムアウト設定
  ・1文字あたりのタイムアウト係数
  ・トータルタイムアウト時間の定数
  ・受信時のインターバル
  　タイムアウト内で送受信できる時間間隔＝タイムアウト係数×送受信文字数＋タイムアウト定数までが、
  タイムアウト内で送受信できる時間間隔。これを超えると、ReadFile,WriteFileがタイムアウトになり、処理が
  返ってくる。
  　ReadIntervalTimeoutにMAXWARDを設定して、ReadTotalTimeoutMultiplier,ReadtotalTimeoutConstant
  に0を指定すると、指定文字数受信までもReadFileから返ってくる。
  */
  comtime->ReadIntervalTimeout = MAXDWORD;   // 文字読み込みの間の時間(ミリ秒) MAXDWAOR:即時戻り 0:永久待ち
  comtime->ReadTotalTimeoutMultiplier = 1;   // 受信1Byteあたりの時間
  comtime->ReadTotalTimeoutConstant = 1;     // 受信関数コール時間（インターバル0～250ms）
  comtime->WriteTotalTimeoutConstant = 1;    // 送信1Byteあたりの時間
  comtime->WriteTotalTimeoutMultiplier = 1;  // 送信関数コール時間

  if (SetCommTimeouts(hComPort, comtime) == FALSE)
  {
    printf("Failed set timeouts\n");
    free(hComPort);
    exit(-1);
  }
}