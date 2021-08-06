/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
/// 対象鍵で暗号化
/// 
/// Summary:
/// 暗号化には①対象鍵と②非対称鍵の方式があります。
/// 今回は対象鍵のアルゴリズムAESを使います
/// WIN32 APIはCNG APIを使いましょう。
/// 
/// 関数の仕様として、必要なバイト数を確認した後に
/// メモリ領域を割り当ててから関数を実行しています。
/// 
/// 重要
/// 復号はブロックサイズです。
/// ENDにNULLを入れてくる仕様です。
/// つまり、バイナリファイルはサイズが狂うことがあるので要注意。
/// 暗号化する前にBASE64などにしましょう。
/// 
/// 2021/08/05      Retar.jp
/// 
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
#pragma comment(lib,"Bcrypt.lib")
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
int main()
{
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//コンソールの扱いをUTF-8に変更
	SetConsoleOutputCP(CP_UTF8);
	setvbuf(stdout, nullptr, _IOFBF, 4096);
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//bin拡張子はバイナリ、そのまま読み込み使えます。
	//txt拡張子はテキストファイル・入力も出力も好きなフォーマットでOK
	std::wstring InFileName = L"InText.txt";									//テスト入力ファイル名・UNICODE指定	
	std::wstring rgbIVEncData = L"rgbIVEnc.bin";								//IV・暗号化・暗号鍵と復号鍵で共通化させています・NULLでも構いません・この文字列を追加して暗号化しています
	std::wstring rgbIVDecData = L"rgbIVDec.bin";								//IV・復号化・暗号鍵と復号鍵で共通化させています・NULLでも構いません・この文字列を追加して暗号化しています
	std::wstring commonKeyData = L"CommonKey.bin";								//対象鍵・乱数で設定
	std::wstring cryptProtectData = L"CryptProtectData.bin";					//暗号化バイナリ出力ファイル名・UNICODE指定	
	std::wstring decryptProtectData = L"DeCryptProtectData.txt";				//復号ファイル出力ファイル名・UNICODE指定	
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//ディレクトリ設定
	auto InFilePath = makeFilePath(&InFileName);
	auto commonKeyDataPath = makeFilePath(&commonKeyData);
	auto cryptProtectPath = makeFilePath(&cryptProtectData);
	auto decryptProtectDataPath = makeFilePath(&decryptProtectData);
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//ファイル読み込み
	std::vector<BYTE>  ifsInDataBYTE = readFileData(&InFilePath[0]);
	binStdOut((LPBYTE)&ifsInDataBYTE[0], (unsigned long)ifsInDataBYTE.size(), "In File : ");
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	BCRYPT_ALG_HANDLE	hAlg = NULL;											//ハンドラ
	NTSTATUS	status = STATUS_UNSUCCESSFULL;									//関数のレスポンス・ステータス
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//アルゴリズムプロバイダオープン
	//BCryptOpenAlgorithmProviderから、何を使うか宣言します。
	//対象鍵なのでAESを宣言します。
	statusCheck(
		BCryptOpenAlgorithmProvider(
			&hAlg																//ハンドラのポインタ
			, BCRYPT_AES_ALGORITHM												//アルゴリズム
			, MS_PRIMITIVE_PROVIDER												//プロバイダ　MS_PRIMITIVE_PROVIDER or 0
			, 0																	//dwFlags・HMACとかの指定
		)
		, L"BCryptOpenAlgorithmProvider Open"
	);
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//テンポラリ
	ULONG	cbData;																//フラグ・サイズ用のバッファー・テンポラリとして使っています
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//ブロックサイズを取得
	//AESのブロックサイズは決まっているので定義を取得します
	DWORD	cbBlockLen;
	statusCheck(
		BCryptGetProperty(
			hAlg																//ハンドラのポインタ
			, BCRYPT_BLOCK_LENGTH												//受け取りデータのバイト数
			, (PBYTE)&cbBlockLen												//ブロックの長さ
			, sizeof(DWORD)														//第３引数に用意した領域のサイズ
			, &cbData															//第３引数にコピーされたデータのバイト数
			, 0																	//フラグ・0
		)
		, L"BCryptGetProperty : BCRYPT_BLOCK_LENGTH"
	);
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//初期化ベクタ生成
	//同じ鍵を使っても違った暗号化データが生成されるようにする
	//この文字列を追加して暗号化しています
	//暗号化、復号化で共通化させないと上手くいきません
	std::vector<BYTE> rgbIVEnc;
	std::vector<BYTE> rgbIVDec;
	for (auto i = 0; i < (int)cbBlockLen; i++)									//キーサイズ分のデータ
	{
		int temp = randnumber(0x00, 0xff);
		rgbIVEnc.push_back((BYTE)temp);
		rgbIVDec.push_back((BYTE)temp);
	}
	writeFileData((LPBYTE)&rgbIVEnc[0], (unsigned long)rgbIVEnc.size(), &rgbIVEncData, 16);
	writeFileData((LPBYTE)&rgbIVDec[0], (unsigned long)rgbIVDec.size(), &rgbIVDecData, 16);
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//暗号化オプション
	//前ブロックのデータを含ませながらの非対称ブロック暗号
	//設定しなくてもDefalt Modeです。
	statusCheck(
		BCryptSetProperty(
			hAlg																//ハンドラのポインタ
			, BCRYPT_CHAINING_MODE												//L"ChainingMode"モードの指定
			, (PBYTE)BCRYPT_CHAIN_MODE_CBC										//L"ChainingModeCBC"サイズ
			, sizeof(BCRYPT_CHAIN_MODE_CBC)										//第３引数のバイトサイズ
			, 0																	//フラグ・0
		)
		, L"BCryptSetProperty"
	);
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// キーオブジェクトを格納する領域のサイズを取得
	DWORD	cbKeyObj;															//サイズ
	statusCheck(
		BCryptGetProperty(
			hAlg																//ハンドラのポインタ
			, BCRYPT_OBJECT_LENGTH												//プロバイダオブジェクトのサイズ
			, (PBYTE)&cbKeyObj													//秘密鍵・初期化ベクタ
			, sizeof(DWORD)														//秘密鍵のデータサイズ
			, &cbData															//秘密鍵のバイト数
			, 0																	//0固定
		)
		, L"BCryptGetProperty : BCRYPT_OBJECT_LENGTH"
	);
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 暗号化用のキーオブジェクトを生成。この状態ではメモリ上のオブジェクト
	std::vector<BYTE> pbKeyObj(cbKeyObj, 0);
	std::vector<BYTE> secret;
	for (auto i = 0; i < (int)cbBlockLen; i++)									//乱数でカギを生成しています
	{
		int temp = randnumber(0x00, 0xff);
		secret.push_back(temp);
	}
	BCRYPT_KEY_HANDLE	hKey = NULL;
	statusCheck(
		BCryptGenerateSymmetricKey(
			hAlg																//ハンドラのポインタ
			, &hKey																//キー
			, &pbKeyObj[0]														//領域
			, cbKeyObj															//キーオブジェクトを格納する領域
			, &secret[0]														//秘密の鍵
			, sizeof(secret)													//秘密の鍵サイズ
			, 0																	//フラグ・0
		)
		, L"BCryptGenerateSymmetricKey"
	);
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//鍵のBLOB生成・サイズ測定
	//カギをファイルに保存できるようにしています。
	DWORD	cbBlob;
	statusCheck(
		BCryptExportKey(
			hKey																//ハンドラのポインタ
			, NULL																//NULLを指定
			, BCRYPT_OPAQUE_KEY_BLOB											//出力フォーマット
			, NULL																//鍵情報を格納するアドレス
			, 0																	//鍵情報のための領域のバイトサイズ
			, &cbBlob															//出力データバイト数
			, 0																	//フラグ・0
		)
		, L"BCryptExportKey Size Get"
	);
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//鍵のBLOB生成・ファイルへの書き出し
	std::vector<BYTE> pbBlob(cbBlob, NULL);										//バイト長さ、初期化データ
	statusCheck(
		BCryptExportKey(
			hKey																//ハンドラのポインタ
			, NULL																//NULLを指定
			, BCRYPT_OPAQUE_KEY_BLOB											//出力フォーマット
			, &pbBlob[0]														//出力キー
			, cbBlob															//出力キーサイズ
			, &cbData
			, 0
		)
		, L"BCryptExportKey Key BLOB"
	);
	writeFileData((LPBYTE)&pbBlob[0], (unsigned long)pbBlob.size(), &commonKeyData, 10);
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//キーを解放
	statusCheck(
		BCryptDestroyKey(hKey)
		, L"BCryptDestroyKey Close"
	);
	//キーオブジェクトの解放
	pbKeyObj.resize(cbKeyObj, NULL);
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///暗号化
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 鍵情報をBLOBからオブジェクトにインポート
	statusCheck(
		BCryptImportKey(
			hAlg
			, NULL
			, BCRYPT_OPAQUE_KEY_BLOB
			, &hKey																//再取得するキーハンドラ
			, &pbKeyObj[0]														//キーオブジェクト
			, cbKeyObj															//キーオブジェクトサイズ
			, &pbBlob[0]														//キーデータ
			, cbBlob															//キーデータサイズ
			, 0
		)
		, L"BCryptImportKey"
	);
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//暗号化データ・出力サイズ測定
	DWORD	cbCipherText;
	auto x = sizeof(ifsInDataBYTE);
	auto y = ifsInDataBYTE.size();
	statusCheck(
		BCryptEncrypt(
			hKey																//ハンドラのポインタ
			, &ifsInDataBYTE[0]													//暗号化対象のテキスト
			, (ULONG)ifsInDataBYTE.size()										//暗号化対象のテキストサイズ
			, NULL																//NULL指定
			, &rgbIVEnc[0]														//IV
			, cbBlockLen														//IVサイズ	
			, NULL																
			, 0																	//出力データポインタ
			, &cbCipherText														//出力データサイズ
			, BCRYPT_BLOCK_PADDING												//フラグ
		)
		, L"BCryptEncrypt Size Get"
	);
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 暗号化データの取得
	std::vector<BYTE> pbCipherText(cbCipherText, NULL);							//バイト長さ、初期化データ
	statusCheck(
		BCryptEncrypt(
			hKey
			, &ifsInDataBYTE[0]													//暗号化対象のテキスト
			, (ULONG)ifsInDataBYTE.size()										//暗号化対象のテキストサイズ
			, NULL
			, &rgbIVEnc[0]														//IV
			, cbBlockLen														//IVサイズ
			, &pbCipherText[0]													//暗号出力データ
			, cbCipherText														//暗号出力データサイズ
			, &cbData
			, BCRYPT_BLOCK_PADDING
		)
		, L"BCryptEncrypt BLOB"
	);
	writeFileData((LPBYTE)&pbCipherText[0], (unsigned long)pbCipherText.size(), &cryptProtectData, 0);
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//キーを解放
	statusCheck(
		BCryptDestroyKey(hKey)
		, L"BCryptDestroyKey Close"
	);
	//キーオブジェクトの解放
	pbKeyObj.resize(cbKeyObj, NULL);
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///復号化
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 鍵情報をオブジェクトにインポート
	statusCheck(
		BCryptImportKey(
			hAlg
			, NULL
			, BCRYPT_OPAQUE_KEY_BLOB
			, &hKey																//再取得するキーハンドラ
			, &pbKeyObj[0]														//キーオブジェクト
			, cbKeyObj															//キーオブジェクトサイズ
			, &pbBlob[0]														//キーデータ
			, cbBlob															//キーデータサイズ
			, 0			
		)
		, L"BCryptImportKey"
	);
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//復号・バイト数取得
	DWORD	cbPlainText;
	statusCheck(
		BCryptDecrypt(
			hKey
			, &pbCipherText[0]													//暗号化データ
			, cbCipherText														//暗号化データサイズ
			, NULL
			, &rgbIVDec[0]														//IV
			, cbBlockLen														//IVサイズ
			, NULL
			, 0
			, &cbPlainText														//復号データサイズ
			, BCRYPT_BLOCK_PADDING
		)
		, L"BCryptImportKey"
	);
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 暗号化データの取得
	std::vector<BYTE> pbPlainText(cbPlainText, NULL);							//復号データ初期化
	statusCheck(
		BCryptDecrypt(
			hKey
			, &pbCipherText[0]													//暗号化データ
			, cbCipherText														//暗号化データサイズ
			, NULL
			, &rgbIVDec[0]														//IV
			, cbBlockLen														//IVサイズ
			, &pbPlainText[0]													//復号データ
			, cbPlainText														//復号データサイズ
			, &cbData															
			, BCRYPT_BLOCK_PADDING
		)
		, L"BCryptImportKey"
	);
	//NULL文字を削除・ブロック単位で復号されるので、最後にNULL文字が追加されます。バイナリファイルは要注意！！！！
	auto e = std::remove(pbPlainText.begin(), pbPlainText.end(), NULL);
	pbPlainText.erase(e, pbPlainText.end());
	writeFileData((LPBYTE)&pbPlainText[0], (unsigned long)pbPlainText.size(), &decryptProtectData, 0);
	binStdOut((LPBYTE)&pbPlainText[0], (unsigned long)pbPlainText.size(),"BCryptDecrypt : ");
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//キーを解放
	statusCheck(
		BCryptDestroyKey(hKey)
		, L"BCryptDestroyKey Close"
	);
	//キーオブジェクトの解放
	pbKeyObj.resize(cbKeyObj, NULL);
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//使い終わったアルゴリズムプロバイダを閉じる。
	statusCheck(
		BCryptCloseAlgorithmProvider(hAlg, 0)
		, L"BCryptCloseAlgorithmProvider Close"
	);
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

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