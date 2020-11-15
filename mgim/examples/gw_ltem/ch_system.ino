/*
 *  ch_system.ino
 *
 *  R0  2020.03.08 605e
 */

#include "gw_ltem.h"

int _controlLed(char *request, char *response, int *nbytes) {
 	int stat = ERROR;
 	char *params[MAX_PARAMS];
 	int nParams = parseRequest(request, params, NULL);
 	if (nParams == 0) {
 		// No params
        sprintf(response, "$YL=OK %d", ledStatus);
        stat = SUCCESS;
 	}
 	else if (nParams == 1 && isnumber(params[0])) {
 		int n = atoi(params[0]);
 		if (n == 0 || n == 1) {
 			if (ledStatus != n) {
 				mgim.setLed(n);
 				ledStatus = n;
 			}
 			sprintf(response, "$YL=OK %d", ledStatus);
 			stat = SUCCESS;
 		}
 		else {
            strcpy(response, "$YL=NG 191");
            stat = ERROR;
 		}
 	}
 	else {
 		// too many parameters or bad parameter
 		strcpy(response, "$YL=NG 191");
 		stat = ERROR;
 	}

 	*nbytes = strlen(response);
    return stat;
}

int	_getVersion(char *request, char *response, int *nbytes) {
	int	stat = ERROR;
	char *params[MAX_PARAMS];
	int nParams = parseRequest(request, params, NULL);
	if (nParams != 0) {
		strcpy(response, "$YV=NG 141");
	}
    else {
        sprintf(response, "$YV=OK %s", GW_LTEM_VERSION);
        stat = SUCCESS;
    }

	*nbytes = strlen(response);
	return (stat);
}

int	_getIMEI(char *request, char *response, int *nbytes) {
	int	stat = ERROR;
	char *params[MAX_PARAMS];
	int nParams = parseRequest(request, params, NULL);
	if (nParams != 0) {
		strcpy(response, "$YI=NG 146");
	}
    else {
        char imei[IMEI_DIGITS+1];
        imei[IMEI_DIGITS] =  '\0';
        if (hl7800.getIMEI(imei) == h78SUCCESS)
	       sprintf(response, "$YI=OK %s", imei);
        else
		   strcpy(response, "$YI=NG 143");
    }

	*nbytes = strlen(response);
	return (stat);
}

int	_getTime(char *request, char *response, int *nbytes) {
	int	stat = ERROR;
	char *params[MAX_PARAMS];
	int nParams = parseRequest(request, params, NULL);
	if (nParams != 0) {
		strcpy(response, "$YI=NG 146");
	}
    else {
        char datetime[DATETIME_DIGITS+1];
        if (hl7800.getDateTime(datetime) == h78SUCCESS)
	       sprintf(response, "$YT=OK %s", datetime);
        else
		   strcpy(response, "$YT=NG 122");
    }

	*nbytes = strlen(response);
	return (stat);
}

int	_getRSSI(char *request, char *response, int *nbytes) {
	int	stat = ERROR;
	char *params[MAX_PARAMS];
	int nParams = parseRequest(request, params, NULL);
	if (nParams != 0) {
		strcpy(response, "$YR=NG 102");
	}
    else {
        int rssi = 0;
        if (hl7800.getRSSI(&rssi) == h78SUCCESS)
	       sprintf(response, "$YR=OK %d", rssi);
        else
		   strcpy(response, "$YR=NG 101");
    }

	*nbytes = strlen(response);
	return (stat);
}

int	_controlAirplaneMode(char *request, char *response, int *nbytes)
{
	int stat = ERROR;
	char *params[MAX_PARAMS];
	int nParams = parseRequest(request, params, NULL);
	if (nParams == 0) {
		// No params
		sprintf(response, "$YP=OK %d", airplaneMode);
		stat = SUCCESS;
	}
	else if (nParams == 1 && isnumber(params[0])) {
		// with one param
		int newMode = atoi(params[0]);
		if (newMode == 0 && airplaneMode == 1) {
			// exit from airplane mode, retry 3 times
            if (hl7800.powerOn() == h78SUCCESS) {
                h78SERIAL.begin(h78BAUDRATE);
                delay(10000);
                while (h78SERIAL.available() > 0)
                    (void)h78SERIAL.read();   // Discard gomi
				airplaneMode = 0;
                stat = SUCCESS;
            }
			else {
				strcpy(response, "$YP=NG 172");
			}
		}
		else if (newMode == 1 && airplaneMode == 0) {
			// enter into airplane mode
            if (hl7800.powerOff() == h78SUCCESS) {
                // discardResponse(WAITTIME_LOCAL);
                h78SERIAL.flush();
                h78SERIAL.end();
				airplaneMode = 1;
                stat = SUCCESS;
            }
			else {
				strcpy(response, "$YP=NG 171");
			}
		}
		else if (newMode == airplaneMode) {
			// Change of the airplaneMode is not necessary
			stat = SUCCESS;
		}
		else {
			// Bad parameter
			strcpy(response, "$YP=NG 171");
		}
	}
	else {
		strcpy(response, "$YP=NG 171");
	}

	if (stat == SUCCESS)
		sprintf(response, "$YP=OK %d", airplaneMode);

	*nbytes = strlen(response);

	return (stat);
}
