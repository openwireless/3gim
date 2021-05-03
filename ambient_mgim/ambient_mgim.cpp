/*
 * ambient_3gim.h - Library for sending data to Ambient via 3gim
 *   Ported this library to use 3GIM by @605e, March 1, 2020
 *   Special thanks for AmbientData Inc.
 */

/*
 * ambient.cpp - Library for sending data to Ambient
 * Created by Takehiko Shimojima, April 21, 2016
 */

#include "Ambient_mgim.h"

//#define AMBIENT_DEBUG       1                   // シリアルプリントを用いたデバッグをするかしないか

/*
 *  Symbols
 */
#define DBG_SERIAL          SerialUSB           // シリアルモニタに対応するシリアルポート
#define MAX_RETRY_POST      5                   // HTTP/POSTの最大リトライ回数
  // デバッグ用マクロ
#if AMBIENT_DEBUG
#define DBG(...) { DBG_SERIAL.print(__VA_ARGS__); }
#define ERR(...) { DBG_SERIAL.print(__VA_ARGS__); }
#else
#define DBG(...)
#define ERR(...)
#endif /* AMBIENT_DBG */

/**
 *  @fn
 *
 *  本ライブラリambient_mgimを初期化する
 *
 *	@param(channelId)    チャネルID
 *	@param(writeKey)     当該チャネルのライトキー
 *  @return              0:成功時、0以外:エラー時(エラー番号)
 *  @detail
 */
int Ambient_mgim::begin(unsigned int channelId, const char * writeKey) {
    if (sizeof(writeKey) > amWRITEKEY_SIZE) {
        ERR("writeKey length > amWRITEKEY_SIZE");
        return (amERR_BAD_PARAM);
    }
    strcpy(_writeKey, writeKey);

    _channelId = channelId;
    sprintf(_url, "http://54.65.206.59/api/v2/channels/%d/data", _channelId);

    strcpy(_header, "Content-Type: application/json$r$n");

    for (int i = 0; i < amNUM_FIELDS; i++)
        _data[i].used = false;

    return (amERR_NONE);
}

/**
 *  @fn
 *
 *  フィールドに文字列の値をセットする
 *
 *	@param(field)        フィールド番号(0～amNUM_FIELDS-1)
 *	@param(data)         セットする値(char[]型)
 *  @return              0:成功時、0以外:エラー時(エラー番号)
 *  @detail
 */
int Ambient_mgim::set(int field, const char * data) {
    field--;
    if (field < 0 || field >= amNUM_FIELDS)
        return (amERR_BAD_PARAM);

    if (strlen(data) > amDATA_SIZE)
        return (amERR_TOO_LONG_DATA);

    _data[field].used = true;
    strcpy(_data[field].item, data);

    return (amERR_NONE);
}

/**
 *  @fn
 *
 *  フィールドに倍精度浮動小数点数の値をセットする
 *
 *	@param(field)        フィールド番号(0～amNUM_FIELDS-1)
 *	@param(data)         セットする値(double型)
 *  @return              0:成功時、0以外:エラー時(エラー番号)
 *  @detail
 */
int Ambient_mgim::set(int field, double data) {
	return (set(field, String(data).c_str()));
}

/**
 *  @fn
 *
 *  フィールドに整数の値をセットする
 *
 *	@param(field)        フィールド番号(0～amNUM_FIELDS-1)
 *	@param(data)         セットする値(int型)
 *  @return              0:成功時、0以外:エラー時(エラー番号)
 *  @detail
 */
int Ambient_mgim::set(int field, int data) {
	return (set(field, String(data).c_str()));
}

/**
 *  @fn
 *
 *  指定されたフィールドをクリアする
 *
 *	@param(field)        フィールド番号(0～amNUM_FIELDS-1)
 *  @return              0:成功時、0以外:エラー時(エラー番号)
 *  @detail
 */
int Ambient_mgim::clear(int field) {
    field--;
    if (field < 0 || field >= amNUM_FIELDS)
        return (amERR_BAD_PARAM);

    _data[field].used = false;

    return (amERR_NONE);
}

/**
 *  @fn
 *
 *  Ambientへデータをアップロードする
 *
 *	@param               なし
 *  @return              0:成功時、0以外:エラー時(エラー番号)
 *  @detail              アップロードするデータはあらかじめset()を使ってセットしておくこと
 *                       正しくアップロードできた時は、すべてのフィールドをクリアする
 */
int Ambient_mgim::send(void) {
	const char *keys[] = {"\"d1\":\"", "\"d2\":\"", "\"d3\":\"", "\"d4\":\"", "\"d5\":\"", "\"d6\":\"", "\"d7\":\"", "\"d8\":\"", "\"lat\":\"", "\"lng\":\"", "\"created\":\""};
    char body[256];

    // Bodyを作る
    memset(body, 0, sizeof(body));
    strcat(body, "{$\"writeKey$\":$\"");
    strcat(body, _writeKey);
    strcat(body, "$\"");

    for (int i = 0; i < amNUM_FIELDS; i++) {
        if (_data[i].used) {
            strcat(body, ",");
            strcat(body, keys[i]);
            strcat(body, _data[i].item);
            strcat(body, "$\"");
        }
    }
    strcat(body, "}");

    // Postする
    char result[100];
    int retry, len = sizeof(result) - 1;
    for (retry = 0; retry < MAX_RETRY_POST; retry++) {
        int ret = _hl7800.doHttpPost(_url, _header, body, strlen(body), result, &len);
        if (ret == 0)
            break;    // post succeed
    }
#ifdef AMBIENT_DEBUG
    DBG_SERIAL.print("RES=\"");
    DBG_SERIAL.print(result);
    DBG_SERIAL.println("\"");
#endif
    if (retry == MAX_RETRY_POST) {
        ERR("Could not post to host\r\n");
        return (amERR_POST);
    }

    // Clear data entries
    for (int i = 0; i < amNUM_FIELDS; i++)
        _data[i].used = false;

    return (amERR_NONE);  // Ok
}
