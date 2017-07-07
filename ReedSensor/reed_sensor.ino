#include <ESP8266WiFi.h>

// network settings
char ssid[] = "SepaNetwork";
char sepaHost[] = "192.168.1.100";
int sepaPort = 8000;

// device data
String thingId = ""; 
char thingName[] = "ARCES Reed Sensor";

// Things Properties
char rsPropertyId[] = "wot:ReedSensorValueProperty";
char rsPropertyName[] = "Reed Sensor Value Property";
char rsPropertyValueType[] = "wot:ReedSensorPropertyValueType";

// Thing Events
char rsEventId[] = "wot:ReedSensorValueChangedEvent";
char rsEventName[] = "Reed Sensor Value Changed";
char rsEventValueType[] = "wot:ReedSensorEventValueType";
char hbEventId[] = "wot:Ping";
char hbEventName[] = "Ping";

// sensor data
int sigPin = 14;
int value = 0;
int lastReading = -1;

// heartbeat
int loopCount = 0;
int instanceCounter = 0;

// declare namespaces and other updated-related variables
String ns = String("PREFIX wot:<http://wot.arces.unibo.it/sepa#> PREFIX rdf:<http://www.w3.org/1999/02/22-rdf-syntax-ns#> PREFIX dul:<http://www.ontologydesignpatterns.org/ont/dul/DUL.owl#> PREFIX td:<http://www.w3.org/ns/td#> ");
String td;

// declare a wifi client
WiFiClient client;

void connect(){
  
  // connect to a WI-FI network
  WiFi.persistent(false);
  WiFi.mode(WIFI_OFF);
  WiFi.mode(WIFI_STA);
  WiFi.begin("SepaNetwork");
  while (WiFi.status() != WL_CONNECTED){
    Serial.println("[INFO] Not Connected!");
    delay(1000);    
  }
  Serial.println("[INFO] Connected to SepaNetwork!");
  
}


