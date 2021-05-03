/*
 * ambient_mgim.h - Library for sending data to Ambient via 3gim
 *   Ported this library to use 3GIM by @605e, March 1, 2020
 *   Special thanks for AmbientData Inc.
 */

/*
 * ambient.h - Library for sending data to Ambient
 * Created by Takehiko Shimojima, April 21, 2016
 */

#ifndef Ambient_mgim_h
#define Ambient_mgim_h

#include "Arduino.h"
#include "hl7800.h"

  // 定数など
#define amWRITEKEY_SIZE     18      // ライトキーの桁数[Bytes]
#define amDATA_SIZE         24      // データの最大桁数[Bytes]
#define amNUM_FIELDS        10      // フィールドの最大数

  // エラーコード
#define amERR_NONE          0       // エラーなし
#define amERR_BAD_PARAM     100     // パラメータがおかしい
#define amERR_TOO_LONG_DATA 101     // データが長すぎる
#define amERR_POST          2000    // Postに失敗した

class Ambient_mgim {
  public:
    Ambient_mgim(HL7800 hl7800): _hl7800(hl7800) { }

    int begin(unsigned int channelId, const char * writeKey);
    int set(int field,const char * data);
	int set(int field, double data);
	int set(int field, int data);
    int clear(int field);
    int send(void);

  private:
    HL7800 _hl7800;

    unsigned int _channelId;
    char _writeKey[amWRITEKEY_SIZE];
    char _url[256];
    char _header[128];
    struct {
        boolean  used;
        char     item[amDATA_SIZE];
    } _data[amNUM_FIELDS];
};

#endif // Ambient_mgim_h
