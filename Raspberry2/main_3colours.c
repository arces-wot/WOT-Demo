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
 * 24 june 2017
 *
gcc main_lcd.c ../../../sepa-C-kpi/sepa_utilities.c ../../../sepa-C-kpi/sepa_consumer.c
../../../sepa-C-kpi/sepa_secure.c ../../../sepa-C-kpi/jsmn.c -o main_lcd -pthread -lcurl
`pkg-config --cflags --libs glib-2.0 libwebsockets` -lwiringPi -lwiringPiDev
 */

#include <wiringPi.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <math.h>
#include "../../sepa-C-kpi/sepa_aggregator.h"

#define SEPA_LOGGER_ERROR
//USE WIRINGPI PIN NUMBERS --> see https://it.pinout.xyz/pinout/wiringpi
#define R_PIN   0
#define G_PIN   2
#define B_PIN   3
#define ALIVE_SECONDS	10

#define THING_UUID			    "wot:Raspberry2"
#define THING_NAME              "Raspi3ColourLed"
#define LOCATION_UUID		    "wot:MyLocation"
#define RGB_HEART               "wot:3ColourHeartBeatEvent"
#define RGB_HEART_NAME          "Raspi3ColourAlive"
#define RGB_COLOURACTION        "wot:ChangeColourAction"
#define RGB_COLOURACTION_NAME   "ChangeRGBLedColour"
#define RGB_FREQ_ACTION         "wot:ChangeFrequencyAction"
#define RGB_FREQ_ACTION_NAME    "ChangeRGBBlinkFrequency"
#define RGB_COLOUR_PROPERTY_UUID	"wot:RGBcolourProperty"
#define RGB_COLOUR_PROPERTY_NAME	"Raspi3ColourProperty"
#define RGB_COLOUR_VALUETYPE		"wot:RGB_colour_JSON"
#define RGB_FREQ_PROPERTY_UUID	"wot:RGBfreqProperty"
#define RGB_FREQ_PROPERTY_NAME	"Raspi3FreqProperty"
#define RGB_FREQ_VALUETYPE		"wot:RGB_freq_JSON"


#define PREFIX_WOT              "PREFIX wot:<http://www.arces.unibo.it/wot#> "
#define PREFIX_RDF              "PREFIX rdf:<http://www.w3.org/1999/02/22-rdf-syntax-ns#> "
#define PREFIX_TD               "PREFIX td:<http://w3c.github.io/wot/w3c-wot-td-ontology.owl#> "
#define PREFIX_DUL              "PREFIX dul:<http://www.ontologydesignpatterns.org/ont/dul/DUL.owl#> "
#define SEPA_SUBSCRIPTION_ADDRESS			"ws://192.168.1.146:9000/subscribe"
#define SEPA_UPDATE_ADDRESS					"http://192.168.1.146:8000/update"

volatile sig_atomic_t alive = 1;
int pipeFD[2];

typedef struct rgbf {
    int r,g,b,f;
} rgbf;

void INThandler(int sig) {
    signal(sig, SIG_IGN);
    alive = 0;
    signal(SIGINT, SIG_DFL);
}

void blink_process() {
    rgbf input;
    while (1) {
        read()
        digitalWrite(R_PIN,input.r);
        digitalWrite(R_PIN,input.g);
        digitalWrite(R_PIN,input.b);
        usleep(lround(1000/input.f));
    }
}

