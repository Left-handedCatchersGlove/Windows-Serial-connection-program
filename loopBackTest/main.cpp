#include <stdio.h>
#include <Windows.h>
#include <string>
#include <tchar.h>
#include <conio.h>
#include <process.h>

/*-------- prototype --------*/
// �V���A���ʐM
int SCI(HWND hWnd);
// ��M�X���b�h�̃v���g�^�C�v�錾
unsigned __stdcall SCI_Receive_Thread(void *);
// ���������邩���Ȃ���
// 0 : �X���b�h�����[�v�����ɓ���Ȃ�
// 1 : �X���b�h�����[�v�����ɓ���
int RecievedEndFlag = 0;
// �󂯎�������ǂ����̃t���O
bool flag = false;

/*-------- Global parameters --------*/
std::string SCI_pBufferRecieved;   // ��M���������̃o�b�t�@
HANDLE hComPort;                   // �V���A���|�[�g�̃n���h��
HANDLE hSCI_Receive_Thread;        // ��M�X���b�h�̃O���[�o���ϐ�
OVERLAPPED sendOverlapped, recieveOverlapped;//�uReadFile()�v�uWriteFile()�v�����s���鎞�ɁuOVERLAPPED�v���w�肷�邱�Ƃ��K�{�Ȃ̂Œ�`����
unsigned int dwThreadId;           //��M�X���b�h��ID

