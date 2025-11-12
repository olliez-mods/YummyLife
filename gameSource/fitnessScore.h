#include "minorGems/game/doublePair.h"


void initFitnessScore();

void freeFitnessScore();


void triggerFitnessScoreUpdate();

void triggerFitnessScoreDetailsUpdate();


// false if either have been triggered and result not ready yet
char isFitnessScoreReady();
// NULL if not ready
const char * getLeaderboardName();

// YummyLife: Call from outside to step the request when not drawing fitness score
void stepActiveRequest();

// These draw nothing if latest data (after last trigger) not ready yet

void drawFitnessScore( doublePair inPos, char inMoreDigits = false );


// inSkip controls paging through list 
void drawFitnessScoreDetails( doublePair inPos, int inSkip );

int getMaxFitnessListSkip();

char canFitnessScroll();



// returns true if using
char usingFitnessServer();
