#include <stdio.h>
#include <Windows.h>
#include <string>
#include <tchar.h>

//#define RECV_DEBUG
//#define SEND_DEBUG

void initDCB(DCB * dcb);
void initTimeOut(COMMTIMEOUTS * comtime);

/*-------- Global parameters --------*/
std::string SCI_pBufferRecieved;   // ��M���������̃o�b�t�@
HANDLE hComPort;                   // �V���A���|�[�g�̃n���h��
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
  // �ʐM�o�b�t�@�[�̐ݒ�
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

  // �o�b�t�@�̏���
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
  dcb->fOutxCtsFlow = FALSE;               // CTS�t���[����Ȃ�(���M���ɁACTS���Ď�)
  dcb->fOutxDsrFlow = FALSE;               // DSR�t���[����Ȃ�(���M���ɁADSR���Ď�)
  dcb->fDtrControl = DTR_CONTROL_DISABLE;  // DTR���C������
  dcb->fRtsControl = RTS_CONTROL_DISABLE;  // RTS���C������
  dcb->fDsrSensitivity = FALSE;   // DSR����Ȃ�
  dcb->fOutX = FALSE;             // XON�Ȃ�
  dcb->fInX = FALSE;              // XOFF�Ȃ�
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
  ����M���̃^�C���A�E�g�ݒ�
  �E1����������̃^�C���A�E�g�W��
  �E�g�[�^���^�C���A�E�g���Ԃ̒萔
  �E��M���̃C���^�[�o��
  �@�^�C���A�E�g���ő���M�ł��鎞�ԊԊu���^�C���A�E�g�W���~����M�������{�^�C���A�E�g�萔�܂ł��A
  �^�C���A�E�g���ő���M�ł��鎞�ԊԊu�B����𒴂���ƁAReadFile,WriteFile���^�C���A�E�g�ɂȂ�A������
  �Ԃ��Ă���B
  �@ReadIntervalTimeout��MAXWARD��ݒ肵�āAReadTotalTimeoutMultiplier,ReadtotalTimeoutConstant
  ��0���w�肷��ƁA�w�蕶������M�܂ł�ReadFile����Ԃ��Ă���B
  */
  comtime->ReadIntervalTimeout = MAXDWORD;   // �����ǂݍ��݂̊Ԃ̎���(�~���b) MAXDWAOR:�����߂� 0:�i�v�҂�
  comtime->ReadTotalTimeoutMultiplier = 1;   // ��M1Byte������̎���
  comtime->ReadTotalTimeoutConstant = 1;     // ��M�֐��R�[�����ԁi�C���^�[�o��0�`250ms�j
  comtime->WriteTotalTimeoutConstant = 1;    // ���M1Byte������̎���
  comtime->WriteTotalTimeoutMultiplier = 1;  // ���M�֐��R�[������

  if (SetCommTimeouts(hComPort, comtime) == FALSE)
  {
    printf("Failed set timeouts\n");
    free(hComPort);
    exit(-1);
  }
}