/*-------- Main function --------*/
int main()
{
  // ������
  ZeroMemory(&sendOverlapped, sizeof(OVERLAPPED));
  ZeroMemory(&recieveOverlapped, sizeof(OVERLAPPED));

  /*-------------------------------------
    �V���A���ʐM�̃t�@�C���f�B�X�N���v�^����
    -------------------------------------*/
  hComPort = CreateFile( _T("COM3")
    , GENERIC_READ | GENERIC_WRITE
    , 0                      // �I�u�W�F�N�g�����L���@���Ȃ�
    , NULL                   // �Z�L�����e�B�����f�t�H���g//�n���h�����q�v���Z�X�֌p�����邱�Ƃ������邩�ǂ���//NULL�͌p���ł��Ȃ�
    , OPEN_EXISTING          // �t�@�C�����J���܂��B�w�肵���t�@�C�������݂��Ă��Ȃ��ꍇ�A���̊֐��͎��s���܂��B
    , FILE_FLAG_OVERLAPPED   // 0//FILE_FLAG_OVERLAPPED�́A�񓯊��ł��邱�Ƃ��Ӗ�����B//�����Ƃ́A���̊֐������s����Ə�������������܂ő҂������B�񓯊��Ƃ�CPU�̋󂫎��Ԃ��g���Ċ֐������������
                             //�ǂ�������Ȃ��̂�����ǁAFILE_FLAG_OVERLAPPED��ݒ肷��Ƒ��M����肭�s���Ă��Ȃ��C������c�c�B
                             //�ˁuFILE_FLAG_OVERLAPPED�v���w�肵���ꍇ�́uReadFile()�v�uWriteFile()�v�����s���鎞�ɁuOVERLAPPED�v���w�肷�邱�Ƃ��K�{�B���Ȃ��ƃo�O��B
    , NULL
    );
  if (hComPort == INVALID_HANDLE_VALUE)
  {
    OutputDebugString(_T("�|�[�g���J���܂���"));
    printf("�|�[�g���J���܂���\n");
    return -1;
  }

  /*-------------------------------------
    ����M�o�b�t�@�w��
    -------------------------------------*/
  SetupComm(hComPort, // �ʐM�f�o�C�X�̃n���h��:CreateFile()�Ŏ擾�����n���h�����w��
    1024,             // ��M�o�b�t�@�̃T�C�Y(�o�C�g�P��)
    1024              // ���M�o�b�t�@�̃T�C�Y(�o�C�g�P��)
    );
  // �w�肵���ʐM�����̏o�́A���̓o�b�t�@�̏�����
  PurgeComm(hComPort, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);

  /*-------------------------------------
    �ʐM�̐ݒ�
    -------------------------------------*/
  DCB dcb; // �V���A���|�[�g�̍\����񂪓���\����
  GetCommState(hComPort, &dcb); // ���݂̐ݒ��ǂݍ���

  dcb.BaudRate = 9600;   // ���x
  dcb.ByteSize = 8;      // �f�[�^��
  dcb.Parity = NOPARITY; // �p���e�B
  dcb.StopBits = ONESTOPBIT; // �X�g�b�v�r�b�g��
  dcb.fOutxCtsFlow = FALSE;  // ���M��CTS�t���[
  dcb.fRtsControl = RTS_CONTROL_DISABLE; // RTS�t���[�Ȃ�
  dcb.EvtChar = NULL;    // '\0'�Ɠ����Ӗ��B�����Ŏw�肵�����������M�����ꍇ�AEV_RXFLAG�C�x���g������

  SetCommState(hComPort, &dcb); // �ύX�����ݒ����������

  /*-------------------------------------
    �V���A���|�[�g�̃^�C���A�E�g��ԑ���
  -------------------------------------*/
  //COMMTIMEOUTS cto;
  //GetCommTimeouts( hComPort, &cto ); // �^�C���A�E�g�̐ݒ��Ԃ��擾
  //cto.ReadIntervalTimeout = 1000;
  //cto.ReadTotalTimeoutMultiplier = 0;
  //cto.ReadTotalTimeoutConstant = 1000;
  //cto.WriteTotalTimeoutMultiplier = 0;
  //cto.WriteTotalTimeoutConstant = 0;
  //SetCommTimeouts( hComPort, &cto ); // �^�C���A�E�g�̏�Ԃ�ݒ�

  /*-------------------------------------
    ��M����
  -------------------------------------*/
  // ��M�X���b�h�̍쐬
  hSCI_Receive_Thread = (HANDLE)_beginthreadex(
    NULL,                // _beginthreadex �̌Ăяo���K��� __stdcall �܂��� __clrcall
    0,                   // �X�^�b�N�T�C�Y
    SCI_Receive_Thread,  // �X���b�h�֐�
    NULL,                // �V�K�X���b�h�ɓn�����������X�g�܂���NULL
    0,                   // ���s���� 0�A�ꎞ��~���� CREATE_SUSPENDED(�X���b�h�����s����ɂ́AResumeThread)
    &dwThreadId          // �X���b�hID
    );
  if (hSCI_Receive_Thread == NULL)
  {
    OutputDebugString(_T("�X���b�h�̋N���Ɏ��s\n"));
    return -1;
  }
  Sleep(1000); // ��M�����������܂ő҂�

  /*-------------------------------------
    ���M����
  -------------------------------------*/
  std::string SCI_pBufferSend;   // ���M������e���i�[�����o�b�t�@
  SCI_pBufferSend = "2\0";
  //char * sendBuf = "2";
  DWORD SCI_LengthOfPutOrRecieved;
  while (1)
  {
    if (flag)
    {
      // �|�[�g�֑��M//�ulength()�v�͕����̏I����\��NULL�u\0�v�𕶎��̒����Ƃ��Ĉ���Ȃ��̂Łu+1�v���Ă���
      WriteFile(hComPort, SCI_pBufferSend.c_str(), SCI_pBufferSend.length() + 1, &SCI_LengthOfPutOrRecieved, &sendOverlapped);
      //WriteFile(hComPort, sendBuf, sizeof(sendBuf)+1, &SCI_LengthOfPutOrRecieved, &sendOverlapped);

      printf("SEND__ : %s\n\n", SCI_pBufferSend.c_str());// ���M�����������\��
      puts("-------------------------------\n");
      flag = false;
    }
    Sleep(100);
  }
  _getch();// Dos�����ꎞ��~����B

  /*-------------------------------------
    �I������
  -------------------------------------*/
  // �t���O��0�ɂ���while�����甲���o����悤�ɂ���B
  RecievedEndFlag = 0;
  // (�uWaitCommEvent();�v�́A�����Ă����Ă��C�x���g�̔�����҂�������̂ŁA)�����I�Ƀ��b�Z�[�W�𔭍s���āuWaitCommEvent();�v���I������B
  SetCommMask(hComPort, EV_ERR);
  // �X���b�h���I������܂őҋ@
  WaitForSingleObject(hSCI_Receive_Thread, INFINITE);
  // �X���b�h���I������
  CloseHandle(hSCI_Receive_Thread);

  // �o�b�t�@�̏���
  PurgeComm(hComPort, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
  // ���O�ŁuSetCommMask(hComPort,EV_ERR);�v�����s���Ă��邪�A�Z�b�g���Ă��邾���ŁuEV_ERR�v�C�x���g���������Ă����ł͂Ȃ��̂ł��̍s�͕K�v�B(�܂�������������Ƃ����ĉ��̃G���[��f����ł͂Ȃ��̂����B)
  // �V���A���|�[�g�����
  CloseHandle(hComPort);

  return 0;
}

/*-------- Receive thread --------*/
unsigned __stdcall SCI_Receive_Thread(void *)
{
  //����̒ʐM�f�o�C�X�ŊĎ������A�̃C�x���g���w�肵�܂��B//WaitCommEvent �֐��ɒʐM�����̃n���h����n���āA�C�x���g����������̂�ҋ@���܂��B
  SetCommMask(hComPort, EV_RXFLAG/*EV_RXCHAR*/ | EV_ERR);        //EV_RXCHAR:1������M���A���̓o�b�t�@�ɓ��ꂽ�Ƃ��B
  // EV_RXFLAG:DCB�\���̂Ŏw�肵���I���R�[�h����M������
  // EV_RXCHAR���w�肷��ƘA������������𑗐M�����ꍇ�A(�������x�̕�����M���x������������)��������ꕶ�����i�[���悤�Ƃ��邽�߁A�\������ƃo�O���Ă���

  RecievedEndFlag = 1; // �O���[�o���ϐ��ł���uRecievedEndFlag�v���u0(1�ȊO)�v�ɂȂ�܂ŏ����𑱂���B
  DWORD Event;

  while (RecievedEndFlag){
    // ��M�����f�[�^��ǂݍ���
    DWORD numberOfPut;
    //�uNULL�v��������M�����ꍇ
    //��M�f�[�^���𒲂ׂ�
    DWORD errors;
    COMSTAT comStat;

    // �w�肵���ʐM�f�o�C�X�ŃC�x���g����������̂�ҋ@���܂��B���̊֐��ŊĎ�����C�x���g�́A�f�o�C�X�̃n���h���Ɋ֘A�t�����Ă���C�x���g�}�X�N�ɂ���Ď�����܂��B
    // WaitCommEvent(hComPort,&Event,&recieveOverlapped);//FILE_FLAG_OVERLAPPED���w�肵���ꍇ��overlapped�\���̂��w�肷��K�v�����適���������CPU�g�p�����オ���Ă��܂��B
    WaitCommEvent(hComPort, &Event, NULL); // FILE_FLAG_OVERLAPPED���w�肵���ꍇ��overlapped�\���̂��w�肷��K�v������
    // MSDN�uhFile ���J���Ƃ��� FILE_FLAG_OVERLAPPED �t���O���Z�b�g���āAlpOverlapped �� NULL �ȊO�̃|�C���^��n���ƁAWaitCommEvent �֐��͏d������Ƃ��Ď��s����܂��B�v
    // �t�ɁA�uOVERLAPPED �\���̂ւ̃|�C���^�v��n���ƁA��񂾂����b�Z�[�W�̗L�����m�F����ƒ����Ɋ֐��𔲂��Ă��܂��B����̓X���b�h�ŏ������Ă���̂Ŏ��̏�����S�z����K�v�͖����B����đ�O�����́uNULL�v���w�肷��B

    if (Event == EV_RXFLAG/*EV_RXCHAR*/)
    {
      ClearCommError(hComPort, &errors, &comStat);
      int lengthOfRecieved = comStat.cbInQue; // ��M�������b�Z�[�W�����擾����
      //printf("�ǂݎ�����o�C�g�� : %d\n", lengthOfRecieved);

      // �o�b�t�@�����荞��
      ReadFile(hComPort, (LPVOID*)SCI_pBufferRecieved.c_str(), lengthOfRecieved, &numberOfPut, &recieveOverlapped);
      // ���̂����킩��Ȃ����Ǖ�������0�ƂȂ��Ă��܂��Ă����̂őΏ��Ö@�Ő��������Ă݂�B//�܂�A��������Ȃ���length()�Ő��������������A���ė��Ȃ��B
      SCI_pBufferRecieved._Mysize = lengthOfRecieved;
      printf("RECEIVE : %s, %d\n", SCI_pBufferRecieved.c_str(), SCI_pBufferRecieved.length());

      // ��M�C�x���g��EV_RXCHAR�Ȃ�E�C���h�E���b�Z�[�W���|�X�g����
      // ���b�Z�[�W��]����̃E�C���h�E�̃E�C���h�E�v���V�[�W���ɒ��ڑ��M���܂��B�����͓����ŁA�]����̃E�C���h�E�v���V�[�W�������b�Z�[�W���������I����܂ł͌Ăяo�����̏����̓u���b�N����܂��B
      // SendMessage( hWnd, WM_SCI_Receive,(WPARAM)recievedData,(LPARAM)numberOfPut);
      // ���b�Z�[�W��]����̃E�C���h�E�̃��b�Z�[�W�L���[�̖����ɑ��M���܂��B�����͔񓯊������ŁA���b�Z�[�W�𑗐M�������ォ�珈�����p�����邱�Ƃ��ł��܂��B
      // PostMessage(hWnd,WM_SCI_Receive,(WPARAM)recievedData,(LPARAM)numberOfPut);
      // InvalidateRect(hWnd, NULL, FALSE);// �ĕ`��𑣂�
      SCI_pBufferRecieved.clear();
      flag = true;
    }
    else if (Event == EV_ERR)
    {
      //�G���[�����������ꍇ
      // �A�C�h���A�T�X�y���h�ɂȂ�����ׂ̈̏����A������s��Ȃ��ƕ�����ʐM�s�\�ɂȂ�c�c�炵�����܂��m�F���Ă��Ȃ��B(http://yatsute.s22.xrea.com/contents/article//everything/pc/program/C_Builder,Windows/windows/Com%83%7C%81%5B%83g/%90V%8BK%C3%B7%BD%C4%95%B6%8F%91.txt)
      PurgeComm(hComPort, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);

      //puts("Receive error!!!\n");
    }
  }
  //ExitThread(0);
  return 0;
}