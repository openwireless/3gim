# HL7800ライブラリ

## リリース
  * R1  2020/06/21  初回リリース
  * R3  2021/04/25  MGIM(V4.1)用 リリース
  * R4  2021/06/23  Bug fix版
  * R5  2021/08/16  HTTPSサポート他
    * begin()にresetパラメータ(省略時はfalse)を追加（trueを指定すると、"AT+CFUN=1,1"を実行して15秒程度余分に待つ）
    * ルート証明書を設定するsetRootCA()メソッドを追加
    * 最後のHTTP/GETまたはPOSTのレスポンスのHTTPステータスコードを取得するgetLastHttpStatusCode()メソッドを追加

## 概要
本ライブラリは、TABrainが製造・販売するLTE-Mモジュール(HL7800M)内蔵の小型マイコンMGIM用のライブラリです。
マイコンとして、Arduino Zero互換のマイコンチップ(ATSAMD21G18)を採用していますので、Arduno IDE上でアプリケーションを開発することができます。

本ライブラリは、以下の機能を提供します：
  * ライブラリの初期化・終了
  * HL7800の電源のOn/Off
  * HL7800のリブート
  * APNの設定・取得
  * 現在日時(JST)の取得
  * 電波強度の取得
  * HTTP GET/POSTの実行、ルート証明書の設定
  * TCP通信（同時に1つのTCPコネクションのみ利用可）
  * UDP通信（送信のみで、受信は不可）

TCP通信とUDP通信は、同時に利用することができます。
また、HTTP通信は、TCPやUDPを使用中でも利用することができます。

## 使い方
  * Arduino 1.8.13以降の利用を推奨します。
  * ボードマネージャで、"Arduino SAMD Boards(32-bits ARM Cortex-M0+) Ver1.8.11以降"をインストールして使用してください。
  * クラスライブラリHL7800の使い方は、サンプルスケッチをご参照ください。
  * Arduino CoreにあるRingBuffer.hで、シンボルSERIAL_BUFFER_SIZEを256または512に拡大することを推奨します。詳しくは、 [HL7800ライブラリを利用するにあたっての補足](https://github.com/openwireless/docs/blob/master/mgim/docs/hl7800_notes.md) を参照してください。
  * HTTPS/GETおよびHTTPS/POSTの実行に必要なルート証明書を取得する方法は、[ルート証明書の取得方法について](https://github.com/openwireless/3gim/blob/master/hl7800/doc/how_to_get_rootca.md) を参照してください。

## 制約事項・留意事項
  * TCP関数の一部(connectTCP,disconnectTCP,writeTCP)、UDP関数の一部(sendUDP)、HTTP関数(doHttpGet,doHttpPost)は、同期型の関数です。
  * GNSS機能には対応していません。今後のリリースで対応していく予定です。
  * R5から、HTTPSに対応しました（ただしすべてのHTTPSサーバとの接続を保証するものではありません）。使い方は、サンプルスケッチhttps_get等を参照ください。
  * ATSAMD21～HL7800間のUART通信で、ハードウェアフロー制御を利用できます。詳しくは[MGIM(V4.1)で、ハードウェアフロー制御を使用する手順](https://github.com/openwireless/docs/blob/master/mgim/docs/hw_flowctrl.md)を参照くださいを参照ください。
---
