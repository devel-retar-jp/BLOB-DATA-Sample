/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
/// CString、TCHAR、WCHAR、char
///
/// Summary:
/// MSの古いライブラリ、内部形式で残るCStringは厄介
/// 全てcharで表現出来るようにしましょう
/// 
/// 日本語が必要になるとC++での扱いは厄介。
/// 特段の必要性がなければ日本語はあきらめる
/// 
/// Active Template Library (ATL) 
/// 今更のATLですが、残っている部分もあります。
/// 
/// char,std::vector<BYTE>で扱うのが楽です
///  
///  
/// 2021/07/26      Retar.jp
/// 
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include <windows.h>														//これを入れるだけでWindows
#include <iostream>
#include <atlstr.h>															//CStringが使えるようになる
#include <io.h>
#include <fcntl.h>
#include <vector>
int main()
{
	std::cout << "////////////////////////////////////////////////////////////////////////////////////////////////////////" << std::endl;
	//コンソールの扱いをUTF-8に変更
	SetConsoleOutputCP(CP_UTF8);
	setvbuf(stdout, nullptr, _IOFBF, 4096);
	std::cout << "////////////////////////////////////////////////////////////////////////////////////////////////////////" << std::endl;
	//コンソールの扱いをUTF-8に変更
	std::cout << "Set Console" << std::endl;
	setlocale(LC_CTYPE, "C");												//英文しか使わないなら"C"
	SetConsoleOutputCP(CP_UTF8);											//UTF8出力
	auto retmode = _setmode(_fileno(stdout), _O_BINARY);					//テキストバイナリ
	std::cout << "////////////////////////////////////////////////////////////////////////////////////////////////////////" << std::endl;
	char char_strText[] = "char[]_MNL567";
	CString cstring_strText = _T("CString_XYZ987");							//CStringは、配列指定しなくてもオブジェクトになっている
	TCHAR tchar_strText[] = TEXT("TCHAR_ABC12");
	WCHAR wchar_strText[] = TEXT("WCHAR_ABC12");

	std::cout << "////////////////////////////////////////////////////////////////////////////////////////////////////////" << std::endl;
	//CString	出力時はLPCTSTRでキャスト
	printf("printf CString  (LPCTSTR)          : %S\n", (LPCTSTR)cstring_strText);
	std::cout << "std::cout  CString                 : " << cstring_strText << std::endl;
	std::cout << "std::cout  CString (LPCTSTR)       : " << (LPCTSTR)cstring_strText << std::endl;
	std::wcout << "std::wcout CString                 : " << cstring_strText << std::endl;
	std::wcout << "std::wcout CString (LPCTSTR)       : " << (LPCTSTR)cstring_strText << std::endl;
	std::cout << "////////////////////////////////////////////////////////////////////////////////////////////////////////" << std::endl;
	//TCHAR
	printf("printf TCHAR                       : %S\n", tchar_strText);
	std::cout << "std::cout  TCHAR                   : " << tchar_strText << std::endl;
	std::wcout << "std::wcout TCHAR                   : " << tchar_strText << std::endl;
	std::cout << "////////////////////////////////////////////////////////////////////////////////////////////////////////" << std::endl;
	//WCHAR
	printf("printf WCHAR                       : %S\n", wchar_strText);
	std::cout << "std::cout  WCHAR                   : " << wchar_strText << std::endl;
	std::wcout << "std::wcout WCHAR                   : " << wchar_strText << std::endl;
	std::cout << "////////////////////////////////////////////////////////////////////////////////////////////////////////" << std::endl;
	//CSting -> char *
	//文字列のサイズをあらかじめ測っておく
	int size_needed = WideCharToMultiByte(CP_ACP, 0, cstring_strText, -1, NULL, 0, NULL, NULL);	
	//領域確保・HeapAlloc
	auto cstring_strText_convhp = (LPSTR)HeapAlloc(GetProcessHeap(), 0, size_needed);
	//文字列を領域にコピー
	WideCharToMultiByte(CP_ACP, 0, cstring_strText, -1, cstring_strText_convhp, size_needed, NULL, NULL);
	std::cout << "CONV char *    (LPSTR)HeapAlloc    : " << cstring_strText_convhp << std::endl;
	std::cout << "////////////////////////////////////////////////////////////////////////////////////////////////////////" << std::endl;
	//CSting -> char *
	//領域確保・new char
	char* cstring_strText_conv = new char[size_needed];
	//文字列を領域にコピー
	WideCharToMultiByte(CP_ACP, 0, cstring_strText, -1, cstring_strText_conv, size_needed, NULL, NULL);
	std::cout << "CONV char *  new char[size_needed] : " << cstring_strText_conv << std::endl;
	std::cout << "////////////////////////////////////////////////////////////////////////////////////////////////////////" << std::endl;
	// char * -> std::vector<BYTE>		今風
	std::vector<BYTE> bvector;
	bvector.reserve(size_needed - 1);
	char* csc = cstring_strText_conv;
	for (auto i = 0; i < size_needed - 1; i++)
	{
		bvector.push_back(*csc);
		csc++;
	}
	std::cout << "CONV std::vector<BYTE>             : ";
	for (BYTE& bv : bvector) {
		std::cout << bv;
	}
	std::cout << std::endl;
	std::cout << "////////////////////////////////////////////////////////////////////////////////////////////////////////" << std::endl;
	//CSting -> TCHAR
	const int tchr_Size = sizeof(char_strText) + 1;
	TCHAR tchr_Text[tchr_Size] = { _T('¥0') };
	int res = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, char_strText, sizeof(char_strText), tchr_Text, tchr_Size);
	std::wcout << "CONV TCHAR[]                       : " << tchr_Text << std::endl;
	std::cout << "////////////////////////////////////////////////////////////////////////////////////////////////////////" << std::endl;
	//TCHAR -> CString
	CString CS_tchr_Text = CString(tchr_Text);
	std::wcout << "std::wcout CString (LPCTSTR)       : " << (LPCTSTR)CS_tchr_Text << std::endl;
	std::cout << "////////////////////////////////////////////////////////////////////////////////////////////////////////" << std::endl;


}
