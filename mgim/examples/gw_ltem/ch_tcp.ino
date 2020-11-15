/*
 *  ch_tcp.ino
 *
 *  R0  2020.03.08 605e
 */

/*
 *  $TC host_or_ip port
 *
 */
int	_connectTCP(char *request, char *response, int *nbytes) {
 	int	port, stat = ERROR;
 	// Handle parameters
 	char *params[MAX_PARAMS];
 	int nParams = parseRequest(request, params, NULL);
 	if (nParams == 2 && isnumber(params[1])) {
 		// handle host_or_ip and port
 		port = atoi(params[1]);
 		if (port < 0 || port > MAX_PORT_NUMBER) {
 			strcpy(response, "$TC=NG 601");
 		}
        else {
            // Connect
            int rc;
            if ((rc = hl7800.connectTCP(params[0], port)) == h78SUCCESS) {
     	        strcpy(response, "$TC=OK");
     	        stat = SUCCESS;
            }
            else
                sprintf(response, "$TC=NG %d", rc);
        }
 	}
 	else {
 		strcpy(response, "$TC=NG 601");
    }

 	*nbytes = strlen(response);

 	return (stat);
}

/*
 *  $TD
 *
 */
int	_disconnectTCP(char *request, char *response, int *nbytes)
{
	int	stat = ERROR;
	char *params[MAX_PARAMS];
	int nParams = parseRequest(request, params, NULL);
	if (nParams != 0) {
		strcpy(response, "$TD=NG 611");
	}
    else {
        if (hl7800.disconnectTCP() == h78SUCCESS) {
            strcpy(response, "$TD=OK");
            stat = SUCCESS;
        }
        else
    		strcpy(response, "$TD=NG 614");
    }

	*nbytes = strlen(response);

  	return stat;
}

/*
 *  $TR nbytes
 *
 */
int	_readTCP(char *request, char *response, int *nbytes) {
 	int	stat = ERROR;
 	// Check command format and parameter
 	char	*params[MAX_PARAMS];
 	int nParams = parseRequest(request, params, NULL);
 	int requestBytes = atoi(params[0]);
 	if (nParams != 1 || ! isnumber(params[0]) || requestBytes <= 0 || requestBytes > MAX_TCPREAD_SIZE) {
 		// bad format or parameter
 		strcpy(response, "$TR=NG 631");
        *nbytes = strlen(response);
     	return (ERROR);
 	}

    const int prefixLength = 12;		// 12 is length of "$TR=OK n***\n"
    int rc, len = requestBytes;
    if ((rc = hl7800.readTCP((response + prefixLength), len)) >= 0) {
        char prefix[prefixLength+1];
        len = rc;
        makePrefix(prefix, prefixLength, "$TR=OK %d", len);
        for (int i = 0; i < prefixLength; i++)
            response[i] = prefix[i];
        *nbytes = len + prefixLength;
        return (SUCCESS);
    }
    else {
 	    sprintf(response, "$TR=NG %d", -rc);
        *nbytes = strlen(response);
       	return (ERROR);
    }
}

/*
 *  $TW "data"
 *
 */
int	_writeTCP(char *request, char *response, int *nbytes) {
 	int	 stat = ERROR;
 	int length;
 	// Handle parameters
 	char *params[MAX_PARAMS];
 	int sizes[MAX_PARAMS];
 	int nParams = parseRequest(request, params, sizes);
 	if (nParams == 1) {
 		// handle data
 		length = sizes[0];
 		if (length <= 0 || length > MAX_TCPWRITE_SIZE) {
 			strcpy(response, "$TW=NG 621");
 		}
        else {
            int rc;
            if ((rc = hl7800.writeTCP((void *)params[0], length)) >= 0) {
                sprintf(response, "$TW=OK %d", length);
         		stat = SUCCESS;
         	}
            else
                sprintf(response, "$TW=NG %d", -rc);
        }
 	}
 	else {
 		strcpy(response, "$TW=NG 621");
 	}

 	*nbytes = strlen(response);

 	return (stat);
}

/*
 *  $TS [1]
 *
 */
int	_getStatusTCP(char *request, char *response, int *nbytes) {
 	int	stat = ERROR;
 	int status, tcpNotif, remainedBytes, recievedBytes;
 	char *params[MAX_PARAMS];
 	int nParams = parseRequest(request, params, NULL);
 	if (nParams > 1) {
 		strcpy(response, "$TS=NG 641");		// Bad parameter(s)
 	}
    else {
        int status, tcpNotif, remainedBytes, recievedBytes;
        if (hl7800.getStatusTCP(&status, &tcpNotif, &remainedBytes, &recievedBytes) == h78SUCCESS) {
            if (nParams == 1 && ! strcmp(params[0], "1"))
                sprintf(response, "$TS=OK %d %d %d %d", status, tcpNotif, remainedBytes, recievedBytes);
            else
                sprintf(response, "$TS=OK %d", status);
            stat = SUCCESS;
        }
        else
            strcpy(response, "$TS=NG 642");
    }

 	*nbytes = strlen(response);

 	return (stat);
}

/*
*  $TN
 *
 */
int	_getNameTCP(char *request, char *response, int *nbytes) {
 	int	stat = ERROR;
 	char *params[MAX_PARAMS];
 	int nParams = parseRequest(request, params, NULL);
 	if (nParams != 0) {
 		strcpy(response, "$TN=NG 663");		// Bad format
 	}
    else {
        // Get TCP address
        char ipAddress[MAX_IP_ADDRESS_LENGTH+1];
        if (hl7800.getNameTCP(ipAddress) == h78SUCCESS) {
     		sprintf(response, "$TN=OK %s", ipAddress);
     		stat = SUCCESS;
     	}
     	else
     		strcpy(response, "$TN=NG 661");		// Can't get ip
    }

 	*nbytes = strlen(response);

 	return (stat);
}

/*
 *  $TX [timeout_to_connect timeout_to_write]
 *
 */
int _configureTCP(char *request, char *response, int *nbytes) {
    int	stat = ERROR;
	char *params[MAX_PARAMS];
	int nParams = parseRequest(request, params, NULL);
	if (nParams == 0) {
		// No params
		sprintf(response, "$TX=OK %d %d", hl7800.getTimeoutToConnectTCP(), hl7800.getTimeoutToWriteTCP());
		stat = SUCCESS;
	}
	else if (nParams == 2 && isnumber(params[0]) && isnumber(params[0])) {
		int rc, t1 = atoi(params[0]), t2 = atoi(params[1]);
		if ((rc = hl7800.configureTCP(t1, t2)) == h78SUCCESS) {
            strcpy(response, "$TX=OK");
			stat = SUCCESS;
		}
		else {
			sprintf(response, "$TX=NG %d", rc);
		}
	}
	else {
		// bad parameter
		strcpy(response, "$TX=NG 671");
	}

	*nbytes = strlen(response);

	return (stat);
}
