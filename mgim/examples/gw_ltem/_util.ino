/*
 *  util.ino
 *
 *  R0  2020.03.08 605e
 */

#include "gw_ltem.h"

void blinkLed(int times, int halfCycle) {
    while (times-- > 0) {
        mgim.setLed(1);
        delay(halfCycle);
        mgim.setLed(0);
        delay(halfCycle);
    }
}


int	isnumber(char *s) {
	for (; *s != '\0'; s++) {
		if (! isdigit(*s))
			return (0);	// not a number
	}
	return (1);	// numnber
}

void output(char *s) {
    Serial1.print(s);
}

void outputln(char *s) {
    Serial1.println(s);
}

void output(void *s, int nbytes) {
    char *p = (char *)s;
    while (nbytes-- > 0)
        Serial1.write(*p++);
}

void outputln(void *s, int nbytes) {
    char *p = (char *)s;
    while (nbytes-- > 0)
        Serial1.write(*p++);
    Serial1.println();
}

void discardInput(void) {
    while (Serial1.available() > 0)
        (void *)Serial1.read();
}

int parseRequest(char *request, char *params[], int sizes[]) {
	char *cp = request + 3;		// skip "$XX"
	for (int nth = 0; nth < MAX_PARAMS; nth++) {
		int nbytes = 0;
		// Skip white spaces
		for ( ; *cp != '\0' && isspace(*cp); cp++)
			;
		if (*cp == '\0') {
			// No more parameters
			return (nth);
		}
		if (*cp == '\"') {
			// In string - $ escape prosessing
			params[nth] = ++cp;		// skip "
			while (*cp != '\0' && *cp != '\"') {	//@@
				if (*cp++ == '$') {
					if (*cp == 'x' || *cp == 'X') {
						// $xhh or $Xhh
						*(cp-1) = (ascii2hex(tolower(*(cp+1))) * 16) + ascii2hex(tolower(*(cp+2)));
						strcpy(cp, cp + 3);		// Shift string
					}
					else {
						// $t, $r, $n, $$ or $"
						switch (*cp) {
						  case 'r' :	// CR
							*(cp-1) = '\r';
							break;
						  case 'n' :	// LF/NL
							*(cp-1) = '\n';
							break;
						  case 't' :	// TAB
							*(cp-1) = '\t';
							break;
						  case '$' :	// $ self
							*(cp-1) = '$';
							break;
						  case '"' :	// " self
							*(cp-1) = '"';
							break;
						  default :		// *cp self
							*(cp-1) = *cp;
							break;
						}
						strcpy(cp, cp + 1);		// Shift string
					}
				}
				nbytes++;
			}
		}
		else {
			// In literal - no $ escape prosessing
			params[nth] = cp;
			while (*cp != '\0' && ! isspace(*cp)) {
				if (*cp++ == '$') {
					if (*cp == 'x' || *cp == 'X')
						cp += 3;	// $xhh or $Xhh
					else
						cp += 1;	// $t, $r, $n, $$ or $"
				}
				nbytes++;
			}
		}

		if (*cp != '\0') {
			*cp = '\0';		// to terminate parameter, replace last space or '"' with '\0'
			cp++;			// next char
		}

#if 0
		// Dump params[nth]
		debugPrints("params[");
		debugPrints(FormatItoa(nth));
		debugPrints("]:\n");
		for (int i = 0; i < nbytes; i++) {
			debugPrints(" [");
			debugPrints(FormatItoa(i));
			debugPrints("] 0x");
			debugPrints(FormatHex(params[nth][i], 2));
			debugPrints("\n");
		}
		debugPrints("\n");
#endif

		if (sizes != NULL)
			sizes[nth] = nbytes;
	}

	return (MAX_PARAMS);
}

void makePrefix(char *prefix, int prefixLength, char *ok, int nbytes) {
	sprintf(prefix, ok, nbytes);
	if (nbytes < 10)
		strcat(prefix, "   ");	// fill 3 spaces
    else if (nbytes < 100)
		strcat(prefix, "  ");	// fill 2 spaces
    else if (nbytes < 1000)
		strcat(prefix, " ");	// fill one space
	strcat(prefix, "\n");
}
