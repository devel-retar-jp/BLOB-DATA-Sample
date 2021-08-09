/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
/// CNG TPM2.0
/// 暗号化
/// 
/// Summary:
/// 暗号化には①対称鍵と②非対称鍵の方式があります。
/// CNG Key Storage Providersの半導体保存型がTPM2.0です。
/// 半導体内に保存されている秘密キーを呼び出します。
/// 秘密鍵をファイル保存できないため、
/// 移動プロファイルよりも更に強力なキー保存ができます。
/// 
/// 基本的にはKey Storage Providersと同じ手順です。
///  
/// 2021/08/09      Retar.jp
/// 
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <Windows.h>
#include <ncrypt.h>
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
#pragma comment (lib, "ncrypt.lib")
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
	std::wstring InFileName = L"InText.txt";											//テスト入力ファイル名UNICODE指定	
	std::wstring privateKeyData = L"privateKey.bin";									//Private鍵・乱数で設定
	std::wstring publicKeyData = L"publicKey.bin";										//Public鍵・乱数で設定
	std::wstring cryptProtectData = L"CryptProtectData.bin";							//暗号化バイナリ出力ファイル名・UNICODE指定	
	std::wstring decryptProtectData = L"DeCryptProtectData.txt";						//復号ファイル出力ファイル名・UNICODE指定	
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//ディレクトリ設定
	auto InFilePath = makeFilePath(&InFileName);
	auto privateKeyDataPath = makeFilePath(&privateKeyData);
	auto publicKeyDataPath = makeFilePath(&publicKeyData);
	auto cryptProtectPath = makeFilePath(&cryptProtectData);
	auto decryptProtectDataPath = makeFilePath(&decryptProtectData);
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//ファイル読み込み
	std::vector<BYTE>  ifsInDataBYTE = readFileData(&InFilePath[0]);					//ファイル読み込み
	//binStdOut((LPBYTE)&ifsInDataBYTE[0], (unsigned long)ifsInDataBYTE.size(), "In File : ");
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	NCRYPT_PROV_HANDLE hProvider = NULL;												//アルゴリズムハンドラ
	NTSTATUS status = STATUS_UNSUCCESSFULL;												//関数のレスポンス・ステータス
	NCRYPT_KEY_HANDLE  hKey = NULL;														//キーハンドラ
	WCHAR              szKeyName[] = L"TPM20SaveKey";									//保存名・名前が重要！！！！・特段のキーロックなどを設けなければ誰でもキーアクセスできます
	ULONG	cbData;																		//サイズ用のバッファー・テンポラリとして使っています
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	//アルゴリズムプロバイダオープン
	// 
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//アルゴリズムプロバイダオープン
	statusCheck(
		NCryptOpenStorageProvider(
			&hProvider																	//ハンドラのポインタ
			, MS_PLATFORM_CRYPTO_PROVIDER												//TPM2.0を呼び出し
			, 0																			//Flag
		)
		, L"NCryptOpenStorageProvider Open : MS_PLATFORM_CRYPTO_PROVIDER : TPM2.0"		//CNG TPM2.0を指定する
	);
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//Public Key読み込み
	std::vector<BYTE>  publicKeyDataBYTE = readFileData(&publicKeyDataPath[0]);		//ファイル読み込み
	if (publicKeyDataBYTE.size() == 0) {
		std::cout << "*** Check Key File : Exit" << std::endl;
		throw std::exception();
	}
	binStdOut((LPBYTE)&publicKeyDataBYTE[0], (unsigned long)publicKeyDataBYTE.size(), "Public Key : Read");
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 公開鍵のBLOBからキーオブジェクトにインポート
	// dwPublicData		公開鍵のBLOB
	NCRYPT_KEY_HANDLE PublicKeyHandle = NULL;
	statusCheck(
		NCryptImportKey(
			hProvider
			, NULL																		//AES以外はNULL
			, BCRYPT_RSAPUBLIC_BLOB														//BCRYPT_RSAPUBLIC_BLOBしか、受け付けません！、汎用のBCRYPT_PUBLIC_KEY_BLOBは使えません
			, NULL																		//キー情報が得られるらしいが、必要ないのでNULL
			, &PublicKeyHandle															//キーオブジェクト
			, &publicKeyDataBYTE[0]														//キーデータ
			, (ULONG)publicKeyDataBYTE.size()											//キーデータサイズ
			, NULL																		//フラグ
		)
		, L"NCryptImportKey : Public Key"
	);
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// 暗号化する
	// 
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//Public Keyで暗号化・出力サイズ測定・RSAは固定長なので決まった値でわかっています。パケット化するときは工夫を！
	DWORD cbCipherTextPublicSize;
	statusCheck(
		NCryptEncrypt(
			PublicKeyHandle																//秘密鍵のキーオブジェクト
			, &ifsInDataBYTE[0]															//暗号化対象の元テキスト
			, (ULONG)ifsInDataBYTE.size()												//暗号化対象の元テキストサイズ
			, NULL																		//NULL指定
			, NULL																		//暗号化データBLOB
			, 0																			//暗号化データBLOBサイズ
			, &cbCipherTextPublicSize													//暗号化データBLOBサイズ取得
			, NCRYPT_PAD_PKCS1_FLAG														//フラグNCRYPT_PAD_PKCS1_FLAGしか受け付けない・かならずBIT長を揃えましょう！
		)
		, L"NCryptEncrypt : Public Key : Size"
	);
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////Public Keyで暗号化・暗号化実行
	std::vector<BYTE> pbCipherTextPublic(cbCipherTextPublicSize, NULL);					//暗号データ初期化
	statusCheck(
		NCryptEncrypt(
			PublicKeyHandle																//公開鍵のキーオブジェクト
			, &ifsInDataBYTE[0]															//暗号化対象の元テキスト
			, (ULONG)ifsInDataBYTE.size()												//暗号化対象の元テキストサイズ
			, NULL																		//NULL指定
			, &pbCipherTextPublic[0]													//暗号化データBLOB
			, cbCipherTextPublicSize													//暗号化データBLOBサイズ
			, &cbData																	//暗号化データBLOBサイズ取得
			, NCRYPT_PAD_PKCS1_FLAG														//フラグNCRYPT_PAD_PKCS1_FLAGしか受け付けない・かならずBIT長を揃えましょう！
		)
		, L"BCryptEncrypt : Public Key : Execute"
	);
	//バイナリファイル書き出し・最後の引数は（16/10/0）。カギとしてバイナリ保存するときは「０」書き出し。Debugしやすいようになっています。
	//今回は再読み込みなので「０」
	writeFileData((LPBYTE)&pbCipherTextPublic[0], (unsigned long)pbCipherTextPublic.size(), &cryptProtectData, 0);
	//コンソール出力
	binStdOut((LPBYTE)&pbCipherTextPublic[0], (unsigned long)pbCipherTextPublic.size(), "Cipher Text : ");
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	//　アルゴリズムプロバイダクローズ
	// 
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	statusCheck(
		NCryptFreeObject(hProvider)
		, L"NCryptFreeObject Close : hProvider"
	);
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
