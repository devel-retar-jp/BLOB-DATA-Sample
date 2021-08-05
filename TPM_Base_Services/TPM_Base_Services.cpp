/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
/// TPM Base Servicesの扱い方
///
/// Summary:
/// 結論から言うと、このコードは動きません。
/// tbs.hのヘッダを使って、ダイレクトにTPM2.0のハードウェアにコマンドを送ってもエラーになります。
/// TPM2.0のハードウェアの存在しか、わかりません。
/// 
/// Windows 10では、CPU内のTPMハードウェアも、別チップもハードウェアの存在しかわかりません。
/// 
/// 2021/07/27      Retar.jp
/// 
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <windows.h>
#include <tbs.h>
#include <vector>
#include <iostream>
#include <iomanip>
#include <string>
//リンクするライブラリ
//#pragma comment(lib, "bcrypt.lib")
//#pragma comment(lib, "Crypt32.lib")
#pragma comment(lib, "tbs.lib")
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//結果出力
void TbsipSubmitCommand_Func_Result(std::vector<BYTE> datavect, TBS_RESULT rv, TBS_HCONTEXT hContext, BYTE* buf, UINT32 buf_len);
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main()
{
	std::cout << "////////////////////////////////////////////////////////////////////////////////////////////////////////" << std::endl;
	//コンソールの扱いをUTF-8に変更
	SetConsoleOutputCP(CP_UTF8);
	setvbuf(stdout, nullptr, _IOFBF, 4096);
	std::cout << "////////////////////////////////////////////////////////////////////////////////////////////////////////" << std::endl;
	//TPM 1.2 Device Open
	//https://docs.microsoft.com/en-us/windows/win32/api/tbs/ns-tbs-tbs_context_params
	std::cout << "TPM Check 1.2" << std::endl;
	TBS_CONTEXT_PARAMS		pContextParams1;											//バージョン設定の構造体
	pContextParams1.version = TBS_CONTEXT_VERSION_ONE;									//TBS_CONTEXT_VERSION_ONEしか設定できません
	TBS_HCONTEXT			hContext1;													//コンテックスハンドラ
	TBS_RESULT				rv1;														//リターン
	//https://docs.microsoft.com/en-us/windows/win32/api/tbs/nf-tbs-tbsi_context_create
	rv1 = Tbsi_Context_Create(&pContextParams1, &hContext1);							//デバイスオープン
	std::cout << "Version 1.2 : Return: " << std::dec << rv1 << " (0x" << std::hex << rv1 << ")" << std::endl;
	//  TBS_SUCCESS				0 (0x0)					=>　TPM 2.0のハードウェアがある
	//	TBS_E_TPM_NOT_FOUND		2150121487 (0x8028400F)	=>	TPM 1.2のハードウェアがない
	if (rv1 == TBS_SUCCESS)
	{
		Tbsip_Context_Close(hContext1);													//デバイスクローズ
	}
	std::cout << "////////////////////////////////////////////////////////////////////////////////////////////////////////" << std::endl;
	//TPM 2.0 Device Open
	//https://docs.microsoft.com/en-us/windows/win32/api/tbs/ns-tbs-tbs_context_params2
	std::cout << "TPM Check 2.0" << std::endl;
	TBS_CONTEXT_PARAMS2		pContextParams2;
	pContextParams2.version = TPM_VERSION_20;
	pContextParams2.includeTpm12 = 0;
	pContextParams2.includeTpm20 = 1;
	TBS_HCONTEXT			hContext2;													//コンテックスハンドラ
	TBS_RESULT				rv2;														//リターン
	rv2 = Tbsi_Context_Create((PCTBS_CONTEXT_PARAMS)&pContextParams2, &hContext2);
	std::cout << "Version 2.0 : Return: " << std::dec << rv2 << " (0x" << std::hex << rv2 << ")" << std::endl;
	
	std::cout << "////////////////////////////////////////////////////////////////////////////////////////////////////////" << std::endl;
	//Device Info
	//https://docs.microsoft.com/en-us/windows/win32/api/tbs/nf-tbs-tbsi_getdeviceinfo
	std::cout << "TPM 2.0 Device Info" << std::endl;
	TPM_DEVICE_INFO info;
	info.structVersion = TPM_VERSION_20;												//TPM_VERSION_20に設定しろと書いてあるが？
	auto retinfo = Tbsi_GetDeviceInfo(sizeof(info), &info);
	std::cout 
		<< "Version 2.0 DeviceInfo : structVersion: "<< info.structVersion				//1で帰ってくるのでオカシイ！
		<< "   tpmVersion: " << info.tpmVersion											//デバイスのバージョン
		<< "   Return: " << std::dec << retinfo << " (0x" << std::hex << retinfo << ")"
		<< "\n";

	std::cout << "////////////////////////////////////////////////////////////////////////////////////////////////////////" << std::endl;
	//コマンド送り込み・エラーになります！
	// 	   Windows 10では使えません
	//https://docs.microsoft.com/en-us/windows/win32/api/tbs/nf-tbs-tbsip_submit_command
	UINT32 buf_len = 512;																//固定長・結果出力バッファーサイズ
	BYTE buf[512];																		//固定長・結果出力バッファー
	unsigned char* response = buf;														//ポインタ・結果出力バッファー
	uint32_t* resp_len = &buf_len;														//ポインタ・結果出力バッファーサイズ
	TBS_RESULT				rvvect;														//リターンコード
	//コマンドBLOB
	std::vector<BYTE> datavect = {
		//////////		
		//Startup
		0, 193,																			// TPM_TAG_RQU_COMMAND 
		0, 0, 0, 12,																	// 12バイト・データ長 -> 間違うとエラーになる
		0, 0, 0, 153,																	// TPM_ORD_Startup 
		0, 1																			// ST_CLEAR 
		//////////		
		//Selftest Full
		//0, 193,																			/* TPM_TAG_RQU_COMMAND */
		//0, 0, 0, 10,																		/* length */
		//0, 0, 0, 80																		/* TPM_ORD_SelfTestFull */
	};
	///
	buf_len = sizeof(buf);
	memset(buf, 0, buf_len * sizeof(buf[0]));
	rvvect = Tbsip_Submit_Command(
		hContext2																		//コンテックス・The handle of the context that is submitting the command.	
		, TBS_COMMAND_LOCALITY_ZERO														//0のみ・Used to set the locality for the TPM command. This must be one of the following values.
		, TBS_COMMAND_PRIORITY_NORMAL													//プライオリティ・The priority level that the command should have. This parameter can be one of the following values.
		, &datavect[0]																	//入力データのポインタ・A pointer to a buffer that contains the TPM command to process.
		, (UINT32)datavect.size()														//入力データのバイト数
		, response																		//リターンbuffer
		, resp_len																		//リターンbufferサイズ
	);
	//どんなコマンドを入れても同じリターンになります。
	//つまり、Windows 10では使えません。
	TbsipSubmitCommand_Func_Result(datavect, rvvect, hContext2, buf, buf_len);			//結果出力
	std::cout << "////////////////////////////////////////////////////////////////////////////////////////////////////////" << std::endl;
	//Close
	if (rv2 == TBS_SUCCESS)
	{
		rv2 = Tbsip_Context_Close(hContext2);											//ハンドラクローズ
		std::cout << "Version 2.0 : Close  : Return: " << std::dec << rv2 << " (0x" << std::hex << rv2 << ")" << std::endl;
	}
	std::cout << "////////////////////////////////////////////////////////////////////////////////////////////////////////" << std::endl;

}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////
////TbsipSubmitCommand_Func_Result
void TbsipSubmitCommand_Func_Result(std::vector<BYTE> datavect, TBS_RESULT rv, TBS_HCONTEXT hContext, BYTE* buf, UINT32 buf_len)
{
	std::cout << "Tbsip_Submit_Command ROW: ";
	for (auto i = 0; i < (int)datavect.size(); i++)
	{
		std::cout << std::setfill('0') << std::setw(4) << std::to_string(datavect[i]) << " : ";
	}
	std::cout << std::endl;
	std::cout << "Tbsip_Submit_Command Result: " << std::hex << rv << "\t\t  STATUS : " << std::hex << hContext << "\n";
	std::cout << "Tbsip_Submit_Command ROW: ";
	for (auto i = 0; i < (int)buf_len; i++)
	{
		auto s = buf[i];
		std::cout << std::setfill('0') << std::setw(4) << std::to_string(buf[i]) << " : ";
	}
	std::cout << std::endl;
	std::cout << "Tbsip_Submit_Command HEX: ";
	for (auto i = 0; i < (int)buf_len; i++)
	{
		auto s = buf[i];
		std::cout <<"0x"<< std::setfill('0') << std::setw(2) << (unsigned long)s << " : ";
	}
	std::cout << std::endl;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
