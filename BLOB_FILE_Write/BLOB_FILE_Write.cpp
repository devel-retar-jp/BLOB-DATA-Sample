/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
/// バイナリデータのオープン・クローズ②
///
/// Summary:
/// バイナリファイルの書き込み
/// 
/// 昔式の書き方をすると冗長になり、読み辛いです。
/// C++11/17の書き方をしましょう！
/// 
/// 
/// 2021/08/03      Retar.jp
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
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void binStdOut(LPBYTE outstring, unsigned long outlength, std::string banner);	//出力
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main()
{
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	std::wstring InFileName = L"InText.txt";									//テスト入力ファイル名・UNICODE指定		->	適時書き換えを！
	//std::wstring InFileName = L"cryptbin.dat";									//テスト入力ファイル名・UNICODE指定		->	適時書き換えを！
	std::wstring OutFileNameWin32 = L"SaveWin32.txt";							//テスト出力ファイル名・UNICODE指定		->	適時書き換えを！
	std::wstring OutFileNameofstream = L"Saveofstream.txt";						//テスト出力ファイル名・UNICODE指定		->	適時書き換えを！
	std::wstring OutFileNamePofstream = L"SavePofstream.txt";					//テスト出力ファイル名・UNICODE指定		->	適時書き換えを！
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
	std::wstring currentDirStringSV = currentDirString;							//保存
	std::wcout << "Current Dir String: " << currentDirString << std::endl;

	currentDirString += DirSep;													//文字列連結
	currentDirString += InFileName;												//文字列連結
	//LPWSTR  lpwInFileName = (LPWSTR)DirSep.c_str();							//std::wstring -> LPWSTR
	//LPCWSTR lpcwInFileName = (LPCWSTR)lpwInFileName;							//LPWSTR -> LPCWSTR 
	LPWSTR pcurrentDirString = &currentDirString[0];							//std::wstring -> LPWSTR
	std::wcout << "Current Dir CONN  : " << pcurrentDirString << std::endl;
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// std::vector<BYTE>に読み込む・イキナリ読み込み・その2
	std::basic_ifstream<BYTE> file2(pcurrentDirString, std::ios::binary);		//ファイルオープン
	// basic_ifstreamのイテレータがあるので読んでやる
	auto ifsInDataBYTE = std::vector<BYTE>((std::istreambuf_iterator<BYTE>(file2)), std::istreambuf_iterator<BYTE>());
	//出力関数・std::vector<BYTE> -> LPBYTEに変換・関数呼び出しでは多用・関数によっては上手くいかない？ので、HeapAllocする。
	LPBYTE lpifsInDataBYTE = (LPBYTE)&ifsInDataBYTE[0];
	binStdOut(lpifsInDataBYTE, (unsigned long)ifsInDataBYTE.size(), "ORIGINAL IN FILE :");
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//入力データをビット演算する
	std::vector<BYTE> ifsInDataBYTEConv;
	for (unsigned long i = 0; i < (unsigned long)ifsInDataBYTE.size(); i++)
	{
		auto s = ifsInDataBYTE[i];
		//s = s >> 1;															//ビットシフト
		s = s + 32;																//大文字ー＞小文字サンプル
		ifsInDataBYTEConv.push_back(s);
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//CreateFile WIN32 API
	HANDLE hFile;
	currentDirString = currentDirStringSV;										//文字列連結
	currentDirString += DirSep;													//文字列連結
	currentDirString += OutFileNameWin32;										//文字列連結
	pcurrentDirString = &currentDirString[0];									//UNICODE文字列Dir
	//https://docs.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-writefile
	hFile = CreateFile(														
		(LPWSTR) pcurrentDirString
		, GENERIC_WRITE
		, FILE_SHARE_WRITE
		, NULL
		, CREATE_ALWAYS
		, FILE_ATTRIBUTE_NORMAL
		, NULL
	);
	//書き出し
	if (hFile != INVALID_HANDLE_VALUE) 
	{
		LPBYTE lpifsInDataBYTEConv = (LPBYTE)&ifsInDataBYTEConv[0];
		WriteFile(
			hFile
			, lpifsInDataBYTEConv
			, (DWORD)ifsInDataBYTEConv.size()
			, NULL
			, NULL
		);
		//出力関数・std::vector<BYTE> -> LPBYTEに変換・関数呼び出しでは多用・関数によっては上手くいかない？ので、HeapAllocする。
		binStdOut(lpifsInDataBYTEConv, (unsigned long)ifsInDataBYTEConv.size(), "CONVERT FILE :");
	}
	else
	{
		std::cout << "*** Error returned by CreateFile\n";
		std::wcout << pcurrentDirString << "\n";
		return 1;
	}
	//クローズ
	CloseHandle(hFile);															//昔のコンパイラならクローズ忘れはバグの元ですが、無くても問題なし
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 書き込み・その1
	currentDirString = currentDirStringSV;										//文字列連結
	currentDirString += DirSep;													//文字列連結
	currentDirString += OutFileNameofstream;									//文字列連結
	//書き出し・関数で丸っと書き出し
	std::ofstream ofs(currentDirString, std::ios::binary);
	ofs.write((const char*)&ifsInDataBYTEConv[0], ifsInDataBYTEConv.size());
	ofs.close();
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 書き込み・その2
	currentDirString = currentDirStringSV;										//文字列連結
	currentDirString += DirSep;													//文字列連結
	currentDirString += OutFileNamePofstream;									//文字列連結
	//書き出し・ストリームなので、マンマ書き込んでOK
	std::basic_ofstream<BYTE> ofsP(currentDirString, std::ios::binary);			//ファイルオープン
	for (unsigned long i = 0; i < ifsInDataBYTEConv.size(); i++)
	{
		ofsP << ifsInDataBYTEConv[i];
	}
	ofsP.close();																//昔のコンパイラならクローズ忘れはバグの元ですが、無くても問題なし
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
			//16進数
			std::cout << std::setfill('0') << std::setw(2) << std::hex << (unsigned long)s << " ";
			//10進数
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