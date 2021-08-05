/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
/// 乱数処理の扱い方
///
/// Summary:
/// TPM2.0を使った乱数生成のアルゴリズムの検証
/// 設定値はわからないので、出来た乱数の分布をみてみる。
/// バイトのデータの個数を比較して、標準偏差が小さい程べき分布していると考えられる
/// 
/// Windows 10では、CPU内のTPMハードウェアも、別チップもハードウェアの存在しかわかりません。
/// 
/// 2021/07/28      Retar.jp
/// 
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <windows.h>
#include <stdio.h>
#include <bcrypt.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <numeric>
#include <iterator>
#include <algorithm>
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma comment(lib, "bcrypt.lib")
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define NT_SUCCESS(Status)          (((NTSTATUS)(Status)) >= 0)
#define STATUS_UNSUCCESSFUL         ((NTSTATUS)0xC0000001L)
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//ランダムバイトの取得
BYTE* GetRandomBytes(int desiredBytes, LPCWSTR pszAlgId, LPCWSTR pszImplementation, ULONG dwFlags);
//ランダムバイトの出力
void RondomBytesOut(BYTE* TPMRand, int desiredBytes);
//ランダムバイトの勘定
void RondomBytesCount(std::vector<int>* TPMRandCount, BYTE* TPMRand, int desiredBytes);
//標準偏差
double stdev(std::vector<int>* TPMRandCount);
//平均値
double mean(std::vector<int>* TPMRandCount);
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main()
{
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//コンソールの扱いをUTF-8に変更
	SetConsoleOutputCP(CP_UTF8);
	setvbuf(stdout, nullptr, _IOFBF, 4096);
	//TPMを使った乱数発生
	int desiredBytes;											//出力バイト数	
	LPCWSTR pszAlgId;											//アルゴリズム
	LPCWSTR pszImplementation;									//プロバイダ
	ULONG dwFlags;												//フラグ
	BYTE* TPMRand;												//出力データ
	std::cout << "////////////////////////////////////////////////////////////////////////////////////////////////////////" << std::endl;
	desiredBytes = 4096;
	//////////
	//https://docs.microsoft.com/en-us/windows/win32/api/bcrypt/nf-bcrypt-bcryptopenalgorithmprovider
	pszAlgId = BCRYPT_RNG_ALGORITHM;							//Windows 10ではコレで良いらしい？
	//BCRYPT_RNG_ALGORITHM
	//BCRYPT_RNG_DUAL_EC_ALGORITHM
	//BCRYPT_RNG_FIPS186_DSA_ALGORITHM
	//////////
	pszImplementation = MS_PLATFORM_CRYPTO_PROVIDER;
	//MS_PRIMITIVE_PROVIDER										//MSの標準
	//MS_PLATFORM_CRYPTO_PROVIDER								//TPMを使う
	//////////
	dwFlags = 0 | BCRYPT_ALG_HANDLE_HMAC_FLAG;					//BEST？
	//dwFlags = 0 | BCRYPT_PROV_DISPATCH;						//WORST？
	//dwFlags = 0 | BCRYPT_HASH_REUSABLE_FLAG | BCRYPT_PROV_DISPATCH | BCRYPT_ALG_HANDLE_HMAC_FLAG;	//ALL
	//① 0
	//② BCRYPT_ALG_HANDLE_HMAC_FLAG			
	//③ BCRYPT_PROV_DISPATCH
	//④ BCRYPT_HASH_REUSABLE_FLAG
	///
	///実行パラメータ
	std::cout << "pszAlgId          : " << "0x" << std::hex << pszAlgId << std::endl;
	std::cout << "pszImplementation : " << "0x" << std::hex << pszImplementation << std::endl;
	std::cout << "dwFlags           : " << std::dec << dwFlags << " (0x" << std::hex << dwFlags << ")" << std::endl;
	////////////////////////////////////////////
	/////乱数を作るだけなら下記の２行だけ
	/////バイト計算
	//TPMRand = GetRandomBytes(desiredBytes, pszAlgId, pszImplementation, dwFlags);
	/////バイト出力
	//RondomBytesOut(TPMRand, desiredBytes);
	////////////////////////////////////////////
	std::cout << "////////////////////////////////////////////////////////////////////////////////////////////////////////" << std::endl;
	//生成した乱数の評価・勘定
	std::vector<int> TPMRandCount(256, 0);
	struct results {
		int times;
		double mean;
		double stdev;
	};
	std::vector<results> randombench;
	for (auto i = 0; i < 2000; i++)
	{
		TPMRand = GetRandomBytes(desiredBytes, pszAlgId, pszImplementation, dwFlags);
		RondomBytesCount(&TPMRandCount, TPMRand, desiredBytes);

		results res = { i, mean(&TPMRandCount) ,stdev(&TPMRandCount) };
		randombench.push_back(res);
	}
	std::cout << "////////////////////////////////////////////////////////////////////////////////////////////////////////" << std::endl;
	//生成した乱数の評価・勘定・結果出力
	std::cout
		<< "Times,Average,Stdev"
		<< std::endl;

	for (auto itr = randombench.begin(); itr != randombench.end(); ++itr) {
		std::cout
			<< std::dec << (*itr).times
			<< "," << (*itr).mean
			<< "," << (*itr).stdev
			<< std::endl;
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//平均
double mean(std::vector<int>* TPMRandCount)
{
	std::vector<double> resultSet;
	for (auto i = 0; i < 256; i++)
	{
		resultSet.push_back((double)((*TPMRandCount).at(i)));
	}
	double sum = std::accumulate(std::begin(resultSet), std::end(resultSet), 0.0);
	return sum / resultSet.size();									//mean
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//標準偏差
double stdev(std::vector<int>* TPMRandCount)
{
	std::vector<double> resultSet;
	for (auto i = 0; i < 256; i++)
	{
		resultSet.push_back((double)((*TPMRandCount).at(i)));
	}

	double sum = std::accumulate(std::begin(resultSet), std::end(resultSet), 0.0);
	double mean = sum / resultSet.size();							//mean

	double accum = 0.0;
	std::for_each(std::begin(resultSet), std::end(resultSet), [&](const double d) {
		accum += (d - mean) * (d - mean);
		});
	return sqrt(accum / (double)((double)resultSet.size() - 1.0));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//乱数出力勘定
void RondomBytesCount(std::vector<int>* TPMRandCount, BYTE* TPMRand, int desiredBytes)
{
	//乱数を勘定してみる
	for (auto i = 0; i < desiredBytes; i++)
	{
		auto tr = (int)TPMRand[i];
		(*TPMRandCount).at(tr) = (*TPMRandCount).at(tr)++;
	}
	//std::cout << std::endl;

	//結果出力
	//for (auto i = 0; i < 256; i++)
	//{
	//	std::cout << std::setfill('0') << std::setw(2) << std::hex << std::uppercase << i << "," << std::dec << (*TPMRandCount).at(i) << std::endl;
	//}
	//std::cout << "////////////////////////////////////////////////////////////////////////////////////////////////////////" << std::endl;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//乱数出力
void RondomBytesOut(BYTE* TPMRand, int desiredBytes)
{
	std::cout << "RANDOM DEC " << std::setfill('0') << std::setw(4) << std::dec << desiredBytes << "BYTE: ";
	for (auto i = 0; i < desiredBytes; i++)
	{
		auto s = TPMRand[i];
		std::cout << std::setfill('0') << std::setw(2) << std::uppercase << std::hex << (unsigned long)s << " ";
	}
	std::cout << std::endl;

}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//乱数発生
BYTE* GetRandomBytes(int desiredBytes, LPCWSTR pszAlgId, LPCWSTR pszImplementation, ULONG dwFlags)
{
	BYTE* result = new BYTE[desiredBytes];
	BCRYPT_ALG_HANDLE   hAlg = NULL;
	NTSTATUS status;
	std::cout << "////////////////////////////////////////////////////////////////////////////////////////////////////////" << std::endl;
	//最大最小は1～4096バイト
	if (desiredBytes <= 0) { desiredBytes = 1; }
	if (desiredBytes >= 4096) { desiredBytes = 4096; }
	std::cout << "Make BYTE: " << std::dec << desiredBytes << std::endl;
	std::cout << "////////////////////////////////////////////////////////////////////////////////////////////////////////" << std::endl;
	//オープン
	//https://docs.microsoft.com/en-us/windows/win32/api/bcrypt/nf-bcrypt-bcryptopenalgorithmprovider
	status = BCryptOpenAlgorithmProvider(
		&hAlg,
		pszAlgId,			
		pszImplementation,						
		dwFlags									
	);
	if (status)
	{
		std::cout << "BCryptOpenAlgorithmProvider Success: Status: " << std::dec << status << " (0x" << std::hex << status << ")" << '\n';
	}
	else
	{
		std::cout << "BCryptOpenAlgorithmProvider Fail: Status: " << std::dec << status << " (0x" << std::hex << status << ")" << '\n';
		return NULL;
	}
	std::cout << "////////////////////////////////////////////////////////////////////////////////////////////////////////" << std::endl;
	try
	{
		//乱数生成
		//https://docs.microsoft.com/en-us/windows/win32/api/bcrypt/nf-bcrypt-bcryptgenrandom
		status = BCryptGenRandom(
			BCRYPT_RNG_ALG_HANDLE
			, &result[0]
			, desiredBytes
			, 0
		);
		if (!status)
		{
			std::cout << "BCryptGenRandom Success: Status: " << std::dec << status << " (0x" << std::hex << status << ")" << '\n';
		}
		else
		{
			std::cout << "BCryptGenRandom Fail: Status: " << std::dec << status << " (0x" << std::hex << status << ")" << '\n';
			return NULL;
		}
	}
	catch (std::runtime_error const& e)
	{
		std::cout << e.what() << '\n';
	}
	std::cout << "////////////////////////////////////////////////////////////////////////////////////////////////////////" << std::endl;
	try
	{
		//クローズ
		//https://docs.microsoft.com/en-us/windows/win32/api/bcrypt/nf-bcrypt-bcryptclosealgorithmprovider
		status = BCryptCloseAlgorithmProvider(hAlg, 0);
		if (status)
		{
			std::cout << "BCryptCloseAlgorithmProvider Success: Status: " << std::dec << status << " (0x" << std::hex << status << ")" << '\n';
		}
		else
		{
			std::cout << "BCryptCloseAlgorithmProvider Fail: Status: " << std::dec << status << " (0x" << std::hex << status << ")" << '\n';
			return NULL;
		}

	}
	catch (std::runtime_error const& e)
	{
		std::cout << e.what() << '\n';
	}
	std::cout << "////////////////////////////////////////////////////////////////////////////////////////////////////////" << std::endl;
	return result;
}

