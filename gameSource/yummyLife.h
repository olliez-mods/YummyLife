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
            static float waitingSeconds;
            static bool bIsWaiting;
            static void wait(float seconds);
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
                        std::string account_access_token;
                        std::string owner_access_token; // May be empty

                };
                static void loadSavedAccounts(); // Reads accounts from disk
                static void saveAccounts(); // Saves accounts to disk
                static int addAccountLocal(const char* email, const char* key, const char* notes, const char* leaderboardName = nullptr);
                static int addAccountShared(const char* account_access_token, const char* notes, const char* leaderboardName, const char* owner_access_token = nullptr);
                static bool deleteAccountAtIndex(int index); // Returns true on success
                static int findAccountByIndex(int index, Account* outAccount);
                static int findMatchingLocalAccount(const char* email, const char* key, Account* outAccount = nullptr);

                // OHOLCurse API functions for shared accounts
                static const char* standerdizeToken(const char* token);
                static bool createSharedAccount(const char* email, const char* key, const char* lb_name, int& account_index_out); // Create a shared account
                static bool fetchSharedAccountInfo(const char* account_access_token, std::string& out_email, std::string& out_leaderboardName, int& uses_left, double& expiration);
                static bool loginSharedAccount(const char* account_access_token, const char* challenge, std::string& out_hashed_challenge, std::string& out_email);
                static bool editSharedAccount(const char* account_owner_token, const char* action, const char* value = nullptr);

                static std::vector<Account> accounts;
                static int loginSharedAccountIndex;
        };

        class FriendCodeSharing {

            static std::string lastSessionToken;
            static std::string lastChallenge;

            public:
                static bool isSharedFriendToken(const char* twinCode);
                static const char* begin(const char* friendToken); // Sends API request and returns the challenge hash
                static bool complete(const char* email, const char* hashedChallenge); // Completes the login process with the challenge response
        };

        // Chat scrollback: history of recent nearby speech, toggled with a
        // key. Lines that mention our first name are highlighted.
        // Local fork extension: attention-ping EVENT entries (addEvent).
        class ChatLog {
            public:
                struct Entry {
                    enum Kind { SPEECH, MENTION, EVENT };
                    Kind kind;
                    std::string name;
                    std::string text;
                    doublePair pos; // Position associated with this entry
                    float distanceToUs; // Distance from our character to this entry
                    time_t timestamp; // When this entry was added
                    int age;            // EVENT only: our age when it happened
                    float color[3];     // EVENT only
                };
            private:

            static std::vector<Entry> entries;
            static int scrollPos;   // -1 = stuck to bottom; >= 0 = index of top visible line
            static doublePair lastDistanceCalcPos;
            // whole-word match of our first name in (all-caps) speech
            static bool mentionsUs(const char* speech);
            static void refreshDistancesForCurrentPosition();

            public:
                static int filterLevel; // inf, 5, 10, 15
                static int indexChatHovered; // index of the currently hovered chat entry, -1 if none

                static void add(const char* speakerName, const char* text, bool isSelf,  time_t timestamp, const doublePair& pos);
                // age-stamped ping event line (fork extension); never
                // distance-filtered
                static void addEvent(const char* text, float r, float g, float b);
                static Entry* getHoveredEntry();
                // wheel scrolling while the panel is open; dir +1 = up (back
                // in time), -1 = down
                static void scroll(int dir);
                // snap back to the newest lines
                static void resetScroll();
                static void draw();     // call from livingLifeDraw when bShow
                static void newLife();
        };

        // Attention pings: sound + screen flash when something needs the
        // player's attention (name mentioned, baby born, got cursed)
        class Pings {
            static SoundSpriteHandle chimeSound;
            static SoundSpriteHandle curseSound;

            static double flashStartTime;   // <= 0 when no flash active
            static float flashColor[3];
            static double lastNamePingTime;
            static double lastBabyPingTime;
            static double lastCursePingTime;
            static int lastOwnCurseLevel;   // -1 = unknown (no CU seen yet this life)
            static double lastAgeSeen;      // -1 = unknown (first update records only)

            static char milestoneMsg[64];   // on-screen label for age milestones
            static double milestoneMsgTime;

            static void cursePing();
            static void agePing(const char* msg);
            static void labelPing(const char* msg, bool curseSound,
                    float r, float g, float b);

            static double lastAfkReplyTime;

            static bool speechMentionsUs(const char* speech);
            static void playChime(bool curse);
            static void flash(float r, float g, float b);

            public:
                static bool bEnabled;
                static bool bOnNameMention;
                static bool bOnBabyBorn;
                static bool bOnCursed;
                static bool bOnAgeMilestone;
                static bool bOnFamilyDying;
                static bool bAfkAutoReply;
                static bool bFlashScreen;

                // family-survival warning (called from HetuwMod)
                static void familyAlert(const char* msg);

                // speakerID != ourID is checked by the caller
                static void onPlayerSays(int speakerID, const char* speech, int curseFlag);
                static void onBabyBorn();
                static void onSelfCurseLevel(int curseLevel);
                static void stepAge(double age, char gender);
                static void newLife();
                static void draw();     // flash overlay, call from livingLifeDraw
                static void cleanUpSounds();
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


// ---------------------------------------------------------------------------
// YummyLife decay timer system — stores per-world-tile ETA in world coords
// so timers survive rolling-buffer resets and chunk reloads.
// ---------------------------------------------------------------------------

// Returns true if this objectID should be tracked at all:
//   - has autoDecaySeconds > 0 and is not epoch-based
//   - cannot be held in hand (can't be picked up), OR
//     can be held but when dropped produces a different ID
//     (so the server restarts decay on placement → our MAP_CHANGE is fresh)
bool yumIsDecayTrackable( int objectID );

// Returns true if the timer is reliable across chunk reloads:
//   - object cannot be held in hand / ridden, AND
//   - no interaction leaves the object as the same target ID
//     (all interactions either change it or leave it to decay naturally)
bool yumDecaySurvivesChunkReload( int objectID );

// Called from MAP_CHANGE handler.
// newID / oldID are what's at the cell now vs before the message.
// worldX / worldY are the full world-coordinate tile position.
void yumOnMapChange( int worldX, int worldY, int newID, int oldID );

// Called from MAP_CHUNK cell handler.
// newID / oldID are chart-based: newID is from the chunk, oldID was in mMap.
void yumOnMapChunkCell( int worldX, int worldY, int newID );

// Returns the decay ETA (absolute game_getCurrentTime() seconds) for a cell,
// or 0.0 if no timer is tracked.
double yumGetDecayETA( int worldX, int worldY );

// Explicitly clears any timer at this world position.
void yumClearDecayTimer( int worldX, int worldY );

// Clears all decay timers (called when the local map is reset).
void yumClearAllDecayTimers();


#endif