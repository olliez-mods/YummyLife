// ==============================================================================
//    A lot of this code and logic is inspired by or adapted from Selb/YumLife
//  ==============================================================================

#include "yummyGPS.h"
#include "hetuwmod.h"
#include "phex.h"
#include "minorGems/game/game.h"

#include <sstream>

#define STEPS_PER_SAVE_CHECK 600 // Every 600 steps, check if we need to save coords

#define WORLD_GRID_SIZE 40

#define STEPS_PER_SCAN 200 // Based on FPS, so not exact time

#define SAVED_COORDS_FILENAME "yummyGPSCoords.txt"

// Biome boundary constants (global Y)
const int ARCTIC_MIN_Y = 220;
const int ARCTIC_MAX_Y = 420;
const int JUNGLE_MIN_Y = -180;
const int JUNGLE_MAX_Y = 19;
const int DESERT_MIN_Y = -382;
const int DESERT_MAX_Y = -182;

// Biome IDs (Since OHOL has its own ids)
const int BIOME_ARCTIC_ID = 4;
const int BIOME_JUNGLE_ID = 6;
const int BIOME_DESERT_ID = 5;

bool GPS::enabled = false;
LivingLifePage* GPS::livingLifePage = NULL;

std::vector<GPS::SavedCoord> GPS::savedCoords;
bool GPS::triggerSaveProcess = false;

GPS::BiomeBounds GPS::biomeBounds[GPS::BIOME_COUNT];

bool GPS::foundGridOffset = false;
int GPS::gridOffsetX = 0;
int GPS::gridOffsetY = 0;

bool GPS::knowsYPosition = false;
bool GPS::hasYRange = false;
int GPS::minGlobalYPosition = 0;
int GPS::maxGlobalYPosition = 0;

bool GPS::foundGlobalBirth = false;
int GPS::globalBirthX = 0;
int GPS::globalBirthY = 0;

int GPS::stepCount = 0;


// Align coordinate down to world grid (assume coord is globaly aligned)
int snapToGrid(int coord) { return (WORLD_GRID_SIZE - (((coord % WORLD_GRID_SIZE) + WORLD_GRID_SIZE) % WORLD_GRID_SIZE)) % WORLD_GRID_SIZE; }

void GPS::setGlobalBirth(int x, int y){
    globalBirthX = x;
    globalBirthY = y;
    foundGlobalBirth = true;

    knowsYPosition = true; // We now know our Y position exactly
    minGlobalYPosition = y;
    maxGlobalYPosition = y;

    HetuwMod::bDrawGPSStatus = false; // Turn off status when we find GPS

    createGPSHomeMarker();

    loadSavedCoords();
    for(auto globalCoord : savedCoords) {
        // coord is in global terms, must convert to relative
        int relX = globalCoord.x - globalBirthX;
        int relY = globalCoord.y - globalBirthY;
        HetuwMod::addHomeLocation(relX, relY, HetuwMod::hpt_custom, globalCoord.name);
        onHomeLocationChange(HetuwMod::hpt_custom);
    }

    Phex::onGlobalBirthSet(x, y);
}

bool GPS::getGlobalBirth(int &x, int &y){
    if(!foundGlobalBirth) return false;
    x = globalBirthX;
    y = globalBirthY;
    return true;
}

void GPS::createGPSHomeMarker(){
    if(!foundGlobalBirth) return;
    // check if one already exists
    for(auto homePos : HetuwMod::homePosStack) 
    if(homePos->type == HetuwMod::hpt_gps) return;
    
    HetuwMod::addHomeLocation(-globalBirthX, -globalBirthY, HetuwMod::hpt_gps);
    HetuwMod::bDrawHomeCords = true; // Enable coords when we find GPS
}

// All wells are on the world grid, but not all world grid markers are wells
// Find both here
void GPS::checkObject(int &obj, int x, int y) {
    if(obj <= 0) return;
    ObjectRecord* objRec = getObject(obj);
    if(objRec == NULL) return;
    if(objRec->isUseDummy) objRec = getObject(objRec->useDummyParent);
    if(objRec == NULL) return;

    int result = 0;
    
    // Check for wells
    if(strstr(objRec->description, "Well") != nullptr) result = 1;
    else if(strstr(objRec->description, "Water Pump") != nullptr) result = 1;
    else if(strstr(objRec->description, "Newcomen Pump") != nullptr) result = 1;
    else if(strstr(objRec->description, "Pump Head") != nullptr) result = 1;

    // Else check for grid markers that are not wells but sit on the grid
    else if(strstr(objRec->description, "Natural Spring") != nullptr) result = 2;
    else if(strstr(objRec->description, "Dry Spring") != nullptr) result = 2;
    else if(strstr(objRec->description, "Mine") != nullptr) result = 2;
    else if(strstr(objRec->description, "Diesel Mining") != nullptr) result = 2;
    else if(strstr(objRec->description, "Iron Vein") != nullptr) result = 2;
    else if(strstr(objRec->description, "Tarry Spot") != nullptr) result = 2;

    if(result == 0) return; // Not a well or grid marker

    if(!foundGridOffset) {
        gridOffsetX = snapToGrid(x);
        gridOffsetY = snapToGrid(y);
        foundGridOffset = true;
        printf("Grid corner at World (%d, %d) (%s) triggered grid offset found\n", x, y, objRec->description);
        printf("GPS Found World Grid Offset: (%d, %d)\n", gridOffsetX, gridOffsetY);
    }

    if(result == 1) {
        // No longer keep track of wells, maybe again later
        printf("GPS Found Well at (%d, %d)\n", x, y);
    }
}

