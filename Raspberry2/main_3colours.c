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
 * 29 june 2017
 *
gcc main_3colours.c ../../sepa-C-kpi/sepa_utilities.c ../../sepa-C-kpi/sepa_consumer.c ../../sepa-C-kpi/sepa_secure.c ../../sepa-C-kpi/jsmn.c ../../sepa-C-kpi/sepa_producer.c -o main_3colours -pthread -lcurl `pkg-config --cflags --libs glib-2.0 libwebsockets` -lwiringPi -lwiringPiDev
 */

#include <wiringPi.h>
#include <unistd.h>
#include <signal.h>
#include "../../sepa-C-kpi/sepa_aggregator.h"

#define SEPA_LOGGER_ERROR
//USE WIRINGPI PIN NUMBERS --> see https://it.pinout.xyz/pinout/wiringpi
#define R_PIN   0
#define G_PIN   2
#define B_PIN   3
#define ALIVE_SECONDS	10
#define uHALF_SECOND	500000

#define THING_UUID			    	"wot:Raspberry2"
#define THING_NAME              	"Raspi3ColourLed"
#define LOCATION_UUID		    	"wot:MyLocation"
#define RGB_HEART               	"wot:3ColourHeartBeatEvent"
#define RGB_HEART_NAME          	"Raspi3ColourAlive"
#define RGB_COLOURACTION        	"wot:ChangeColourAction"
#define RGB_COLOURACTION_NAME   	"ChangeRGBLedColour"
#define RGB_COLOUR_INPUT_TYPE		"wot:ChangeRGBColourInputType"
#define RGB_FREQ_ACTION         	"wot:ChangeFrequencyAction"
#define RGB_FREQ_ACTION_NAME    	"ChangeRGBBlinkFrequency"
#define RGB_FREQ_INPUT_TYPE			"wot:ChangeRGBBlinkInputType"
#define RGB_COLOUR_PROPERTY_UUID	"wot:RGBcolourProperty"
#define RGB_COLOUR_PROPERTY_NAME	"Raspi3ColourProperty"
#define RGB_COLOUR_VALUETYPE		"wot:RGB_colour_JSON"
#define RGB_FREQ_PROPERTY_UUID		"wot:RGBfreqProperty"
#define RGB_FREQ_PROPERTY_NAME		"Raspi3FreqProperty"
#define RGB_FREQ_VALUETYPE			"wot:RGB_freq_JSON"


#define PREFIX_WOT              	"PREFIX wot:<http://www.arces.unibo.it/wot#> "
#define PREFIX_RDF              	"PREFIX rdf:<http://www.w3.org/1999/02/22-rdf-syntax-ns#> "
#define PREFIX_TD               	"PREFIX td:<http://w3c.github.io/wot/w3c-wot-td-ontology.owl#> "
#define PREFIX_DUL              	"PREFIX dul:<http://www.ontologydesignpatterns.org/ont/dul/DUL.owl#> "

#define SEPA_SUBSCRIPTION_ADDRESS	"ws://192.168.0.1:9000/subscribe"
#define SEPA_UPDATE_ADDRESS			"http://192.168.0.1:8000/update"

#define HIGH						1
#define LOW							0
#define KEEP_OLD_VALUE				-1

typedef struct rgbf {
    int r,g,b,f;
} rgbf;

volatile sig_atomic_t alive=1,new_data=0;
int blink_pid;
int pipeFD[2];
pSEPA_subscriber subClient;
rgbf newData;

void HeartBeatHandler(int sig) {
    signal(sig, SIG_IGN);
    alive = 0;
    signal(SIGINT, SIG_DFL);
}

void BlinkHandler(int sig) {
	signal(sig, SIG_IGN);
	read(pipeFD[0],&newData,sizeof(rgbf));
	logD("BlinkHandler run");
	new_data = 1;
	signal(SIGUSR1,BlinkHandler);
}

