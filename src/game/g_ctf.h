#ifndef G_CTF_H
#define G_CTF_H

extern "C" {
#include "g_local.h"
}

class CTFSystem {
public:
    CTFSystem();
    ~CTFSystem();

    void Init();
    void AlliesScored();
    void AxisScored();

    void StartGame();
    void StopGame();
private:
    void UpdateScoreToPlayers() const;
    int     axisScore;
    int     alliedScore;
    int     axisFlagStatus;
    int     alliedFlagStatus;
    bool    gameIsRunning;
    int     endTime;
};

#endif // G_CTF_H