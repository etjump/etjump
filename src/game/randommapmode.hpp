#ifndef RANDOMMAPMODE_HH
#define RANDOMMAPMODE_HH

class RandomMapMode
{
public:
    // this is called on every server frame. checks for when to inform players about
    // a future map change and when to change the map
    void checkTime(int levelTime);

    // Used to update the set interval
    // returns true, if new interval time was exceeded
    // and one minute was added to the interval
    bool updateInterval(int newInterval);

    RandomMapMode(int startTime,
        int interval, 
        void (*minutesLeft)(int minutes),
        void (*changeMap)()): 
        startTime_(startTime),
        interval_(interval), 
        minutesLeft_(minutesLeft),
        changeMap_(changeMap),
        lastPrintedMinute_(0) {}
    ~RandomMapMode() {}
private:
    // How often to change map
    int interval_;
    // When did we change to this map
    int startTime_;
    // Current time is stored incase we need to update the interval
    int currentTime_;
    // what was the last minute we printed about
    int lastPrintedMinute_;

    // This function is called when N minutes till map change is left
    void(*minutesLeft_)(int minutes);

    // This function is called when 0 is reached
    void(*changeMap_)();
};

#endif