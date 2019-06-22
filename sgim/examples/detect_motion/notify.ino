/*
 * notify.ino -- Detect motion sample with RTC
 * 
 * [説明]
 *  3軸の最大加速度をSigfoxクラウドへ通知する。
 *  シンボルACK_SIGFOXが定義されていれば、ACKあり（レスポンスあり）で通知を行う。
 *
 * [補足]
 *  あらかじめ、Sigfox backendサイトで、適切な登録・設定を行っておく必要がある。
 *  詳しくは、https://3gim.wiki/doku.php?id=sgim_v1 を参照のこと。
 */

void notifyCloud(uint16_t maxAccX, uint16_t maxAccY, uint16_t maxAccZ) {
    if (sgim.isReady()) {
        String message = sgim.toHex("EQ");
        message += sgim.toHex(maxAccX);
        message += sgim.toHex(maxAccY);
        message += sgim.toHex(maxAccZ);

        sgim.wakeup();

#ifdef ACK_SIGFOX
        String response;
        if (sgim.sendMessageAndGetResponse(message, response)) {
            blinkLed(3, 400);   // Okey

            if (response.charAt(2) != 0x03)
              blinkLed(9, 200);       // Failed
        }
        else {
            blinkLed(7, 200);       // Failed
        }
#else
        if (sgim.sendMessage(message)) {
            blinkLed(3, 400);       // Okey
        }
        else {
            blinkLed(7, 200);       // Failed
        }
#endif
    }
}