/*
 *  ch_http.ino
 *
 *  R0  2020.03.08 605e
 */

/*
 *  $WG url ["header"]
 *
 */
int	_doHttpGet(char *request, char *response, int *nbytes) {
    int	status;
 	char *params[MAX_PARAMS];
 	int sizes[MAX_PARAMS];
 	for (int i = 0; i < MAX_PARAMS; i++)
 		params[i] = NULL;
 	int nParams = parseRequest(request, params, sizes);
 	if (nParams == 1 || nParams == 2) {
 		// GET with/without header
 		const int prefixLength = 12;		// 12 is length of "$WG=OK n***\n"
 		*nbytes -= prefixLength;
 		if ((status = hl7800.doHttpGet(params[0], params[1], (response + prefixLength), nbytes)) == h78SUCCESS) {
 			char prefix[prefixLength+1];
 			makePrefix(prefix, prefixLength, "$WG=OK %d", *nbytes);
 			for (int i = 0; i < prefixLength; i++)
 				response[i] = prefix[i];
 			*nbytes += prefixLength;
 			return (SUCCESS);
 		}
 		else {
 			sprintf(response, "$WG=NG %d", status);
 		}
 	}
 	else {
 		strcpy(response, "$WG=NG 301");
 	}

 	*nbytes = strlen(response);
 	return (ERROR);
}

/*
 *  $WP url "body" ["header"]
 *
 */
int	_doHttpPost(char *request, char *response, int *nbytes) {
	int status;
	char *params[MAX_PARAMS];
	int sizes[MAX_PARAMS];
	for (int i = 0; i < MAX_PARAMS; i++)
		params[i] = NULL;
	int nParams = parseRequest(request, params, sizes);
	if (nParams == 2 || nParams == 3) {
		// POST with/without header
		const int prefixLength = 12;		// 12 is length of "$WP=OK n***\n"
		*nbytes -= prefixLength;
		if ((status = hl7800.doHttpPost(params[0], params[2], params[1], sizes[1], (response + prefixLength), nbytes)) == h78SUCCESS) {
			char prefix[prefixLength+1];
            makePrefix(prefix, prefixLength, "$WP=OK %d", *nbytes);
			for (int i = 0; i < prefixLength; i++)
				response[i] = prefix[i];
			*nbytes += prefixLength;
			return (SUCCESS);
		}
		else {
            sprintf(response, "$WP=NG %d", status);
		}
	}
	else {
		strcpy(response, "$WP=NG 301");
	}

	*nbytes = strlen(response);
	return (ERROR);
}
