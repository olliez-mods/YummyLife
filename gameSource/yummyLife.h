#ifndef YUMMY_LIFE_INCLUDED
#define YUMMY_LIFE_INCLUDED

#include <vector>

#include "minorGems/game/game.h"

// Returns a translation of the key if avaliable, otherwise returns inDefault
const char* translateWithDefault(const char* inTranslationKey, const char* inDefault = nullptr);

class YummyLife {
    public:
        static void drawLeaderboardName(doublePair pos);

        class Gallery{
            public:
                static void initGallery(const char* galleryDirPath);
                static void drawGallery(doublePair pos);
                static void setGalleryMaxDimensions(int maxWidth, int maxHeight);
                static void setGalleryMaxDimensions(doublePair maxDimentions);
                static void loadGalleryIndex(int index);
                static int getGalleryImageIndex();
                static int loadNextGalleryImage();
                static int loadPreviousGalleryImage();
                static int loadRandomGalleryImage(int excludeIndex = -1);
                ~Gallery();
        };

        static void cleanUp();
        static void takingScreenshot();


        static void yumEaten(int objectID, int lifeID);
        static std::vector<int> getLastYums(int lifeID);
        static void clearLastYums();

    protected:
        static SoundSpriteHandle screenshotSound;

    private:

        static std::vector<int> lastYums;
        static int lastYumsLifeID;

        static void readLastYumsFile();
        static bool appendLastYumsFile(int itemID);
        static bool setupLastYumsFile(int lifeID);
        static bool deleteLastYumsFile();
};

#endif