#include "yummyLife.h"

#include <stdio.h>
#include <fstream>
#include <string>

#include <CPP-HTTPLib/httplib.h>

#include "minorGems/io/file/File.h"

#include "hetuwmod.h"
#include "LivingLifePage.h"

#include "minorGems/game/gameGraphics.h"

#include "minorGems/util/random/JenkinsRandomSource.h"

#include "fitnessScore.h"
#include "soundBank.h"
#include "message.h"

#include <iostream>

#define LAST_YUMS_FILE "lastYums.txt"
#define ONLY_ADD_UNIQUE_YUMS true

JenkinsRandomSource jRand;

extern doublePair lastScreenViewCenter;

int YummyLife::AFK::numCycles = 0;
bool YummyLife::AFK::is_afk = false;
bool YummyLife::AFK::bIsWaiting = false;
int YummyLife::AFK::waitingForNotID = -1;
bool YummyLife::AFK::is_enabled = false;
const char* YummyLife::AFK::statusMessage = nullptr;
time_t YummyLife::AFK::waitStartTime = 0;
time_t YummyLife::AFK::startAfkTime = 0;
doublePair YummyLife::AFK::startAfkPos = {0, 0};
int YummyLife::AFK::timesEaten = 0;

std::vector<std::string> galleryFileNames;
int galleryImageIndex = -1;
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

// Does a given fileName or path end in '.tga'
bool isTgaFileName(std::string fileName) {
    // Check if the filename is at least 4 characters long
    if (fileName.size() < 4)
        return false;

    // Get the last 4 characters of the filename
    std::string extension = fileName.substr(fileName.size() - 4);

    // Compare the extracted part with ".tga"
    return extension == ".tga";
}

void YummyLife::livingLifeStep(){
    AFK::step();
}

void YummyLife::livingLifeDraw(){
    if(AFK::isAFK()){
        HetuwFont *customFont = HetuwMod::customFont;
        double scale = customFont->hetuwGetScaleFactor();
	    customFont->hetuwSetScaleFactor(scale * HetuwMod::guiScale * 1.4);
	    doublePair drawPos = lastScreenViewCenter;
        int afkDuration = HetuwMod::curStepSecondsSince1970 - AFK::getStartAfkTime();
        char afkDurationStr[50];
        if (afkDuration >= 60) {
            snprintf(afkDurationStr, sizeof(afkDurationStr), " (%.1f min)", afkDuration / 60.0);
        } else {
            snprintf(afkDurationStr, sizeof(afkDurationStr), " (%d sec)", afkDuration);
        }
        char afkMessage[100];
        snprintf(afkMessage, sizeof(afkMessage), "You are AFK%s", afkDurationStr);
        drawPos = HetuwMod::drawCustomTextWithBckgr(drawPos, afkMessage);
        char timesEatenStr[50];
        snprintf(timesEatenStr, sizeof(timesEatenStr), "Eaten %d times", AFK::getTimesEaten());
        drawPos = HetuwMod::drawCustomTextWithBckgr(drawPos, timesEatenStr);
        if(AFK::isEnabled()) {
            drawPos = HetuwMod::drawCustomTextWithBckgr(drawPos, "Status: Running");
        }else{
            drawPos = HetuwMod::drawCustomTextWithBckgr(drawPos, "Status: Disabled"); 
            if(AFK::getStatusMessage()) {
                drawPos = HetuwMod::drawCustomTextWithBckgr(drawPos, AFK::getStatusMessage());
            }
            drawPos = HetuwMod::drawCustomTextWithBckgr(drawPos, "Please turn AFK off, then on to re-enable");
        }
	    customFont->hetuwSetScaleFactor(scale);
    }
}

// Return true to block command from being proccessed
bool YummyLife::handlePlayerCommand(char* inCommand){
    if(strcmp(inCommand, "/AFK") == 0){
        if(!AFK::isAFK()) AFK::setAFK(true);
        else              AFK::setAFK(false);
        return true;
    }
    return false;
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
    // If the Gallery is disabled, we will return instantly
    if(!HetuwMod::bGalleryEnabled) return;

    int numDirChildren;
    gallerySize = 0;

    // No index will be loaded
    galleryImageIndex = -1;

    File** files = getDirectoryFiles(galleryDirPath, &numDirChildren);

    if(!files) {
        printf("Failed to load gallery\n");
        return;
    }

    if(numDirChildren == 0){
        printf("No files in the gallery folder\n");
        return;
    }

    for (int i = 0; i < numDirChildren; i++){
        const char* fullFileName = files[i]->getFullFileName();
        if(isTgaFileName(fullFileName))
            galleryFileNames.push_back(fullFileName);
    }

    gallerySize = galleryFileNames.size();

    if(gallerySize == 0){
        printf("No files ending in '.tga' in gallery");
        return;
    }

    printf("found %d screenshots\n", gallerySize);
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

    const std::string fileName = galleryFileNames[galleryImageIndex];

    if (isTgaFileName(fileName))
        galleryImageSprite = loadSpriteBase(fileName.c_str()); // Try to load .tga file
    else 
        // this shouldn't happen anymore, since I filter out non-tga files
        galleryImageSprite = loadSprite("swapButton.tga"); // Non-.tga files default to placeholder

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
    if (galleryImageSprite)
        freeSprite(galleryImageSprite);
}