void setup() {

  // configure serial port
  Serial.begin(9600);  

  // read the mac address and set the Thing ID
  String macAddress = WiFi.macAddress();  
  macAddress.replace(":", "");
  thingId = "wot:" + macAddress;

  // connect to the network
  connect();

  ///////////////////////////////////////////////////////////////////////////
  //
  // put Thing Description into SEPA -- 1
  //
  ///////////////////////////////////////////////////////////////////////////
  
  td = String(ns + "DELETE { " + thingId + " wot:isDiscoverable ?discoverable .  " + thingId + " td:hasName ?oldName .  " + thingId + " wot:hasComponent ?component. ?component rdf:type td:Thing .  " + thingId + " td:hasProperty ?property. ?property td:hasName ?pName. ?property td:hasStability ?pStability. ?property td:isWritable ?pWrite. ?pValueType rdf:type ?pDataType . ?pValueType dul:hasDataValue ?pDataValue .  " + thingId + " td:hasEvent ?event. ?event td:hasName ?eName. ?event td:forProperty ?eProperty .  " + thingId + " td:hasAction ?action. ?action td:hasName ?aName. ?action wot:isAccessibleBy ?aProtocol. ?action td:forProperty ?aProperty} INSERT { " + thingId + " rdf:type td:Thing .  " + thingId + " td:hasName '" + thingName + "'.  " + thingId + " wot:isDiscoverable 'true'} WHERE { OPTIONAL { " + thingId + " rdf:type td:Thing.  " + thingId + " wot:isDiscoverable ?discoverable .  " + thingId + " td:hasName ?oldName} . OPTIONAL { " + thingId + " wot:hasComponent ?component. ?component rdf:type td:Thing} . OPTIONAL { " + thingId + " td:hasProperty ?property. ?property td:hasName ?pName. ?property td:hasStability ?pStability. ?property td:isWritable ?pWrite. ?pValueType rdf:type ?pDataType . ?pValueType dul:hasDataValue ?pDataValue} . OPTIONAL { " + thingId + " td:hasEvent ?event. ?event td:hasName ?eName. OPTIONAL {?event td:forProperty ?eProperty}} . OPTIONAL { " + thingId + " td:hasAction ?action. ?action td:hasName ?aName. ?action wot:isAccessibleBy ?aProtocol. OPTIONAL {?action td:forProperty ?aProperty}}}");   
  Serial.println(td);  
  if (WiFi.status() != WL_CONNECTED){  
    connect();
  }
  if (client.connect(sepaHost, sepaPort)){
    client.println("POST /update HTTP/1.1");
    client.println("Host: 192.168.1.100");
    client.println("Cache-Control: no-cache");
    client.println("Content-Type: application/sparql-update");
    client.println("Accept: application/json");
    client.print("Content-Length: ");
    client.println(td.length());
    client.println();
    client.println(td);  
    client.stop();
  }
  Serial.println(td);    
  Serial.println("INIT_TD put into SEPA");

  ///////////////////////////////////////////////////////////////////////////
  //
  // put Thing Description into SEPA -- 2
  // add reed sensor property
  //
  ///////////////////////////////////////////////////////////////////////////

  td = String(ns + "DELETE {  " + thingId + " td:hasProperty  " + rsPropertyId + " .  " + rsPropertyId + " rdf:type td:Property .  " + rsPropertyId + " td:hasStability ?oldStability.  " + rsPropertyId + " dul:hasDataValue ?oldValue .  " + rsPropertyId + " td:hasName ?oldName .  " + rsPropertyId + " td:isWritable ?oldWritable .  " + rsPropertyId + " td:hasValueType ?oldDataType } INSERT { " + thingId + " td:hasProperty  " + rsPropertyId + " .  " + rsPropertyId + " rdf:type td:Property .  " + rsPropertyId + " td:hasName  '" + rsPropertyName + "' .  " + rsPropertyId + " td:hasStability '-1' .  " + rsPropertyId + " td:isWritable 'false' .  " + rsPropertyId + " td:hasValueType " + rsPropertyValueType + " .  " + rsPropertyId + " dul:hasDataValue 'false' } WHERE { " + thingId + " rdf:type td:Thing . OPTIONAL {  " + thingId + " td:hasProperty  " + rsPropertyId + " .  " + rsPropertyId + " rdf:type td:Property .  " + rsPropertyId + " td:hasStability ?oldStability.  " + rsPropertyId + " dul:hasDataValue ?oldValue .  " + rsPropertyId + " td:hasName ?oldName .  " + rsPropertyId + " td:isWritable ?oldWritable .  " + rsPropertyId + " td:hasValueType ?oldDataType}}");
  if (WiFi.status() != WL_CONNECTED){  
    connect();
  }
  if (client.connect(sepaHost, sepaPort)){
    client.println("POST /update HTTP/1.1");
    client.println("Host: 192.168.1.100");
    client.println("Cache-Control: no-cache");
    client.println("Content-Type: application/sparql-update");
    client.println("Accept: application/json");
    client.print("Content-Length: ");
    client.println(td.length());
    client.println();
    client.println(td);  
    client.stop();
  }
  Serial.println(td);    
  Serial.println("TD_ADD_PROPERTY put into SEPA");

  ///////////////////////////////////////////////////////////////////////////
  //
  // put Thing Description into SEPA -- 3
  // add reed sensor changed event
  //
  ///////////////////////////////////////////////////////////////////////////

  td = String(ns + " DELETE { " + thingId + " td:hasEvent  " + rsEventId + " . " + rsEventId + " rdf:type td:PropertyChangedEvent . " + rsEventId + " td:hasName ?oldName . " + rsEventId + " td:hasDataType ?oldDataType } INSERT { " + thingId + " td:hasEvent  " + rsEventId + " . " + rsEventId + " rdf:type td:Event .  " + rsEventId + " rdf:type td:PropertyChangedEvent . " + rsEventId + " td:hasName '" + rsEventName + "' .  " + rsEventId + " td:hasDataType " + rsEventValueType + " } WHERE { " + thingId + " rdf:type td:Thing . OPTIONAL { " + thingId + " td:hasEvent " + rsEventId + " . " + rsEventId + " rdf:type td:PropertyChangedEvent . " + rsEventId + " td:hasName ?oldName . " + rsEventId + " td:hasDataType ?oldDataType }}");
  if (WiFi.status() != WL_CONNECTED){  
    connect();
  }
  if (client.connect(sepaHost, sepaPort)){
    Serial.println("Sending request");
    client.println("POST /update HTTP/1.1");
    client.println("Host: 192.168.1.100");
    client.println("Cache-Control: no-cache");
    client.println("Content-Type: application/sparql-update");
    client.println("Accept: application/json");
    client.print("Content-Length: ");
    client.println(td.length());
    client.println();
    client.println(td);  
    client.stop();
  }
  Serial.println(td);    
  Serial.println("TD_ADD_PROPERTY_CHANGED_EVENT put into SEPA");

  ///////////////////////////////////////////////////////////////////////////
  //
  // put Thing Description into SEPA -- 4
  // add heartbeat event
  //
  ///////////////////////////////////////////////////////////////////////////

  td = String(ns + " DELETE { " + hbEventId + " td:hasName ?oldName} INSERT { " + thingId + " td:hasEvent  " + hbEventId + " .  " + hbEventId + " rdf:type td:Event.  " + hbEventId + " td:hasName  '" + hbEventName + "' } WHERE { " + thingId + " rdf:type td:Thing . OPTIONAL{ " + thingId + " td:hasEvent  " + hbEventId + " .  " + hbEventId + " rdf:type td:Event.  " + hbEventId + " td:hasName ?oldName}}");
  if (WiFi.status() != WL_CONNECTED){  
    connect();
  } 
  if (client.connect(sepaHost, sepaPort)){
    Serial.println("Sending request");
    client.println("POST /update HTTP/1.1");
    client.println("Host: 192.168.1.100");
    client.println("Cache-Control: no-cache");
    client.println("Content-Type: application/sparql-update");
    client.println("Accept: application/json");
    client.print("Content-Length: ");
    client.println(td.length());
    client.println();
    client.println(td);  
    client.stop();
  }
  Serial.println(td);    
  Serial.println("TD_ADD_EVENT put into SEPA");

  ///////////////////////////////////////////////////////////////////////////
  //
  // put Thing Description into SEPA -- 5
  // bind rs property to rs event
  //
  ///////////////////////////////////////////////////////////////////////////

  td = String(ns + "INSERT {" + rsEventId +" td:forProperty " + rsPropertyId + " } WHERE { {{" + rsEventId +" rdf:type td:Action } UNION {" + rsEventId + " rdf:type td:Event }} . " + rsPropertyId + " rdf:type td:Property }");
  if (WiFi.status() != WL_CONNECTED){  
    connect();
  }
  if (client.connect(sepaHost, sepaPort)){
    Serial.println("Sending request");
    client.println("POST /update HTTP/1.1");
    client.println("Host: 192.168.1.100");
    client.println("Cache-Control: no-cache");
    client.println("Content-Type: application/sparql-update");
    client.println("Accept: application/json");
    client.print("Content-Length: ");
    client.println(td.length());
    client.println();
    client.println(td);  
    client.stop();
  }
  Serial.println(td);    
  Serial.println("TD_APPEND_TARGET_PROPERTY_TO_ACTION_OR_EVENT put into SEPA");

  // initialize hardware
  pinMode(sigPin, INPUT);
  delay(1000);
  
}

