#ifndef YUMMY_LIFE_INCLUDED
#define YUMMY_LIFE_INCLUDED

#include "minorGems/game/game.h"

// Returns a translation of the key if avaliable, otherwise returns inDefault
const char* translateWithDefault(const char* inTranslationKey, const char* inDefault = nullptr);

class YummyLife {
    public:
        static void drawLeaderboardName(doublePair pos);
};

#endif