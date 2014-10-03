#include <stdio.h>
#include <Windows.h>
#include <string>
#include <tchar.h>
#include <conio.h>
#include <process.h>

/*-------- prototype --------*/
// シリアル通信
int SCI(HWND hWnd);
// 受信スレッドのプロトタイプ宣言
unsigned __stdcall SCI_Receive_Thread(void *);
// 処理をするかしないか
// 0 : スレッドがループ処理に入らない
// 1 : スレッドがループ処理に入る
int RecieveEndFlag = 0;

/*-------- Global parameters --------*/
std::string SCI_pBufferRecieved;   // 受信した文字のバッファ
HANDLE hComPort;                   // シリアルポートのハンドル
HANDLE hSCI_Receive_Thread;        // 受信スレッドのグローバル変数
OVERLAPPED sendOverlapped, recieveOverlapped;//「ReadFile()」「WriteFile()」を実行する時に「OVERLAPPED」を指定することが必須なので定義する
unsigned int dwThreadId;           //受信スレッドのID

int main()
{
  // 初期化
  ZeroMemory(&sendOverlapped, sizeof(OVERLAPPED));
  ZeroMemory(&recieveOverlapped, sizeof(OVERLAPPED));

  /*-------------------------------------
    シリアル通信のファイルディスクリプタ生成
    -------------------------------------*/
  hComPort = CreateFile( _T("COM7")
    , GENERIC_READ | GENERIC_WRITE
    , 0                      // オブジェクトを共有方法しない
    , NULL                   // セキュリティ属性デフォルト//ハンドルを子プロセスへ継承することを許可するかどうか//NULLは継承できない
    , OPEN_EXISTING          // ファイルを開きます。指定したファイルが存在していない場合、この関数は失敗します。
    , FILE_FLAG_OVERLAPPED   // 0//FILE_FLAG_OVERLAPPEDは、非同期であることを意味する。//同期とは、この関数を実行すると処理が完了するまで待たされる。非同期とはCPUの空き時間を使って関数が処理される
                             //良く分からないのだけれど、FILE_FLAG_OVERLAPPEDを設定すると送信が上手く行われていない気がする……。
                             //⇒「FILE_FLAG_OVERLAPPED」を指定した場合は「ReadFile()」「WriteFile()」を実行する時に「OVERLAPPED」を指定することが必須。しないとバグる。
    , NULL
    );
  if (hComPort == INVALID_HANDLE_VALUE)
  {
    OutputDebugString(_T("ポートが開きません"));
    //return -1;
    exit(1);
  }

  /*-------------------------------------
    送受信バッファ指定
    -------------------------------------*/
  SetupComm(hComPort, // 通信デバイスのハンドル:CreateFile()で取得したハンドルを指定
    1024,             // 受信バッファのサイズ(バイト単位)
    1024              // 送信バッファのサイズ(バイト単位)
    );
  // 指定した通信資源の出力、入力バッファの初期化
  PurgeComm(hComPort, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);

  /*-------------------------------------
    通信の設定
    -------------------------------------*/
  DCB dcb; // シリアルポートの構成情報が入る構造体
  GetCommState(hComPort, &dcb); // 現在の設定を読み込み

  dcb.BaudRate = 9600;   // 速度
  dcb.ByteSize = 8;      // データ長
  dcb.Parity = NOPARITY; // パリティ
  dcb.StopBits = ONESTOPBIT; // ストップビット長
  dcb.fOutxCtsFlow = FALSE;  // 送信時CTSフロー
  dcb.fRtsControl = RTS_CONTROL_DISABLE; // RTSフローなし
  dcb.EvtChar = NULL;    // '\0'と同じ意味。ここで指定した文字列を受信した場合、EV_RXFLAGイベントが発生

  SetCommState(hComPort, &dcb); // 変更した設定を書き込み

  /*
  // シリアルポートのタイムアウト状態操作
  COMMTIMEOUTS cto;
  GetCommTimeouts( hComPort, &cto ); // タイムアウトの設定状態を取得
  cto.ReadIntervalTimeout = 1000;
  cto.ReadTotalTimeoutMultiplier = 0;
  cto.ReadTotalTimeoutConstant = 1000;
  cto.WriteTotalTimeoutMultiplier = 0;
  cto.WriteTotalTimeoutConstant = 0;
  SetCommTimeouts( hComPort, &cto ); // タイムアウトの状態を設定
  */


}