# Demo for the WoT IG/WG F2F Meeting (9-13 July 2017 in Dusseldorf) PlugFest

# Things list
1. [RFID Reader](https://github.com/arces-wot/WOT-Demo/blob/master/README.md#rfid-reader)
2. [32 Chars LCD Display](https://github.com/arces-wot/WOT-Demo/blob/master/README.md#32-chars-lcd-display)
3. [Reed sensor](https://github.com/arces-wot/WOT-Demo/blob/master/README.md#reed-sensor)
4. [RGB blinking LED](https://github.com/arces-wot/WOT-Demo/blob/master/README.md#rgb-blinking-led)

# Web Things - Templates and Thing Descriptions

## RFID Reader ##
### ID Card (according to W3C template)

* **Name:** ARCES RFID Reader
* **Picture:** http://www.lab-id.com/wordpress/wp-content/uploads/2016/03/KITNLO.pdf

![](https://github.com/arces-wot/WOT-Demo/blob/master/images/RFID.png)
* **Logo:** 
* **Hardware:** LabID RFID Reader 13,56 MHz (ISO 14443, ISO 15693)
* **Software:** Java
* **WoT Functions**
  * __Role:__ client
  * __Protocols:__ HTTP
  * __Encodings:__ UTF-8
  * __Discovery:__ can be discovered both locally or remotely through the discovery mechanism provided by SEPA
  * __Application Logic:__ communicates the reading of tags in the reader range
* __Textual description__: This Web Thing performs updates on changes of the RFID tags within the RF field of the reader.

### Thing Description

```json
{
  "@context": {
    "wot": "http://wot.arces.unibo.it/sepa#",
    "td": "http://www.w3.org/ns/td#"
  },
  "@type": "td:Thing",
  "name": "RFID Reader",
  "interactions": [
    {
      "@type": [
        "td:Event",
        "wot:RFIDReaderPing"
      ],
      "name": "RFID Reader is alive even"
    },
    {
      "@type": [
        "td:Event",
        "wot:TagsPollChanged"
      ],
      "name": "TagsPollChanged",
      "outputData": {
        "valueType": {
          "type": "string"
        }
      }
    }
  ]
}
```

## 32 Chars LCD Display
### ID Card (according to W3C template)

* **Name:** ARCES_32char
* **Picture:** https://www.sparkfun.com/datasheets/LCD/HD44780.pdf

![](https://github.com/arces-wot/WOT-Demo/blob/master/images/LCD.jpg)
* **Logo:** 
* **Hardware:** RaspberryPi3 + HD44780 Display
* **Software:** C
* **WoT Functions**
  * __Role:__ server
  * __Protocols:__ HTTP/Websocket
  * __Encodings:__ UTF-8
  * __Discovery:__ discovery through SPARQL query/subscription on SEPA
  * __Application Logic:__ clients may program this display according to its Thing Description
* __Textual description__: This is a programmable display that can be discovered and programmed through its Thing Description mapped into a SPARQL Event Processing Architecture (SEPA). 

### Thing Description

```json
{
  "@context": {
    "wot": "http://wot.arces.unibo.it/sepa#",
    "td": "http://www.w3.org/ns/td#"
  },
  "@type": "td:Thing",
  "name": "ARCES_32char",
  "interactions": [
    {
      "@type": [
        "td:Event",
        "wot:LCDHeartBeatEvent"
      ],
      "name": "Raspi16x2LCDAlive"
    },
    {
      "@type": [
        "td:Action",
        "wot:LCDWriteAction"
      ],
      "name": "Raspi16x2LCD_Write",
      "inputData": {
        "valueType": {
          "type": "string"
        }
      }
    }
  ]
}
```
## RGB blinking led
### ID Card (according to W3C template)

* **Name:** ARCES_RGB_Led
* **Picture:** 

![](https://github.com/arces-wot/WOT-Demo/blob/master/images/LED.jpg)
* **Logo:** 
* **Hardware:** RaspberryPi3 + RGB Led Ky-009
* **Software:** C
* **WoT Functions**
  * __Role:__ servient
  * __Protocols:__ HTTP/Websocket
  * __Encodings:__ UTF-8
  * __Discovery:__ discovery through SPARQL query/subscription on SEPA
  * __Application Logic:__ clients may program this LED according to its Thing Description to set colour and blinking frequency
* __Textual description__: This is a programmable RGB Led. It can be discovered and programmed through its Thing Description mapped into a SPARQL Event Processing Architecture (SEPA). Colour and blinking frequency are its programmable parameters.
 
### Thing Description 
 
```json
{
  "@context": {
    "wot": "http://wot.arces.unibo.it/sepa#",
    "td": "http://www.w3.org/ns/td#"
  },
  "@type": "td:Thing",
  "name": "ARCES_RGB_Led",
  "interactions": [
    {
      "@type": [
        "td:Event",
        "wot:3ColourHeartBeatEvent"
      ],
      "name": "Raspi3ColourAlive"
    },
    {
      "@type": [
        "td:Action",
        "wot:ChangeColourAction"
      ],
      "name": "ChangeRGBLedColour",
      "inputData": {
        "valueType": {
          "type": "object",
          "properties": {
            "r": {
              "type": "integer",
              "minimum": 0,
              "maximum": 1
            },
            "g": {
              "type": "integer",
              "minimum": 0,
              "maximum": 1
            },
            "b": {
              "type": "integer",
              "minimum": 0,
              "maximum": 1
            }
          },
          "required": [
            "r",
            "g",
            "b"
          ]
        }
      }
    },
    {
      "@type": [
        "td:Action",
        "wot:ChangeFrequencyAction"
      ],
      "name": "ChangeRGBBlinkFrequency",
      "inputData": {
        "valueType": {
          "type": "object",
          "properties": {
            "frequency": {
              "type": "integer",
              "minimum": 0
            }
          },
          "required": [
            "frequency"
          ]
        }
      }
    },
    {
      "@type": [
        "td:Property",
        "wot:RGBcolourProperty"
      ],
      "name": "Raspi3ColourProperty",
      "outputData": {
        "valueType": {
          "type": "object",
          "properties": {
            "r": {
              "type": "integer",
              "minimum": 0,
              "maximum": 1
            },
            "g": {
              "type": "integer",
              "minimum": 0,
              "maximum": 1
            },
            "b": {
              "type": "integer",
              "minimum": 0,
              "maximum": 1
            }
          }
        }
      },
      "writable": true,
      "stability": -1
    },
    {
      "@type": [
        "td:Property",
        "wot:RGBfreqProperty"
      ],
      "name": "Raspi3FreqProperty",
      "outputData": {
        "valueType": {
          "type": "object",
          "properties": {
            "frequency": {
              "type": "integer",
              "minimum": 0
            }
          },
          "required": [
            "frequency"
          ]
        }
      },
      "writable": true,
      "stability": -1
    }
  ]
}
```

## Reed Sensor
### ID Card (according to W3C template)

* **Name:** ARCES Reed Sensor
* **Picture:**

![](https://github.com/arces-wot/WOT-Demo/blob/master/images/REED.png)
* **Logo:** 
* **Hardware:** LoLin V3 (ESP8266) + Reed Sensor KY-025
* **Software:** C firmware
* **WoT Functions**
  * __Role:__ client
  * __Protocols:__ HTTP
  * __Encodings:__ UTF-8
  * __Discovery:__ discovery through SPARQL query/subscription on SEPA
  * __Application Logic:__ communicates the reading of its reed sensor
* __Textual description__: This Web Thing is discoverable through a SPARQL Event Processing Architecture (SEPA) and exploits it to communicate the value sensed by its reed sensor (true/false).

### Thing Description

```
{
    "@context": {
	"wot": "http://wot.arces.unibo.it/sepa#",
	"td": "http://www.w3.org/ns/td#"
    },
    "@type": "td:Thing",
    "name": "Reed Sensor",
    "interactions": [
	{
	    "@type": [
		"td:Event",
		"wot:ReedSensorHeartbeatEvent"
	    ],
	    "name": "Reed Sensor Heartbeat"
	},
	{
	    "@type": [
		"td:Event",
		"wot:ReedSensorValueChangedEvent"
	    ],
	    "name": "Reed Sensor Value Changed",
	    "outputData": {
		"valueType": {
		    "type": "boolean"
		}
	    }
	},
      	{
	    "@type": [
		"td:Property",
		"wot:ReedSensorValueProperty"
	    ],
	    "name": "Reed Sensor Value Property",
	    "outputData": {
		"valueType": {
		    "type": "boolean",
		}
	    },
	    "writable": false,
	    "stability": -1
	}
    ]
}
```
