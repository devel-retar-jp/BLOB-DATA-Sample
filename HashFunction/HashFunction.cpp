/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
/// BCrypt とCrypt関数 Hashの使い方
///
/// Summary:
/// ハッシュ関数の使い方を学びます
/// 
/// 2021/07/29      Retar.jp
/// 
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// PowerShell用Debug Code
// "XXXX.ps1"に保存して確認用に。
// 
//###########################################################
//#
//# Hashを計算するPowerShell Script Debug用
//#
//# 2021 / 07 / 14
//#
//# Retar.jp 作成
//#
//###########################################################
//#テスト用文字列
//$HashStr = "ABCDEFG0123456"
//###########################################################
//$stringAsStream = [System.IO.MemoryStream]::new()
//$writer = [System.IO.StreamWriter]::new($stringAsStream)
//$writer.write($HashStr)
//$writer.Flush()
//$stringAsStream.Position = 0
//#SHA256
//#Get - FileHash - Algorithm SHA256 - InputStream $stringAsStream | Select - Object Hash
//#SHA1
//Get - FileHash - Algorithm SHA1 - InputStream $stringAsStream | Select - Object Hash
//#MD5
//#Get - FileHash - Algorithm MD5 - InputStream $stringAsStream | Select - Object Hash
//###########################################################
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <iostream>
#include <iomanip>

//リンクするライブラリ
#pragma comment(lib, "bcrypt.lib")									//bcryptライブラリ					
//NT_SUCCESS(Status)
#ifndef NT_SUCCESS
#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)				//Windowsの作法
#endif

struct HashCalc														//結果出力・可変長
{
	DWORD HashBufSize;
	LPBYTE HashBuf;
};

//Hash関数 CPU
void CreateHash(BYTE* tohash, ALG_ID alg, HashCalc* hashCalc);
//Hash関数 BCrypt
BYTE* CreateHashBCrypt(BYTE* tohash, wchar_t* alg, ULONG outputlen = 20);

