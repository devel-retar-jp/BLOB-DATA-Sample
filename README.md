# WindowsでTPM2.0を扱うサンプルコード
WIN32 APIからTPM2.0のハードウェア、CPU内の機能を使ってみるサンプルコードです。<br>
CUDAバージョンも余裕があればやってみたいですね<br>
C++なんて楽勝の方は、④からがTPM2.0の機能検証です。<br>
ソースが散らかり気味なので後から整理します。<br>


 * ①BLOB_Access<p>
  Folder : BLOB_Access<br>
  はじめにデータ型に慣れる<p>
  
 * ②TCHAR_char<p>
  Folder : TCHAR_char<br>
  char,std::vector<BYTE>の扱い方のベース<p>
  
 * ③バイナリファイルの読み込み<p>
  Folder : BLOB_FILE_Read<br>
  昔風のReadFileと今風のイテレータを使った<br>
  バイナリファイルの読み込み<p>

 * ④バイナリファイルの書き込み<p>
  Folder : BLOB_FILE_Write<br>
  昔風のReadFileと今風のイテレータを使った<br>
  バイナリファイルの読み込み<br>
  イテレータは理解しやすい！<p>
    
 * ⑤TPM_Base_Services<p>
  Folder : TPM_Base_Services<br>
  tpm.hライブラリの使い方<br>
  残念ならが、Windows10ではTPM2.0のハードウェアに<br>
  生のBLOBデータを送信できないテスト<br>
  Windows8以降は、高級関数で扱います。<p>
  生TPM2.0を体験したいなら、Linuxのデバイスドライバがあります。<p>

 * ⑥Random<p>
  Folder : HashingCNG<br>
  BCryptを使って乱数生成<br>
  TPM2.0を使って生成しています。<br>
  本気で使うならstd::random、CUDAの乱数などを併せて評価を！<p>
 
 * ⑦Hash関数<p>
  Folder : HashFunction<br>
  TPM2.0の機能を使ったHASHをWIN32 APIでは出来ません。<BR>
  BCrypt関数とCrypt関数でHASHできますが、BCryptを使いましょう。<br>
  Crypt関数は廃止されるそうです。<p>

 * ⑧CryptProtectData / CryptUnprotectData<p>
  Folder : CryptProtectData<br>
  自分のマシンだけでデータを暗号化、自分で復号する方法。<br>
  Windows NTの時代からある端末単位で暗号化するAPI<br> 
  将来的には廃止される予定なので使用しない方がいいです。<p>

 * ⑨共通鍵でファイルを暗号化<p>
  Folder : CommonKey<br>
  CNGでの暗号化で、共通鍵を使った方式<br>
  無線LANなどは、この方式で暗号化しています。<br>
  仕組みがわかるようにプログラムを書いてみました。<p>
  
  

<H2>開発環境</H2>

 * Windows10 Pro
 * Visual Studio 2019
 * C++

First : 2021/07/26
