# Demo for the WoT IG/WG F2F Meeting (9-13 July 2017 in Dusseldorf) PlugFest

# Things list
1. RFID Reader (http://www.lab-id.com/wordpress/wp-content/uploads/2016/03/KITNLO.pdf)
2. Display
3. REED
4. Multicolor LED

# Web Thing Descriptions

## RFID Reader

<pre class="example" title="More Capabilities">
{
  "@context":
  {
     "wot": "http://wot.arces.unibo.it/sepa#",
     "rdf": "http://www.w3.org/1999/02/22-rdf-syntax-ns#",
     "dul": "http://www.ontologydesignpatterns.org/ont/dul/DUL.owl#",
     "ire": "http://w3c.github.io/wot/w3c-wot-td-ire.owl#",
     "rdfs": "http://www.w3.org/1999/02/22-rdf-syntax-ns#",
     "td": "http://w3c.github.io/wot/w3c-wot-td-ontology.owl#"
  },
  "@type": "Thing",
  "name": "wot:RFIDReader",
  "interactions": [
    {
      "@type": ["Event","wot:Ping"],
      "name": "ping",
      "outputData": {"valueType": { "type": "string" }}
    },
    {
      "@type": ["Event","wot:RFIDReading"],
      "name": "RFIDReading",
      "outputData": {"valueType": { "type": "string" }}
    }
  ]
}
</pre>
