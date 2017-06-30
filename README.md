# Demo for the WoT IG/WG F2F Meeting (9-13 July 2017 in Dusseldorf) PlugFest

# Things list
1. RFID Reader (http://www.lab-id.com/wordpress/wp-content/uploads/2016/03/KITNLO.pdf)
2. Display
3. REED
4. Multicolor LED

# Web Thing Descriptions

## RFID Reader
```json
{
  "@context":
  {
     "wot": "http://wot.arces.unibo.it/sepa#",
     "td": "http://www.w3.org/ns/td#"
  },
  "@type": "td:Thing",
  "name": "RFID Reader",
  "interactions": [
    {
      "@type": ["td:Event","wot:Ping"],
      "name": "Ping"
    },
    {
      "@type": ["td:Event","wot:TagsPollChanged"],
      "name": "TagsPollChanged",
      "outputData": {"valueType": { "type": "string" }}
    }
  ]
}
```

## Reed Sensor
### ID Card (according to W3C template)

* **Name:** ARCES Reed Sensor
* **Picture:**
* **Logo:** 
* **Hardware:** LoLin V3 (ESP8266) + Reed Sensor KY-025
* **Software:** C firmware
* **WoT Functions**
  * __Role:__ client
  * __Protocols:__ HTTP/Websocket
  * __Encodings:__ UTF-8
  * __Discovery:__ discovery through SPARQL query/subscription on SEPA
  * __Application Logic:__ communicates the reading of its reed sensor
* __Textual description__: This Web Thing is discoverable through a SPARQL Event Processing
Architecture (SEPA) and exploits it to communicate the value sensed
by its reed sensor (true/false).

