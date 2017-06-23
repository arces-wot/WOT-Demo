/*
 * raspberry_wot_lcd.c
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
 * 14 june 2017
 *
gcc main_lcd.c ../../../sepa-C-kpi/sepa_utilities.c ../../../sepa-C-kpi/sepa_consumer.c ../../../sepa-C-kpi/sepa_secure.c ../../../sepa-C-kpi/jsmn.c -o main_lcd -pthread -lcurl `pkg-config --cflags --libs glib-2.0 libwebsockets` -lwiringPi -lwiringPiDev
 */

#include <wiringPi.h>
#include <lcd.h>
#include <unistd.h>
#include "../../../sepa-C-kpi/sepa_aggregator.h"

#define SEPA_LOGGER_ERROR
//USE WIRINGPI PIN NUMBERS
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

#define THING_UUID			"wot:Raspberry1"
#define THING_NAME          "Raspi16x2LCD"
#define LOCATION_UUID		"wot:MyLocation"

#define SEPA_SUBSCRIPTION_ADDRESS			"ws://wot.arces.unibo.it:9000/subscribe"
#define SEPA_UPDATE_ADDRESS					"http://wot.arces.unibo.it:8000/sparql"

int lcd;

void actionRequestNotification(sepaNode * added,int addedlen,sepaNode * removed,int removedlen) {
	int i;
	if (added!=NULL) {
		if (addedlen>1) printf("%d new requested detected.\n On the screen only the last will be shown.\n",addedlen);
		else printf("New request detected!\n!");
		for (i=0; i<addedlen; i++) {
			if (!strcmp(added[i].bindingName,"value")) {
				lcdPosition(lcd,0,0);
				lcdPuts(lcd,"                               ");
				lcdPosition(lcd,0,0);
				lcdPuts(lcd,added[i].value);
}
		}
		fprintfSepaNodes(stdout,added,addedlen,"value");
		freeSepaNodes(added,addedlen);
	}
	if (removed!=NULL) {
		printf("Removed %d items:\n",removedlen);
		fprintfSepaNodes(stdout,removed,removedlen,"");
		freeSepaNodes(removed,removedlen);
	}
	printf("\n");
}

int main(int argc, char **argv) {
	int o;
	printf("*\n* WOT Demo: 16x2 LCD screed actuator \n");
	printf("* WOT Team (ARCES University of Bologna) - francesco.antoniazzi@unibo.it\n");
	printf("\nUSAGE: ./main_lcd [sepa subscription address]\nPress Ctrl-C to exit\n\n");

	sprintf(thing_description,THING_DESCRIPTION,thingUUID,thingUUID,thingUUID,thingUUID,location,thingUUID,location);

	wiringPiSetup();
	lcd = lcdInit(ROW_NUMBER,COL_NUMBER,DATA_BITS,
			LCD_RS,LCD_E,LCD_D4,LCD_D5,LCD_D6,LCD_D7,
			0,0,0,0);
	lcdPosition(lcd,0,0);

	// insert thing description
	o=kpProduce(
             "PREFIX wot:<http://www.arces.unibo.it/wot#> PREFIX rdf:<http://www.w3.org/1999/02/22-rdf-syntax-ns#> PREFIX td:<http://w3c.github.io/wot/w3c-wot-td-ontology.owl#> PREFIX dul:<http://www.ontologydesignpatterns.org/ont/dul/DUL.owl#> PREFIX saref:<http://ontology.tno.nl/saref#> DELETE {" THING_UUID " wot:isDiscoverable ?oldDiscoverable. " THING_UUID " dul:hasLocation ?oldThingLocation} INSERT {" THING_UUID " rdf:type td:Thing. " THING_UUID " rdf:type ?thingType. " THING_UUID " td:hasName '" THING_NAME "'. " THING_UUID " wot:isDiscoverable 'true'. " THING_UUID " dul:hasLocation ?newThingLocation} WHERE {OPTIONAL{" THING_UUID " rdf:type td:Thing. " THING_UUID " dul:hasLocation ?oldThingLocation. " THING_UUID " wot:isDiscoverable ?oldDiscoverable. ?oldThingLocation rdf:type dul:PhysicalPlace}. ?newThingLocation rdf:type dul:PhysicalPlace}"
             ,SEPA_UPDATE_ADDRESS
    );

    while (1) {

        sleep(ALIVE_SECONDS);
    }

	return 0;
}

void son_process() {
	while (1) {
		o=kpProduce(thing_description,SEPA_UPDATE_ADDRESS);
		if (o!=HTTP_200_OK) {
			logE("Thing Description alive process error in %s\n",thingUUID);
			return EXIT_FAILURE;
		}
		sleep(ALIVE_SECONDS);
	}
}

void father_process() {
	SEPA_subscription_params action_subscription = _initSubscription();
	int o;

	sepa_subscriber_init();
	sepa_subscription_builder(SPARQL_SUBSCRIPTION,NULL,NULL,SEPA_SUBSCRIPTION_ADDRESS,&action_subscription);
	sepa_setSubscriptionHandlers(actionRequestNotification,NULL,&action_subscription);
	fprintfSubscriptionParams(stdout,action_subscription);

	kpSubscribe(&action_subscription);
	lcdPuts(lcd,"Init OK");
	scanf("%d",&o);
}
