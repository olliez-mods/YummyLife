#ifndef YUMMY_LIFE_INCLUDED
#define YUMMY_LIFE_INCLUDED

#include <vector>
#include <string>

#include "minorGems/game/game.h"

// Returns a translation of the key if avaliable, otherwise returns inDefault
const char* translateWithDefault(const char* inTranslationKey, const char* inDefault = nullptr);

class YummyLife {
    public:
        static void gameStep();
        static void drawLeaderboardName(doublePair pos);

        static void livingLifeStep();
        static void livingLifeDraw();
        static bool handlePlayerCommand(char* inCommand);

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
                static int getGallerySize();
                ~Gallery();
        };

        class API{
            static const char* getValueFromJSON(const char* json, const char* key);
            public:
                // Github version functions
                static const char* getLatestVersionTag(const char* repoTag); // Calls a web API - Expect delays
                static void parseVersionTag(const char* versionTag, int* major, int* minor);
                static const char* getGitHubRepoURL(); // Unused
        };

        class AFK{
            static bool is_afk;

            static int numCycles;

            static int timesEaten;

            static doublePair startAfkPos;
            static time_t startAfkTime;

            static bool is_enabled;
            static const char* statusMessage;


            static time_t waitStartTime;
            static bool bIsWaiting;
            static int waitingForNotID;
            static void wait();
            static void stopWaiting();
            static bool updateWaitingStatus();
            static int secondsWaited();

            public:
                static void step();
                static void setAFK(bool afk, const char* msg = nullptr);
                static void setEnabled(bool disable, const char* msg = nullptr);
                static bool isEnabled(){return is_enabled;}
                static const char* getStatusMessage(){return statusMessage;}
                static time_t getStartAfkTime(){return startAfkTime;}
                static bool isAFK(){return is_afk;}
                static bool isWaiting(){return bIsWaiting;}
                static int getTimesEaten(){return timesEaten;}
        };

        // Handles the downloading of live resources (e.g. images) from the GitHub repo
        class LiveResources {
            public:
                static const char* getGitHubLiveResourcesPath();
                static bool ensureDirectoriesExistForFile(const char* filepath);
                static bool downloadLiveResourceFile(const char* path, const char* localPath);
                static void initLiveResources(const char* clientVersionTag); // Downloads/updates all live resources
        };

        // Handles local and shared account data
        class AccountManager {
            public:
                struct Account {
                    enum Type {
                        LOCAL,
                        SHARED
                    };
                    public:
                        Type type;

                        std::string leaderboardName;
                        std::string notes; // May be empty

                        // Local accounts have these fields
                        std::string email;
                        std::string key;

                        // Shared accounts have these fields
                        bool isOwner = false;
                        std::string email_decrypt_key; // Decrypted on fetch
                        std::string account_access_token;
                        std::string owner_access_token; // May be empty
                        // Fetched from OHOLcurse server - get account info
                        // Fetched from OHOLCurse server - login process

                };
                static void loadSavedAccounts(); // Reads accounts from disk
                static void saveAccounts(); // Saves accounts to disk
                static int addAccountLocal(const char* email, const char* key, const char* notes, const char* leaderboardName = nullptr);
                static bool deleteAccountAtIndex(int index); // Returns true on success
                static int findAccountByIndex(int index, Account* outAccount);
                static int findMatchingLocalAccount(const char* email, const char* key, Account* outAccount = nullptr);
                static void addAccountShared(const char* account_access_token, const char* owner_access_token); // leaderboardName and notes are fetched from server
                static std::vector<Account> accounts;
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