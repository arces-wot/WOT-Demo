package it.arces.wot.statemonitorthing;

import it.unibo.arces.wot.sepa.commons.sparql.*;
import it.unibo.arces.wot.sepa.pattern.Aggregator;
import it.unibo.arces.wot.sepa.pattern.ApplicationProfile;

import javax.crypto.BadPaddingException;
import javax.crypto.IllegalBlockSizeException;
import javax.crypto.NoSuchPaddingException;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.net.URISyntaxException;
import java.security.*;
import java.security.cert.CertificateException;
import java.util.concurrent.ConcurrentHashMap;

/**
 * Created by reluc on 03/07/2017.
 */
public class WatchDog extends Thread implements IWatchDog{

    private final ApplicationProfile appProfile;
    private final int millisec;
    private final int ping_latency;
    private final ConcurrentHashMap<String,ThingStatusListner> things = new ConcurrentHashMap<>();

    public WatchDog(ApplicationProfile appProfile, int millisec,int ping_latency) throws IllegalArgumentException, UnrecoverableKeyException, KeyManagementException, KeyStoreException, NoSuchAlgorithmException, CertificateException, FileNotFoundException, IOException, URISyntaxException {
        this.appProfile = appProfile;
        this.millisec = millisec;
        this.ping_latency = ping_latency;
    }


    @Override
    public void run() {
        while (true){
            try {
                Thread.sleep(millisec);
                for(ThingStatusListner listner : things.values()){
                    listner.update();
                }
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
    }

    @Override
    public void monitorThing(String thingUrl) {
        try {
            final ThingStatusListner listner = new ThingStatusListner(appProfile, thingUrl);
            Bindings bindings = new Bindings();
            bindings.addBinding("event",new RDFTermURI("wot:Ping"));
            bindings.addBinding("thing",new RDFTermURI(thingUrl));
            listner.subscribe(bindings);
            things.put(thingUrl, listner);
        } catch (IOException e) {
            e.printStackTrace();
        } catch (UnrecoverableKeyException e) {
            e.printStackTrace();
        } catch (KeyManagementException e) {
            e.printStackTrace();
        } catch (KeyStoreException e) {
            e.printStackTrace();
        } catch (NoSuchAlgorithmException e) {
            e.printStackTrace();
        } catch (CertificateException e) {
            e.printStackTrace();
        }  catch (URISyntaxException e) {
            e.printStackTrace();
        } catch (NoSuchPaddingException e) {
            e.printStackTrace();
        } catch (InvalidKeyException e) {
            e.printStackTrace();
        } catch (InterruptedException e) {
            e.printStackTrace();
        } catch (IllegalBlockSizeException e) {
            e.printStackTrace();
        } catch (BadPaddingException e) {
            e.printStackTrace();
        }
    }

    @Override
    public void ignoreThing(String thingUrl) {
        things.remove(thingUrl);
    }

    class ThingStatusListner extends Aggregator {
        private final String thing;
        private long last_ping;
        private boolean updateOnline = false;
        private boolean updateOffilne = true;


        public ThingStatusListner(ApplicationProfile appProfile,String thing) throws IllegalArgumentException, UnrecoverableKeyException, KeyManagementException, KeyStoreException, NoSuchAlgorithmException, CertificateException, FileNotFoundException, IOException, URISyntaxException {
            super(appProfile, "THING_EVENT", "UPDATE_DISCOVER");
            this.thing = thing;
            last_ping = System.currentTimeMillis();
        }

        @Override
        public synchronized void onResults(ARBindingsResults results) {

        }

        @Override
        public void onAddedResults(BindingsResults results) {

        }

        @Override
        public void onRemovedResults(BindingsResults results) {

        }

        @Override
        public void onSubscribe(BindingsResults results) {

        }

        @Override
        public void onUnsubscribe() {

        }

        public synchronized void update(){
            final long currentTime = System.currentTimeMillis();
            if( currentTime - last_ping > ping_latency){
                if( updateOffilne ){
                    Bindings bindings = new Bindings();
                    bindings.addBinding("thing",new RDFTermURI(this.thing));
                    bindings.addBinding("value",new RDFTermLiteral("false"));
                    update(bindings);
                    updateOffilne = false;
                    updateOnline = true;
                }
            }else{
                if(updateOnline){
                    Bindings bindings = new Bindings();
                    bindings.addBinding("thing",new RDFTermURI(this.thing));
                    bindings.addBinding("value",new RDFTermLiteral("true"));
                    update(bindings);
                    updateOnline = false;
                    updateOffilne = true;
                }
            }

        }
    }
}
