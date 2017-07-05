package it.arces.wot.statemonitorthing;

/**
 * Created by reluc on 03/07/2017.
 */
public interface IWatchDog {

    void monitorThing(String thingUrl);
    void ignoreThing(String thingUrl);

}
