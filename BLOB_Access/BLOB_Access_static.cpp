/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
/// BLOB(binary large object)の扱い方
///
/// Summary:
/// BLOBデータの扱い方をC++11/17らしく取り扱う方法
/// 従来のアクセスと比較しながら。
/// 日本語は除外
///  
/// 主にバイト文字列の使い方・静的に扱う方法
/// 動的に扱うのは別のソースで
/// 
/// 2021/07/25      Retar.jp
/// 
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <windows.h>												//これを入れるだけでWindows
#include <iostream>
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main()
{
	std::cout << "////////////////////////////////////////////////////////////////////////////////////////////////////////" << std::endl;
	//コンソールの扱いをUTF-8に変更
	SetConsoleOutputCP(CP_UTF8);
	setvbuf(stdout, nullptr, _IOFBF, 4096);
	std::cout << "////////////////////////////////////////////////////////////////////////////////////////////////////////" << std::endl;
	//伝統的なchar
	const char strchar[] = "ABCDEF";								//データ設定
	const char* psrtchar = &strchar[0];								//ポインタ指定

	//伝統的な伝統的なcharの出力
	printf("strchar %%s : %s\n", strchar);							//文字列出力
	printf("strchar %%p : %p\n", strchar);							//ポインタのアドレス・・・出力しても意味無いかも？

	//伝統的な１文字づつ出力 : strchar
	printf("sizeof(strchar) : %d\n", sizeof(strchar));				//文字列の長さ・端っこの"0"まで
	for (auto i = 0; i < sizeof(strchar) - 1; i++)
	{
		printf("strchar[%d]  : %c\n", i, strchar[i]);				//１文字づつ出力
	}

	//伝統的な１文字づつ出力 : psrtchar
	auto psrtchar_len = strlen(psrtchar);
	printf("strlen(psrtchar) : %d \n", psrtchar_len);				//文字列の長さ・端っこの"0"抜き
	for (auto i = 0; i < (int)psrtchar_len; i++)
	{
		printf("pstrchar : %d : %c\n", i, *psrtchar);				//１文字づつ出力
		psrtchar++;													//ポインタを移動
	}

	//伝統的な文字列を加工
	psrtchar = &strchar[0];											//ポインタ指定
	for (auto i = 0; i < (int)psrtchar_len; i++)
	{
		auto sift_srtchar = *psrtchar + 1;							//一文字シフト
		printf("sift_srtchar : %d : %c : %c\n", i, *psrtchar, sift_srtchar);
		//１文字と１文字シフトして出力
		psrtchar++;													//ポインタを移動
	}
	std::cout << "////////////////////////////////////////////////////////////////////////////////////////////////////////" << std::endl;
	//伝統的なunsigned char
	//文字列を扱う分にはunsigned charでも、charでも大した変わりない
	const unsigned char ustrchar[] = "HIJKLMN";						//データ設定
	const unsigned char* pusrtchar = &ustrchar[0];					//ポインタ指定

	//伝統的な１文字づつ出力 : strchar
	printf("sizeof(ustrchar) : %d\n", sizeof(ustrchar));			//文字列の長さ
	for (auto i = 0; i < sizeof(ustrchar) - 1; i++)
	{
		printf("ustrchar[%d]  : %c\n", i, ustrchar[i]);				//１文字づつ出力
	}

	std::cout << "////////////////////////////////////////////////////////////////////////////////////////////////////////" << std::endl;
	//MFCの内部定義
	//BYTE WORD 	   
	//sizeofはバイト数を表示
	std::cout << "sizeof(BYTE) : " << sizeof(BYTE) << std::endl;	//BYTEのサイズ
	std::cout << "sizeof(WORD) : " << sizeof(WORD) << std::endl;	//WORDのサイズ

	std::cout << "////////////////////////////////////////////////////////////////////////////////////////////////////////" << std::endl;
	//BYTE		8ビット符号なし・・・unsigned charと同じですが、何でもこれにすると便利
	//バイト文字列を設定・BYTE
	BYTE bstrchar[] = "ABCDEF0123456";								//バイト列で文字列を初期化
	BYTE* pbstrchar = &bstrchar[0];									//ポインタ位置で指定
	auto pbstrchar_len = sizeof(bstrchar);							//サイズ
	printf("pbstrchar_len  : %d\n", pbstrchar_len);					//文字列長出力
	for (auto i = 0; i < (int)pbstrchar_len - 1; i++)
	{
		//１文字づつ出力
		printf("bstrchar[%d]  : %c : %c\n", i, bstrchar[i], *pbstrchar);	
		pbstrchar++;												//ポインタ移動
	}

	//バイト文字列を設定・LPCSTR
	//reinterpret_castは重要な関数
	LPCSTR lpbstrchar = reinterpret_cast<LPCSTR>(&bstrchar);		//LPCSTR : 定数文字列へのポインタキャスト
	auto lpbstrchar_len = strlen(lpbstrchar);						//ポインタキャスト
	for (auto i = 0; i < (int)lpbstrchar_len; i++)
	{
		//１文字づつ出力
		printf("pbstrchar[%d]  : %c : %c\n", i, bstrchar[i], lpbstrchar[i]);
	}

	std::cout << "////////////////////////////////////////////////////////////////////////////////////////////////////////" << std::endl;
	//MFCの内部定義
	//WORD		16ビット符号なし
	BYTE bstrword[] = "AB";											//0x41(65) 0x42(66)の順番
	WORD* wbstrchar = reinterpret_cast<WORD*>(&bstrword);
	printf("BYTE Byte Order : %d : %d\n", bstrword[0], bstrword[1]);

	//バイトオーダーが反対になっている確認
	WORD wleft = *wbstrchar >> 8;									//ビットシフト
	WORD wright = *wbstrchar << 8;
	wright = wright >> 8;
	//0x4241(16961)の表示になって正解・リトルエンディアン(WORDは後ろから)
	std::cout << "WORD Byte Order : Left : " << wleft << " : Right : " << wright << std::endl;						

	std::cout << "////////////////////////////////////////////////////////////////////////////////////////////////////////" << std::endl;
	std::cout << "std::wstring -> LPWSTR -> std::wstring -> std::string" << std::endl;
	std::wstring wstr = L"ABCD1234";								//wstringを用意する
	LPWSTR lpwstr = &wstr[0];										//アタマのバイトがポインタ
	std::wstring convwstr = L"";									//変換先
	if (lpwstr != NULL)												//逆参照とか言われるので・・・
	{
		convwstr = lpwstr;
	}
	else
	{
		convwstr = L"";
	}
	//stringに直接変換するとデータが失われる
	std::string convstr = std::string(convwstr.begin(), convwstr.end());
	std::wcout << "convwstr :" << convwstr << std::endl;
	std::cout << "convstr :" << convstr << std::endl;
	std::cout << "////////////////////////////////////////////////////////////////////////////////////////////////////////" << std::endl;

}
