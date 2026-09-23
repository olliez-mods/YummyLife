#include "minorGems/game/doublePair.h"


void initAgeControl();

// YummyLife:  converts a real age, as the server counts it, into the age the character art is laid out for
double getDisplayAge( double inRealAge );


// returns 0,0 if inAge is -1
doublePair getAgeHeadOffset( double inAge, doublePair inHeadSpritePos,
                             doublePair inBodySpritePos,
                             doublePair inFrontFootSpritePos );

doublePair getAgeBodyOffset( double inAge, doublePair inBodySpritePos );