// Expand biome boundaries based on observed biomes
// Allows us to be more accurate in guessing our global Y position
void GPS::checkBiome(int biome, int y) {
    if(biome < 0) return;
    if(knowsYPosition) return; // No need to track anymore
    BiomeType bType;
    if(biome == BIOME_ARCTIC_ID) bType = BIOME_ARCTIC;
    else if(biome == BIOME_JUNGLE_ID) bType = BIOME_JUNGLE;
    else if(biome == BIOME_DESERT_ID) bType = BIOME_DESERT;
    else return; // Not a biome we care about

    BiomeBounds& bounds = biomeBounds[bType];
    if(!bounds.hasMin || y < bounds.minY) {
        bounds.minY = y;
        bounds.hasMin = true;
    }
    if(!bounds.hasMax || y > bounds.maxY) {
        bounds.maxY = y;
        bounds.hasMax = true;
    }
}

void GPS::scanWorld(){
    if(livingLifePage == NULL) return;
    LiveObject* ourLiveObject = livingLifePage->getOurLiveObject();
    if(ourLiveObject == NULL) return;

    int *mapBiomes = livingLifePage->mMapBiomes;
    int *mapObjects = livingLifePage->mMap;
    if(mapBiomes == NULL || mapObjects == NULL) return;

    int MAP_D = livingLifePage->mMapD;

    for(int mapY = 0; mapY < MAP_D; mapY++) {
        for(int mapX = 0; mapX < MAP_D; mapX++) {
            int mapIndex = mapY * MAP_D + mapX;
            int biomeID = mapBiomes[mapIndex];
            int objId = mapObjects[mapIndex];

            int worldX = mapX + livingLifePage->mMapOffsetX - MAP_D / 2;
            int worldY = mapY + livingLifePage->mMapOffsetY - MAP_D / 2;
            if(!foundGridOffset) checkObject(objId, worldX, worldY);
            if(biomeID >= 0) checkBiome(biomeID, worldY);
        }
    }
}

void GPS::createYRange() {
    if(knowsYPosition) return; // Already know

    // Build list of all observed potential boundaries
    struct BoundaryObservation {
        int birthRelativeY;
        int expectedGlobalY;
    };

    hasYRange = false;
    std::vector<BoundaryObservation> boundaries;
    
    // Add observed biome boundaries
    if (biomeBounds[BIOME_DESERT].hasMin) boundaries.push_back({biomeBounds[BIOME_DESERT].minY, DESERT_MIN_Y});
    if (biomeBounds[BIOME_DESERT].hasMax) boundaries.push_back({biomeBounds[BIOME_DESERT].maxY, DESERT_MAX_Y});
    if (biomeBounds[BIOME_JUNGLE].hasMin) boundaries.push_back({biomeBounds[BIOME_JUNGLE].minY, JUNGLE_MIN_Y});
    if (biomeBounds[BIOME_JUNGLE].hasMax) boundaries.push_back({biomeBounds[BIOME_JUNGLE].maxY, JUNGLE_MAX_Y});
    if (biomeBounds[BIOME_ARCTIC].hasMin) boundaries.push_back({biomeBounds[BIOME_ARCTIC].minY, ARCTIC_MIN_Y});
    if (biomeBounds[BIOME_ARCTIC].hasMax) boundaries.push_back({biomeBounds[BIOME_ARCTIC].maxY, ARCTIC_MAX_Y});

    if (boundaries.empty()) return; // No biome data yet
    hasYRange = true; // We have at least some data

    // Check if any two boundaries are at their expected distance
    for (size_t i = 0; i < boundaries.size(); i++) {
        for (size_t j = i + 1; j < boundaries.size(); j++) {
            int observedDist = boundaries[j].birthRelativeY - boundaries[i].birthRelativeY;
            int expectedDist = boundaries[j].expectedGlobalY - boundaries[i].expectedGlobalY;

            if (observedDist == expectedDist) {
                // Found matching pair! We know our exact Y position
                int birthGlobalY = boundaries[i].expectedGlobalY - boundaries[i].birthRelativeY;
                knowsYPosition = true;
                minGlobalYPosition = birthGlobalY;
                maxGlobalYPosition = birthGlobalY;
                printf("GPS Found Exact Y Position: %d\n", birthGlobalY);
                return;
            }
        }
    }

    // Create Y range from all possible boundary interpretations
    std::vector<int> possibleY;
    for (const auto &boundary : boundaries) {
        int birthGlobalY = boundary.expectedGlobalY - boundary.birthRelativeY;
        possibleY.push_back(birthGlobalY);
    }

    // Set range to min/max of all possibilities
    minGlobalYPosition = *std::min_element(possibleY.begin(), possibleY.end());
    maxGlobalYPosition = *std::max_element(possibleY.begin(), possibleY.end());

    printf("GPS Y Range: %d to %d\n", minGlobalYPosition, maxGlobalYPosition);
}

