#include "yummyLife.h"

#include <stdio.h>
#include <fstream>
#include <string>

#include "minorGems/io/file/File.h"

#include "hetuwmod.h"

#include "minorGems/game/gameGraphics.h"

#include "minorGems/util/random/JenkinsRandomSource.h"

#include "fitnessScore.h"
#include "soundBank.h"
#include "message.h"

#include <iostream>

#define LAST_YUMS_FILE "lastYums.txt"
#define ONLY_ADD_UNIQUE_YUMS true

JenkinsRandomSource jRand;

int galleryImageIndex = -1;
char** galleryFileNames = nullptr;
int gallerySize = 0;

doublePair maxGalleryImageDim = {500, 500};

float galleryImageScale;
SpriteHandle galleryImageSprite;

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

template <typename T>
void freePointerArray(T** array, int size) {
    if (array != nullptr) {
        for (int i = 0; i < size; ++i) {
            delete[] array[i];  // Free each sub-array (T* element)
        }
        delete[] array;  // Free the main array (T**)
        array = nullptr; // Optional: Set pointer to nullptr to avoid dangling pointer
    }
}

// Returns a list of files inside a given directory
File** getDirectoryFiles(const char* directoryName, int* inNumResults){
    File dir(NULL, directoryName);
    return dir.getChildFiles(inNumResults);
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


void YummyLife::Gallery::setGalleryMaxDimensions(doublePair maxDimentions){
    maxGalleryImageDim = maxDimentions;
}

void YummyLife::Gallery::setGalleryMaxDimensions(int maxWidth, int maxHeight){
    doublePair dPair;
    dPair.x = maxWidth;
    dPair.y = maxHeight;
    setGalleryMaxDimensions(dPair);
}

// Initilize the gallery's files and length, start at index 0
void YummyLife::Gallery::initGallery(const char* galleryDirPath){

    File** files = getDirectoryFiles(galleryDirPath, &gallerySize);

    freePointerArray(galleryFileNames, gallerySize); // Free if not null_ptr

    if(!files) {
        printf("Failed to load gallery\n");
        return;
    }

    if(gallerySize == 0){
        printf("Gallery loaded with a size of %d\n", gallerySize);
        return;
    }

    galleryFileNames = new char*[gallerySize];

    for (int i = 0; i < gallerySize; i++){
        const char* fullFileName = files[i]->getFullFileName();
        galleryFileNames[i] = new char[strlen(fullFileName) + 1];
        strcpy(galleryFileNames[i], fullFileName);
    }

    //freePointerArray(files, gallerySize);

    // No index is loaded at the moment
    galleryImageIndex = -1;
}

// Returns the current index of the selected image, -1 if none
int YummyLife::Gallery::getGalleryImageIndex() {
    return galleryImageIndex;
}

void YummyLife::Gallery::loadGalleryIndex(int inIndex) {
    // something happened
    if(inIndex == -1) {
        printf("Cannot load an index of -1\n");
        // Don't want to crash :(
        return;
    }

    if(inIndex < 0 || inIndex >= gallerySize) {
        std::cerr << "Index " << inIndex << " is out of bounds for gallerySize " << gallerySize << "\n";
        throw std::runtime_error("Passed in index below 0 or above gallerySize");
    }

    if(galleryImageSprite)
        freeSprite(galleryImageSprite);

    galleryImageIndex = inIndex;

    const char* fileName = galleryFileNames[galleryImageIndex];

    if (strstr(fileName, ".tga") != nullptr) {
        galleryImageSprite = loadSpriteBase(fileName); // Try to load .tga file
    } else {
        galleryImageSprite = loadSprite("swapButton.tga"); // Non-.tga files default to placeholder
    }

    // Check if the image failed, try to load swapButton again
    if (!galleryImageSprite) {
        std::cerr << "Failed to load image: " << fileName 
                << ". Loading placeholder image instead.\n";
        galleryImageSprite = loadSprite("swapButton.tga");
    }

    // Final fallback in case the placeholder also fails
    if (!galleryImageSprite) {
        throw std::runtime_error("Failed to load both primary and placeholder images.");
    }

    int imageWidth = getSpriteWidth(galleryImageSprite);
    int imageHeight = getSpriteHeight(galleryImageSprite);

    float widthScale = (float)maxGalleryImageDim.x / imageWidth;
    float heightScale = (float)maxGalleryImageDim.x / imageHeight;

    galleryImageScale = std::min(widthScale, heightScale);

    std::cout << "Name: " << fileName << ", Scale: " << galleryImageScale << ", imgW: " << imageWidth << ", imgH: " << imageHeight <<  "\n";
}

void YummyLife::Gallery::drawGallery(doublePair pos){
    if (!galleryImageSprite) return;
    drawSprite(galleryImageSprite, pos, galleryImageScale);
}

int YummyLife::Gallery::loadNextGalleryImage(){
    if(gallerySize > 0){
        galleryImageIndex++;
        if(galleryImageIndex >= gallerySize) galleryImageIndex = 0;
        loadGalleryIndex(galleryImageIndex);
        return galleryImageIndex;
    }
    return -1;
}

int YummyLife::Gallery::loadPreviousGalleryImage(){
    if(gallerySize > 0){
        galleryImageIndex--;
        if(galleryImageIndex < 0) galleryImageIndex = gallerySize-1;
        loadGalleryIndex(galleryImageIndex);
        return galleryImageIndex;
    }
    return -1;
}

int getRandomGalleryIndex(int gallerySize, int excludeIndex = -1){
    if(gallerySize <= 0) return -1;
    if(gallerySize == 1) return 0;
    int cap = 0;
    int res;
    while(cap < 20){
        res = jRand.getRandomBoundedInt(0, gallerySize - 1);
        if(res != excludeIndex) return res;
        cap++;
    }
    return 0;
}

int YummyLife::Gallery::loadRandomGalleryImage(int excludeIndex){
    int newIndex = getRandomGalleryIndex(gallerySize, excludeIndex);
    std::cout << "rand: " << newIndex << "\n";
    loadGalleryIndex(newIndex);
    return newIndex;
}

int YummyLife::Gallery::getGallerySize(){
    return gallerySize;
}

YummyLife::Gallery::~Gallery() {
    freePointerArray(galleryFileNames, gallerySize);
    if (galleryImageSprite) {
        freeSprite(galleryImageSprite);
    }
}