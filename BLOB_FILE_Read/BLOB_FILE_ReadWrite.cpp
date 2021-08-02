/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
/// バイナリデータのオープン・クローズ①
///
/// Summary:
/// バイナリファイルの読み込み
/// 
/// 昔式の書き方をすると冗長になり、読み辛いです。
/// C++11/17の書き方をしましょう！
/// 
/// 
/// 2021/08/02      Retar.jp
/// 
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <Windows.h>
#include <iostream>
#include <vector>
#include <iterator>
#include <fstream>
#include <iomanip>
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define DirSep  L"\\"															//ディレクトリの区切り文字列
///
void binStdOut(LPBYTE outstring, unsigned long outlength, std::string banner);	//出力
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main()
{
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//std::wstring InFileName = L"InText.txt";									//テスト入力ファイル名・UNICODE指定		->	適時書き換えを！
	std::wstring InFileName = L"cryptbin.dat";									//テスト入力ファイル名・UNICODE指定		->	適時書き換えを！
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//UNICODE・カレント実行パスを取得
	//WindowsのUNICODEで文字列を加工する時は何でも「std::wstring」に持ち込む
	//必要に応じてLPWSTRやLPCWSTRにする。
	TCHAR dir[MAX_PATH] = { 0 };												//パスの最大値の配列を用意
	GetModuleFileName(NULL, dir, MAX_PATH);										//実行ファイル名を取得
	std::wcout << "Current Dir ROW   : " << dir << std::endl;
	//
	std::wstring::size_type pos = std::wstring(dir).find_last_of(DirSep);		//最後の"\"の位置をposに取得
	std::wstring currentDirString = std::wstring(dir).substr(0, pos);			//currentDirStringに文字を取得
	std::wcout << "Current Dir String: " << currentDirString << std::endl;

	currentDirString += DirSep;													//文字列連結
	currentDirString += InFileName;												//文字列連結
	//LPWSTR  lpwInFileName = (LPWSTR)DirSep.c_str();							//std::wstring -> LPWSTR
	//LPCWSTR lpcwInFileName = (LPCWSTR)lpwInFileName;							//LPWSTR -> LPCWSTR 
	LPWSTR pcurrentDirString = &currentDirString[0];							//std::wstring -> LPWSTR
	std::wcout << "Current Dir CONN  : " << pcurrentDirString << std::endl;
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//バイナリデータ読み込み・昔の書き方は冗長
	HANDLE          hFile, hFileBegin;											//ファイルハンドラ
	DWORD           InDataSize = 0;												//読み込みデータサイズ
	///	LPBYTE形式
	LPBYTE          InData;														//読み込みBuffer
	BOOL			readstat;													//読み込みステータス
	/////////
	//ファイル読み込み
	hFile = CreateFileW(
		(LPCWSTR)pcurrentDirString												//DIRをUNICODE指定する
		, GENERIC_READ
		, 0
		, NULL
		, OPEN_EXISTING
		, FILE_ATTRIBUTE_NORMAL
		, NULL
	);
	//読み込めなかったときの処理
	if (hFile == INVALID_HANDLE_VALUE)
	{
		std::cout << "*** Error returned by CreateFile\n";
		std::wcout << pcurrentDirString << "\n";
		return 1;
	}
	else
	{
		printf("CreateFileW : Open OK\n");
		hFileBegin = hFile;
	}
	/////////
	//読み込みバイト数
	InDataSize = GetFileSize(hFile, NULL);
	/////////
	//LPBYTE形式に読み込み・伝統的なWindows
	hFile = hFileBegin;															//ファイルポインタのアタマにする
	ZeroMemory(&InData, sizeof(InDataSize));
	InData = (LPBYTE)HeapAlloc(GetProcessHeap(), 0, InDataSize);				//領域を確保・とらないとエラーになる。
	readstat = ReadFile(														//確保した領域に読み込み
		hFile
		, InData																//std::vector<BYTE>のアドレスを入れても入らない可愛くない奴
		, InDataSize
		, NULL
		, NULL
	);
	CloseHandle(hFile);															//ハンドラクローズ
	/////////
	if (readstat)
	{
		//出力関数・std::vector<BYTE> -> LPBYTEに変換・関数呼び出しでは多用・関数によっては上手くいかない？ので、HeapAllocする。
		binStdOut(InData, (unsigned long)InDataSize, "LPBYTE :");
	}
	else
	{
		printf("LPBYTE ReadFile FAIL\n");
		return 1;
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//LPBYTE -> std::vector<BYTE> に変換・ポインタからベクタは芋臭い奴
	/// std::vector<BYTE>形式
	std::vector<BYTE> InDataBYTE;												//読み込みBuffer
	for (int i = 0; i < (int)InDataSize; i++)
	{
		InDataBYTE.push_back((BYTE)InData[i]);									//コンパイラで警告が出ます。今更使わないので仕方ありません。
	}
	//出力関数・std::vector<BYTE> -> LPBYTEに変換・関数呼び出しでは多用・関数によっては上手くいかない？ので、HeapAllocする。
	LPBYTE lpInDataBYTE = (LPBYTE)&InDataBYTE[0];
	binStdOut(lpInDataBYTE, (unsigned long)InDataBYTE.size(), "LPBYTE -> BYTE vector :");
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// std::vector<BYTE>に読み込む・イキナリ読み込み・その1
	std::ifstream file1(pcurrentDirString, std::ios::binary);					//ファイルオープン
	// ifstreamのイテレータがあるので読んでやる
	auto ifs1InDataBYTE = std::vector<BYTE>((std::istreambuf_iterator<char>(file1)), std::istreambuf_iterator<char>());
	//出力関数・std::vector<BYTE> -> LPBYTEに変換・関数呼び出しでは多用・関数によっては上手くいかない？ので、HeapAllocする。
	LPBYTE lpifs1InDataBYTE = (LPBYTE)&ifs1InDataBYTE[0];
	binStdOut(lpifs1InDataBYTE, (unsigned long)ifs1InDataBYTE.size(), "BYTE vector ifstream :");
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// std::vector<BYTE>に読み込む・イキナリ読み込み・その2
	std::basic_ifstream<BYTE> file2(pcurrentDirString, std::ios::binary);		//ファイルオープン
	// basic_ifstreamのイテレータがあるので読んでやる
	auto ifs2InDataBYTE = std::vector<BYTE>((std::istreambuf_iterator<BYTE>(file2)), std::istreambuf_iterator<BYTE>());
	//出力関数・std::vector<BYTE> -> LPBYTEに変換・関数呼び出しでは多用・関数によっては上手くいかない？ので、HeapAllocする。
	LPBYTE lpifs2InDataBYTE = (LPBYTE)&ifs2InDataBYTE[0];
	binStdOut(lpifs2InDataBYTE, (unsigned long)ifs2InDataBYTE.size(), "BYTE vector basic_ifstream :");
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// 出力関数
/// </summary>
/// <param name="outstring">LPBYTE型の文字列</param>
/// <param name="outlength">文字列の長さ</param>
/// <param name="banner">バナー</param>
void binStdOut(LPBYTE outstring, unsigned long outlength, std::string banner)
{
	//Banner 出力
	std::cout << "----------------------------------------------------------" << "\n";
	std::cout << banner << std::endl;

	//データ出力
	unsigned long datapoint = 0;
	unsigned long countdiv = 16;
	unsigned long i = 0;
	///
	std::cout << "         | 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F" << "\n";
	std::cout << "----------------------------------------------------------" << "\n";
	//for (unsigned long i = 0; i < countdiv; i++)
	while (1)
	{
		std::cout << std::setfill('0') << std::setw(8) << std::uppercase << std::hex << i * countdiv << " | ";
		for (unsigned long j = 0; j < countdiv; j++)
		{
			auto s = outstring[datapoint];
			//16
			std::cout << std::setfill('0') << std::setw(2) << std::hex << (unsigned long)s << " ";
			//std::cout << std::setfill('0') << std::setw(3) << std::uppercase << std::dec << (unsigned long)outstring[datapoint] << " ";
			datapoint++;
			if (outlength <= datapoint) { break; }
		}
		std::cout << "\n";
		if (outlength <= datapoint) { break; }
		i++;
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////