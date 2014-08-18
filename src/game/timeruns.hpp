#ifndef TIMERUNS_HH
#define TIMERUNS_HH

#include <vector>
struct gentity_s;
typedef struct gentity_s gentity_t;

class Timeruns
{
public:
    struct Run
    {
        Run();
        std::string name;
    };
    void initTimeruns();
    bool createNewTimerun();
    const Run* findRun(std::string name);
private:
    std::vector<Run> runs_;
};

#endif