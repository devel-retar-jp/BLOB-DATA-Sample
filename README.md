# WindowsでTPM2.0を扱うサンプルコード
WIN32 APIからTPM2.0のハードウェア、CPU内の機能を使ってみるサンプルコードです。<br>
CUDAバージョンも余裕があればやってみたいですね<br>
C++なんて楽勝の方は、④からがTPM2.0の機能検証です。

 * ①BLOB_Access<p>
  初めにデータ型に慣れる<p>
  
 * ②TCHAR_char<p>
  char,std::vector<BYTE>の扱い方のベース<p>
  
 * ③バイナリファイルの読み込み<p>
  昔風のReadFileと今風のイテレータを使った<br>
  バイナリファイルの読み込み<p>

 * ④バイナリファイルの読み込み<p>
  昔風のReadFileと今風のイテレータを使った<br>
  バイナリファイルの読み込み<br>
  イテレータは理解しやすい！<p>
    
 * ⑤TPM_Base_Services<p>
  tpm.hライブラリの使い方<br>
  残念ならが、Windows10ではTPM2.0のハードウェアに<br>
  生のBLOBデータを送信できないテスト<br>
  Windows8以降は、高級関数で扱います。<p>
  生TPM2.0を体験したいなら、Linuxのデバイスドライバがあります。

 * ⑥Random<p>
  BCryptを使って乱数生成<br>
  TPM2.0を使って生成しています。<br>
  本気で使うならstd::random、CUDAの乱数などを併せて評価を！
 
 * ⑦Hash関数<p>
  TPM2.0の機能を使ったHASHをWIN32 APIでは出来ません。<BR>
  BCrypt関数とCrypt関数でHASHできますが、BCryptを使いましょう。<br>
  Crypt関数は廃止されるそうです。

<H2>開発環境</H2>

 * Windows10 Pro
 * Visual Studio 2019
 * C++

First : 2021/07/26
