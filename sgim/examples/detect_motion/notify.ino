
void notifyCloud(uint16_t maxAccX, uint16_t maxAccY, uint16_t maxAccZ) {
    if (sgim.readyToSend()) {
        String message = sgim.toHex("EQ");
        message += sgim.toHex(maxAccX);
        message += sgim.toHex(maxAccY);
        message += sgim.toHex(maxAccZ);

        sgim.wakeup();

#if 0
        if (sgim.sendMessage(message)) {
            blinkLed(yellowLedPin, 2, 200);   // Okey
        }
        else {
            blinkLed(redLedPin, 5, 200);       // Failed
        }
#else
        String response;
        if (sgim.sendMessageWithResponse(message, response)) {
            blinkLed(yellowLedPin, 2, 200);   // Okey

            if (response.charAt(2) != 0x03)
              blinkLed(redLedPin, 60, 500);       // Failed
        }
        else {
            blinkLed(redLedPin, 5, 200);       // Failed
        }
#endif
    }
}
