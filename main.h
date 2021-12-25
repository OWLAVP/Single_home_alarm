#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED
//#define F_CPU 16000000UL
#define F_CPU 7372800UL

#define DEBUG   1

#define DS18B20_SENSOR	PB0
#define DS18B20_PORT	PORTB
#define PIN_BUZZER		B, 1
#define PIN_BEEP        B, 2
#define DOOR    		B, 3
#define GREEN_LED	    B, 4
#define RED_LED	    	B, 5

#define EXT_POWER       D, 2
#define BTN_SOS 	    D, 4
#define IBUTTON         D, 6
#define EXT_MOTION_SENSOR   D, 5
#define PIN_PWR_GSM		D, 7

#define BATTERY         ANALOG0
//#define BTN_PROG        C, 1
#define BTN_GUARD	    C, 2
#define MOTION_SENSOR   C, 3
#define BTN_DEFAULT     C, 4
#define SMOKE_SENSOR    C, 5
#define INT_TEMP        ANALOG8

//Analog
#define ANALOG0     0
#define ANALOG1     1
#define ANALOG2     2
#define ANALOG3     3
#define ANALOG4     4
#define ANALOG5     5
#define ANALOG6     6
#define ANALOG7     7
#define ANALOG8     8







#define TOTAL_ADMIN_NUMBER      5
#define ADMIN_LIST				1


#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <avr/eeprom.h>

#include "IO_Macros.h"
#include "softuart.h"
#include "timer.h"
#include "UART.h"
#include "port.h"
#include "GSM.h"
#include "sms_parse.h"
#include "eeprom.h"
#include "app.h"
#include "ds18b20.h"

#endif // MAIN_H_INCLUDED