int main()
{
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//Sample String
	const char* orgstr = "ABCDEFG0123456";
	///
	std::cout << "ORIGINAL STRING     : " << orgstr << std::endl;
	std::cout << "ORIGINAL STRING HEX : ";
	for (auto i = 0; i < (int)strlen(orgstr); i++)
	{
		auto s = orgstr[i];
		std::cout << (unsigned long)s << " ";
	}
	std::cout << std::endl;

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	std::cout << "bcrypt.h Function   : Use This!" << std::endl;
	///SHA1 HASHエンコード
	///
	auto sha1B = CreateHashBCrypt((BYTE*)orgstr, (wchar_t*)BCRYPT_SHA1_ALGORITHM, 20);
	std::cout << "WIN32 API SHA1      : ";
	for (auto i = 0; i < 20; i++)
	{
		auto s = sha1B[i];
		std::cout << std::setfill('0') << std::setw(2) << std::uppercase << std::hex << (unsigned int)s << " ";
	}
	std::cout << std::endl;

	///MD5 HASHエンコード
	///
	auto md5B = CreateHashBCrypt((BYTE*)orgstr, (wchar_t*)BCRYPT_MD5_ALGORITHM, 16);
	std::cout << "WIN32 API MD5       : ";
	for (auto i = 0; i < 16; i++)
	{
		auto s = md5B[i];
		std::cout << std::setfill('0') << std::setw(2) << std::uppercase << std::hex << (unsigned int)s << " ";
	}
	std::cout << std::endl;

	///SHA256 HASHエンコード
	///
	auto sha256b = CreateHashBCrypt((BYTE*)orgstr, (wchar_t*)BCRYPT_SHA256_ALGORITHM, 32);
	std::cout << "WIN32 API SHA256    : ";
	for (auto i = 0; i < 32; i++)
	{
		auto s = sha256b[i];
		std::cout << std::setfill('0') << std::setw(2) << std::uppercase << std::hex << (unsigned int)s << " ";
	}
	std::cout << std::endl;

	///SHA512 HASHエンコード
	///
	auto sha512b = CreateHashBCrypt((BYTE*)orgstr, (wchar_t*)BCRYPT_SHA512_ALGORITHM, 64);
	std::cout << "WIN32 API SHA512    : ";
	for (auto i = 0; i < 64; i++)
	{
		auto s = sha512b[i];
		std::cout << std::setfill('0') << std::setw(2) << std::uppercase << std::hex << (unsigned int)s << " ";
	}
	std::cout << std::endl;

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	std::cout << "wincrypt.h Function : Don't Use!" << std::endl;
	///SHA1 HASHエンコード
	HashCalc hashCalc_SHA1;
	CreateHash((BYTE*)orgstr, CALG_SHA1, &hashCalc_SHA1);
	std::cout << "WIN32 API SHA1      : ";
	for (auto i = 0; i < (int)(hashCalc_SHA1.HashBufSize); i++)
	{
		auto s = hashCalc_SHA1.HashBuf[i];
		std::cout << std::setfill('0') << std::setw(2) << std::uppercase << std::hex << (unsigned int)s << " ";
	}
	std::cout << std::endl;

	///MD5 HASHエンコード
	HashCalc hashCalc_MD5;
	CreateHash((BYTE*)orgstr, CALG_MD5, &hashCalc_MD5);
	std::cout << "WIN32 API MD5       : ";
	for (auto i = 0; i < (int)(hashCalc_MD5.HashBufSize); i++)
	{
		auto s = hashCalc_MD5.HashBuf[i];
		std::cout << std::setfill('0') << std::setw(2) << std::uppercase << std::hex << (unsigned int)s << " ";
	}
	std::cout << std::endl;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//HASH WIN 32 API CPU
// 
//wincrypt.hで定義されている関数・そのうち廃止されるらしい
void CreateHash(BYTE* tohash, ALG_ID alg, HashCalc* hashCalc)
{
	HCRYPTPROV hProv;
	HCRYPTHASH hash;
	//
	if (CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
	{
		if (CryptCreateHash(hProv, alg, 0, 0, &hash))
		{
			if (CryptHashData(hash, (BYTE*)tohash, strlen((const char*)tohash), 0))
			{
				if (!CryptGetHashParam(hash, HP_HASHVAL, NULL, &(hashCalc->HashBufSize), 0))
				{
					printf("CryptGetHashParam Error : %d\n", GetLastError());
				}
				hashCalc->HashBuf = (LPBYTE) new BYTE[hashCalc->HashBufSize];		//固定長でとれば１回でOK
				if (!CryptGetHashParam(hash, HP_HASHVAL, (BYTE*)hashCalc->HashBuf, &hashCalc->HashBufSize, 0))
				{
					printf("CryptGetHashParam Error : %d\n", GetLastError());
				}
			}
			else
			{
				printf("CryptHashData Error : %d\n", GetLastError());
			}
			if (!CryptDestroyHash(hash))
			{
				printf("CryptDestroyHash Error : %d\n", GetLastError());
			}
		}
		else
		{
			printf("CryptCreateHash Error : %d\n", GetLastError());
		}
		if (!CryptReleaseContext(hProv, 0))
		{
			printf("CryptReleaseContext Error : %d\n", GetLastError());
		}
	}
	else
	{
		printf("CryptAcquireContext Error : %d\n", GetLastError());
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//HASH WIN 32 API BCrypt
BYTE* CreateHashBCrypt(BYTE* tohash, wchar_t* alg, ULONG outputlen)
{
	BCRYPT_HASH_HANDLE hashHandle = NULL;			//ハッシュ・ハンドラ
	BCRYPT_ALG_HANDLE hAlgorithm = NULL;			//アルゴリズム・ハンドラ
	BYTE* result = new BYTE[outputlen];				//出力バイト数
	NTSTATUS status = NULL;							//ステータス

	//BCryptOpenAlgorithmProvider・プロバイダをオープン(クローズは必要ないらしい？)
	//https://docs.microsoft.com/en-us/windows/win32/api/bcrypt/nf-bcrypt-bcryptopenalgorithmprovider
	status = BCryptOpenAlgorithmProvider(
		&hAlgorithm
		, alg
		, MS_PRIMITIVE_PROVIDER						//MS_PLATFORM_CRYPTO_PROVIDERを指定できずCPUベース
		, 0
	);
	if (!NT_SUCCESS(status)) {
		printf("BCryptHashData failed : %d\n", status);
		goto End;
	}

	//BCryptCreateHash・ハッシュを生成
	//https://docs.microsoft.com/en-us/windows/win32/api/bcrypt/nf-bcrypt-bcryptcreatehash
	status = BCryptCreateHash(
		hAlgorithm
		, &hashHandle
		, nullptr
		, 0
		, nullptr
		, 0
		, 0
	);
	if (!NT_SUCCESS(status)) {
		printf("BCryptCreateHash failed : %d\n", status);
		goto End;
	}

	//
	//BCryptHashData・一方向ハッシュ実行
	//https://docs.microsoft.com/en-us/windows/win32/api/bcrypt/nf-bcrypt-bcrypthashdata
	status = BCryptHashData(hashHandle, static_cast<UCHAR*>(tohash), strlen((const char*)tohash), 0);
	if (!NT_SUCCESS(status)) {
		printf("BCryptHashData failed : %d\n", status);
		goto End;
	}

	//BCryptFinishHash・BCryptHashData関数からのデータ回収
	//https://docs.microsoft.com/en-us/windows/win32/api/bcrypt/nf-bcrypt-bcryptfinishhash
	status = BCryptFinishHash(hashHandle, result, outputlen, 0);
	if (!NT_SUCCESS(status)) {
		printf("BCryptFinishHash failed : %d\n", status);
		goto End;
	}

	//終了処理・Gotoでぶっ飛ばし
End:
	if (hashHandle) {
		//BCryptDestroyHash・終了処理
		//https://docs.microsoft.com/en-us/windows/win32/api/bcrypt/nf-bcrypt-bcryptdestroyhash
		BCryptDestroyHash(hashHandle);
	}

	//リターン
	return result;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////