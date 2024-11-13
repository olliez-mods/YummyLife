#include "yummyLife.h"

#include "fitnessScore.h"

#include "message.h"

const char* translateWithDefault(const char* inTranslationKey, const char* inDefault){
    const char* tResult = translate(inTranslationKey);

    if(inDefault && strcmp(tResult, inTranslationKey) == 0) {
        return inDefault;
    }
    return tResult;
}

// Draws the players leaderboars name (if avaliable) at a given position
void YummyLife::drawLeaderboardName(doublePair pos){
    const char* leaderboardName = getLeaderboardName();
    if(leaderboardName == NULL) return;
    //char* message = new char[strlen(leaderboardName) + 10]; // 10 is for "LB Name: "
    //sprintf(message, "LB Name: %s", leaderboardName);
    drawMessage(leaderboardName, pos);
    //delete[] message;
}