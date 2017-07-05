package it.arces.wot.statemonitorthing;



import it.unibo.arces.wot.sepa.commons.sparql.ARBindingsResults;
import it.unibo.arces.wot.sepa.commons.sparql.Bindings;
import it.unibo.arces.wot.sepa.commons.sparql.BindingsResults;
import it.unibo.arces.wot.sepa.pattern.ApplicationProfile;
import it.unibo.arces.wot.sepa.pattern.Consumer;
import org.apache.commons.cli.*;

import javax.crypto.BadPaddingException;
import javax.crypto.IllegalBlockSizeException;
import javax.crypto.NoSuchPaddingException;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.net.URISyntaxException;
import java.security.*;
import java.security.cert.CertificateException;
import java.util.NoSuchElementException;

/**
 * Created by reluc on 30/06/2017.
 */
public class StateMonitorThing extends Consumer{

    private final IWatchDog watchDog;

    public StateMonitorThing(ApplicationProfile applicationProfile, IWatchDog watchDog) throws IllegalArgumentException, FileNotFoundException, NoSuchElementException, IOException, UnrecoverableKeyException, KeyManagementException, KeyStoreException, NoSuchAlgorithmException, CertificateException, InvalidKeyException, NullPointerException, ClassCastException, NoSuchPaddingException, IllegalBlockSizeException, BadPaddingException, URISyntaxException {
        super(applicationProfile,"ALL_THINGS");
        this.watchDog = watchDog;
    }

    public static void main(String[] args) throws IOException, CertificateException, NoSuchAlgorithmException, UnrecoverableKeyException, InvalidKeyException, NoSuchPaddingException, BadPaddingException, URISyntaxException, KeyStoreException, IllegalBlockSizeException, KeyManagementException, InterruptedException {
        Options opts = new Options();

        Option jsap = new Option("j","jsap",true,"Use a differten jsap configuration file");
        Option sleep_time = new Option("t","sleep-time",true,"Sleep time in millisec of the watchdog thread. Default: 1000 mills");
        Option alive = new Option("a","alive-time",true,"Define the timeout after which a thing is considered dead. Default: 5000 mills");
        opts.addOption(sleep_time);
        opts.addOption(alive);
        opts.addOption(jsap);

        CommandLineParser parser = new DefaultParser();
        HelpFormatter formatter = new HelpFormatter();
        CommandLine cmd;
        String jsapfile;
        int sleep_time_arg = 0;
        int alive_time_arg = 0;

        try {
           cmd = parser.parse(opts,args);
            jsapfile = cmd.getOptionValue("jsap","td.jsap");
            sleep_time_arg = Integer.parseInt( cmd.getOptionValue("sleep-time","1000"));
            alive_time_arg = Integer.parseInt( cmd.getOptionValue("alive-time","5000"));
        } catch (ParseException | NumberFormatException w) {
            System.out.print("Malformed arguments, see usage");
            formatter.printHelp("utility-name", opts);
            System.exit(1);
            return;
        }


        final ApplicationProfile applicationProfile = new ApplicationProfile(jsapfile);
        final WatchDog watchDog = new WatchDog(applicationProfile, sleep_time_arg,alive_time_arg);
        final StateMonitorThing stateMonitorThing = new StateMonitorThing(applicationProfile, watchDog);

        stateMonitorThing.subscribe(new Bindings());
        watchDog.start();
    }


    @Override
    public void onResults(ARBindingsResults results) {

    }

    @Override
    public void onAddedResults(BindingsResults results) {
            for (Bindings b : results.getBindings()){
                final String thing = b.getBindingValue("thing");
                watchDog.monitorThing(thing);
            }
    }

    @Override
    public void onRemovedResults(BindingsResults results) {
        for (Bindings b : results.getBindings()){
            final String thing = b.getBindingValue("thing");
            watchDog.ignoreThing(thing);
        }
    }

    @Override
    public void onSubscribe(BindingsResults results) {
        onAddedResults(results);
    }

    @Override
    public void onUnsubscribe() {

    }
}
