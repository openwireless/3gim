/*
 *  ch_profile.ino
 *
 *  R0  2020.03.08 605e
 */

#include <FlashStorage.h>

// Profile type
typedef struct {
    int     valid;
    char    apn[28];
    char    id[16];
    char    password[16];
} PROFILE;

// profile space on flash
FlashStorage(_defaultProfile, PROFILE);

int	_setProfile(char *request, char *response, int *nbytes) {
	int stat = ERROR;
	char *params[MAX_PARAMS];
	int nParams = parseRequest(request, params, NULL);
	if (nParams == 0) {
		// Get default profile
		sprintf(response, "$PS=OK %s", getDefaultProfile());
		stat = SUCCESS;
	}
	else if (nParams == 3) {
		// Set default profile
		if (hl7800.setProfile(params[0], params[1], params[2]) == h78SUCCESS) {
            if (storeDefaultProfile(params[0], params[1], params[2]) == SUCCESS) {
			    strcpy(response, "$PS=OK");
                stat = SUCCESS;
            }
            else
                strcpy(response, "$PS=NG 213");		// store error
        }
        else {
            strcpy(response, "$PS=NG 212");		   // change error
        }
	}
	else {
		strcpy(response, "$PS=NG 211");		// format error
	}

	*nbytes = strlen(response);
	return (stat);
}

char *getDefaultProfile(void) {
    static char defaultProfile[sizeof(PROFILE)+10];

    PROFILE profile = _defaultProfile.read();
    if (! profile.valid) {
        // Not initialized yet, so initialize default profile on flash
        storeDefaultProfile("soracom.io", "sora", "sora");
        profile = _defaultProfile.read();
    }
    sprintf(defaultProfile, "\"%s\",\"%s\",\"%s\"", profile.apn, profile.id, profile.password);

    return (defaultProfile);
}

int storeDefaultProfile(char *apn, char *id, char *password) {
    PROFILE profile;
    profile.valid = true;
    strcpy(profile.apn, apn);
    strcpy(profile.id, id);
    strcpy(profile.password, password);
    _defaultProfile.write(profile);

    return (SUCCESS);
}
