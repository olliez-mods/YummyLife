// ==============================================================================
//    A lot of this code and logic is inspired by or adapted from Selb/YumLife
//  ==============================================================================

#ifndef YUMMYGPS_INCLUDED
#define YUMMYGPS_INCLUDED

#include <vector>
#include <string>

class LivingLifePage;

class GPS {
public:
    static bool enabled;

    static LivingLifePage* livingLifePage;
    static void step();
    static void onBirth(LivingLifePage* inLivingLifePage);

    static void scanWorld(); // Scans surroundings for wells and grid markers
    static void createYRange(); // Create Y range guesses based on biome bounds

    // Checks objects for wells, and biomes for specific biome types
    static void checkObject(int &obj, int x, int y);
    static void checkBiome(int biome, int y);

    // Override
    static void setGlobalBirth(int x, int y);
    static bool getGlobalBirth(int &x, int &y);

    static void createGPSHomeMarker();
    
    static std::string getStatusString();

    // Coord system
    static void loadSavedCoords();
    static void writeSavedCoords();
    static void onHomeLocationChange(int type);

private:
    enum BiomeType {
        BIOME_ARCTIC,
        BIOME_JUNGLE,
        BIOME_DESERT,
        BIOME_COUNT
    };

    struct BiomeBounds {
        int minY;
        int maxY;
        bool hasMin;
        bool hasMax;
    };

    // When we figure out our global birth position, we load these in from file
    struct SavedCoord {
        char name;
        int x;
        int y;
    };

    static void saveCoordsFromHomePosStack();

    static BiomeBounds biomeBounds[BIOME_COUNT];

    // Have we found our offset to the global grid? (lines us up for statue checks)
    static bool foundGridOffset;
    static int gridOffsetX;
    static int gridOffsetY;

    // Do we know our global Y position yet?
    static bool knowsYPosition;
    static bool hasYRange;
    static int minGlobalYPosition;
    static int maxGlobalYPosition;

    // Global birth position once found
    static int globalBirthX;
    static int globalBirthY;
    static bool foundGlobalBirth;

    static int stepCount;

    static bool triggerSaveProcess;
    static std::vector<SavedCoord> savedCoords;
};

#endif