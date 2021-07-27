# BLOB DATA Sample
C++開発のサンプルコード

BLOBデータを扱うためのC++の基本コード<br>
基本的なデータ型の扱い。

 * ①BLOB_Access<p>
  初めにデータ型に慣れる<p>
  
 * ②TCHAR_char<p>
  char,std::vector<BYTE>の扱い方のベース<p>
  
 * ③TPM_Base_Services<p>
  tpm.hライブラリの使い方<br>
  残念ならが、Windows10ではTPM2.0のハードウェアに<br>
  生のBLOBデータを送信できないテスト<br>
  Windows8以降は、高級関数で扱います。<p>
  生TPM2.0を体験したいなら、Linuxのデバイスドライバがあります。

<H2>開発環境</H2>

 * Windows10 Pro
 * Visual Studio 2019
 * C++

First : 2021/07/26
