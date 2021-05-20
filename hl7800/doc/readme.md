# HL7800ライブラリを利用するにあたっての補足

## UARTバッファのサイズ拡張
HTTP/GET等の機能を安定して利用するためには、HL7800とATSAMD21との間で使用しているSerial(UART0)のバッファサイズを、以下の方法で拡張する必要があります。

+ Windowsの場合

(1) 下記のソースファイルをテキストエディタで開きます。
C:\Users\dai\AppData\Local\Arduino15\packages\arduino\hardware\samd\1.8.11\cores\arduino\api\RingBuffer.h

(2) シンボルSERIAL_BUFFER_SIZE の定義値を 256 に変更する

[変更前]　#define SERIAL_BUFFER_SIZE 64

[変更後]　#define SERIAL_BUFFER_SIZE 256

---
2021.5.20 更新
