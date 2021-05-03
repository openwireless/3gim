# HL7800ライブラリ

* リリース
 * R1  2020/06/21  初回リリース
 * R3  2021/04/25  MGIM(V4.1)用 リリース

* 概要
本ライブラリは、TABrainが製造・販売するLTE-Mモジュール(HL7800M)内蔵の小型マイコンMGIM用のライブラリです。
マイコンとして、Arduino Zero互換のマイコンチップ(ATSAMD21G18)を採用していますので、Arduno IDE上でアプリケーションを開発することができます。

本ライブラリは、以下の機能を提供します：
 * ライブラリの初期化・終了
 * HL7800の電源のOn/Off
 * HL7800のリブート
 * APNの設定・取得
 * 現在日時(JST)の取得
 * 電波強度の取得
 * HTTP GET/POST
 * TCP通信（同時に1つのTCPコネクションのみ利用可）
 * UDP通信（送信のみで、受信は不可）

TCP通信とUDP通信は、同時に利用することができます。
また、HTTP通信は、TCPやUDPを使用中でも利用することができます。

* 使い方
 * Arduino 1.8.13以降の利用を推奨します。
 * ボードマネージャで、"Arduino SAMD Boards(32-bits ARM Cortex-M0+) Ver1.8.11以降"をインストールして使用してください。
 * クラスライブラリHL7800の使い方は、サンプルスケッチをご参照ください。
 * Arduino CoreにあるRingBuffer.hで、シンボルSERIAL_BUFFER_SIZEを512または1024に拡大することを推奨します。

* 制約事項・留意事項
 * R3では、HTTPSには対応していません。今後のリリースで対応する予定です。
 * R3では、GNSS機能には対応していません。今後のリリースで対応する予定です。
 * TCP関数の一部(connectTCP,disconnectTCP,writeTCP)、UDP関数の一部(sendUDP)、HTTP関数(doHttpGet,doHttpPost)は、同期型の関数です。

---