void GPS::step() {
    if(!enabled) return;
    if(livingLifePage == NULL) return;
    stepCount++;

    // Periodically scan the world for new wells and biome data
    if(stepCount % STEPS_PER_SCAN == 0) {
        scanWorld();
        createYRange();
    }

    // Periodically check if we need to save coords
    if(triggerSaveProcess && stepCount % STEPS_PER_SAVE_CHECK == 0) {
        saveCoordsFromHomePosStack();
        triggerSaveProcess = false;
    }

    if(foundGlobalBirth) return; // Already know position

    // GPS logic would go here
}

// Load saved coords from file
void GPS::loadSavedCoords(){
    savedCoords.clear();
    int num = 0;
    std::ifstream inFile(SAVED_COORDS_FILENAME);
    if (inFile.is_open()) {
        std::string line;
        while (std::getline(inFile, line)) {
            try {
            std::istringstream ss(line);
            SavedCoord sc;
            ss >> sc.name >> sc.x >> sc.y;
            savedCoords.push_back(sc);
            num++;
            } catch (...) { printf("GPS Failed to parse saved coord line: %s\n", line.c_str()); }
        }
        inFile.close();
    }
    printf("GPS Loaded %d custom coords from file\n", num);
}

void GPS::onHomeLocationChange(int type){
    if(type != HetuwMod::hpt_custom) return;
    if(!enabled || !foundGlobalBirth) return;
    triggerSaveProcess = true; // Trigger save process on next step lined up with save check
}

void GPS::saveCoordsFromHomePosStack() {
    if(!foundGlobalBirth) {
        printf("GPS Cannot save coords before finding global birth\n");
        throw std::runtime_error("GPS::saveCoordsFromHomePosStack called before finding global birth");
    }
    // Now save updated list as global (since homPos list can have items added before we find GPS)
    savedCoords.clear();
    for(auto homePos : HetuwMod::homePosStack) {
        if(!homePos) {
            printf("GPS Warning: Null homePos in homePosStack during save\n");
            continue;
        }
        if(homePos->type != HetuwMod::hpt_custom) continue;
        int globalX = homePos->x + globalBirthX;
        int globalY = homePos->y + globalBirthY;
        SavedCoord sc;
        sc.name = homePos->c;
        sc.x = globalX;
        sc.y = globalY;
        savedCoords.push_back(sc);
    }
    writeSavedCoords();
}

void GPS::writeSavedCoords(){
    try {
        int num = 0;
        std::ofstream outFile(SAVED_COORDS_FILENAME);
        if (outFile.is_open()) {
            for (const SavedCoord& sc : savedCoords) {
                outFile << sc.name << " " << sc.x << " " << sc.y << "\n";
                num++;
            }
            outFile.close();
        }
        printf("GPS Saved %d custom coords to file\n", num);
    } catch (...) {
        printf("GPS Failed to save coords to file... trying again\n");
        triggerSaveProcess = true; // Try again
    }
}

std::string GPS::getStatusString() {
    if(!enabled) return "GPS DISABLED";

    if(foundGlobalBirth) {
        std::ostringstream ss;
        ss << "FOUND GPS (" << globalBirthX << ", " << globalBirthY << ")";
        return ss.str();
    }

    if(!hasYRange) return "FIND SPECIAL BIOMES";
    if(!foundGridOffset) return "SEARCHING FOR WORLD GRID";

    return "GPS NOT IMPLEMENTED";
}

// Acts as a reset and init
void GPS::onBirth(LivingLifePage* inLivingLifePage){
    livingLifePage = inLivingLifePage;
    stepCount = 0;
    enabled = HetuwMod::bGPSEnabled;

    // Disabled for now until further notice
    enabled = false; // GPS is disabled for now
    printf("GPS is currently disabled\n");

    for (int i = 0; i < BIOME_COUNT; i++) {
        biomeBounds[i].hasMin = false;
        biomeBounds[i].hasMax = false;
        biomeBounds[i].minY = 0;
        biomeBounds[i].maxY = 0;
    }

    foundGridOffset = false;
    gridOffsetX = 0;
    gridOffsetY = 0;

    knowsYPosition = false;
    hasYRange = false;
    minGlobalYPosition = 0;
    maxGlobalYPosition = 0;

    foundGlobalBirth = false;
    globalBirthX = 0;
    globalBirthY = 0;
}
