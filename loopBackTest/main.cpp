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
int RecievedEndFlag = 0;
// 受け取ったかどうかのフラグ
bool flag = false;

/*-------- Global parameters --------*/
std::string SCI_pBufferRecieved;   // 受信した文字のバッファ
HANDLE hComPort;                   // シリアルポートのハンドル
HANDLE hSCI_Receive_Thread;        // 受信スレッドのグローバル変数
OVERLAPPED sendOverlapped, recieveOverlapped;//「ReadFile()」「WriteFile()」を実行する時に「OVERLAPPED」を指定することが必須なので定義する
unsigned int dwThreadId;           //受信スレッドのID

/*-------- Main function --------*/
int main()
{
  // 初期化
  ZeroMemory(&sendOverlapped, sizeof(OVERLAPPED));
  ZeroMemory(&recieveOverlapped, sizeof(OVERLAPPED));

  /*-------------------------------------
    シリアル通信のファイルディスクリプタ生成
    -------------------------------------*/
  hComPort = CreateFile( _T("COM3")
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
    printf("ポートが開きません\n");
    return -1;
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

  /*-------------------------------------
    シリアルポートのタイムアウト状態操作
  -------------------------------------*/
  //COMMTIMEOUTS cto;
  //GetCommTimeouts( hComPort, &cto ); // タイムアウトの設定状態を取得
  //cto.ReadIntervalTimeout = 1000;
  //cto.ReadTotalTimeoutMultiplier = 0;
  //cto.ReadTotalTimeoutConstant = 1000;
  //cto.WriteTotalTimeoutMultiplier = 0;
  //cto.WriteTotalTimeoutConstant = 0;
  //SetCommTimeouts( hComPort, &cto ); // タイムアウトの状態を設定

  /*-------------------------------------
    受信部分
  -------------------------------------*/
  // 受信スレッドの作成
  hSCI_Receive_Thread = (HANDLE)_beginthreadex(
    NULL,                // _beginthreadex の呼び出し規約は __stdcall または __clrcall
    0,                   // スタックサイズ
    SCI_Receive_Thread,  // スレッド関数
    NULL,                // 新規スレッドに渡される引数リストまたはNULL
    0,                   // 実行中は 0、一時停止中は CREATE_SUSPENDED(スレッドを実行するには、ResumeThread)
    &dwThreadId          // スレッドID
    );
  if (hSCI_Receive_Thread == NULL)
  {
    OutputDebugString(_T("スレッドの起動に失敗\n"));
    return -1;
  }
  Sleep(1000); // 受信処理が整うまで待つ

  /*-------------------------------------
    送信部分
  -------------------------------------*/
  std::string SCI_pBufferSend;   // 送信する内容を格納したバッファ
  SCI_pBufferSend = "2\0";
  //char * sendBuf = "2";
  DWORD SCI_LengthOfPutOrRecieved;
  while (1)
  {
    if (flag)
    {
      // ポートへ送信//「length()」は文字の終わりを表すNULL「\0」を文字の長さとして扱わないので「+1」しておく
      WriteFile(hComPort, SCI_pBufferSend.c_str(), SCI_pBufferSend.length() + 1, &SCI_LengthOfPutOrRecieved, &sendOverlapped);
      //WriteFile(hComPort, sendBuf, sizeof(sendBuf)+1, &SCI_LengthOfPutOrRecieved, &sendOverlapped);

      printf("SEND__ : %s\n\n", SCI_pBufferSend.c_str());// 送信した文字列を表示
      puts("-------------------------------\n");
      flag = false;
    }
    Sleep(100);
  }
  _getch();// Dos窓を一時停止する。

  /*-------------------------------------
    終了処理
  -------------------------------------*/
  // フラグを0にしてwhile文から抜け出せるようにする。
  RecievedEndFlag = 0;
  // (「WaitCommEvent();」は、放っておいてもイベントの発生を待ち続けるので、)強制的にメッセージを発行して「WaitCommEvent();」を終了する。
  SetCommMask(hComPort, EV_ERR);
  // スレッドが終了するまで待機
  WaitForSingleObject(hSCI_Receive_Thread, INFINITE);
  // スレッドを終了する
  CloseHandle(hSCI_Receive_Thread);

  // バッファの消去
  PurgeComm(hComPort, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
  // 直前で「SetCommMask(hComPort,EV_ERR);」を実行しているが、セットしているだけで「EV_ERR」イベントが発生している訳ではないのでこの行は必要。(まあ無かったからといって何のエラーを吐く訳ではないのだが。)
  // シリアルポートを閉じる
  CloseHandle(hComPort);

  return 0;
}

/*-------- Receive thread --------*/
unsigned __stdcall SCI_Receive_Thread(void *)
{
  //特定の通信デバイスで監視する一連のイベントを指定します。//WaitCommEvent 関数に通信資源のハンドルを渡して、イベントが発生するのを待機します。
  SetCommMask(hComPort, EV_RXFLAG/*EV_RXCHAR*/ | EV_ERR);        //EV_RXCHAR:1文字受信し、入力バッファに入れたとき。
  // EV_RXFLAG:DCB構造体で指定した終了コードを受信した時
  // EV_RXCHARを指定すると連続した文字列を送信した場合、(処理速度の方が受信速度よりも速いため)文字列を一文字ずつ格納しようとするため、表示するとバグっている

  RecievedEndFlag = 1; // グローバル変数である「RecievedEndFlag」が「0(1以外)」になるまで処理を続ける。
  DWORD Event;

  while (RecievedEndFlag){
    // 受信したデータを読み込む
    DWORD numberOfPut;
    //「NULL」文字を受信した場合
    //受信データ数を調べる
    DWORD errors;
    COMSTAT comStat;

    // 指定した通信デバイスでイベントが発生するのを待機します。この関数で監視するイベントは、デバイスのハンドルに関連付けられているイベントマスクによって示されます。
    // WaitCommEvent(hComPort,&Event,&recieveOverlapped);//FILE_FLAG_OVERLAPPEDを指定した場合はoverlapped構造体を指定する必要がある←これをやるとCPU使用率が上がってしまう。
    WaitCommEvent(hComPort, &Event, NULL); // FILE_FLAG_OVERLAPPEDを指定した場合はoverlapped構造体を指定する必要がある
    // MSDN「hFile を開くときに FILE_FLAG_OVERLAPPED フラグをセットして、lpOverlapped に NULL 以外のポインタを渡すと、WaitCommEvent 関数は重複操作として実行されます。」
    // 逆に、「OVERLAPPED 構造体へのポインタ」を渡すと、一回だけメッセージの有無を確認すると直ぐに関数を抜けてしまう。今回はスレッドで処理しているので次の処理を心配する必要は無い。よって第三引数は「NULL」を指定する。

    if (Event == EV_RXFLAG/*EV_RXCHAR*/)
    {
      ClearCommError(hComPort, &errors, &comStat);
      int lengthOfRecieved = comStat.cbInQue; // 受信したメッセージ長を取得する
      //printf("読み取ったバイト数 : %d\n", lengthOfRecieved);

      // バッファから取り込み
      ReadFile(hComPort, (LPVOID*)SCI_pBufferRecieved.c_str(), lengthOfRecieved, &numberOfPut, &recieveOverlapped);
      // 何故だかわからないけど文字数が0となってしまっていたので対処療法で数を代入してみる。//つまり、これをしないとlength()で正しい文字数が帰って来ない。
      SCI_pBufferRecieved._Mysize = lengthOfRecieved;
      printf("RECEIVE : %s, %d\n", SCI_pBufferRecieved.c_str(), SCI_pBufferRecieved.length());

      // 受信イベントがEV_RXCHARならウインドウメッセージをポストする
      // メッセージを転送先のウインドウのウインドウプロシージャに直接送信します。処理は同期で、転送先のウインドウプロシージャがメッセージを処理し終えるまでは呼び出し元の処理はブロックされます。
      // SendMessage( hWnd, WM_SCI_Receive,(WPARAM)recievedData,(LPARAM)numberOfPut);
      // メッセージを転送先のウインドウのメッセージキューの末尾に送信します。処理は非同期処理で、メッセージを送信した直後から処理を継続することができます。
      // PostMessage(hWnd,WM_SCI_Receive,(WPARAM)recievedData,(LPARAM)numberOfPut);
      // InvalidateRect(hWnd, NULL, FALSE);// 再描画を促す
      SCI_pBufferRecieved.clear();
      flag = true;
    }
    else if (Event == EV_ERR)
    {
      //エラーが発生した場合
      // アイドル、サスペンドになった後の為の処理、これを行わないと復旧後通信不能になる……らしいがまだ確認していない。(http://yatsute.s22.xrea.com/contents/article//everything/pc/program/C_Builder,Windows/windows/Com%83%7C%81%5B%83g/%90V%8BK%C3%B7%BD%C4%95%B6%8F%91.txt)
      PurgeComm(hComPort, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);

      //puts("Receive error!!!\n");
    }
  }
  //ExitThread(0);
  return 0;
}