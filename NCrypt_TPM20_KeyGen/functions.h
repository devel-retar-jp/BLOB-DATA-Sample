#pragma once
//#include <Windows.h>
//#include <Bcrypt.h>
//#include <iostream>
//#include <vector>
//#include <iterator>
//#include <string>
//#include <fstream>
//#include <sstream>
//#include <iomanip>
//#include <random>
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define DirSep  L"\\"															//ディレクトリの区切り文字列
#define	NT_SUCCESS(Status)		(((NTSTATUS)(Status)) >= 0)
#define	STATUS_UNSUCCESSFULL	((NTSTATUS)0xC0000001L)
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// ファイル・パス取得
std::wstring makeFilePath(std::wstring* FileName);
/// 出力関数
void binStdOut(LPBYTE outstring, unsigned long outlength, std::string banner);
/// ファイル読み込み
std::vector<BYTE> readFileData(LPWSTR pcurrentDirString);
/// ファイル書き出し
void writeFileData(LPBYTE outstring, unsigned long outlength, std::wstring* FileName, int type);
/// ステータスチェック
void statusCheck(NTSTATUS	status, std::wstring Message);
/// 乱数生成
int randnumber(int min, int max);
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
