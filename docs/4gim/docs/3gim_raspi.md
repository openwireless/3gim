## 3GIM(V2)をラズパイで使用する

3GIM(V2/V2.1)は、Arduinoだけではなく、ラズベリーパイ等のLinux搭載マイコンでも簡単に利用できます。
ここでは、ラズベリーパイで3GIMを3Gモデムとして利用するための手順をご紹介します。

3GIM(V2)を3Gモデムとして利用することで、UART経由で利用する方法に比べて、下記の利点が得られます：

  * 高速な通信が可能となる
    * デフォルトで460kbpsの通信速度（設定ファイルで変更可能）
  * Linux上の豊富なネットワーク系コマンドが利用できる
    * ブラウザ等のインターネットアプリケーションや、いわゆるSocket APIが利用できるのでC/C++やPython/JavaScript等で簡単にインターネット通信が利用できる

## 準備

ここでは、ラズベリーパイ 2B/B/B+ で使用する例をご説明します。A/A+でも同様に利用できます。ただし、A/A+ではUSBコネクタの口が一つしかないため、（少なくとも設定する時には）USBハブが別途必要になります。

  * ラズベリーパイ自身のセットアップ
　様々なサイトで、マイクロSDカードのセットアップ方法が紹介されていますので、それらを参考にまずはマイクロSDカードに最新のRasbianをインストールしてください。分かりやすくセットアップ方法が紹介されているページをいくつかご紹介しておきます：

