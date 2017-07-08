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
#include <unistd.h>
#include <signal.h>
#include "../../sepa-C-kpi/sepa_aggregator.h"

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
#define LCD_WRITEACTION_NAME    			"Raspi16x2LCD_Write"
#define LCD_WRITEACTION_INPUT_TYPE 			"wot:LCDWriteActionInputType"
#define SEPA_PROTOCOL						"wot:SEPAProtocol"
#define WOT									"http://wot.arces.unibo.it/sepa#"

#define PREFIX_WOT              			"PREFIX wot:<http://wot.arces.unibo.it/sepa#> "
#define PREFIX_RDF              			"PREFIX rdf:<http://www.w3.org/1999/02/22-rdf-syntax-ns#> "
#define PREFIX_TD               			"PREFIX td:<http://www.w3.org/ns/td#> "
#define PREFIX_DUL              			"PREFIX dul:<http://www.ontologydesignpatterns.org/ont/dul/DUL.owl#> "
#define PREFIX_XSD							"PREFIX xsd:<http://www.w3.org/2001/XMLSchema#> "
#define SEPA_SUBSCRIPTION_ADDRESS			"ws://192.168.1.100:9000/subscribe"
#define SEPA_UPDATE_ADDRESS					"http://192.168.1.100:8000/update"

int lcd;
volatile sig_atomic_t alive = 1;

void INThandler(int sig) {
    signal(sig, SIG_IGN);
    alive = 0;
    signal(SIGINT, SIG_DFL);
}

void actionRequestNotification(sepaNode * added,int addedlen,sepaNode * removed,int removedlen) {
	int i;
	if (added!=NULL) {
		if (addedlen>1) printf("%d new requested detected.\n On the screen only the last will be shown.\n",addedlen);
		else printf("New request detected!\n!");
		for (i=0; i<addedlen; i++) {
			if (!strcmp(added[i].bindingName,"value")) {
				lcdPosition(lcd,0,0);
				lcdClear(lcd);
				lcdPosition(lcd,0,0);
				lcdPuts(lcd,added[i].value);
            }
		}
		//fprintfSepaNodes(stdout,added,addedlen,"");
		freeSepaNodes(added,addedlen);
	}
	printf("\n");
}