void blink_process() {
	int data_read;
    rgbf input={.r=LOW,.g=LOW,.b=LOW,.f=LOW};
    while (1) {
        if (new_data) {
			logD("Got new values! r=%d,g=%d,b=%d,f=%d\n",newData.r,newData.g,newData.b,newData.f);
			new_data = 0;
			if (newData.r!=KEEP_OLD_VALUE) input.r=newData.r;
			if (newData.g!=KEEP_OLD_VALUE) input.g=newData.g;
			if (newData.b!=KEEP_OLD_VALUE) input.b=newData.b;
			if (newData.f!=KEEP_OLD_VALUE) input.f=newData.f;
		}
		if (input.f>0) {
			if (input.r==HIGH) digitalWrite(R_PIN,HIGH);
			if (input.g==HIGH) digitalWrite(G_PIN,HIGH);
			if (input.b==HIGH) digitalWrite(B_PIN,HIGH);
			usleep(uHALF_SECOND/input.f);
			if (input.r==HIGH) digitalWrite(R_PIN,LOW);
			if (input.g==HIGH) digitalWrite(G_PIN,LOW);
			if (input.b==HIGH) digitalWrite(B_PIN,LOW);
			usleep(uHALF_SECOND/input.f);
		}
		else {
			digitalWrite(R_PIN,input.r);
			digitalWrite(G_PIN,input.g);
			digitalWrite(B_PIN,input.b);
			pause();
		}
    }
}

void changeColorRequestNotification(sepaNode * added,int addedlen,sepaNode * removed,int removedlen) {
	int i,o;
	char updateSPARQL[1000];
	rgbf newColour = {.r=KEEP_OLD_VALUE,.g=KEEP_OLD_VALUE,.b=KEEP_OLD_VALUE,.f=KEEP_OLD_VALUE};
	if (added!=NULL) {
		if (addedlen>1) printf("%d new request detected. On the screen only the last will be shown.\n",addedlen);
		else printf("New request detected!\n!");
		for (i=0; i<addedlen; i++) {
			if (!strcmp(added[i].bindingName,"value")) {
				sscanf(added[i].value,"{\\\"r\\\":%d,\\\"g\\\":%d,\\\"b\\\":%d}",&(newColour.r),&(newColour.g),&(newColour.b));
				write(pipeFD[1],&newColour,sizeof(rgbf));
				kill(blink_pid,SIGUSR1);

                // updates on the sepa the property value
                sprintf(updateSPARQL,PREFIX_WOT PREFIX_RDF PREFIX_DUL PREFIX_TD "DELETE { " RGB_COLOUR_VALUETYPE " dul:hasDataValue ?oldValue} INSERT { " RGB_COLOUR_VALUETYPE " dul:hasDataValue '{\"r\":%d,\"g\":%d,\"b\":%d}'} WHERE { " RGB_COLOUR_PROPERTY_UUID " rdf:type td:Property. " RGB_COLOUR_PROPERTY_UUID " td:isWritable 'true'. " RGB_COLOUR_PROPERTY_UUID " td:hasValueType " RGB_COLOUR_VALUETYPE " }",newColour.r,newColour.g,newColour.b);
                o=kpProduce(updateSPARQL,SEPA_UPDATE_ADDRESS,NULL);
				if (o!=HTTP_200_OK) logE("Property " RGB_COLOUR_PROPERTY_UUID " update error\n");
            }
		}
		//fprintfSepaNodes(stdout,added,addedlen,"changeColorRequestNotification ");
		freeSepaNodes(added,addedlen);
	}
	printf("\n");
}

void changeFrequencyRequestNotification(sepaNode * added,int addedlen,sepaNode * removed,int removedlen) {
	int i,o;
	char updateSPARQL[1000];
	rgbf newFrequency = {.r=-1,.g=-1,.b=-1,.f=-1};
	if (added!=NULL) {
		if (addedlen>1) printf("%d new request detected. On the screen only the last will be shown.\n",addedlen);
		else printf("New request detected!\n!");
		for (i=0; i<addedlen; i++) {
			if (!strcmp(added[i].bindingName,"value")) {
				sscanf(added[i].value,"{\\\"frequency\\\":%d}",&(newFrequency.f));
				write(pipeFD[1],&newFrequency,sizeof(rgbf));
				kill(blink_pid,SIGUSR1);
				
				// updates on the sepa the property value
                sprintf(updateSPARQL,PREFIX_WOT PREFIX_RDF PREFIX_DUL PREFIX_TD "DELETE { " RGB_FREQ_VALUETYPE " dul:hasDataValue ?oldValue} INSERT { " RGB_FREQ_VALUETYPE " dul:hasDataValue '{\"frequency\":%d}'} WHERE { " RGB_FREQ_PROPERTY_UUID " rdf:type td:Property. " RGB_FREQ_PROPERTY_UUID " td:isWritable 'true'. " RGB_FREQ_PROPERTY_UUID " td:hasValueType " RGB_FREQ_VALUETYPE " }",newFrequency.f);
                o=kpProduce(updateSPARQL,SEPA_UPDATE_ADDRESS,NULL);
				if (o!=HTTP_200_OK) logE("Property " RGB_FREQ_PROPERTY_UUID " update error\n");
            }
		}
		//fprintfSepaNodes(stdout,added,addedlen,"changeFrequencyRequestNotification ");
		freeSepaNodes(added,addedlen);
	}
	printf("\n");
}

