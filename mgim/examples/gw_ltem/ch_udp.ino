/*
 *  ch_udp.ino
 *
 *  R0  2020.03.09 605e
 */

/*
 *  $UB
 *
 */
int	_beginUDP(char *request, char *response, int *nbytes) {
 	int	stat = ERROR;
 	// Handle parameters
 	char	*params[MAX_PARAMS];
 	int nParams = parseRequest(request, params, NULL);
 	if (nParams != 0) {
 		strcpy(response, "$UB=NG 801");		// Bad command format
    }
    else {
        // Configure UDP connection
        int rc;
     	if ((rc = hl7800.beginUDP()) != h78SUCCESS) {
            strcpy(response, "$UB=OK");
         	stat = SUCCESS;
     	}
        else
            sprintf(response, "$UB=NG %d", rc);
 	}

 	*nbytes = strlen(response);

 	return (stat);
}

/*
 *  $US host port nbytes
 *
 */
int _sendUDP(char *request, char *response, int *nbytes) {
    int	stat = ERROR;
	int length = 0, remotePort = 0;
	// Handle parameters
	char	*params[MAX_PARAMS];
	int nParams = parseRequest(request, params, NULL);
	if (nParams == 3 && isnumber(params[1]) && isnumber(params[2])) {
		remotePort = atoi(params[1]);
		length = atoi(params[2]);
		if ((remotePort < 0 || remotePort > MAX_PORT_NUMBER)) {
			strcpy(response, "$US=NG 802");		// Bad port number
		}
		else if ((length < 0 || length > MAX_UDPTUNNEL_SIZE)) {
			strcpy(response, "$US=NG 802");		// Bad data size
		}
        else {
            int rc;
            if ((rc = hl7800.sendUDP(params[0], remotePort, (void *)params[2], length)) == h78SUCCESS) {
                strcpy(response, "$US=OK");
                stat = SUCCESS;
            }
            else
                sprintf(response, "$US=NG %d", rc);
        }
	}
	else {
		strcpy(response, "$US=NG 801");		// Bad command format
	}

	*nbytes = strlen(response);

	return (stat);
}

/*
 *  $UE
 *
 */
int _endUDP(char *request, char *response, int *nbytes) {
    int	stat = ERROR;
 	// Handle parameters
 	char	*params[MAX_PARAMS];
 	int nParams = parseRequest(request, params, NULL);
 	if (nParams != 0) {
 		strcpy(response, "$UE=NG 801");		// Bad command format
    }
    else {
        // Configure UDP connection
        int rc;
     	if ((rc = hl7800.endUDP()) != h78SUCCESS) {
            strcpy(response, "$UE=OK");
         	stat = SUCCESS;
     	}
        else
            sprintf(response, "$UE=NG %d", rc);
 	}

 	*nbytes = strlen(response);

 	return (stat);
}

/*
 *  $UN
 *
 */
int _getNameUDP(char *request, char *response, int *nbytes) {
    int	stat = ERROR;
	char	*params[MAX_PARAMS];
	int nParams = parseRequest(request, params, NULL);
	if (nParams != 0) {
		strcpy(response, "$UN=NG 863");		// Bad format
	}
    else {
        char	ipAddress[MAX_IP_ADDRESS_LENGTH+1];
        int rc;
        if ((rc = hl7800.getNameUDP(ipAddress)) == h78SUCCESS) {
            sprintf(response, "$UN=OK %s", ipAddress);
            stat = SUCCESS;
        }
        else
            strcpy(response, "$UN=NG 861");		// Can't get ip
    }

	*nbytes = strlen(response);

	return (stat);
}