int main(int argc, char **argv) {
	int o;
	unsigned int count = 0;
	SEPA_subscription_params action_subscription = _initSubscription();

	printf("*\n* WOT Demo: 16x2 LCD screed actuator \n");
	printf("* WOT Team (ARCES University of Bologna) - francesco.antoniazzi@unibo.it\n");
	printf("\n\nPress Ctrl-C to exit\n\n");

	lws_set_log_level(LLL_ERR | LLL_WARN,NULL);
    //LCD initialization
	wiringPiSetup();
	lcd = lcdInit(ROW_NUMBER,COL_NUMBER,DATA_BITS,LCD_RS,LCD_E,LCD_D4,LCD_D5,LCD_D6,LCD_D7,0,0,0,0);
	lcdPosition(lcd,0,0);

    // As more than one curl request will be done by this process, by calling this function we inhibit continuous memory allocation and deallocation by curl libraries.
    http_client_init();

	// insert thing description
	o=kpProduce(PREFIX_WOT PREFIX_RDF PREFIX_DUL PREFIX_TD "DELETE {"THING_UUID" wot:isDiscoverable ?discoverable . "THING_UUID" td:hasName ?oldName . "THING_UUID" wot:hasComponent ?component. ?component rdf:type td:Thing . "THING_UUID" td:hasProperty ?property. ?property td:hasName ?pName. ?property td:hasStability ?pStability. ?property td:isWritable ?pWrite. ?pValueType rdf:type ?pDataType . ?pValueType dul:hasDataValue ?pDataValue . "THING_UUID" td:hasEvent ?event. ?event td:hasName ?eName. ?event td:forProperty ?eProperty . "THING_UUID" td:hasAction ?action. ?action td:hasName ?aName. ?action wot:isAccessibleBy ?aProtocol. ?action td:forProperty ?aProperty} INSERT {"THING_UUID" rdf:type td:Thing . "THING_UUID" td:hasName '"THING_NAME"'. "THING_UUID" wot:isDiscoverable 'true'} WHERE { OPTIONAL {"THING_UUID" rdf:type td:Thing. "THING_UUID" wot:isDiscoverable ?discoverable . "THING_UUID" td:hasName ?oldName} . OPTIONAL {"THING_UUID" wot:hasComponent ?component. ?component rdf:type td:Thing} . OPTIONAL {"THING_UUID" td:hasProperty ?property. ?property td:hasName ?pName. ?property td:hasStability ?pStability. ?property td:isWritable ?pWrite. ?pValueType rdf:type ?pDataType . ?pValueType dul:hasDataValue ?pDataValue} . OPTIONAL {"THING_UUID" td:hasEvent ?event. ?event td:hasName ?eName. OPTIONAL {?event td:forProperty ?eProperty}} . OPTIONAL {"THING_UUID" td:hasAction ?action. ?action td:hasName ?aName. ?action wot:isAccessibleBy ?aProtocol. OPTIONAL {?action td:forProperty ?aProperty}} }",SEPA_UPDATE_ADDRESS,NULL);
    if (o!=HTTP_200_OK) {
        logE("Thing Description insert update error in " THING_UUID "\n");
        return EXIT_FAILURE;
    }
    // declare Action LCDWrite
    o=kpProduce(PREFIX_WOT PREFIX_RDF PREFIX_DUL PREFIX_TD "DELETE {"THING_UUID" td:hasAction "LCD_WRITEACTION". "LCD_WRITEACTION" rdf:type td:Action. "LCD_WRITEACTION" td:hasName ?oldName. "LCD_WRITEACTION" td:hasInput ?oldInput . ?oldInput rdf:type wot:ActionInput . ?oldInput td:hasDataType ?oldDataType . "LCD_WRITEACTION" wot:isAccessibleBy "SEPA_PROTOCOL"} INSERT {"THING_UUID" td:hasAction "LCD_WRITEACTION". "LCD_WRITEACTION" rdf:type td:Action. "LCD_WRITEACTION" td:hasName '"LCD_WRITEACTION_NAME"'. "LCD_WRITEACTION" td:hasInput ?input . ?input rdf:type wot:ActionInput . ?input td:hasDataType xsd:string . "LCD_WRITEACTION" wot:isAccessibleBy "SEPA_PROTOCOL"} WHERE { "THING_UUID" rdf:type td:Thing . BIND(IRI(concat('"WOT"Input_',STRUUID())) AS ?input) . OPTIONAL {"THING_UUID" td:hasAction "LCD_WRITEACTION". "LCD_WRITEACTION" rdf:type td:Action. "LCD_WRITEACTION" td:hasName ?oldName. "LCD_WRITEACTION" td:hasInput ?oldInput . ?oldInput rdf:type wot:ActionInput . ?oldInput td:hasDataType ?oldDataType . "LCD_WRITEACTION" wot:isAccessibleBy "SEPA_PROTOCOL"}}",SEPA_UPDATE_ADDRESS,NULL);
    if (o!=HTTP_200_OK) {
        logE("Thing Description " LCD_WRITEACTION_NAME " insert error\n");
        return EXIT_FAILURE;
    }

    // declare Event HeartBeat
    o=kpProduce(PREFIX_WOT PREFIX_RDF PREFIX_DUL PREFIX_TD "DELETE {"LCD_HEART" td:hasName ?oldName} INSERT {"THING_UUID" td:hasEvent "LCD_HEART". "LCD_HEART" rdf:type td:Event. "LCD_HEART" td:hasName '"LCD_HEART_NAME"' } WHERE {"THING_UUID" rdf:type td:Thing . OPTIONAL{"THING_UUID" td:hasEvent "LCD_HEART". "LCD_HEART" rdf:type td:Event. "LCD_HEART" td:hasName ?oldName}}",SEPA_UPDATE_ADDRESS,NULL);
    if (o!=HTTP_200_OK) {
        logE("Thing Description " LCD_WRITEACTION_NAME " insert error\n");
        return EXIT_FAILURE;
    }

    // subscribe to LCDWrite action requests
    sepa_subscriber_init();
    sepa_subscription_builder(PREFIX_WOT PREFIX_RDF PREFIX_DUL PREFIX_TD "SELECT ?value ?timeStamp WHERE {"LCD_WRITEACTION" rdf:type td:Action. "LCD_WRITEACTION" wot:hasInstance ?instance. ?instance wot:hasTimeStamp ?timeStamp . OPTIONAL {?instance td:hasInput ?input. ?input dul:hasDataValue ?value}}"
              ,NULL,NULL,
              SEPA_SUBSCRIPTION_ADDRESS,
              &action_subscription);
    sepa_setSubscriptionHandlers(actionRequestNotification,NULL,&action_subscription);
    fprintfSubscriptionParams(stdout,action_subscription);
    kpSubscribe(&action_subscription);
    lcdPuts(lcd,"Init OK");

    signal(SIGINT, INThandler);
    // HeartBeat continuous loop
    while (alive) {
        o=kpProduce(PREFIX_WOT PREFIX_RDF PREFIX_DUL PREFIX_TD PREFIX_XSD "DELETE { "LCD_HEART" wot:hasInstance ?oldInstance. ?oldInstance rdf:type wot:EventInstance. ?oldInstance wot:isGeneratedBy "THING_UUID" . "THING_UUID" wot:isDiscoverable ?discoverable. ?oldInstance wot:hasTimeStamp ?eOldTimeStamp} INSERT {"LCD_HEART" wot:hasInstance ?newInstance. ?newInstance wot:isGeneratedBy "THING_UUID" . "THING_UUID" wot:isDiscoverable 'true'. ?newInstance rdf:type wot:EventInstance. ?newInstance wot:hasTimeStamp ?time} WHERE { "LCD_HEART" rdf:type td:Event. BIND(now() AS ?time) . BIND(IRI(concat('"WOT"Event_',STRUUID())) AS ?newInstance) . OPTIONAL {"LCD_HEART" wot:hasInstance ?oldInstance. ?oldInstance rdf:type wot:EventInstance. ?oldInstance wot:isGeneratedBy "THING_UUID" . "THING_UUID" wot:isDiscoverable ?discoverable. ?oldInstance wot:hasTimeStamp ?eOldTimeStamp}}",SEPA_UPDATE_ADDRESS,NULL);
        if (o!=HTTP_200_OK) {
            logE("Thing Description heartbeat update error in " THING_UUID "\n");
            return EXIT_FAILURE;
        }
        count++;
        if (alive) sleep(ALIVE_SECONDS);
    }

    http_client_free();
    kpUnsubscribe(&action_subscription);
    sepa_subscriber_destroy();

	return EXIT_SUCCESS;
}
