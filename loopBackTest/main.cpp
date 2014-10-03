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
int RecieveEndFlag = 0;

/*-------- Global parameters --------*/
std::string SCI_pBufferRecieved;   // ��M���������̃o�b�t�@
HANDLE hComPort;                   // �V���A���|�[�g�̃n���h��
HANDLE hSCI_Receive_Thread;        // ��M�X���b�h�̃O���[�o���ϐ�
OVERLAPPED sendOverlapped, recieveOverlapped;//�uReadFile()�v�uWriteFile()�v�����s���鎞�ɁuOVERLAPPED�v���w�肷�邱�Ƃ��K�{�Ȃ̂Œ�`����
unsigned int dwThreadId;           //��M�X���b�h��ID

int main()
{
  // ������
  ZeroMemory(&sendOverlapped, sizeof(OVERLAPPED));
  ZeroMemory(&recieveOverlapped, sizeof(OVERLAPPED));

  /*-------------------------------------
    �V���A���ʐM�̃t�@�C���f�B�X�N���v�^����
    -------------------------------------*/
  hComPort = CreateFile( _T("COM7")
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
    //return -1;
    exit(1);
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

  /*
  // �V���A���|�[�g�̃^�C���A�E�g��ԑ���
  COMMTIMEOUTS cto;
  GetCommTimeouts( hComPort, &cto ); // �^�C���A�E�g�̐ݒ��Ԃ��擾
  cto.ReadIntervalTimeout = 1000;
  cto.ReadTotalTimeoutMultiplier = 0;
  cto.ReadTotalTimeoutConstant = 1000;
  cto.WriteTotalTimeoutMultiplier = 0;
  cto.WriteTotalTimeoutConstant = 0;
  SetCommTimeouts( hComPort, &cto ); // �^�C���A�E�g�̏�Ԃ�ݒ�
  */


}