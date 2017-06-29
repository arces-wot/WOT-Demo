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
  "name": "RFIDReader",
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