void changeColorRequestNotification(sepaNode * added,int addedlen,sepaNode * removed,int removedlen) {
	int i;
	if (added!=NULL) {
		if (addedlen>1) printf("%d new requested detected.\n On the screen only the last will be shown.\n",addedlen);
		else printf("New request detected!\n!");
		for (i=0; i<addedlen; i++) {
			if ((!strcmp(added[i].bindingName,"value")) && (strlen(added[i].value)==3)) {
                // R
                if ((added[i].value)[0]=='0')
                else ;
                // G
                if ((added[i].value)[1]=='0')
                else ;
                // B
                if ((added[i].value)[2]=='0')
                else ;
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
	// TODO update value!
	printf("\n");
}

void changeFrequencyRequestNotification(sepaNode * added,int addedlen,sepaNode * removed,int removedlen) {
	int i;
	if (added!=NULL) {
		if (addedlen>1) printf("%d new requested detected.\n On the screen only the last will be shown.\n",addedlen);
		else printf("New request detected!\n!");
		for (i=0; i<addedlen; i++) {
			if (!strcmp(added[i].bindingName,"value")) {
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
	// TODO update value!
}

int main(int argc, char **argv) {
	int o,blink_pid;
	char lcdHBInstance[50] = "";
	char HBEventUpdate[1000] = "";
	unsigned int count = 0;
	SEPA_subscription_params colour_action_subscription = _initSubscription();
	SEPA_subscription_params freq_action_subscription = _initSubscription();

	printf("*\n* WOT Demo: RGB led blinker\n");
	printf("* WOT Team (ARCES University of Bologna) - francesco.antoniazzi@unibo.it\n");
	printf("\n\nPress Ctrl-C to exit\n\n");

    wiringPiSetup();
    pinMode(R_PIN,OUTPUT);
    pinMode(G_PIN,OUTPUT);
    pinMode(B_PIN,OUTPUT);
    http_client_init();

	// insert thing description
	o=kpProduce(
             PREFIX_WOT PREFIX_RDF PREFIX_DUL PREFIX_TD "DELETE {" THING_UUID " wot:isDiscoverable ?oldDiscoverable. " THING_UUID " dul:hasLocation ?oldThingLocation} INSERT {" THING_UUID " rdf:type td:Thing. " THING_UUID " td:hasName '" THING_NAME "'. " THING_UUID " wot:isDiscoverable 'true'. " THING_UUID " dul:hasLocation " LOCATION_UUID "} WHERE {OPTIONAL{" THING_UUID " rdf:type td:Thing. " THING_UUID " dul:hasLocation ?oldThingLocation. " THING_UUID " wot:isDiscoverable ?oldDiscoverable. ?oldThingLocation rdf:type dul:PhysicalPlace}. " LOCATION_UUID " rdf:type dul:PhysicalPlace}"
             ,SEPA_UPDATE_ADDRESS,NULL);
    if (o!=HTTP_200_OK) {
        logE("Thing Description insert update error in " THING_UUID "\n");
        return EXIT_FAILURE;
    }
    // declare Action RGB Colour modifier
    o=kpProduce(PREFIX_WOT PREFIX_RDF PREFIX_DUL PREFIX_TD "INSERT { " THING_UUID " td:hasAction " RGB_COLOURACTION ". " RGB_COLOURACTION " rdf:type td:Action. " RGB_COLOURACTION " td:hasName '" RGB_COLOURACTION_NAME "'} WHERE { " THING_UUID " rdf:type td:Thing}"
                ,SEPA_UPDATE_ADDRESS,NULL);
    if (o!=HTTP_200_OK) {
        logE("Thing Description " LCD_WRITEACTION_NAME " insert error\n");
        return EXIT_FAILURE;
    }
     // declare Action RGB frequency modifier
    o=kpProduce(PREFIX_WOT PREFIX_RDF PREFIX_DUL PREFIX_TD "INSERT { " THING_UUID " td:hasAction " RGB_FREQ_ACTION ". " RGB_FREQ_ACTION " rdf:type td:Action. " RGB_FREQ_ACTION " td:hasName '" RGB_FREQ_ACTION_NAME "'} WHERE { " THING_UUID " rdf:type td:Thing}"
                ,SEPA_UPDATE_ADDRESS,NULL);
    if (o!=HTTP_200_OK) {
        logE("Thing Description " LCD_WRITEACTION_NAME " insert error\n");
        return EXIT_FAILURE;
    }

    // declare Event HeartBeat
	o=kpProduce(PREFIX_WOT PREFIX_RDF PREFIX_DUL PREFIX_TD "INSERT { " THING_UUID " td:hasEvent " RGB_HEART ". " RGB_HEART " rdf:type td:Event. " RGB_HEART " td:hasName '" RGB_HEART_NAME "'} WHERE { " THING_UUID " rdf:type td:Thing}"
				,SEPA_UPDATE_ADDRESS,NULL);
	if (o!=HTTP_200_OK) {
		logE("Thing Description " LCD_WRITEACTION_NAME " insert error\n");
		return EXIT_FAILURE;
	}

	// declare Property color
	o=kpProduce(PREFIX_WOT PREFIX_RDF PREFIX_DUL PREFIX_TD "INSERT { " THING_UUID " td:hasProperty " RGB_COLOUR_PROPERTY_UUID ". " RGB_COLOUR_PROPERTY_UUID " rdf:type td:Property. " RGB_COLOUR_PROPERTY_UUID " td:hasName " RGB_COLOUR_PROPERTY_NAME ". " RGB_COLOUR_PROPERTY_UUID " td:hasStability '-1'. " RGB_COLOUR_PROPERTY_UUID " td:isWritable 'true'. " RGB_COLOUR_PROPERTY_UUID " td:hasValueType " RGB_COLOUR_VALUETYPE ". " RGB_COLOUR_VALUETYPE " dul:hasDataValue '{\"r\":0,\"g\":0,\"b\":0}' }WHERE {" THING_UUID " rdf:type td:Thing}"
				,SEPA_UPDATE_ADDRESS,NULL);
	if (o!=HTTP_200_OK) {
		logE("Thing Description " RGB_COLOUR_PROPERTY_UUID " insert error\n");
		return EXIT_FAILURE;
	}
	
	// declare Property frequency
	o=kpProduce(PREFIX_WOT PREFIX_RDF PREFIX_DUL PREFIX_TD "INSERT { " THING_UUID " td:hasProperty " RGB_FREQ_PROPERTY_UUID ". " RGB_FREQ_PROPERTY_UUID " rdf:type td:Property. " RGB_FREQ_PROPERTY_UUID " td:hasName " RGB_FREQ_PROPERTY_NAME ". " RGB_FREQ_PROPERTY_UUID " td:hasStability '-1'. " RGB_FREQ_PROPERTY_UUID " td:isWritable 'true'. " RGB_FREQ_PROPERTY_UUID " td:hasValueType " RGB_FREQ_VALUETYPE ". " RGB_FREQ_VALUETYPE " dul:hasDataValue '{\"frequency\":0}' }WHERE {" THING_UUID " rdf:type td:Thing}"
				,SEPA_UPDATE_ADDRESS,NULL);
	if (o!=HTTP_200_OK) {
		logE("Thing Description " RGB_FREQ_PROPERTY_UUID " insert error\n");
		return EXIT_FAILURE;
	}

    // creation of blink process, communicating with
    if (pipe2(pipeFD,O_NONBLOCK)) {
        perror("Pipe creation error - ");
        return EXIT_FAILURE;
    }
    blink_pid = fork();
    if (!blink_pid) {
        // child process
        close(pipeFD[1]);
        blink_process();
        return EXIT_SUCCESS;
    }
    // father process
    if (blink_pid<0) {
        fprintf(stderr,"Fork error in generating blink process\n");
        return EXIT_FAILURE;
    }
    close(pipeFD[0]);

    // declaration of subscriptions
    sepa_subscriber_init();
    // subscribe to RGB Colour action requests
    sepa_subscription_builder(PREFIX_WOT PREFIX_RDF PREFIX_DUL PREFIX_TD "SELECT ?instance ?input ?value WHERE { " RGB_COLOURACTION " rdf:type td:Action. " RGB_COLOURACTION " wot:hasInstance ?instance. ?instance rdf:type wot:ActionInstance. OPTIONAL {?instance td:hasInput ?input. ?input dul:hasDataValue ?value}}"
              ,NULL,NULL,
              SEPA_SUBSCRIPTION_ADDRESS,
              &colour_action_subscription);
    sepa_setSubscriptionHandlers(changeColorRequestNotification,NULL,&colour_action_subscription);
    fprintfSubscriptionParams(stdout,colour_action_subscription);
    kpSubscribe(&colour_action_subscription);

    // subscribe to RGB Frequency action requests
    sepa_subscription_builder(PREFIX_WOT PREFIX_RDF PREFIX_DUL PREFIX_TD "SELECT ?instance ?input ?value WHERE { " RGB_FREQ_ACTION " rdf:type td:Action. " RGB_FREQ_ACTION " wot:hasInstance ?instance. ?instance rdf:type wot:ActionInstance. OPTIONAL {?instance td:hasInput ?input. ?input dul:hasDataValue ?value}}"
              ,NULL,NULL,
              SEPA_SUBSCRIPTION_ADDRESS,
              &freq_action_subscription);
    sepa_setSubscriptionHandlers(changeColorRequestNotification,NULL,&freq_action_subscription);
    fprintfSubscriptionParams(stdout,freq_action_subscription);
    kpSubscribe(&freq_action_subscription);


    signal(SIGINT, INThandler);
    // HeartBeat continuous loop
    while (alive) {
        sprintf(lcdHBInstance,"wot:3ColourHeartBeatEventInstance%u",count);
        sprintf(HBEventUpdate,
                PREFIX_WOT PREFIX_RDF PREFIX_DUL PREFIX_TD "DELETE { " LCD_HEART " wot:hasInstance ?oldInstance. ?oldInstance rdf:type wot:EventInstance. ?oldInstance wot:hasTimeStamp ?eOldTimeStamp} INSERT { " LCD_HEART " wot:hasInstance %s. %s rdf:type wot:EventInstance. %s wot:hasTimeStamp ?time} WHERE {BIND(now() AS ?time) . " LCD_HEART " rdf:type td:Event. OPTIONAL { " LCD_HEART " wot:hasInstance ?oldInstance. ?oldInstance rdf:type wot:EventInstance. ?oldInstance wot:hasTimeStamp ?eOldTimeStamp}}",
                lcdHBInstance,lcdHBInstance,lcdHBInstance);

        o=kpProduce(HBEventUpdate,SEPA_UPDATE_ADDRESS,NULL);
        if (o!=HTTP_200_OK) {
            logE("Thing Description heartbeat update error in " THING_UUID "\n");
            return EXIT_FAILURE;
        }
        count++;
        if (alive) sleep(ALIVE_SECONDS);
    }

    http_client_free();
    kpUnsubscribe(&colour_action_subscription);
    kpUnsubscribe(&freq_action_subscription);
    kill(blink_pid,SIGKILL);
    sepa_subscriber_destroy();

	return EXIT_FAILURE;
}
