#include <Windows.h>
#include <Bcrypt.h>
#include <iostream>
#include <vector>
#include <iterator>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <random>
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "functions.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// 乱数生成
/// </summary>
/// <param name="min">最小</param>
/// <param name="max">最大</param>
int randnumber(int min, int max)
{
	std::random_device rd;									//乱数生成
	std::mt19937_64 mt(rd());								//メルセンヌ・ツイスター法によって生成される乱数列
	std::uniform_int_distribution<int> uid(min, max);		//最大最小の制限
	return uid(mt);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// ステータスチェック
/// </summary>
/// <param name="status">関数リターンステータス</param>
/// <param name="Message">メッセージ/param>
void statusCheck(NTSTATUS	status, std::wstring Message)
{
	if (NT_SUCCESS(status)) {
		std::wcout << Message << std::endl;
	}
	else {
		std::wcout << "*** Error 0x" << std::hex << status << " returned by " << Message << std::endl;
		throw std::exception();
	}
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
{	
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
			std::cout << std::setfill('0') << std::setw(2) << std::uppercase << std::hex << (unsigned long)s << " ";
			//std::cout << std::setfill('0') << std::setw(3) << std::uppercase << std::dec << (unsigned long)outstring[datapoint] << " ";
			datapoint++;
			if (outlength <= datapoint) { break; }
		}
		std::cout << "\n";
		if (outlength <= datapoint) { break; }
		i++;
	}
	std::cout << "----------------------------------------------------------" << "\n";
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
