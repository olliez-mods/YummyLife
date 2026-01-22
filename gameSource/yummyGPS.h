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
    struct StatueLocation {
        int x;
        int y;
        const char* name;
        const char* clothing;
        const char* finalWords;
    };

    struct Well {
        int x;
        int y;
        bool sentToServer = false;
        bool checkedAgainstGlobal = false;
        Well(int x, int y) { this->x = x; this->y = y; }
        bool operator==(const Well& other) const {
            return (x == other.x) && (y == other.y);
        }
    };

    static bool enabled;

    static LivingLifePage* livingLifePage;
    static void step();
    static void onBirth(LivingLifePage* inLivingLifePage);

    static std::vector<Well> foundWells; // Keeps track of wells found in the world (for sending to Phex)
    static std::vector<Well> globalWells; // Server tells us global positions of last X most recent wells (by name/family)

    static Well* getFoundWell(int x, int y);
    static void checkNewFoundWell(int x, int y);

    static void scanWorld(); // Scans surroundings for wells and grid markers
    static void createYRange(); // Create Y range guesses based on biome bounds

    // Checks objects for wells, and biomes for specific biome types
    static void checkObject(int &obj, int x, int y);
    static void checkBiome(int biome, int y);

    static void sendStatueRequest(int relX, int relY);
    static void onStatueReceived(int birthRelX, int birthRelY, int displayID, const char* name, const char* clothing, const char* finalWords);

    // Override
    static void setGlobalBirth(int x, int y);
    static bool getGlobalBirth(int &x, int &y);

    static void createGPSHomeMarker();

    static void testWellLocation(int x, int y); // Called for a well we see in relative terms, tested against known global wells
    
    static std::string getStatusString();

    // Coord system
    static void loadSavedCoords();
    static void writeSavedCoords();
    static void onHomeLocationChange(int x, int y, int type, char name);

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

    // We have tested locations up to this many steps (in WORLD_GRID units)
    static int xScanCount;

    static int stepCount;
    static time_t birthTimeoutStart;

    static bool triggerSaveProcess;
    static std::vector<SavedCoord> savedCoords;
};

#endif