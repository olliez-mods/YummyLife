#include "yummyLife.h"

#include <stdio.h>
#include <fstream>
#include <string>

#include "minorGems/io/file/File.h"

#include "hetuwmod.h"

#include "fitnessScore.h"
#include "soundBank.h"
#include "message.h"

#define LAST_YUMS_FILE "lastYums.txt"
#define ONLY_ADD_UNIQUE_YUMS true

SoundSpriteHandle YummyLife::screenshotSound;

std::vector<int> YummyLife::lastYums;
int YummyLife::lastYumsLifeID = -1; // Life ID's can't be below 0 (or 1 maybe)

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

// Final cleanup, make sure everything is freed, etc.
void YummyLife::cleanUp() {
    if(screenshotSound != NULL)
        freeSoundSprite(screenshotSound);
}

void YummyLife::takingScreenshot() {
    if(screenshotSound == NULL)
        screenshotSound = loadSoundSprite( "otherSounds", "tutorialChime.aiff" );

    playSoundSprite(screenshotSound, 0.1 * getSoundEffectsLoudness());
}


void YummyLife::yumEaten(int objectID, int lifeID){
    if(!HetuwMod::bStoreEatenYums) return;

    // Cleanup, then start a new file if we need to
    if(lastYumsLifeID == -1 || lastYumsLifeID != lifeID) {
        clearLastYums();
        lastYumsLifeID = lifeID;

        bool result = setupLastYumsFile(lifeID);
        if(!result) {
            clearLastYums();
            return;
        }
    }

    // Don't add food to text file if it's not unique
    if(ONLY_ADD_UNIQUE_YUMS)
    for(unsigned int i = 0; i < lastYums.size(); i++)
        if(objectID == lastYums[i]) return;

    appendLastYumsFile(objectID);
    lastYums.push_back(objectID);
}

std::vector<int> YummyLife::getLastYums(int lifeID){
    if(!HetuwMod::bStoreEatenYums) return {};

    // lastYums.txt hasn't been read yet
    if(lastYumsLifeID == -1) {
        readLastYumsFile();
    }

    // No info from lastYums.txt (it's still -1)
    if(lastYumsLifeID == -1) return {};

    // The stashed lifeID is different then our current one, wiping information
    if(lastYumsLifeID != lifeID) {
        clearLastYums();
        return {};
    }

    return lastYums;
}

void YummyLife::clearLastYums(){
    if(!HetuwMod::bStoreEatenYums) return;

    lastYumsLifeID = -1;
    lastYums.clear();
    deleteLastYumsFile();
}

void YummyLife::readLastYumsFile() {
    std::ifstream lyFile(LAST_YUMS_FILE);

    if(!lyFile) {
        printf("'%s' was not found\n", LAST_YUMS_FILE);
        return;
    }

    std::string firstLine;
    if(!std::getline(lyFile, firstLine)) {
        printf("No first line in '%s', deleting...\n", LAST_YUMS_FILE);
        lyFile.close();
        deleteLastYumsFile();
        return;
    }

    const std::string firstLinePrefix = "lifeid:";
    if(firstLine.rfind(firstLinePrefix, 0) != 0) {
        printf("First line in '%s' is wrong format ('%s'), deleting...\n", LAST_YUMS_FILE, firstLine.c_str());
        lyFile.close();
        deleteLastYumsFile();
        return;
    }

    int readLifeID = 0;
    try {
        readLifeID = std::stoi(firstLine.substr(firstLinePrefix.size()));
    } catch (...) {}

    if(readLifeID <= 0) {
        printf("LifeID in '%s' is wrong format ('%s'), deleting...\n", LAST_YUMS_FILE, firstLine.c_str());
        lyFile.close();
        deleteLastYumsFile();
        return;
    }
    lastYumsLifeID = readLifeID;

    lastYums.clear();
    std::string line;
    while(std::getline(lyFile, line)) {
        try {
            int val = std::stoi(line);
            lastYums.push_back(val);
        } catch (...) {}
    }

    printf("Loaded %d eaten food from '%s' with LifeID:%d\n", lastYums.size(), LAST_YUMS_FILE, lastYumsLifeID);

    lyFile.close();
}

bool YummyLife::appendLastYumsFile(int itemID){
    std::fstream lyFile(LAST_YUMS_FILE, std::ios::in | std::ios::out | std::ios::ate);

    if(!lyFile) {
        printf("Error opening '%s' for appending\n", LAST_YUMS_FILE);
        return false;
    }

    if (lyFile.tellg() > 0) {
        lyFile.seekg(-1, std::ios::end);
        char lastChar;
        lyFile.get(lastChar);

        if (lastChar != '\n') {
            lyFile.clear();
            lyFile.seekp(0, std::ios::end);
            lyFile << '\n';
        }
    }

    lyFile.clear();
    lyFile.seekp(0, std::ios::end);
    lyFile << itemID << "\n";

    lyFile.close();
    return true;
}

bool YummyLife::deleteLastYumsFile(){
    return (std::remove(LAST_YUMS_FILE) == 0);
}

bool YummyLife::setupLastYumsFile(int lifeID){
    std::ofstream lyFile(LAST_YUMS_FILE, std::ios::trunc);

    if (!lyFile) {
        printf("Error creating or clearing '%s'\n", LAST_YUMS_FILE);
        return false;
    }

    // Write the first line to the file
    lyFile << "lifeid:" << lifeID << "\n";
    lyFile.close();

    return true;
}