#include "randommapmode.hpp"

void RandomMapMode::checkTime(int levelTime)
{
    int timeDiff = levelTime - startTime_;

    if (interval_ <= 0)
    {
        interval_ = 1;
    }

    int minutes = interval_ - (timeDiff / (1000 * 60));

    // Store this in case we need to update the interval
    currentTime_ = levelTime;

    if (minutes <= 5)
    {
        if (minutes == 0)
        {
            changeMap_();
        }
        else
        {
            if (lastPrintedMinute_ != minutes)
            {
                minutesLeft_(minutes);
            }
            
            lastPrintedMinute_ = minutes;
        }
    }
}

bool RandomMapMode::updateInterval(int newInterval)
{
    if (newInterval < 1)
    {
        interval_ = 1;
    }
    else
    {
        interval_ = newInterval;
    }
    

    int timeDiff = currentTime_ - startTime_;
    int minutes = interval_ - (timeDiff / (1000 * 60));

    if (minutes <= 0)
    {
        interval_ = minutes + 1;
        return true;
    }

    return false;
}