/*
 * main_lcd.c
 *
 * Copyright 2017 Francesco Antoniazzi <francesco.antoniazzi@unibo.it>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 *
 * This code is made for the W3C Web Of Things Plugfest in Dusseldorf (July 2017)
 * 29 june 2017
 *
gcc main_lcd.c ../../sepa-C-kpi/sepa_producer.c ../../sepa-C-kpi/sepa_utilities.c ../../sepa-C-kpi/sepa_consumer.c ../../sepa-C-kpi/sepa_secure.c ../../sepa-C-kpi/jsmn.c -o main_lcd -pthread -lcurl `pkg-config --cflags --libs glib-2.0 libwebsockets` -lwiringPi -lwiringPiDev
 */

#include <wiringPi.h>
#include <lcd.h>
#include <signal.h>
#include <stdio.h>

#define SEPA_LOGGER_ERROR
//USE WIRINGPI PIN NUMBERS --> see https://it.pinout.xyz/pinout/wiringpi
#define LCD_RS  25               //Register select pin
#define LCD_E   24               //Enable Pin
#define LCD_D4  23               //Data pin 4
#define LCD_D5  22               //Data pin 5
#define LCD_D6  21               //Data pin 6
#define LCD_D7  14               //Data pin 7
#define ROW_NUMBER		2
#define COL_NUMBER		16
#define MAX_LENGHT		32		// 16*2
#define DATA_BITS		4
#define ALIVE_SECONDS	10

#define THING_UUID			    			"wot:Raspberry1"
#define THING_NAME              			"ARCES_32char"
#define LCD_HEART               			"wot:Ping"
#define LCD_HEART_NAME          			"Raspi16x2LCDAlive"
#define LCD_WRITEACTION         			"wot:LCDWriteAction"

int lcd;

int main(int argc, char **argv) {
	int o;
	unsigned int count = 0;

	//LCD initialization
	wiringPiSetup();
	lcd = lcdInit(ROW_NUMBER,COL_NUMBER,DATA_BITS,LCD_RS,LCD_E,LCD_D4,LCD_D5,LCD_D6,LCD_D7,0,0,0,0);
	lcdPosition(lcd,0,0);

	printf("%s\n", argv[1]);
    lcdPuts(lcd,argv[1]);

}
