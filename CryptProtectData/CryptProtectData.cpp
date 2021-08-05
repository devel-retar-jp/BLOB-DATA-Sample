/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
/// CryptProtectData / CryptUnprotectData
///
/// Summary:
/// Windows NTの時代からある端末単位で暗号化するAPI
/// 将来的には廃止される予定なので使用しない方がいいです。
/// 使えるうちに、確認のために暗号化/復号の方法を紹介します。
/// 
/// 2021/08/05      Retar.jp
/// 
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <Windows.h>
#include <iostream>
#include <vector>
#include <iterator>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define DirSep  L"\\"															//ディレクトリの区切り文字列
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// ファイル・パス取得
std::wstring makeFilePath(std::wstring* FileName);
/// 出力関数
void binStdOut(LPBYTE outstring, unsigned long outlength, std::string banner);	
/// ファイル読み込み
std::vector<BYTE> readFileData(LPWSTR pcurrentDirString);
/// ファイル書き出し
void writeFileData(LPBYTE outstring, unsigned long outlength, std::wstring* FileName, int type);
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma comment(lib, "crypt32.lib")
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//メイン
int main()
{
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//コンソールの扱いをUTF-8に変更
	SetConsoleOutputCP(CP_UTF8);
	setvbuf(stdout, nullptr, _IOFBF, 4096);
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	std::wstring InFileName = L"InText.txt";									//テスト入力ファイル名・UNICODE指定		->	適時書き換えを！
	//std::wstring InFileName = L"cryptbin.dat";								//テスト入力ファイル名・UNICODE指定		->	適時書き換えを！
	std::wstring cryptProtectData = L"CryptProtectData.bin";					//暗号化バイナリ出力ファイル名・UNICODE指定		->	適時書き換えを！
	std::wstring decryptProtectData = L"DeCryptProtectData.txt";				//復号ファイル出力ファイル名・UNICODE指定		->	適時書き換えを！
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//入力ファイルのフルパス取得
	auto InFilePath = makeFilePath(&InFileName);
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//ファイル読み込み
	std::vector<BYTE>  ifsInDataBYTE = readFileData(&InFilePath[0]);
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	DATA_BLOB DataIn;															//入力BLOB
	DATA_BLOB DataCrypt;														//暗号化BLOB
	DATA_BLOB DataDeCrypt;														//復号BLOB
	BYTE* pbDataInput = (BYTE*)&ifsInDataBYTE[0];								//読み込みデータ
	DWORD cbDataInput = (DWORD)ifsInDataBYTE.size() + 1;						//読み込みデータサイズ
	DataIn.pbData = pbDataInput;												//BLOBに指定・データポインタ
	DataIn.cbData = cbDataInput;												//BLOBに指定・データサイズ
	CRYPTPROTECT_PROMPTSTRUCT PromptStruct;										//プロンプト構造体・・・暗号強度などのオプション
	LPWSTR pDataInComment = NULL;												//復号したバイナリに含ませたコメント受け取り
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//プロンプト構造体
	ZeroMemory(&PromptStruct, sizeof(PromptStruct));							//初期化
	PromptStruct.cbSize = sizeof(PromptStruct);									//サイズ指定
	PromptStruct.dwPromptFlags = CRYPTPROTECT_PROMPT_REQUIRE_STRONG;			//暗号化強度
	//PromptStruct.szPrompt = L"This is a user prompt.";						//コメント
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//暗号化
	//https://docs.microsoft.com/en-us/windows/win32/api/dpapi/nf-dpapi-cryptprotectdata
	if (
		!CryptProtectData(
			&DataIn
			, L"Crypt Inner Comment..."											//コメント・通常は使わないので適当に
			, NULL																//エントロピー
			, NULL																//Reserved
			, &PromptStruct														//プロンプト構造体
			, CRYPTPROTECT_LOCAL_MACHINE										//ローカルマシンだけが復号できる指定
			, &DataCrypt														//暗号化BLOB
		))
	{
		std::cout << "*** Encryption error\n";
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//入力データのConsole出力
	binStdOut((LPBYTE)DataIn.pbData, (unsigned long)DataIn.cbData - 1, "IN FILE :");
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//暗号化データのConsole出力
	binStdOut((LPBYTE)DataCrypt.pbData, (unsigned long)DataCrypt.cbData - 1, "CryptProtectData :");
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//暗号化データのFile出力
	writeFileData((LPBYTE)DataCrypt.pbData, (unsigned long)DataCrypt.cbData - 1, &cryptProtectData, 0);
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//暗号化
	//https://docs.microsoft.com/en-us/windows/win32/api/dpapi/nf-dpapi-cryptunprotectdata
	if (
		!CryptUnprotectData(
			&DataCrypt,															//暗号化BLOB
			&pDataInComment,													//コメント受け取り
			NULL,																//エントロピー
			NULL,																//Reserved
			&PromptStruct,														//プロンプト構造体
			0,																	//オプション、0で良い
			&DataDeCrypt														//復号化BLOB
		))
	{
		std::cout << "*** Decryption error\n";
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//復号化データのConsole出力
	binStdOut((LPBYTE)DataDeCrypt.pbData, (unsigned long)DataDeCrypt.cbData - 1, "CryptUnprotectData :");
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//復号化データ・コメントのConsole出力
	std::wstring wDescrOut = L"";

	if (pDataInComment != NULL)
	{
		wDescrOut = pDataInComment;
	}
	else
	{
		wDescrOut = L"";
	}
	std::wcout << "Crypt Inner Comment : " << wDescrOut << std::endl;
	//std::string DescrOut = std::string(wDescrOut.begin(), wDescrOut.end());
	//std::cout << "Crypt Inner Comment : " << DescrOut << std::endl;
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//復号化データのFile出力
	writeFileData((LPBYTE)DataDeCrypt.pbData, (unsigned long)DataDeCrypt.cbData - 1, &decryptProtectData, 0);
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//解放・不要ですがやっておきましょう。
	LocalFree(pDataInComment);
	LocalFree(DataCrypt.pbData);
	LocalFree(DataDeCrypt.pbData);
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// ファイル書き出し
/// </summary>
/// <param name="outstring">書き出しデータポインタ</param>
/// <param name="outlength">書き出しデータ長</param>
/// <param name="FileName">ファイル名</param>
/// <param name="type">書き出しスタイル・（16/10/0）・16進数、10進数、0バイナリ</param>
void writeFileData(LPBYTE outstring, unsigned long outlength, std::wstring* FileName, int type)
{
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	TCHAR dir[MAX_PATH] = { 0 };												//パスの最大値の配列を用意
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	GetModuleFileName(NULL, dir, MAX_PATH);										//実行ファイル名を取得
	//std::wcout << "Current Dir ROW   : " << dir << std::endl;
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	std::wstring::size_type pos = std::wstring(dir).find_last_of(DirSep);		//最後の"\"の位置をposに取得
	std::wstring currentDirString = std::wstring(dir).substr(0, pos);			//currentDirStringに文字を取得
	std::wstring currentDirStringSV = currentDirString;							//保存
	//std::wcout << "Current Dir String: " << currentDirString << std::endl;
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	currentDirString = currentDirStringSV;										//文字列連結
	currentDirString += DirSep;													//文字列連結
	currentDirString += *FileName;												//文字列連結
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//書き出し・ストリームなので、マンマ書き込んでOK
	std::basic_ofstream<BYTE> ofsP(currentDirString, std::ios::binary);			//ファイルオープン
	for (unsigned long i = 0; i < outlength; i++)
	{
		std::stringstream decStr, hexStr;
		std::string s;

		switch (type)
		{
			//10進数書き出し
		case 10:
			decStr << std::setfill('0') << std::setw(3) << std::dec << (unsigned long)outstring[i] << " ";
			s = decStr.str();
			ofsP << (BYTE)s[0];
			ofsP << (BYTE)s[1];
			ofsP << (BYTE)s[2];
			if (i != outlength - 1)
			{
				if ((i + 1) % type == 0 && i != 0)
				{
					ofsP << "\n";
				}
				else
				{
					ofsP << (BYTE)s[3];
				}
			}
			break;
			//16進数書き出し
		case 16:
			hexStr << std::setfill('0') << std::setw(2) << std::uppercase << std::hex << (unsigned long)outstring[i] << " ";
			s = hexStr.str();
			ofsP << (BYTE)s[0];
			ofsP << (BYTE)s[1];
			if (i != outlength - 1)
			{
				if ((i + 1) % type == 0 && i != 0)
				{
					ofsP << "\n";
				}
				else
				{
					ofsP << (BYTE)s[2];
				}
			}
			break;
			//バイナリ書き出し
		default:																
			ofsP << outstring[i];
		}
	}
	ofsP.close();																//昔のコンパイラならクローズ忘れはバグの元ですが、無くても問題なし
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// ファイル読み込み・丸っと
/// </summary>
/// <param name="pcurrentDirString">フルパスファイル名</param>
/// <returns>読み込みデータ</returns>
std::vector<BYTE> readFileData(LPWSTR pcurrentDirString)
{
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// std::vector<BYTE>に読み込む・イキナリ読み込み・その2
	std::basic_ifstream<BYTE> file2(pcurrentDirString, std::ios::binary);		//ファイルオープン
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// basic_ifstreamのイテレータがあるので読んでやる
	return std::vector<BYTE>((std::istreambuf_iterator<BYTE>(file2)), std::istreambuf_iterator<BYTE>());
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// ファイル・パス取得
/// </summary>
/// <param name="FileName">ファイル名</param>
/// <returns>フルパスファイル名</returns>
std::wstring makeFilePath(std::wstring* FileName)
{
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	TCHAR dir[MAX_PATH] = { 0 };												//パスの最大値の配列を用意
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	GetModuleFileName(NULL, dir, MAX_PATH);										//実行ファイル名を取得
	//std::wcout << "Current Dir ROW   : " << dir << std::endl;
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	std::wstring::size_type pos = std::wstring(dir).find_last_of(DirSep);		//最後の"\"の位置をposに取得
	std::wstring currentDirString = std::wstring(dir).substr(0, pos);			//currentDirStringに文字を取得
	std::wstring currentDirStringSV = currentDirString;							//保存
	//std::wcout << "Current Dir String: " << currentDirString << std::endl;
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	currentDirString += DirSep;													//文字列連結
	currentDirString += *FileName;												//文字列連結
	//LPWSTR pcurrentDirString = &currentDirString[0];							//std::wstring -> LPWSTR
	//std::wcout << "Current Dir CONN  : " << pcurrentDirString << std::endl;
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	return currentDirString;
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
{	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//Banner 出力
	std::cout << "----------------------------------------------------------" << "\n";
	std::cout << banner << std::endl;
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//データ出力
	unsigned long datapoint = 0;
	unsigned long countdiv = 16;
	unsigned long i = 0;
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////