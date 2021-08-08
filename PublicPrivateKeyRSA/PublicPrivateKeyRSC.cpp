/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
/// 非対称鍵で暗号化
/// 
/// Summary:
/// 暗号化には①対称鍵と②非対称鍵の方式があります。
/// RSA方式でやります。
/// ECCのテストをしていませんが、暗号データ長をキッチリBITにあわせないとイケないらしい。
/// 
/// 理解したいのは以下
/// ①秘密鍵、公開鍵の生成
/// ②秘密鍵、公開鍵、それぞれを使ってのデータの暗号化で結果が同じになる
/// ③秘密鍵から公開鍵を生成できる
/// ④秘密鍵では復号できるが、公開鍵では復号できない
/// ⑤固定ビット長しか暗号化できないので、暗号化したファイルはパディング、パケット化などの工夫が必要
///  
/// 2021/08/08      Retar.jp
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
#include "functions.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma comment(lib,"Bcrypt.lib")
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
	//重要！重要！重要！重要！重要！重要！重要！
	//RSAの暗号・復号なので、ファイル先頭からの固定BITしか使えません
	//ファイルサイズが小さいとエラーになるので要注意
	//重要！重要！重要！重要！重要！重要！重要！
	std::vector<BYTE>  ifsInDataBYTE = readFileData(&InFilePath[0]);					//ファイル読み込み
	//binStdOut((LPBYTE)&ifsInDataBYTE[0], (unsigned long)ifsInDataBYTE.size(), "In File : ");
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	BCRYPT_ALG_HANDLE hAlg = NULL;														//アルゴリズム・ハンドラ
	BCRYPT_KEY_HANDLE hKey = NULL;														//キー生成時のハンドラ
	ULONG	cbData;																		//サイズ用のバッファー・テンポラリとして使っています
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// キーを生成して保存する
	// 
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//アルゴリズムプロバイダオープン
	statusCheck(
		BCryptOpenAlgorithmProvider(
			&hAlg																		//ハンドラのポインタ
			, BCRYPT_RSA_ALGORITHM														//RSAを指定
			//, BCRYPT_ECDH_P256_ALGORITHM												//ECC（ECDH）は固定にしないとダメらしい
			//, BCRYPT_ECDH_P384_ALGORITHM												//ECCはビット長でパディングする必要があるのでメンドクサそう
			//, BCRYPT_ECDH_P521_ALGORITHM
			, MS_PRIMITIVE_PROVIDER														//プロバイダ「MS_PRIMITIVE_PROVIDER or 0」。MS_PLATFORM_CRYPTO_PROVIDERはTPMの時に使います。
			, 0																			//Flagは不要
		)
		, L"BCryptOpenAlgorithmProvider Open : BCRYPT_RSA_ALGORITHM"
	);
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//キー生成
	//指定したバイト数だけファイルが書き込まれます。
	statusCheck(
		BCryptGenerateKeyPair(
			hAlg
			, &hKey
			, 512 * 8																	//RSA -> 512 * x -> MAX 16384(512*32)・512バイトの倍数にしないとエラーになります。1～32の間で。
			//, 256																		//ECC -> 入力データがガッチリ固定バイトになっていないと動かないらしい? bit paddingとか必要
			//, 384																		//ECC -> 入力データがガッチリ固定バイトになっていないと動かないらしい? bit paddingとか必要
			//, 521																		//ECC -> 入力データがガッチリ固定バイトになっていないと動かないらしい? bit paddingとか必要
			, 0																			//フラグ無し
		)
		, L"BCryptGenerateKeyPair :"
	);
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//キー生成実行・必ず実行を！！！・BCryptGenerateKeyPairとセット！・忘れるとエラー
	statusCheck(
		BCryptFinalizeKeyPair(
			hKey
			, 0
		)
		, L"BCryptFinalizeKeyPair :"
	);
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// PrivateKeyキーを保存する
	// 
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//PrivateKeyキー・BLOB生成・サイズ確認
	DWORD dwPrivateDataSize;
	statusCheck(
		BCryptExportKey(
			hKey																		//キーオブジェクト
			, NULL																		//AES以外はNULL
			, BCRYPT_PRIVATE_KEY_BLOB													//専用の定数があるが、汎用にしておくとアルゴリズムを変えてもOK、扱いが楽！
			, NULL																		//サイズを測るだけなのでNULL
			, 0																			//サイズを測るだけなので0
			, &dwPrivateDataSize														//サイズを取得
			, 0																			//フラグなし
		)
		, L"BCryptExportKey : PrivateKey : Size"
	);
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//PrivateKeyキー・BLOB生成・コピー
	std::vector<BYTE> dwPrivateData(dwPrivateDataSize, NULL);							//取得したバイト長で初期化
	statusCheck(
		BCryptExportKey(
			hKey																		//キーオブジェクト
			, NULL																		//AES以外はNULL
			, BCRYPT_PRIVATE_KEY_BLOB													//専用の定数があるが、汎用にしておくとアルゴリズムを変えてもOK、扱いが楽！
			, &dwPrivateData[0]															//確保したメモリのアドレス
			, dwPrivateDataSize															//取得したサイズ
			, &cbData																	//サイズが入ってきますが、テンポラリバッファー
			, 0																			//フラグなし
		)
		, L"BCryptExportKey : PrivateKey : Make"
	);
	//バイナリファイル書き出し・最後の引数は（16/10/0）。カギとしてバイナリ保存するときは「０」書き出し。Debugしやすいようになっています。
	writeFileData((LPBYTE)&dwPrivateData[0], (unsigned long)dwPrivateData.size(), &privateKeyData, 16);
	//コンソール出力
	//binStdOut((LPBYTE)&dwPrivateData[0], (unsigned long)dwPrivateData.size(), "Private Key : ");
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// PublicKeyキーを保存する
	// 
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//PublicKeyキー・BLOB生成・サイズ確認
	DWORD dwPublicDataSize;
	statusCheck(
		BCryptExportKey(
			hKey																		//キーオブジェクト
			, NULL																		//AES以外はNULL
			, BCRYPT_PRIVATE_KEY_BLOB													//専用の定数があるが、汎用にしておくとアルゴリズムを変えてもOK、扱いが楽！
			, NULL																		//サイズを測るだけなのでNULL
			, 0																			//サイズを測るだけなので0
			, &dwPublicDataSize															//サイズを取得
			, 0																			//フラグなし
		)
		, L"BCryptExportKey : PublicKey : Size"
	);
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//PublicKeyキー・生成・保存
	std::vector<BYTE> dwPublicData(dwPublicDataSize, NULL);								//バイト長さ、初期化データ
	statusCheck(
		BCryptExportKey(
			hKey																		//キーオブジェクト
			, NULL																		//AES以外はNULL
			, BCRYPT_PRIVATE_KEY_BLOB													//専用の定数があるが、汎用にしておくとアルゴリズムを変えてもOK、扱いが楽！
			, &dwPublicData[0]															//確保したメモリのアドレス
			, dwPublicDataSize															//取得したサイズ
			, &cbData																	//サイズが入ってきますが、テンポラリバッファー
			, 0																			//フラグなし
		)
		, L"BCryptExportKey : PublicKey : Make"
	);
	//バイナリファイル書き出し・最後の引数は（16/10/0）。カギとしてバイナリ保存するときは「０」書き出し。Debugしやすいようになっています。
	writeFileData((LPBYTE)&dwPublicData[0], (unsigned long)dwPublicData.size(), &publicKeyData, 16);
	//コンソール出力
	//binStdOut((LPBYTE)&dwPublicData[0], (unsigned long)dwPublicData.size(), "Public Key : ");
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// キーオブジェクトを破棄
	// 
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//一旦初期化する・保存したファイルのBLOBで鍵が再現できる確認をする
	//dwPrivateData		秘密鍵のBLOB
	statusCheck(
		BCryptDestroyKey(hKey)
		, L"BCryptDestroyKey Close"
	);
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// Public Keyを保存バイナリからオブジェクトにインポート
	// 
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 公開鍵のBLOBからキーオブジェクトにインポート
	// dwPublicData		公開鍵のBLOB
	BCRYPT_KEY_HANDLE PublicKeyHandle = NULL;
	statusCheck(
		BCryptImportKeyPair(
			hAlg																		//ハンドラのポインタ
			, NULL																		//AES以外はNULL
			, BCRYPT_PRIVATE_KEY_BLOB													//専用の定数があるが、汎用にしておくとアルゴリズムを変えてもOK、扱いが楽！
			, &PublicKeyHandle															//キーオブジェクト
			, &dwPublicData[0]															//キーデータ
			, (ULONG)dwPublicData.size()												//キーデータサイズ
			, 0																			//フラグなし
		)
		, L"BCryptImportKeyPair : Public Key"
	);
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// Private Keyを保存バイナリからオブジェクトにインポート
	// 
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// 秘密鍵のBLOBからキーオブジェクトにインポート
	//dwPrivateData		秘密鍵のBLOB
	BCRYPT_KEY_HANDLE PrivateKeyHandle = NULL;
	statusCheck(
		BCryptImportKeyPair(
			hAlg																		//ハンドラのポインタ
			, NULL																		//AES以外はNULL
			, BCRYPT_PRIVATE_KEY_BLOB													//専用の定数があるが、汎用にしておくとアルゴリズムを変えてもOK、扱いが楽！
			, &PrivateKeyHandle															//キーオブジェクト
			, &dwPrivateData[0]															//キーデータ
			, (ULONG)dwPrivateData.size()												//キーデータサイズ
			, 0																			//フラグなし
		)
		, L"BCryptImportKeyPair : Private Key"
	);
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// Private Keyオブジェクトから、再度PublicキーBLOBを生成する
	// 
	// # openssl rsa -in key.pem -pubout -out pubkey.pem		<- Opensslだとこんな感じ
	// 
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//PublicKeyキー・BLOB生成・サイズ確認
	DWORD cbPublicDataSizeAgain;
	statusCheck(
		BCryptExportKey(
			PrivateKeyHandle															//秘密鍵のキーオブジェクト
			, NULL																		//AES以外はNULL
			, BCRYPT_PUBLIC_KEY_BLOB													//専用の定数があるが、汎用にしておくと扱いが楽！
			, NULL																		//公開鍵のBLOB
			, 0																			//公開鍵のBLOBサイズ
			, &cbPublicDataSizeAgain													//公開鍵のBLOBサイズ取得
			, 0																			//フラグなし
		)
		, L"BCryptExportKey : PublicKey BLOB From PrivateKey Object : Size"
	);
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//PublicKeyキー・BLOB生成・保存
	std::vector<BYTE> dwPublicDataAgain(cbPublicDataSizeAgain, NULL);					//バイト長さ、初期化データ
	statusCheck(
		BCryptExportKey(
			PrivateKeyHandle															//秘密鍵のキーオブジェクト
			, NULL																		//AES以外はNULL
			, BCRYPT_PUBLIC_KEY_BLOB													//専用の定数があるが、汎用にしておくと扱いが楽！
			, &dwPublicDataAgain[0]														//公開鍵のBLOB
			, cbPublicDataSizeAgain														//公開鍵のBLOBサイズ
			, &cbData																	//公開鍵のBLOBサイズ取得
			, 0																			//フラグなし
		)
		, L"BCryptExportKey : PublicKey BLOB From PrivateKey Object : Execute"
	);
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//秘密鍵から作られた公開鍵（dwPublicDataAgain）と最初に作った公開鍵（dwPublicData）を比較する
	//コンソール出力
	//binStdOut((LPBYTE)&dwPublicData[0], (unsigned long)dwPublicData.size(), "dwPublicData : ");
	//binStdOut((LPBYTE)&dwPublicDataAgain[0], (unsigned long)dwPublicDataAgain.size(), "dwPublicDataAgain : ");
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// 公開鍵でデータを暗号化
	// 
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//Public Keyで暗号化・出力サイズ測定
	DWORD cbCipherTextPublicSize;
	statusCheck(
		BCryptEncrypt(
			PublicKeyHandle																//秘密鍵のキーオブジェクト
			, &ifsInDataBYTE[0]															//暗号化対象の元テキスト
			, (ULONG)ifsInDataBYTE.size()												//暗号化対象の元テキストサイズ
			, NULL																		//NULL指定
			, NULL																		//IV
			, 0																			//IVサイズ	
			, NULL																		//暗号化データBLOB
			, 0																			//暗号化データBLOBサイズ
			, &cbCipherTextPublicSize													//暗号化データBLOBサイズ取得
			, BCRYPT_BLOCK_PADDING														//フラグ・NULLで良いと思います
		)
		, L"BCryptEncrypt : Public Key : Size"
	);
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//Public Keyで暗号化・暗号化実行
	std::vector<BYTE> pbCipherTextPublic(cbCipherTextPublicSize, NULL);					//暗号データ初期化
	statusCheck(
		BCryptEncrypt(
			PublicKeyHandle																//公開鍵のキーオブジェクト
			, &ifsInDataBYTE[0]															//暗号化対象の元テキスト
			, (ULONG)ifsInDataBYTE.size()												//暗号化対象の元テキストサイズ
			, NULL																		//NULL指定
			, NULL																		//IV
			, 0																			//IVサイズ	
			, &pbCipherTextPublic[0]													//暗号化データBLOB
			, cbCipherTextPublicSize													//暗号化データBLOBサイズ
			, &cbData																	//暗号化データBLOBサイズ取得
			, BCRYPT_BLOCK_PADDING														//フラグ・NULLで良いと思います
		)
		, L"BCryptEncrypt : Public Key : Execute"
	);
	//バイナリファイル書き出し・最後の引数は（16/10/0）。カギとしてバイナリ保存するときは「０」書き出し。Debugしやすいようになっています。
	//writeFileData((LPBYTE)&pbCipherTextPublic[0], (unsigned long)pbCipherTextPublic.size(), &cryptProtectData, 0);
	//コンソール出力・暗号化するのは公開鍵でも秘密鍵でもできます・暗号化データが同じことを確認しましょう
	binStdOut((LPBYTE)&pbCipherTextPublic[0], (unsigned long)pbCipherTextPublic.size(), "BCryptEncrypt : PublicKey ");
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// 秘密鍵で平文データを暗号化・秘密鍵でしか平文化できません
	// 
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//Private Keyで暗号化・出力サイズ測定
	DWORD	cbCipherTextPrivateSize;
	statusCheck(
		BCryptEncrypt(
			PrivateKeyHandle															//秘密鍵のキーオブジェクト
			, &ifsInDataBYTE[0]															//暗号化対象の元テキスト
			, (ULONG)ifsInDataBYTE.size()												//暗号化対象の元テキストサイズ
			, NULL																		//NULL指定
			, NULL																		//IV
			, 0																			//IVサイズ	
			, NULL																		//暗号化データBLOB
			, 0																			//暗号化データBLOBサイズ
			, &cbCipherTextPrivateSize													//暗号化データBLOBサイズ取得
			, BCRYPT_BLOCK_PADDING														//フラグ・NULLで良いと思います
		)
		, L"BCryptEncrypt : Private Key : Size"
	);
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//Private Keyで暗号化・暗号化実行
	std::vector<BYTE> pbCipherTextPrivate(cbCipherTextPrivateSize, NULL);				//暗号データ初期化
	statusCheck(
		BCryptEncrypt(
			PrivateKeyHandle															//秘密鍵のキーオブジェクト
			, &ifsInDataBYTE[0]															//暗号化対象の元テキスト
			, (ULONG)ifsInDataBYTE.size()												//暗号化対象の元テキストサイズ
			, NULL																		//NULL指定
			, NULL																		//IV
			, 0																			//IVサイズ	
			, &pbCipherTextPrivate[0]													//暗号化データBLOB
			, cbCipherTextPrivateSize													//暗号化データBLOBサイズ
			, &cbData																	//暗号化データBLOBサイズ取得
			, BCRYPT_BLOCK_PADDING														//フラグ・NULLで良いと思います
		)
		, L"BCryptEncrypt : Private Key : Execute"
	);
	//バイナリファイル書き出し・最後の引数は（16/10/0）。カギとしてバイナリ保存するときは「０」書き出し。Debugしやすいようになっています。
	writeFileData((LPBYTE)&pbCipherTextPrivate[0], (unsigned long)pbCipherTextPrivate.size(), &cryptProtectData, 0);
	//コンソール出力・暗号化するのは公開鍵でも秘密鍵でもできます・暗号化データが同じことを確認しましょう
	binStdOut((LPBYTE)&pbCipherTextPrivate[0], (unsigned long)pbCipherTextPrivate.size(), "BCryptEncrypt : PrivateKey");
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// 秘密鍵で暗号データを復号化
	// 
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//Private Keyで復号化・出力サイズ測定
	DWORD	cbPlainTextPrivateSize;
	statusCheck(
		BCryptDecrypt(
			PrivateKeyHandle															//秘密鍵のキーオブジェクト
			, &pbCipherTextPrivate[0]													//暗号化データBLOB
			, (ULONG)pbCipherTextPrivate.size()											//暗号化データBLOBサイズ
			, NULL																		//NULL指定
			, NULL																		//IV
			, 0																			//IVサイズ	
			, NULL																		//復号化データBLOB
			, 0																			//復号化データBLOBサイズ
			, &cbPlainTextPrivateSize													//復号化データBLOBサイズ取得
			, BCRYPT_BLOCK_PADDING														//フラグ・NULLで良いと思います
		)
		, L"BCryptDecrypt : cbPlainTextPrivateSize : Size"
	);
	std::cout << "Private Key : Decrypt Size : " << std::dec << cbPlainTextPrivateSize << "BYTE" << std::endl;
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//Private Keyで復号化・復号化実行
	std::vector<BYTE> pbPlainTextPrivate(cbPlainTextPrivateSize, NULL);					//復号データ初期化
	statusCheck(
		BCryptDecrypt(
			PrivateKeyHandle															//秘密鍵のキーオブジェクト
			, &pbCipherTextPrivate[0]													//暗号化データBLOB
			, (ULONG)pbCipherTextPrivate.size()											//暗号化データBLOBサイズ
			, NULL																		//NULL指定
			, NULL																		//IV
			, 0																			//IVサイズ	
			, &pbPlainTextPrivate[0]													//復号化データBLOB
			, cbPlainTextPrivateSize													//復号化データBLOBサイズ
			, &cbData																	//復号化データBLOBサイズ取得
			, BCRYPT_BLOCK_PADDING														//フラグ・NULLで良いと思います
		)
		, L"BCryptDecrypt : cbPlainTextPrivate : Execute"
	);
	//バイナリファイル書き出し・最後の引数は（16/10/0）。カギとしてバイナリ保存するときは「０」書き出し。Debugしやすいようになっています。
	writeFileData((LPBYTE)&pbPlainTextPrivate[0], (unsigned long)pbPlainTextPrivate.size(), &decryptProtectData, 0);
	//コンソール出力・尻切れトンボになっています。固定バイト数しか暗号化できません。
	binStdOut((LPBYTE)&ifsInDataBYTE[0], (unsigned long)ifsInDataBYTE.size(), "In File : ALL DATA");
	binStdOut((LPBYTE)&pbPlainTextPrivate[0], (unsigned long)pbPlainTextPrivate.size(), "In File : Only First BYTE!!");
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// 公開鍵で暗号データを復号化
	// 
	// 公開鍵から復号出来ない確認・ブロックサイズまではわかります
	// 
	// 
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////Public Keyで復号化・出力サイズ測定
	DWORD	cbPlainTextPublicSize;
	statusCheck(
		BCryptDecrypt(
			PublicKeyHandle																//公開鍵のキーオブジェクト
			, &pbCipherTextPrivate[0]													//暗号化データBLOB
			, (ULONG)pbCipherTextPrivate.size()											//暗号化データBLOBサイズ
			, NULL																		//NULL指定
			, NULL																		//IV
			, 0																			//IVサイズ	
			, NULL																		//復号化データBLOB
			, 0																			//復号化データBLOBサイズ
			, &cbPlainTextPublicSize													//復号化データBLOBサイズ取得
			, BCRYPT_BLOCK_PADDING														//フラグ・NULLで良いと思います
		)
		, L"BCryptDecrypt : cbPlainTextPublicSize : Size"
	);
	std::cout << "Public Key : Decrypt Size : " << std::dec << cbPlainTextPublicSize << "BYTE" << std::endl;
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	//使い終わったアルゴリズムプロバイダを閉じる。
	// 
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	statusCheck(
		BCryptCloseAlgorithmProvider(hAlg, 0)
		, L"BCryptCloseAlgorithmProvider Close"
	);
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