int main(int argc, char **argv) {
	int o;
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
	o=kpProduce(PREFIX_WOT PREFIX_RDF PREFIX_DUL PREFIX_TD "DELETE {" THING_UUID " wot:isDiscoverable ?oldDiscoverable} INSERT {" THING_UUID " rdf:type td:Thing. " THING_UUID " td:hasName '" THING_NAME "'. " THING_UUID " wot:isDiscoverable 'true'} WHERE {OPTIONAL{" THING_UUID " rdf:type td:Thing. " THING_UUID " wot:isDiscoverable ?oldDiscoverable}}",SEPA_UPDATE_ADDRESS,NULL);
    if (o!=HTTP_200_OK) {
        logE("Thing Description insert update error in " THING_UUID "\n");
        return EXIT_FAILURE;
    }
    // declare Action RGB Colour modifier
    o=kpProduce(PREFIX_WOT PREFIX_RDF PREFIX_DUL PREFIX_TD "INSERT { " THING_UUID " td:hasAction " RGB_COLOURACTION ". " RGB_COLOURACTION " rdf:type td:Action. " RGB_COLOURACTION " td:hasName '" RGB_COLOURACTION_NAME "'. " RGB_COLOURACTION " td:hasInput " RGB_COLOUR_INPUT_TYPE ". " RGB_COLOUR_INPUT_TYPE " rdf:type dul:InformationObject} WHERE { " THING_UUID " rdf:type td:Thing}",SEPA_UPDATE_ADDRESS,NULL);
    if (o!=HTTP_200_OK) {
        logE("Action " RGB_COLOURACTION_NAME " insert error\n");
        return EXIT_FAILURE;
    }
     // declare Action RGB frequency modifier
    o=kpProduce(PREFIX_WOT PREFIX_RDF PREFIX_DUL PREFIX_TD "INSERT { " THING_UUID " td:hasAction " RGB_FREQ_ACTION ". " RGB_FREQ_ACTION " rdf:type td:Action. " RGB_FREQ_ACTION " td:hasName '" RGB_FREQ_ACTION_NAME "'. " RGB_FREQ_ACTION " td:hasInput " RGB_FREQ_INPUT_TYPE ". " RGB_FREQ_INPUT_TYPE " rdf:type dul:InformationObject} WHERE { " THING_UUID " rdf:type td:Thing}",SEPA_UPDATE_ADDRESS,NULL);
    if (o!=HTTP_200_OK) {
        logE("Action " RGB_FREQ_ACTION_NAME " insert error\n");
        return EXIT_FAILURE;
    }

    // declare Event HeartBeat
	o=kpProduce(PREFIX_WOT PREFIX_RDF PREFIX_DUL PREFIX_TD "INSERT { " THING_UUID " td:hasEvent " RGB_HEART ". " RGB_HEART " rdf:type td:Event. " RGB_HEART " td:hasName '" RGB_HEART_NAME "'} WHERE { " THING_UUID " rdf:type td:Thing}",SEPA_UPDATE_ADDRESS,NULL);
	if (o!=HTTP_200_OK) {
		logE("Event " RGB_HEART_NAME " insert error\n");
		return EXIT_FAILURE;
	}

	// declare Property color
	o=kpProduce(PREFIX_WOT PREFIX_RDF PREFIX_DUL PREFIX_TD "INSERT { " THING_UUID " td:hasProperty " RGB_COLOUR_PROPERTY_UUID ". " RGB_COLOUR_PROPERTY_UUID " rdf:type td:Property. " RGB_COLOUR_PROPERTY_UUID " td:hasName '" RGB_COLOUR_PROPERTY_NAME "'. " RGB_COLOUR_PROPERTY_UUID " td:hasStability '-1'. " RGB_COLOUR_PROPERTY_UUID " td:isWritable 'true'. " RGB_COLOUR_PROPERTY_UUID " td:hasValueType " RGB_COLOUR_VALUETYPE ". " RGB_COLOUR_VALUETYPE " rdf:type wot:Valuetype. " RGB_COLOUR_VALUETYPE " dul:hasDataValue '{\"r\":0,\"g\":0,\"b\":0}' } WHERE {" THING_UUID " rdf:type td:Thing}"
				,SEPA_UPDATE_ADDRESS,NULL);
	if (o!=HTTP_200_OK) {
		logE("Property " RGB_COLOUR_PROPERTY_NAME " insert error\n");
		return EXIT_FAILURE;
	}
	
	// declare Property frequency
	o=kpProduce(PREFIX_WOT PREFIX_RDF PREFIX_DUL PREFIX_TD "INSERT { " THING_UUID " td:hasProperty " RGB_FREQ_PROPERTY_UUID ". " RGB_FREQ_PROPERTY_UUID " rdf:type td:Property. " RGB_FREQ_PROPERTY_UUID " td:hasName '" RGB_FREQ_PROPERTY_NAME "'. " RGB_FREQ_PROPERTY_UUID " td:hasStability '-1'. " RGB_FREQ_PROPERTY_UUID " td:isWritable 'true'. " RGB_FREQ_PROPERTY_UUID " td:hasValueType " RGB_FREQ_VALUETYPE ". " RGB_FREQ_VALUETYPE " rdf:type wot:Valuetype. " RGB_FREQ_VALUETYPE " dul:hasDataValue '{\"frequency\":0}' } WHERE {" THING_UUID " rdf:type td:Thing}"
				,SEPA_UPDATE_ADDRESS,NULL);
	if (o!=HTTP_200_OK) {
		logE("Thing Description " RGB_FREQ_PROPERTY_NAME " insert error\n");
		return EXIT_FAILURE;
	}
	
	// TODO append target properties!
	o=kpProduce(PREFIX_WOT PREFIX_RDF PREFIX_DUL PREFIX_TD "INSERT { " RGB_COLOURACTION " td:forProperty " RGB_COLOUR_PROPERTY_UUID "} WHERE {{{" RGB_COLOURACTION " rdf:type td:Action} UNION {" RGB_COLOURACTION " rdf:type td:Event}}. " RGB_COLOUR_PROPERTY_UUID " rdf:type td:Property}",SEPA_UPDATE_ADDRESS,NULL);
	if (o!=HTTP_200_OK) {
		logE("Append forProperty to " RGB_COLOURACTION " error\n");
		return EXIT_FAILURE;
	}
	
	o=kpProduce(PREFIX_WOT PREFIX_RDF PREFIX_DUL PREFIX_TD "INSERT { " RGB_FREQ_ACTION " td:forProperty " RGB_FREQ_PROPERTY_UUID "} WHERE {{{" RGB_FREQ_ACTION " rdf:type td:Action} UNION {" RGB_FREQ_ACTION " rdf:type td:Event}}. " RGB_FREQ_PROPERTY_UUID " rdf:type td:Property}",SEPA_UPDATE_ADDRESS,NULL);
	if (o!=HTTP_200_OK) {
		logE("Append forProperty to " RGB_FREQ_ACTION " error\n");		
		return EXIT_FAILURE;
	}

    // creation of blink process, communicating with
    if (pipe(pipeFD)) {
        perror("Pipe creation error - ");
        return EXIT_FAILURE;
    }
    blink_pid = fork();
    if (!blink_pid) {
        // child process
        close(pipeFD[1]);
        signal(SIGUSR1,BlinkHandler);
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
    subClient = sepa_subscriber_init();
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
    sepa_setSubscriptionHandlers(changeFrequencyRequestNotification,NULL,&freq_action_subscription);
    fprintfSubscriptionParams(stdout,freq_action_subscription);
    kpSubscribe(&freq_action_subscription);


    signal(SIGINT, HeartBeatHandler);
    // HeartBeat continuous loop
    while (alive) {
        sprintf(lcdHBInstance,"wot:3ColourHeartBeatEventInstance%u",count);
        sprintf(HBEventUpdate,
                PREFIX_WOT PREFIX_RDF PREFIX_DUL PREFIX_TD "DELETE { " RGB_HEART " wot:hasInstance ?oldInstance. ?oldInstance rdf:type wot:EventInstance. ?oldInstance wot:hasTimeStamp ?eOldTimeStamp} INSERT { " RGB_HEART " wot:hasInstance %s. %s rdf:type wot:EventInstance. %s wot:hasTimeStamp ?time} WHERE {BIND(now() AS ?time) . " RGB_HEART " rdf:type td:Event. OPTIONAL { " RGB_HEART " wot:hasInstance ?oldInstance. ?oldInstance rdf:type wot:EventInstance. ?oldInstance wot:hasTimeStamp ?eOldTimeStamp}}",
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

	return EXIT_SUCCESS;
}