const char* YummyLife::API::getValueFromJSON(const char* json, const char* key){
    std::string jsonStr(json);
    std::string keyStr(key);
    std::string keyStrQuoted = "\"" + keyStr + "\"";

    size_t keyPos = jsonStr.find(keyStrQuoted);
    if(keyPos == std::string::npos) return nullptr;

    size_t valuePos = jsonStr.find(":", keyPos);
    if(valuePos == std::string::npos) return nullptr;

    size_t valueStartPos = jsonStr.find_first_of("\"", valuePos);
    if(valueStartPos == std::string::npos) return nullptr;

    size_t valueEndPos = jsonStr.find_first_of("\"", valueStartPos + 1);
    if(valueEndPos == std::string::npos) return nullptr;

    std::string value = jsonStr.substr(valueStartPos + 1, valueEndPos - valueStartPos - 1);
    return strdup(value.c_str());
}

const char* YummyLife::API::getLatestVersionTag(const char* repoTag) {
    httplib::Client cli("https://api.github.com");
    cli.set_connection_timeout(3);
    const std::string url = "/repos/" + std::string(repoTag) + "/releases/latest";
    auto res = cli.Get(url);

    if (res && res->status == 200)
        return getValueFromJSON(res->body.c_str(), "tag_name");
    std::cerr << "Failed to get latest version tag from GitHub\n";
    return nullptr;
}

void YummyLife::API::parseVersionTag(const char* versionTag, int* major, int* minor) {
    *major = -1;
    *minor = -1;
    if (versionTag == nullptr) return;

    if (versionTag[0] == 'v')
        versionTag++;

    int parsedMinor = 0;
    sscanf(versionTag, "%d.%d", major, &parsedMinor);
    *minor = parsedMinor;
}

// Sets flags to wait until item we're holding has changed
void YummyLife::AFK::wait(){
    waitingForNotID = HetuwMod::ourLiveObject->holdingID;
    waitStartTime = HetuwMod::curStepSecondsSince1970;
    bIsWaiting = true;
}

int YummyLife::AFK::secondsWaited(){
    return HetuwMod::curStepSecondsSince1970 - waitStartTime;
}

void YummyLife::AFK::stopWaiting(){
    bIsWaiting = false;
}

// Check and update waiting status based on held item
bool YummyLife::AFK::updateWaitingStatus(){
    if(!bIsWaiting) return false; // Return early to prevent redundant checks
    if(waitingForNotID != HetuwMod::ourLiveObject->holdingID)
        bIsWaiting = false;
    return bIsWaiting;
}

void YummyLife::AFK::step(){
    if(!is_enabled || !is_afk) return;
    if(startAfkPos != HetuwMod::ourLiveObject->currentPos) setAFK(false, "Moved, AFK stopped");
    if (HetuwMod::curStepSecondsSince1970 - startAfkTime > (600)) setEnabled(false, "AFK timeout after 10 minutes"); // 600 seconds = 10 minutes
    if (updateWaitingStatus()) {
        if (secondsWaited() > 2) setEnabled(false, "Use backpack timeout");
        return; // Return early if we're waiting for an item to change
    }
    if(!is_enabled || !is_afk) return;

    LiveObject *ourLiveObject = HetuwMod::ourLiveObject;

    // If we don't need to eat, nothing more needs to be done
    if(ourLiveObject->foodStore > ourLiveObject->maxFoodCapacity - HetuwMod::iAfkHungerThreshold && ourLiveObject->foodStore > 1) return;

    bool holdingItem = (ourLiveObject->holdingID > 0);
    bool edibleItem = false;
    float itemSize = 0;
    if(holdingItem){
        ObjectRecord* obj = getObject(ourLiveObject->holdingID, true);
        if(obj) {
            edibleItem = (obj->foodValue > 0);
            itemSize = obj->containSize;
            }
    }

    // Eat held item
    if (edibleItem) {
        HetuwMod::useOnSelf();
        timesEaten++;
        numCycles = 0;
        wait();
        return;
    }

    if (!HetuwMod::weAreWearingABackpack()){
        // Not wearing a backpack, so can't put item away, stop AFK
        setEnabled(false, "No backpack");
        return;
    }

    ObjectRecord *backpackItem = ourLiveObject->clothing.backpack;
    if(numCycles > backpackItem->numSlots){
        // We've cycled through all backpack slots, stop AFK
        setEnabled(false, "Cycled through all backpack slots, no food found");
        return;
    }

    if(itemSize > backpackItem->slotSize){
        // Item is too big for backpack, stop AFK
        setEnabled(false, "Item too big for backpack");
        return;
    }

    // Note: I don't know how to check what items are in our backpack, so I just cycle through them all for now
    HetuwMod::useBackpack(true, -1);
    numCycles++;
    wait();
}

void YummyLife::AFK::setAFK(bool afk, const char* msg){
    is_afk = afk;
    numCycles = 0;
    is_enabled = true;
    statusMessage = msg;
    startAfkPos = HetuwMod::ourLiveObject->currentPos;
    startAfkTime = HetuwMod::curStepSecondsSince1970;
    timesEaten = 0;
    if(msg) std::cout << msg << "\n";
}

void YummyLife::AFK::setEnabled(bool enable, const char* msg){
    is_enabled = enable;
    statusMessage = msg;
    if(msg) std::cout << msg << "\n";
}