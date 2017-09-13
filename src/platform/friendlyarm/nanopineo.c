/*
	Copyright (c) 2016 CurlyMo <curlymoo1@gmail.com>

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <signal.h>

#include "../../soc/soc.h"
#include "../../wiringx.h"
#include "../platform.h"
#include "nanopineo.h"

struct platform_t *nanopineo = NULL;

/*
3v|5v
 8|5v
 9|0v
 7|15
0v|16
 0| 1
 2|0v
 3| 4
3v| 5
12|0v
13| 6
14|10

17
18
*/

static int irq[] = {
 /*   0,   1,   2,   3 */
      0,   6,   2,   3,
 /*   4,   5,   6,   7 */
    200, 201,   1, 203,
 /*   8,   9,  10,  XX */
     12,  11,  -1,  -1,
 /*  12,  13,  14,  15 */
     -1,  -1,  -1, 198,
 /*  16,  17,  18 */
    199,  5,    4
};

static int map[] = {
 /*  PA0,  PA6,  PA2,  PA3 */
       0,    6,    2,    3,
 /*  PG8,  PG9,  PA1, PG11 */
      90,   91,    1,   93,
 /* PA12, PA11,  PC3,   XX */
      12,   11,   25,   -1,
 /*  PC0,  PC1,  PC2,  PG6 */
      22,   23,   24,   88,
 /*  PG7,  PA5,  PA4 */
      89,    5,    4
};

static int nanopineoValidGPIO(int pin) {
	if(pin >= 0 && pin < (sizeof(map)/sizeof(map[0]))) {
		if(map[pin] == -1) {
			return -1;
		}
		return 0;
	} else {
		return -1;
	}
}

static int nanopineoPinMode(int i, enum pinmode_t mode) {
	if(map[i] == -1) {
		return -1;
	}
	return nanopineo->soc->pinMode(i, mode);
}

static int nanopineoDigitalWrite(int i, enum digital_value_t value) {
	if(map[i] == -1) {
		return -1;
	}
	return nanopineo->soc->digitalWrite(i, value);
}

static int nanopineoDigitalRead(int i) {
	/* Red LED - Green LED */
	if(i == 19 || i == 20) {
		return -1;
	}
	return nanopineo->soc->digitalRead(i);
}

static int nanopineoSetup(void) {
	const size_t msize = sizeof(map) / sizeof(map[0]);
	const size_t qsize = sizeof(irq) / sizeof(irq[0]);
	nanopineo->soc->setup();
	nanopineo->soc->setMap(map, msize);
	nanopineo->soc->setIRQ(irq, qsize);
	return 0;
}

static int nanopineoISR(int i, enum isr_mode_t mode) {
	if(irq[i] == -1) {
		return -1;
	}
	nanopineo->soc->isr(i, mode);
	return 0;
}

void nanopineoInit(void) {
	platform_register(&nanopineo, "nanopineo");

	nanopineo->soc = soc_get("Allwinner", "H3");

	nanopineo->digitalRead = &nanopineoDigitalRead;
	nanopineo->digitalWrite = &nanopineoDigitalWrite;
	nanopineo->pinMode = &nanopineoPinMode;
	nanopineo->setup = &nanopineoSetup;

	nanopineo->isr = &nanopineoISR;
	nanopineo->waitForInterrupt = nanopineo->soc->waitForInterrupt;

	nanopineo->selectableFd = nanopineo->soc->selectableFd;
	nanopineo->gc = nanopineo->soc->gc;

	nanopineo->validGPIO = &nanopineoValidGPIO;
}
