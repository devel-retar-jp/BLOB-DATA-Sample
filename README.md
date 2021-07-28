# WindowsでTPM2.0を扱うサンプルコード
WIN32 APIからTPM2.0のハードウェア、CPU内の機能を使ってみる。<br>
③からがTPM2.0の機能検証です。

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

 * ④Random<p>
  BCryptを使って乱数生成<br>
  TPM2.0を使って生成しています。<br>
  本気で使うならstd::random、CUDAの乱数などを併せて評価を！

<H2>開発環境</H2>

 * Windows10 Pro
 * Visual Studio 2019
 * C++

First : 2021/07/26
