# ルート証明書の取得方法について

HTTPS/GETあるいはHTTPS/POSTを使用する場合は、予め接続先のサーバのルート証明書を取得しておく必要があります。
取得したルート証明書は、hl7800ライブラリのsetRootCA()メソッドを使ってHL7800内に設定します。

サーバのルート証明書を取得する手順を以下に示します。この手順例では、Google Chromeブラウザを使って、情報通信研究機構(NICT)のサイトからルート証明書を取得する例を示します。他のブラウザ(Microsoft Edge等)でも同様の手順で取得することができます。

(1) [NICTのサイト](https://www.nict.go.jp/) にアクセスします

<img src="png/rootca_00.png" width="800">

(2) HTTPSのアイコンをクリックしてポップアップメニューを表示させ、「証明書（有効）」を選択する

<img src="png/rootca_01.png" width="800">

(3) 以下、画面コピーの通りに順次操作をして、ルート証明書をBase64 X509形式でダウンロードする

<img src="png/rootca_02.png" width="300">
<img src="png/rootca_03.png" width="300">
<img src="png/rootca_04.png" width="300">
<img src="png/rootca_05.png" width="300">
<img src="png/rootca_06.png" width="500">
<img src="png/rootca_07.png" width="500">
<img src="png/rootca_08.png" width="500">
<img src="png/rootca_09.png" width="500">
<img src="png/rootca_10.png" width="200">

(4) 取得したルート証明書をメモ帳で開くと、このような内容となっている。

<img src="png/rootca_11.png" width="800">

上記の操作で取得したルート証明書の内容を、サンプルスケッチhttps_getにあるようにスケッチ中に埋め込みます。

--