　[Raspberry Piのセットアップ – ローカル環境編](https://tool-lab.com/make/raspberrypi-startup-7)
　[ラズベリーパイ2のOSセットアップ](http://usicolog.nomaki.jp/engineering/raspberryPi/raspberryPi2.html)

　以下では、最新バージョンのRasbianを使った例で説明します。

  * 前提条件
    * ラズベリーパイにキーボード、マウス、ディスプレイ、電源が接続されており、正常にRasbianが動作していることを想定します。
    * 標準でインストールされていないパッケージ(wvdial)のインストールを行いますので、設定を行う時にはWiFiまたはイーサネットケーブルによるインターネット接続が必要となります。

  * 3GIM(V2)利用のための準備
    * 3GIMは、USBケーブルでラズベリーパイと接続します。3GIM(V2)は、USBで接続する場合でも外部電源からの電源供給(3.3～4.2Vの安定した電源)が別途必要です。
    * 次に、以下のコマンドをターミナルで実行し、wvdialパッケージをインストールします。最初のsudoコマンドではパスワードを聞かれますので、ログインしているユーザのパスワード（デフォルトでは、piユーザのパスワードである"raspberry"を入力します）

```
pi@raspberrypi:~ $ sudo apt-get update
    ...
pi@raspberrypi:~ $ sudo apt-get install wvdial
    ...
```

正常にwvdialがインストールできたら、次に設定ファイルを修正します。
設定ファイルの内容は、お使いになるSIMカード(ご利用になる通信サービス)により一部が変更となります。ここでは、SORACOM AirのSIMカードを利用する例を説明します。
viエディタで設定ファイルを開いて、修正・追記を行って保存します。

```
pi@raspberrypi:~ $ sudo vi /etc/wvdial.conf
```

設定ファイルは、下記の内容の通りに修正・追記します：

```
[Dialer Defaults]
Init1 = ATZ
Init2 = ATQ0 V1 E1 S0=0 &C1 &D2 +FCLASS=0
Init3 = AT+CGDCONT=4,"IP","soracom.io"
Dial Attempts = 3
Modem Type = USB Modem
Dial Command = ATD
Stupid Mode = yes
Baud = 460800
New PPPD = yes
Modem = /dev/ttyACM3
ISDN = 0
Phone = *99***4#
Password = sora
Username = sora
Carrier Check = no
Auto DNS = 1
Check Def Route =1
```

SORACOM Air以外のSIMカードをお使いの場合には、上記に示す設定ファイルの中で APN(soracom.ioの箇所), Password, Username の3個所を適宜修正してください。
また、通信速度を変更したい場合には、Baudの個所を変更してください。なお、3GIM(V2)に搭載している3G通信コアモジュールHL8548-Gでは、9600, 19200, 38400, 57600, 230400, 460800, 500000, 750000, 921600, 1843200, 3250000, 6000000 の通信速度をサポートしています。ただし、実際にすべての通信速度での正常に動作するかどうかは保証の限りではありません。

なお、4GIM(V1)を利用する場合は、「Modem = /dev/ttyACM3」を「Modem = /dev/ttyACM2」とするとうまく接続できるようになります。

## 3GIM(V2)を使ったインターネット接続（ダイヤルアップ接続）

設定ファイルの修正が終わったら、下記のコマンドを実行することでインターネットに接続することができます。

```
pi@raspberrypi:~ $ sudo wvdial
```

下記のような表示が出力されれば接続は正常に完了しています。

```
pi@raspberrypi:~ $ sudo wvdial
－－> WvDial: Internet dialer version 1.61
－－> Initializing modem.
－－> Sending: ATZ
ATZ
OK
－－> Sending: ATQ0 V1 E1 S0=0 &C1 &D2 +FCLASS=0
ATQ0 V1 E1 S0=0 &C1 &D2 +FCLASS=0
OK
－－> Sending: AT+CGDCONT=2,"IP","soracom.io"
AT+CGDCONT=2,"IP","soracom.io"
OK
－－> Modem initialized.
－－> Sending: ATD*99***2#
－－> Waiting for carrier.
ATD*99***2#
CONNECT
－－> Carrier detected.  Starting PPP immediately.
－－> Starting pppd at Sat Mar 12 03:24:03 2016
－－> Pid of pppd: 611
－－> Using interface ppp0
－－> pppd: °1î¶èjÕ[01]øjÕ[01]
－－> pppd: °1î¶èjÕ[01]øjÕ[01]
－－> pppd: °1î¶èjÕ[01]øjÕ[01]
－－> pppd: °1î¶èjÕ[01]øjÕ[01]
－－> local  IP address 10.234.220.211
－－> pppd: °1î¶èjÕ[01]øjÕ[01]
－－> remote IP address 192.200.1.21
－－> pppd: °1î¶èjÕ[01]øjÕ[01]
－－> primary   DNS address 100.127.0.53
－－> pppd: °1î¶èjÕ[01]øjÕ[01]
－－> secondary DNS address 100.127.1.53
－－> pppd: °1î¶èjÕ[01]øjÕ[01]
```

上記の実行結果例では、3GIMにIPアドレスとして10.234.220.211(ローカルIP)が割り当てられており、
DNSサーバとして100.127.0.53および100.127.1.53が割り当てられたことが分かります。

## 使用するシリアルデバイス
3GIM(V2)をUSBでラズパイに接続すると、通常は下記の7つのデバイスが利用できるようになります（末尾の番号は接続しているほかのデバイスやOSのバージョンにより変化します）：

　/dev/ttyACM0  /dev/ttyACM1  /dev/ttyACM2  /dev/ttyACM3  /dev/ttyACM4  /dev/ttyACM5  /dev/ttyACM6

上記の7つのデバイスのうち、/dev/ttyACM3をインターネット接続で利用します。
ATコマンドの実行やGPSの出力は、他のシリアルデバイス(例えば、/dev/ttyACM4)でやり取りすることができます。

## 実際に試作等で利用される場合の留意点

実際に3GIM(V2)をラズパイで利用される場合には、下記のような対応が必要となります。

  * wvdialが異常終了したときのリトライ処理
    * 電波状態等の理由で3G接続が切断される場合があります。このような場合は、wvdialが異常終了しますので、リトライ(wvdialの再実行)させる必要があります。
    * うまくリトライできない場合は、一旦、3GIMの電源をOFF/ONすることをお勧めします。
    * wvdialコマンドに関しては、[ここ](https://wiki.archlinuxjp.org/index.php/Wvdial) も参考にしてください。

  * 通信以外の機能(GPSやSMS等)の利用
    * 3Gモデムとして利用している間は、wvdial.confで設定しているシリアルデバイス以外のシリアルデバイスを使うことで、通信以外の機能(GPSやSMS)を利用することができます。
    * ラズベリーパイのUARTピンを3GIM(V2)のUARTピンと接続することで、$コマンドを使って3GIMを使用することもできます。ただし、デフォルトではラズパイのUARTはシリアルコンソールとしてほかの用途で使用されていますので、「Raspberry Pi Configuration」コマンドを使って「Serial: Disable」に設定しておく必要があります。