void loop() {

  // count the loop to know wether its time to update the timestamp
  loopCount++;
  if (loopCount == 5){

    // reset loop counter
    loopCount = 0;

    // generate timestamp update
    String td = String(ns + " DELETE { " + hbEventId + " wot:hasInstance ?oldInstance. ?oldInstance rdf:type wot:EventInstance. ?oldInstance wot:isGeneratedBy " + thingId + " . ?oldInstance wot:hasTimeStamp ?eOldTimeStamp . " + thingId + " wot:isDiscoverable ?oldstatus } INSERT { " + hbEventId + " wot:hasInstance ?newInstance. ?newInstance wot:isGeneratedBy " + thingId + " . ?newInstance rdf:type wot:EventInstance. ?newInstance wot:hasTimeStamp ?time . " + thingId + " wot:isDiscoverable 'true'} WHERE { " + hbEventId + " rdf:type td:Event. BIND(now() AS ?time) . BIND(IRI(concat('http://wot.arces.unibo.it/sepa#Event_',STRUUID())) AS ?newInstance) . OPTIONAL { " + hbEventId + " wot:hasInstance ?oldInstance. ?oldInstance rdf:type wot:EventInstance. ?oldInstance wot:isGeneratedBy " + thingId + " . ?oldInstance wot:hasTimeStamp ?eOldTimeStamp  . " + thingId + " wot:isDiscoverable ?oldstatus }}");
    if (WiFi.status() != WL_CONNECTED){  
      connect();
    }
    if (client.connect(sepaHost, sepaPort)){     
      client.println("POST /update HTTP/1.1");
      client.println("Host: 192.168.1.100");
      client.println("Cache-Control: no-cache");
      client.println("Content-Type: application/sparql-update");
      client.println("Accept: application/json");
      client.print("Content-Length: ");
      client.println(td.length());
      client.println();
      client.println(td);  
      client.stop();
    }
    Serial.println(td);    
    Serial.println("Generating a timestamp event");        
  };
  
  // read the value
  value = digitalRead(sigPin);
  if ((value == HIGH) && (value != lastReading)){
    lastReading = HIGH;
    Serial.println(HIGH);

    // update data into the SEPA
    String td = String(ns + " DELETE {  " + rsEventId + "  wot:hasInstance ?oldInstance . ?oldInstance rdf:type wot:EventInstance . ?oldInstance wot:isGeneratedBy  " + thingId + "  . ?oldInstance wot:hasTimeStamp ?eOldTimeStamp . ?oldInstance td:hasOutput ?eOldOutput . ?eOldOutput dul:hasDataValue ?oldValue . ?property dul:hasDataValue ?oldValue } INSERT {  " + rsEventId + "  wot:hasInstance ?newInstance . ?newInstance rdf:type wot:EventInstance . ?newInstance wot:isGeneratedBy  " + thingId + "  . ?newInstance wot:hasTimeStamp ?time . ?newInstance td:hasOutput ?eNewOutput . ?eNewOutput dul:hasDataValue 'true' . ?property dul:hasDataValue 'true' } WHERE {  " + rsEventId + "  rdf:type td:Event .  " + rsEventId + "  td:forProperty ?property . ?property rdf:type td:Property . BIND(now() AS ?time) . BIND(IRI(concat('http://wot.arces.unibo.it/sepa#Event_',STRUUID())) AS ?newInstance) . BIND(IRI(concat('http://wot.arces.unibo.it/sepa#Output_',STRUUID())) AS ?eNewOutput) . OPTIONAL {  " + rsEventId + "  wot:hasInstance ?oldInstance . ?oldInstance rdf:type wot:EventInstance . ?oldInstance wot:isGeneratedBy  " + thingId + "  . ?oldInstance wot:hasTimeStamp ?eOldTimeStamp . ?oldInstance td:hasOutput ?eOldOutput . ?eOldOutput dul:hasDataValue ?oldValue . ?property dul:hasDataValue ?oldValue }}");
    if (WiFi.status() != WL_CONNECTED){  
      connect();
    }
    if (client.connect(sepaHost, sepaPort)){
      Serial.println("Sending request");
      client.println("POST /update HTTP/1.1");
      client.println("Host: 192.168.1.100");
      client.println("Cache-Control: no-cache");
      client.println("Content-Type: application/sparql-update");
      client.println("Accept: application/json");
      client.print("Content-Length: ");
      client.println(td.length());
      client.println();
      client.println(td);  
      client.stop();
    }
    Serial.println(td);    
    Serial.println("Pushing true into SEPA");    
    
  }
  else if ((value == LOW) && (value != lastReading)) {
    lastReading = LOW;
    Serial.println(LOW);

    // update data into the SEPA
    String td = String(ns + " DELETE {  " + rsEventId + "  wot:hasInstance ?oldInstance . ?oldInstance rdf:type wot:EventInstance . ?oldInstance wot:isGeneratedBy  " + thingId + "  . ?oldInstance wot:hasTimeStamp ?eOldTimeStamp . ?oldInstance td:hasOutput ?eOldOutput . ?eOldOutput dul:hasDataValue ?oldValue . ?property dul:hasDataValue ?oldValue } INSERT {  " + rsEventId + "  wot:hasInstance ?newInstance . ?newInstance rdf:type wot:EventInstance . ?newInstance wot:isGeneratedBy  " + thingId + "  . ?newInstance wot:hasTimeStamp ?time . ?newInstance td:hasOutput ?eNewOutput . ?eNewOutput dul:hasDataValue 'false' . ?property dul:hasDataValue 'false' } WHERE {  " + rsEventId + "  rdf:type td:Event .  " + rsEventId + "  td:forProperty ?property . ?property rdf:type td:Property . BIND(now() AS ?time) . BIND(IRI(concat('http://wot.arces.unibo.it/sepa#Event_',STRUUID())) AS ?newInstance) . BIND(IRI(concat('http://wot.arces.unibo.it/sepa#Output_',STRUUID())) AS ?eNewOutput) . OPTIONAL {  " + rsEventId + "  wot:hasInstance ?oldInstance . ?oldInstance rdf:type wot:EventInstance . ?oldInstance wot:isGeneratedBy  " + thingId + "  . ?oldInstance wot:hasTimeStamp ?eOldTimeStamp . ?oldInstance td:hasOutput ?eOldOutput . ?eOldOutput dul:hasDataValue ?oldValue . ?property dul:hasDataValue ?oldValue }}");
    if (WiFi.status() != WL_CONNECTED){  
      connect();
    }
    if (client.connect(sepaHost, sepaPort)){
      Serial.println("Sending request");
      client.println("POST /update HTTP/1.1");
      client.println("Host: 192.168.1.100");
      client.println("Cache-Control: no-cache");
      client.println("Content-Type: application/sparql-update");
      client.println("Accept: application/json");
      client.print("Content-Length: ");
      client.println(td.length());
      client.println();
      client.println(td);  
      client.stop();
    }
    Serial.println(td);    
    Serial.println("Pushing false into SEPA");    
    
  }
   
  // delay
  delay(1000); 
  
}
