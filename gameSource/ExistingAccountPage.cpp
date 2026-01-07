#include "ExistingAccountPage.h"

#include "message.h"
#include "buttonStyle.h"

#include "accountHmac.h"

#include "lifeTokens.h"
#include "fitnessScore.h"

#include "settingsToggle.h"

#include "minorGems/game/Font.h"
#include "minorGems/game/game.h"

#include "minorGems/util/stringUtils.h"
#include "minorGems/util/SettingsManager.h"

#include "minorGems/crypto/hashes/sha1.h"


#include "minorGems/graphics/openGL/KeyboardHandlerGL.h"

#include "minorGems/util/random/JenkinsRandomSource.h"

#include "minorGems/formats/encodingUtils.h"
#include "minorGems/game/drawUtils.h"

#include "hetuwmod.h"
#include "yumRebirthComponent.h"
#include "yummyLife.h"

#define SAVE_ACCOUNT_TIMEOUT_SECONDS 8.0

static JenkinsRandomSource randSource;


extern Font *mainFont;


extern char gamePlayingBack;

extern char *userEmail;
extern char *accountKey;
extern char isAHAP;


extern SpriteHandle instructionsSprite;

extern char loginEditOverride;

// YummyLife: Used for version checking
extern int versionNumber;
extern int softwareVersionNumber;
extern const char *yumSubVersion;

int selectedAccountIndexForEdit = -1;

// These are only set when getModdedClientUpdateStatus() is called, and is successful
int latestMajorClientVersion = -1;
int latestMinorClientVersion = -1;
// Returns -1 for fail, 0 for up-to-date, 1 for minor update, 2 for major update
int getModdedClientUpdateStatus(){
    // First get current version
    int currMajorVersion = softwareVersionNumber;
    int currentMinorVersion = 0;
    sscanf(yumSubVersion, ".%d", &currentMinorVersion);

    printf("YummyLife: Current version: %d.%d\n", currMajorVersion, currentMinorVersion);

    // Poll GitHub for latest release version
    const char* latestVersionTag = YummyLife::API::getLatestVersionTag("olliez-mods/yummylife");
    int latestMajorVersion, latestMinorVersion;
    YummyLife::API::parseVersionTag(latestVersionTag, &latestMajorVersion, &latestMinorVersion);
    delete[] latestVersionTag;

    printf("YummyLife: Latest version: %d.%d\n", latestMajorVersion, latestMinorVersion);

    // Something went wrong
    if(latestMajorVersion < 0) return -1;

    // Update public vars:
    latestMajorClientVersion = latestMajorVersion;
    latestMinorClientVersion = latestMinorVersion;

    if(latestMajorVersion > currMajorVersion) return 2;
    if(latestMajorVersion < currMajorVersion) return 0;
    if(latestMinorVersion > currentMinorVersion) return 1;
    return 0;
}

void forceCompleteTutorial() {
    SettingsManager::setSetting( "tutorialDone", 2 );
}

// result destroyed by caller
static char *getLineageServerURL() {
    char *url;
    
    if( isAHAP ) {
        url = SettingsManager::getStringSetting( "ahapLineageServerURL", "" );
        }
    else {
        url = SettingsManager::getStringSetting( "lineageServerURL", "" );
        }
    
    return url;
    }



ExistingAccountPage::ExistingAccountPage()
        : mEmailField( mainFont, 380, 275, 10, false,
                       translate( "email" ),
                       NULL,
                       // forbid only spaces and backslash and 
                       // single/double quotes 
                       "\"' \\" ),
          mKeyField( mainFont, 380, 205, 15, true,
                     translate( "accountKey" ),
                     // allow only ticket code characters
                     "23456789ABCDEFGHJKLMNPQRSTUVWXYZ-" ),
          mAtSignButton( mainFont, 480, 70, "@" ),
          mPasteButton( mainFont, 380, 70, translate( "paste" ), 'v', 'V' ),
          mPasteEmailButton( mainFont, 380, 70, translate( "paste" ), 'v', 'V' ),
          mDisableCustomServerButton( mainFont, 0, 220, 
                                      translate( "disableCustomServer" ) ),
          mLoginButton( mainFont, 535, -280, translate( "loginButton") ),
          mFriendsButton( mainFont, 385, -280, translateWithDefault( "yummyLifeTwinsButton", "TWINS") ),
          mGenesButton( mainFont, 0, -65, translateWithDefault( "yummyLifeGenesButton", "LIFE DATA" ) ),
          mFamilyTreesButton( mainFont, 0, -140, translate( "familyTrees" ) ),
          mClearAccountButton( mainFont, 220, 135, translateWithDefault( "yummyLifeClearButton", "CLEAR") ),
          mChangeAccountButton( mainFont, 430, 135, translateWithDefault( "yummyLifeChangeAccountButton", "CHANGE ACNT") ),
          mCancelButton( mainFont, -530, -280, translate( "quit" ) ),
          mSettingsButton( mainFont, -522, 275, translate( "settingsButton" ) ),
          mReviewButton( mainFont, -400, -200, 
                         translate( "postReviewButton" ) ),
          mRetryButton( mainFont, -100, 198, translate( "retryButton" ) ),
          mRedetectButton( mainFont, 100, 198, translate( "redetectButton" ) ),
          mViewAccountButton( mainFont, 300, 150, translate( "view" ) ),
          mTutorialButton( mainFont, 205, -280, translate( "tutorial" ) ),
          mTutOneButton( mainFont, 165, -220, translateWithDefault( "yummyLifeTutOneButton", "T#1" ) ),
          mTutTwoButton( mainFont, 245, -220, translateWithDefault( "yummyLifeTutTwoButton", "T#2" ) ),
          mServicesButton( mainFont, -330, 275, translate( "services" ) ),
          mAHAPSettingsButton( mainFont, -522, 0, 
                               translate( "ahapSettings" ) ),
          mNextImageButton( mainFont, -340, -160, translateWithDefault("yummyNextImageButton", "NEXT")),
          mPrevImageButton( mainFont, -460, -160, translateWithDefault("yummyPrevImageButton", "PREV")),
          mUpdateYummyLifeButton( mainFont, -280, -280, translateWithDefault("yummyUpdateYummyLifeButton", "UPDATE YUMMYLIFE")),

          mAcntPrevButton( mainFont, -580, 260, translateWithDefault( "accountPrev", "<" ) ),
          mAcntNextButton( mainFont, -530, 260, translateWithDefault( "accountNext", ">" ) ),
          mAcntSelectButton( mainFont, -450, 260, translateWithDefault( "accountSelect", "S" ) ),
          mAcntEditButton( mainFont, -400, 260, translateWithDefault( "accountEdit", "E" ) ),
          mAcntDeleteButton( mainFont, -350, 260, translateWithDefault( "accountDelete", "D" ) ),
          mAcntDeleteConfirmButton( mainFont, -350, 260, translateWithDefault( "accountDeleteConfirm", "?" ) ),
          mAcntNewButton( mainFont, -300, 260, translateWithDefault( "accountNew", "+" ) ),
          mAcntCloseButton( mainFont, -200, 260, translateWithDefault( "accountClose", "X" ) ),

          mAcntConfirmButton( mainFont, -280, -150, translateWithDefault( "accountConfirm", "Confirm" ) ),
          mAcntCancelButton( mainFont, -490, -150, translateWithDefault( "accountCancel", "Cancel" ) ),
          mAcntPasteButton( mainFont, -540, 260, translate( "paste" ), 'v', 'V' ),
          mAcntEmailField( mainFont, -335, 150, 8, false,
                           translate( "email" ),
                           NULL,
                           "\"' \\" ),
          mAcntKeyField( mainFont, -335, 80, 10, true,
                           translate( "accountKey" ),
                           "23456789ABCDEFGHJKLMNPQRSTUVWXYZ-" ),
          mAcntNotesField( mainFont, -335, 10, 8, false,
                           translateWithDefault( "yummyLifeAccountNotesField", "NOTES" ),
                            NULL,
                            NULL ),

          mYumRebirth( mainFont, -200, -100, -10.0, -50.0 ),
          mPageActiveStartTime( 0 ),
          mFramesCounted( 0 ),
          mFPSMeasureDone( false ),
          mHideAccount( false ) {
    
    
    if( userEmail != NULL && accountKey != NULL ) {
        mEmailField.setText( userEmail );
        mKeyField.setText( accountKey );
        }

    setButtonStyle( &mLoginButton );
    setButtonStyle( &mFriendsButton );
    setButtonStyle( &mGenesButton );
    setButtonStyle( &mFamilyTreesButton );
    setButtonStyle( &mClearAccountButton );
    setButtonStyle( &mChangeAccountButton );
    setButtonStyle( &mCancelButton );
    setButtonStyle( &mSettingsButton );
    setButtonStyle( &mReviewButton );
    setButtonStyle( &mAtSignButton );
    setButtonStyle( &mPasteButton );
    setButtonStyle( &mPasteEmailButton );
    setButtonStyle( &mRetryButton );
    setButtonStyle( &mRedetectButton );
    setButtonStyle( &mViewAccountButton );
    setButtonStyle( &mTutorialButton );
    setButtonStyle( &mTutOneButton );
    setButtonStyle( &mTutTwoButton );
    setButtonStyle( &mServicesButton );
    setButtonStyle( &mAHAPSettingsButton );

    setButtonStyle( &mNextImageButton );
    setButtonStyle( &mPrevImageButton );

    setButtonStyle( &mUpdateYummyLifeButton );

    setButtonStyle( &mDisableCustomServerButton );

    setButtonStyle( &mAcntNextButton );
    setButtonStyle( &mAcntPrevButton );
    setButtonStyle( &mAcntDeleteButton );
    setButtonStyle( &mAcntDeleteConfirmButton );
    setButtonStyle( &mAcntSelectButton );
    setButtonStyle( &mAcntEditButton );
    setButtonStyle( &mAcntNewButton );
    setButtonStyle( &mAcntCloseButton );
    setButtonStyle( &mAcntConfirmButton );
    setButtonStyle( &mAcntCancelButton );
    setButtonStyle( &mAcntPasteButton );
    
    // draw attention to login button
    mLoginButton.setNoHoverColor( 1, 1, 0, 1 );
    mLoginButton.setHoverColor( 1, 1, 0, 1 );
    mLoginButton.setDragOverColor( 1, 1, 0, 1 );
    mLoginButton.setHoverBorderColor( 1, 1, 0, 1 );
    
    
    mFields[0] = &mEmailField;
    mFields[1] = &mKeyField;

    
    addComponent( &mLoginButton );
    addComponent( &mFriendsButton );
    addComponent( &mGenesButton );
    addComponent( &mFamilyTreesButton );
    addComponent( &mClearAccountButton );
    addComponent( &mChangeAccountButton );
    addComponent( &mCancelButton );
    addComponent( &mSettingsButton );
    // addComponent( &mReviewButton ); YummyLife: Disabled for now
    addComponent( &mAtSignButton );
    addComponent( &mPasteButton );
    addComponent( &mPasteEmailButton );
    addComponent( &mEmailField );
    addComponent( &mKeyField );
    addComponent( &mRetryButton );
    addComponent( &mRedetectButton );
    addComponent( &mDisableCustomServerButton );

    addComponent( &mViewAccountButton );
    addComponent( &mTutorialButton );
    addComponent( &mTutOneButton );
    addComponent( &mTutTwoButton );
    addComponent( &mServicesButton );
    addComponent( &mAHAPSettingsButton );

    addComponent( &mNextImageButton );
    addComponent( &mPrevImageButton );

    addComponent( &mUpdateYummyLifeButton );

    addComponent( &mAcntNextButton );
    addComponent( &mAcntPrevButton );
    addComponent( &mAcntDeleteButton );
    addComponent( &mAcntDeleteConfirmButton );
    addComponent( &mAcntSelectButton );
    addComponent( &mAcntEditButton );
    addComponent( &mAcntNewButton );
    addComponent( &mAcntCloseButton );
    addComponent( &mAcntConfirmButton );
    addComponent( &mAcntCancelButton );
    addComponent( &mAcntPasteButton );
    addComponent( &mAcntEmailField );
    addComponent( &mAcntKeyField );
    addComponent( &mAcntNotesField );

    //addComponent( &mYumRebirth ); Yummylife: Disabled for now
    
    mLoginButton.addActionListener( this );
    mFriendsButton.addActionListener( this );
    mGenesButton.addActionListener( this );
    mFamilyTreesButton.addActionListener( this );
    mClearAccountButton.addActionListener( this );
    mChangeAccountButton.addActionListener( this );
    
    mCancelButton.addActionListener( this );
    mSettingsButton.addActionListener( this );
    mReviewButton.addActionListener( this );
    
    mAtSignButton.addActionListener( this );
    mPasteButton.addActionListener( this );
    mPasteEmailButton.addActionListener( this );

    mRetryButton.addActionListener( this );
    mRedetectButton.addActionListener( this );

    mViewAccountButton.addActionListener( this );
    mTutorialButton.addActionListener( this );
    mTutOneButton.addActionListener( this );
    mTutTwoButton.addActionListener( this );
    mServicesButton.addActionListener( this );
    mAHAPSettingsButton.addActionListener( this );

    mAcntNextButton.addActionListener( this );
    mAcntPrevButton.addActionListener( this );
    mAcntDeleteButton.addActionListener( this );
    mAcntDeleteConfirmButton.addActionListener( this );
    mAcntSelectButton.addActionListener( this );
    mAcntEditButton.addActionListener( this );
    mAcntNewButton.addActionListener( this );
    mAcntCloseButton.addActionListener( this );
    mAcntConfirmButton.addActionListener( this );
    mAcntCancelButton.addActionListener( this );
    mAcntPasteButton.addActionListener( this );
    
    // YummyLife: Setup the Gallery control buttons
    mNextImageButton.addActionListener( this );
    mPrevImageButton.addActionListener( this );
    mNextImageButton.setDrawBackground(false);
    mPrevImageButton.setDrawBackground(false);

    mUpdateYummyLifeButton.addActionListener( this );

    mDisableCustomServerButton.addActionListener( this );

    mChangeAccountButton.setVisible( true );

    mRetryButton.setVisible( false );
    mRedetectButton.setVisible( false );
    mDisableCustomServerButton.setVisible( false );
    mTutorialButton.setVisible( false );
    mTutOneButton.setVisible( false );
    mTutTwoButton.setVisible( false );
    mUpdateYummyLifeButton.setVisible( false );

    mAcntNextButton.setVisible( false );
    mAcntPrevButton.setVisible( false );
    mAcntDeleteButton.setVisible( false );
    mAcntDeleteConfirmButton.setVisible( false );
    mAcntSelectButton.setVisible( false );
    mAcntEditButton.setVisible( false );
    mAcntNewButton.setVisible( false );
    mAcntCloseButton.setVisible( false );
    
    mAtSignButton.setMouseOverTip( translate( "atSignTip" ) );

    mLoginButton.setMouseOverTip( translateWithDefault( "yummyLifeLoginButtonTip", "PLAY ONLINE - GOOD LUCK <3") );
    mClearAccountButton.setMouseOverTip( translate( "clearAccountTip" ) );
    mChangeAccountButton.setMouseOverTip( translateWithDefault( "yummyLifeChangeAccountButtonTip", "SWITCH TO A DIFFERENT ACCOUNT") );
    
    mFriendsButton.setMouseOverTip( translate( "friendsTip" ) );
    mGenesButton.setMouseOverTip( translate( "genesTip" ) );
    mFamilyTreesButton.setMouseOverTip( translate( "familyTreesTip" ) );

    mTutorialButton.setMouseOverTip( translateWithDefault("yummyLifeTutorialButtonTip", "CHOOSE A TUTORIAL"));
    mTutOneButton.setMouseOverTip( translateWithDefault("yummyLifeTutOneButtonTip", "PLAY TUTORIAL #1"));
    mTutTwoButton.setMouseOverTip( translateWithDefault("yummyLifeTutTwoButtonTip", "PLAY TUTORIAL #2"));

    mAcntNextButton.setMouseOverTip( translateWithDefault("yummyLifeAcntNextButtonTip", "NEXT ACCOUNT"));
    mAcntPrevButton.setMouseOverTip( translateWithDefault("yummyLifeAcntPrevButtonTip", "PREVIOUS ACCOUNT"));
    mAcntDeleteButton.setMouseOverTip( translateWithDefault("yummyLifeAcntDeleteButtonTip", "DELETE ACCOUNT"));
    mAcntDeleteConfirmButton.setMouseOverTip( translateWithDefault("yummyLifeAcntDeleteConfirmButtonTip", "CONFIRM DELETE"));
    mAcntSelectButton.setMouseOverTip( translateWithDefault("yummyLifeAcntSelectButtonTip", "SELECT ACCOUNT"));
    mAcntEditButton.setMouseOverTip( translateWithDefault("yummyLifeAcntEditButtonTip", "EDIT ACCOUNT"));
    mAcntNewButton.setMouseOverTip( translateWithDefault("yummyLifeAcntNewButtonTip", "NEW ACCOUNT"));
    mAcntCloseButton.setMouseOverTip( translateWithDefault("yummyLifeAcntCloseButtonTip", "CLOSE WINDOW"));
    
    
    mServicesButton.setMouseOverTip( translate( "servicesTip" ) );
    

    int reviewPosted = SettingsManager::getIntSetting( "reviewPosted", 0 );
    
    if( reviewPosted ) {
        mReviewButton.setLabelText( translate( "updateReviewButton" ) );
        }
    

    // YummyLife: Check for updates, and display button if needed
    if(HetuwMod::bCheckGitHubForUpdates) {
        int updateYummyLifeStatus = getModdedClientUpdateStatus(); // This function call may delay game start, usually less than 0.1 seconds
        if(updateYummyLifeStatus > 0) { // 1 or 2 means minor or major update available, set button visible and set tip
            mUpdateYummyLifeButton.setVisible(true);
            const char *updateType = (updateYummyLifeStatus == 1) ? "MINOR" : "MAJOR";
            char *tip = autoSprintf(translateWithDefault("updateAvailable", "%s UPDATE (v%d.%d)"), updateType, latestMajorClientVersion, latestMinorClientVersion);
            mUpdateYummyLifeButton.setMouseOverTip(tip);
            delete [] tip;
        }
    }

    // to dodge quit message
    setTipPosition( true );

    // Overrides above code, places on same height of "YummyLife Vx - Oliver"
    setCustomTipHeight(mServicesButton.getPosition().y + 50);
    
    // YummyLife: Initilize the gallery
    YummyLife::Gallery::initGallery("screenShots");

    // YummyLife: Initilize live resources (for gallery)
    if(HetuwMod::bAllowLiveResources) {
        string clientVersionTag = "v" + std::to_string(versionNumber) + string(yumSubVersion);
        YummyLife::LiveResources::initLiveResources(clientVersionTag.c_str());
    }

    // Couldn't load gallery, or no screenshots taken
    if(YummyLife::Gallery::getGallerySize() <= 0) {
        mNextImageButton.setVisible(false);
        mPrevImageButton.setVisible(false);
    }else{
        // Set bounds, and load a random image
        YummyLife::Gallery::setGalleryMaxDimensions(400, 300);
        YummyLife::Gallery::loadRandomGalleryImage();
    }

    if(true) {
        YummyLife::AccountManager::loadSavedAccounts();
    }
}
          
        
ExistingAccountPage::~ExistingAccountPage() {
    }



void ExistingAccountPage::clearFields() {
    mEmailField.setText( "" );
    mKeyField.setText( "" );
    }



void ExistingAccountPage::showReviewButton( char inShow ) {
    mReviewButton.setVisible( inShow );
    }



void ExistingAccountPage::showDisableCustomServerButton( char inShow ) {
    mDisableCustomServerButton.setVisible( inShow );
    }




void ExistingAccountPage::makeActive( char inFresh ) {

    mAHAPSettingsButton.setVisible( isAHAP );

    mYumRebirth.onMakeActive();


    useContentSettings();
    
    int tutorialEnabled = 
        SettingsManager::getIntSetting( "tutorialEnabled", 0 );
    
    useMainSettings();
    
    
    if( tutorialEnabled && 
        SettingsManager::getIntSetting( "tutorialDone", 0 ) ) {
        
        mTutorialButton.setVisible( true );
        }
    else {
        // tutorial disabled,
        // or 
        // tutorial enabled, and they haven't completed it yet, so
        // tutorial forced anyway (don't need to show button for it)
        mTutorialButton.setVisible( false );
        }

    mFramesCounted = 0;
    mPageActiveStartTime = game_getCurrentTime();    
    
    // don't re-measure every time we return to this screen
    // it slows the player down too much
    // re-measure only at first-startup
    //mFPSMeasureDone = false;
    
    mLoginButton.setVisible( false );
    mTutorialButton.setVisible( false );
    mFriendsButton.setVisible( false );
    mGenesButton.setVisible( false );
    
    int skipFPSMeasure = SettingsManager::getIntSetting( "skipFPSMeasure", 0 );
    
    if( skipFPSMeasure ) {
        mFPSMeasureDone = true;
        mRetryButton.setVisible( false );
        mRedetectButton.setVisible( false );
        }

    if( mFPSMeasureDone && ! mRetryButton.isVisible() ) {
        // skipping measure OR we are returning to this page later
        // and not measuring again
        mLoginButton.setVisible( true );
        mTutorialButton.setVisible( true );
        mFriendsButton.setVisible( true );
        triggerLifeTokenUpdate();
        triggerFitnessScoreUpdate();
        }
    else if( mFPSMeasureDone && mRetryButton.isVisible() ) {
        // left screen after failing
        // need to measure again after returning
        mRetryButton.setVisible( false );
        mRedetectButton.setVisible( false );
        mFPSMeasureDone = false;
        }
    
    setChangeAccountWindow( false );
    setEditAccountWindow( false );

    int pastSuccess = SettingsManager::getIntSetting( "loginSuccess", 0 );
    pastSuccess = true; // YummyLife: Always hide fields if possible

    char *emailText = mEmailField.getText();
    char *keyText = mKeyField.getText();

    // don't hide field contents unless there is something to hide
    if( ! pastSuccess || 
        ( strcmp( emailText, "" ) == 0 
          &&
          strcmp( keyText, "" ) == 0 ) ) {

        mEmailField.focus();

        mFamilyTreesButton.setVisible( false );
        }
    else {
        mEmailField.unfocus();
        mKeyField.unfocus();
        
        mEmailField.setContentsHidden( true );
        mKeyField.setContentsHidden( true );
        
        char *url = getLineageServerURL();

        char show = ( strcmp( url, "" ) != 0 )
            && isURLLaunchSupported();
        mFamilyTreesButton.setVisible( show );
        delete [] url;
        }
    
    delete [] emailText;
    delete [] keyText;
    
    mPasteButton.setVisible( false );
    mPasteEmailButton.setVisible( false );
    mAtSignButton.setVisible( false );

    mTutOneButton.setVisible( false );
    mTutTwoButton.setVisible( false );

    int reviewPosted = SettingsManager::getIntSetting( "reviewPosted", 0 );
    
    if( reviewPosted ) {
        mReviewButton.setLabelText( translate( "updateReviewButton" ) );
        }
    else {
        mReviewButton.setLabelText( translate( "postReviewButton" ) );
        }

    // YumLife: always show review button
    mReviewButton.setVisible( true );

    if( SettingsManager::getIntSetting( "useSteamUpdate", 0 ) ) {
        if( ! loginEditOverride ) {
            mEmailField.setVisible( false );
            mKeyField.setVisible( false );
            mEmailField.unfocus();
            mKeyField.unfocus();
            
            mClearAccountButton.setVisible( false );
            
            mViewAccountButton.setVisible( true );
            mHideAccount = true;
            }
        else {
            mEmailField.setVisible( true );
            mKeyField.setVisible( true );       
     
            mClearAccountButton.setVisible( true );

            mEmailField.setContentsHidden( false );
            mKeyField.setContentsHidden( false );
            mEmailField.focus();
            
            loginEditOverride = false;
            
            mViewAccountButton.setVisible( false );
            mHideAccount = false;
            }
        }
    else {
        mHideAccount = false;
        mViewAccountButton.setVisible( false );
        }
    }



void ExistingAccountPage::makeNotActive() {
    for( int i=0; i<2; i++ ) {
        mFields[i]->unfocus();
        }
    }



void ExistingAccountPage::hideLeftScreenItems(bool hide) {
    mNextImageButton.setVisible(!hide);
    mPrevImageButton.setVisible(!hide);
    mSettingsButton.setVisible(!hide);
    mServicesButton.setVisible(!hide);
}

void ExistingAccountPage::setChangeAccountWindow(bool open, int hoveredAccountIndexOverride) {
    confirmDeleteAccount = false;
    showChangeAccountWindow = open;
    hideLeftScreenItems(showChangeAccountWindow);

    mAcntNextButton.setVisible(showChangeAccountWindow);
    mAcntPrevButton.setVisible(showChangeAccountWindow);
    mAcntDeleteButton.setVisible(showChangeAccountWindow);
    mAcntDeleteConfirmButton.setVisible(false);
    mAcntSelectButton.setVisible(showChangeAccountWindow);
    mAcntEditButton.setVisible(showChangeAccountWindow);
    mAcntNewButton.setVisible(showChangeAccountWindow);
    mAcntCloseButton.setVisible(showChangeAccountWindow);

    mAcntEmailField.setVisible(false);
    mAcntKeyField.setVisible(false);
    mAcntNotesField.setVisible(false);
    mAcntConfirmButton.setVisible(false);
    mAcntCancelButton.setVisible(false);
    mAcntPasteButton.setVisible(false);

    if(showChangeAccountWindow) {
        selectedAccountIndex = YummyLife::AccountManager::findMatchingLocalAccount(
            mEmailField.getText(),
            mKeyField.getText()
        );
        printf("selectedAccountIndex = %d\n", selectedAccountIndex);

        // Choose either override index, or selected index, or 0
        if(hoveredAccountIndexOverride != -1) hoveredAccountIndex = hoveredAccountIndexOverride;
        else if(hoveredAccountIndex == -1) 
            hoveredAccountIndex = (selectedAccountIndex != -1) ? selectedAccountIndex : 0;
    }
}

void ExistingAccountPage::setEditAccountWindow(bool open, int accountIndex) {
    showEditAccountWindow = open;

    hideLeftScreenItems(showEditAccountWindow);

    mAcntCloseButton.setVisible(showEditAccountWindow); // Close button still visible in edit mode
    mAcntCancelButton.setVisible(showEditAccountWindow);
    mAcntPasteButton.setVisible(showEditAccountWindow);
    mAcntConfirmButton.setVisible(showEditAccountWindow);
    mAcntEmailField.setVisible(showEditAccountWindow);
    mAcntKeyField.setVisible(showEditAccountWindow);
    mAcntNotesField.setVisible(showEditAccountWindow);

    if(open) {
        int idx = YummyLife::AccountManager::findAccountByIndex(accountIndex, nullptr);
        if(idx == -1 || accountIndex == -1) { // New account
            mAcntEmailField.setText("");
            mAcntKeyField.setText("");
            mAcntNotesField.setText("");
            return;
        }
        YummyLife::AccountManager::Account acc = YummyLife::AccountManager::accounts[idx];
        mAcntEmailField.setText(acc.email.c_str());
        mAcntKeyField.setText(acc.key.c_str());
        mAcntNotesField.setText(acc.notes.c_str());
    }
}

void ExistingAccountPage::onAccountDeleteConfirmed() {
    printf("Deleting account at index %d\n", hoveredAccountIndex);
    bool result = YummyLife::AccountManager::deleteAccountAtIndex(hoveredAccountIndex);
    if(!result) {
        printf("Failed to delete account at index %d\n", hoveredAccountIndex);
        return;
    }
    // Adjust hovered index
    if(hoveredAccountIndex >= (int)YummyLife::AccountManager::accounts.size()) {
        hoveredAccountIndex = (int)YummyLife::AccountManager::accounts.size() - 1;
    }
    YummyLife::AccountManager::saveAccounts();
    setChangeAccountWindow(true, hoveredAccountIndex); // Refresh
}
void ExistingAccountPage::onAccountDeleteCancelled() {
    confirmDeleteAccount = false;
    mAcntDeleteButton.setVisible(true);
    mAcntDeleteConfirmButton.setVisible(false);
}
void ExistingAccountPage::onAccountDeleteStarted() {
    confirmDeleteAccount = true;
    mAcntDeleteButton.setVisible(false);
    mAcntDeleteConfirmButton.setVisible(true);
}


void ExistingAccountPage::step() {
    mPasteButton.setVisible( isClipboardSupported() &&
                             mKeyField.isFocused() );
    mPasteEmailButton.setVisible( isClipboardSupported() &&
                                  mEmailField.isFocused() );
    mAtSignButton.setVisible( mEmailField.isFocused() );

    shouldAcntPasteButtonBeVisable = isClipboardSupported() && !inSaveAccountProcess && showEditAccountWindow && 
                                  (mAcntEmailField.isFocused() || mAcntKeyField.isFocused() || mAcntNotesField.isFocused());
    mAcntPasteButton.setVisible( shouldAcntPasteButtonBeVisable );
    
        // Handle save account process
    if( inSaveAccountProcess ) {
        bool timedOut = (accountSaveStartTime + SAVE_ACCOUNT_TIMEOUT_SECONDS < game_getCurrentTime());
        bool gotName = (getLeaderboardName() != NULL);

        // Timed out or leaderboard name ready
        if(timedOut || getLeaderboardName() != NULL) {
            inSaveAccountProcess = false;
            
            // We got leaderboard name or timed out, ready to save changes and exit editor
            YummyLife::AccountManager::Account &selectedAccountForEdit = YummyLife::AccountManager::accounts[selectedAccountIndexForEdit];
            selectedAccountForEdit.leaderboardName = "";
            if(gotName) selectedAccountForEdit.leaderboardName = getLeaderboardName();

            YummyLife::AccountManager::saveAccounts();
            if(timedOut) printf("No leaderboard name saved.\n");
            else printf("LeaderboardName %s received.\n", getLeaderboardName());
            setEditAccountWindow(false, -1);
            setChangeAccountWindow(true, selectedAccountIndexForEdit); // Hover edited account
        }
    }
}



void ExistingAccountPage::actionPerformed( GUIComponent *inTarget ) {
    if( inTarget == &mLoginButton ) {
        forceCompleteTutorial();
        processLogin( true, "done" );
        }
    else if( inTarget == &mTutorialButton ) {
        bool v = mTutOneButton.isVisible();
        mTutOneButton.setVisible(!v);
        mTutTwoButton.setVisible(!v);
        }
    else if( inTarget == &mTutOneButton ) {
        forceCompleteTutorial();
        processLogin( true, "tutorial1" );
        }
    else if( inTarget == &mTutTwoButton ) {
        forceCompleteTutorial();
        processLogin( true, "tutorial2" );
        }
    else if( inTarget == &mServicesButton ) {
        setSignal( "services" );
        }
    else if( inTarget == &mAHAPSettingsButton ) {
        setSignal( "ahapSettings" );
        }
    else if( inTarget == &mClearAccountButton ) {
        SettingsManager::setSetting( "email", "" );
        SettingsManager::setSetting( "accountKey", "" );
        SettingsManager::setSetting( "loginSuccess", 0 );
        SettingsManager::setSetting( "twinCode", "" );

        mEmailField.setText( "" );
        mKeyField.setText( "" );
        
        if( userEmail != NULL ) {
            delete [] userEmail;
            }
        userEmail = mEmailField.getText();
        
        if( accountKey != NULL ) {
            delete [] accountKey;
            }
        accountKey = mKeyField.getText();
        
        mEmailField.setContentsHidden( false );
        mKeyField.setContentsHidden( false );
        }
    else if( inTarget == &mFriendsButton ) {
        processLogin( true, "friends" );
        }
    else if( inTarget == &mGenesButton ) {
        setSignal( "genes" );
        }
    else if( inTarget == &mFamilyTreesButton ) {
        char *url = getLineageServerURL();

        if( strcmp( url, "" ) != 0 ) {
            char *email = mEmailField.getText();
            
            char *string_to_hash = 
                autoSprintf( "%d", 
                             randSource.getRandomBoundedInt( 0, 2000000000 ) );
             
            char *pureKey = getPureAccountKey();
            
            char *ticket_hash = hmac_sha1( pureKey, string_to_hash );

            delete [] pureKey;

            char *lowerEmail = stringToLowerCase( email );

            char *emailHash = computeSHA1Digest( lowerEmail );

            delete [] lowerEmail;

            char *fullURL = autoSprintf( "%s?action=front_page&email_sha1=%s"
                                         "&ticket_hash=%s"
                                         "&string_to_hash=%s",
                                         url, emailHash, 
                                         ticket_hash, string_to_hash );
            delete [] email;
            delete [] emailHash;
            delete [] ticket_hash;
            delete [] string_to_hash;
            
            launchURL( fullURL );
            delete [] fullURL;
            }
        delete [] url;
        }
    else if( inTarget == &mViewAccountButton ) {
        if( mHideAccount ) {
            mViewAccountButton.setLabelText( translate( "hide" ) );
            }
        else {
            mViewAccountButton.setLabelText( translate( "view" ) );
            }
        mHideAccount = ! mHideAccount;
        }
    else if( inTarget == &mCancelButton ) {
        setSignal( "quit" );
        }
    else if( inTarget == &mSettingsButton ) {
        setSignal( "settings" );
        }
    else if( inTarget == &mReviewButton ) {
        if( userEmail != NULL ) {
            delete [] userEmail;
            }
        userEmail = mEmailField.getText();
        
        if( accountKey != NULL ) {
            delete [] accountKey;
            }
        accountKey = mKeyField.getText();
        
        setSignal( "review" );
        }
    else if( inTarget == &mAtSignButton ) {
        mEmailField.insertCharacter( '@' );
        }
    else if( inTarget == &mPasteButton ) {
        char *clipboardText = getClipboardText();
        
        mKeyField.setText( clipboardText );
    
        delete [] clipboardText;
        }
    else if( inTarget == &mPasteEmailButton ) {
        char *clipboardText = getClipboardText();
        
        mEmailField.setText( clipboardText );
    
        delete [] clipboardText;
        }
    else if( inTarget == &mRetryButton ) {
        mFPSMeasureDone = false;
        mPageActiveStartTime = game_getCurrentTime();
        mFramesCounted = 0;
        
        mRetryButton.setVisible( false );
        mRedetectButton.setVisible( false );
        
        setStatus( NULL, false );
        }
    else if( inTarget == &mRedetectButton ) {
        SettingsManager::setSetting( "targetFrameRate", -1 );
        SettingsManager::setSetting( "countingOnVsync", -1 );
        
        char relaunched = relaunchGame();
        
        if( !relaunched ) {
            printf( "Relaunch failed\n" );
            setSignal( "relaunchFailed" );
            }
        else {
            printf( "Relaunched... but did not exit?\n" );
            setSignal( "relaunchFailed" );
            }
        }
    else if( inTarget == &mDisableCustomServerButton ) {
        SettingsManager::setSetting( "useCustomServer", 0 );
        mDisableCustomServerButton.setVisible( false );
        processLogin( true, "done" );
        }
    else if( inTarget == &mNextImageButton) {
        YummyLife::Gallery::loadNextGalleryImage();
        } 
    else if( inTarget == &mPrevImageButton) {
        YummyLife::Gallery::loadPreviousGalleryImage();
        }
    else if( inTarget == &mUpdateYummyLifeButton) {
        char YummyLifeRepoURL[] = "https://github.com/olliez-mods/YummyLife/releases/latest";
        launchURL(YummyLifeRepoURL);
        }
    else if( inTarget == &mChangeAccountButton ) {
        setEditAccountWindow( false );
        setChangeAccountWindow( !showChangeAccountWindow );
        inSaveAccountProcess = false;
        }
    else if (inTarget == &mAcntCloseButton) {
        setEditAccountWindow(false, -1);
        setChangeAccountWindow(false);
        inSaveAccountProcess = false;
        }
    else if( inTarget == &mAcntCancelButton ) {
        setEditAccountWindow(false, -1);
        setChangeAccountWindow(true);
        inSaveAccountProcess = false;
        }
    else if(inTarget == &mAcntPasteButton) {
        char *clipboardText = getClipboardText();
        if(mAcntEmailField.isFocused()) mAcntEmailField.setText( clipboardText );
        else if(mAcntKeyField.isFocused()) mAcntKeyField.setText( clipboardText );
        else if(mAcntNotesField.isFocused()) mAcntNotesField.setText( clipboardText );
        delete [] clipboardText;
        }
    else if(inTarget == &mAcntNextButton) {
        onAccountDeleteCancelled();
        if(hoveredAccountIndex != -1) {
            if(hoveredAccountIndex < (int)YummyLife::AccountManager::accounts.size() - 1) {
                hoveredAccountIndex++;
                }
            }
        }
    else if(inTarget == &mAcntPrevButton) {
        onAccountDeleteCancelled();
        if(hoveredAccountIndex != -1) {
            if(hoveredAccountIndex > 0) {
                hoveredAccountIndex--;
                }
            }
        }
    else if(inTarget == &mAcntSelectButton) {
        if(hoveredAccountIndex != -1) {
            selectAccountAtIndex(hoveredAccountIndex);
            setChangeAccountWindow(false);
            }
        }
    else if(inTarget == &mAcntEditButton) {
        if(hoveredAccountIndex != -1) {
            setChangeAccountWindow(false);
            setEditAccountWindow(true, hoveredAccountIndex);
            }
        }
    else if(inTarget == &mAcntNewButton) {
        setChangeAccountWindow(false);
        hoveredAccountIndex = -1; // Not hovering any existing account, new account
        setEditAccountWindow(true, hoveredAccountIndex);
    }
    else if( inTarget == &mAcntDeleteButton ) {
        if(hoveredAccountIndex != -1) {
            onAccountDeleteStarted();
            }
        }
    else if( inTarget == &mAcntDeleteConfirmButton ) {
        if(hoveredAccountIndex != -1) {
            onAccountDeleteConfirmed();
        }
    }
    else if( inTarget == &mAcntConfirmButton ) {
        // New account
        if(hoveredAccountIndex == -1) {
            int newIndex = YummyLife::AccountManager::addAccountLocal(
                mAcntEmailField.getText(),
                mAcntKeyField.getText(),
                mAcntNotesField.getText()
            );

            if(newIndex == -1) {
                printf("Failed to add new account.\n");
                setEditAccountWindow(true, -1);
                return;
            }

            printf("Adding new account at index %d\n", newIndex);
            mAcntConfirmButton.setVisible(false);
            accountSaveStartTime = game_getCurrentTime();
            inSaveAccountProcess = true;
            selectedAccountIndexForEdit = newIndex;
            selectAccountAtIndex(newIndex);
        } else { // Save changes to existing account
            YummyLife::AccountManager::Account &acc = YummyLife::AccountManager::accounts[hoveredAccountIndex];
            acc.email = mAcntEmailField.getText();
            acc.key = mAcntKeyField.getText();
            acc.notes = mAcntNotesField.getText();

            mAcntConfirmButton.setVisible(false);

            printf("Saving changes to account index %d\n", hoveredAccountIndex);
            selectAccountAtIndex(hoveredAccountIndex);
            accountSaveStartTime = game_getCurrentTime();
            inSaveAccountProcess = true;
            selectedAccountIndexForEdit = hoveredAccountIndex;
            }
        }
    }



void ExistingAccountPage::switchFields() {
    if( mFields[0]->isFocused() ) {
        mFields[1]->focus();
        }
    else if( mFields[1]->isFocused() ) {
        mFields[0]->focus();
        }
    }

    

void ExistingAccountPage::keyDown( unsigned char inASCII ) {
    if( inASCII == 9 ) {
        // tab
        switchFields();
        return;
        }

    if( inASCII == 10 || inASCII == 13 ) {
        // enter key
        
        if( mKeyField.isFocused() ) {

            processLogin( true, "done" );
            
            return;
            }
        else if( mEmailField.isFocused() ) {
            switchFields();
            }
        }
    }



void ExistingAccountPage::specialKeyDown( int inKeyCode ) {
    if( inKeyCode == MG_KEY_DOWN ||
        inKeyCode == MG_KEY_UP ) {
        
        switchFields();
        return;
        }
    }


// YummyLife: Store email and key, and optionally save to settings
void ExistingAccountPage::updateLoginInfo( const char *inEmail, const char *inKey, bool saveToSettings ) {
    mEmailField.setText( inEmail );
    mKeyField.setText( inKey );

    if( userEmail != NULL ) delete [] userEmail;
    userEmail = stringDuplicate( inEmail );
        
    if( accountKey != NULL ) delete [] accountKey;
    accountKey = stringDuplicate( inKey );

    if( saveToSettings ) {
        SettingsManager::setSetting( "email", userEmail );
        SettingsManager::setSetting( "accountKey", accountKey );
    }
}

void ExistingAccountPage::selectAccountAtIndex(int index) {
    if(index < 0 || index >= (int)YummyLife::AccountManager::accounts.size()) return;
    YummyLife::AccountManager::Account acc = YummyLife::AccountManager::accounts[index];
    if(acc.type == acc.LOCAL && acc.email != "" && acc.key != "") {
        updateLoginInfo( acc.email.c_str(), acc.key.c_str(), true );
        mEmailField.setContentsHidden(true);
        mKeyField.setContentsHidden(true);
        printf("Switched to local account: %s\n", acc.leaderboardName.c_str());

        // Update leaderboard info
        freeFitnessScore();
        initFitnessScore();
        triggerLifeTokenUpdate();
        triggerFitnessScoreUpdate();
    }
}


void ExistingAccountPage::processLogin( char inStore, const char *inSignal ) {
    if( userEmail != NULL ) {
        delete [] userEmail;
        }
    userEmail = mEmailField.getText();
        
    if( accountKey != NULL ) {
        delete [] accountKey;
        }
    accountKey = mKeyField.getText();

    if( !gamePlayingBack ) {
        
        if( inStore ) {
            SettingsManager::setSetting( "email", userEmail );
            SettingsManager::setSetting( "accountKey", accountKey );
            }
        else {
            SettingsManager::setSetting( "email", "" );
            SettingsManager::setSetting( "accountKey", "" );
            }
        }
    
                
    setSignal( inSignal );
    }



void ExistingAccountPage::draw( doublePair inViewCenter, 
                                double inViewSize ) {
    
    
    if( !mFPSMeasureDone ) {
        double timePassed = game_getCurrentTime() - mPageActiveStartTime;
        double settleTime = 0.1;

        if ( timePassed > settleTime ) {
            mFramesCounted ++;
            }
        
        if( timePassed > 1 + settleTime ) {
            double fps = mFramesCounted / ( timePassed - settleTime );
            int targetFPS = 
                SettingsManager::getIntSetting( "targetFrameRate", -1 );
            char fpsFailed = true;
            
            if( targetFPS != -1 ) {
                
                double diff = fabs( fps - targetFPS );
                
                if( diff / targetFPS > 0.1 ) {
                    // more than 10% off

                    fpsFailed = true;
                    }
                else {
                    // close enough
                    fpsFailed = false;
                    }
                }

            if( !fpsFailed ) {
                mLoginButton.setVisible( true );
                mTutorialButton.setVisible( true );
                mFriendsButton.setVisible( true ); // YummyLife: Always show friends button
                
                int pastSuccess = 
                    SettingsManager::getIntSetting( "loginSuccess", 0 );
                if( pastSuccess ) {
                    mFriendsButton.setVisible( true );
                    }
                
                triggerLifeTokenUpdate();
                triggerFitnessScoreUpdate();
                }
            else {
                // show error message
                
                char *message = autoSprintf( translate( "fpsErrorLogin" ),
                                                        fps, targetFPS );
                setStatusDirect( message, true );
                delete [] message;

                setStatusPosition( true );
                mRetryButton.setVisible( true );
                mRedetectButton.setVisible( true );
                }
            

            mFPSMeasureDone = true;
            }
        }
    

    setDrawColor( 1, 1, 1, 1 );
    
    doublePair pos = {-400, 0};

    if(YummyLife::Gallery::getGallerySize() > 0 && !showChangeAccountWindow && !showEditAccountWindow) {
        YummyLife::Gallery::drawGallery(pos);
    }

    pos.x = -9;
    pos.y = -225;
    
    if (!mYumRebirth.isEnabled()) {
        //drawSprite( instructionsSprite, pos ); // YummyLife: Disable for testing
    }

    if (HetuwMod::privateModeEnabled) {
        mServicesButton.setVisible( false );
    }

    if( ! mEmailField.isVisible() ) {
        char *email = mEmailField.getText();
        
        const char *transString = "email";
        
        char *steamSuffixPointer = strstr( email, "@steamgames.com" );
        
        char coverChar = 'x';

        if( steamSuffixPointer != NULL ) {
            // terminate it
            steamSuffixPointer[0] ='\0';
            transString = "steamID";
            coverChar = 'X';
            }

        if( mHideAccount ) {
            int len = strlen( email );
            for( int i=0; i<len; i++ ) {
                email[i] = coverChar;
                }
            if( len > 13 ) {
                // truncate.  Don't overlap with GENETIC FITNESS
                email[ 13 ] = '\0';
                }
            }
        

        char *s = autoSprintf( "%s %s", translate( transString ), email );
        
        pos = mEmailField.getPosition();

        pos.x -= 80;
        setDrawColor( 1, 1, 1, 1.0 );
        mainFont->drawString( s, pos, alignCenter );
        
        delete [] email;
        delete [] s;
        }

    if( ! mKeyField.isVisible() ) {
        char *key = mKeyField.getText();

        if( mHideAccount ) {
            int len = strlen( key );
            for( int i=0; i<len; i++ ) {
                if( key[i] != '-' ) {
                    key[i] = 'X';
                    }
                }   
            }

        char *s = autoSprintf( "%s %s", translate( "accountKey" ), key );
        
        pos = mKeyField.getPosition();
        
        pos.x -= 80;
        pos.y += 10;
        setDrawColor( 1, 1, 1, 1.0 );
        mainFont->drawString( s, pos, alignCenter );
        
        delete [] key;
        delete [] s;
        }
    

    // YummyLife: Adjust position of lifeTokensString to be in top right
    //pos = mEmailField.getPosition();
    //pos.y += 100;
    pos.x = 0;
    pos.y = 60;

    if( mFPSMeasureDone && 
        ! mRedetectButton.isVisible() &&
        ! mDisableCustomServerButton.isVisible() ) {
        
        drawTokenMessage( pos );
        
        /* YummyLife: We want it to not be aligned with the email field
        pos = mEmailField.getPosition();
        
        pos.x = 
            ( mTutorialButton.getPosition().x + 
              mLoginButton.getPosition().x )
            / 2;

        pos.x -= 32;
        */
        pos.x = 0;
        pos.y = 75;

        YummyLife::drawLeaderboardName(pos);

        pos.x = 0;
        pos.y = 20;
        
        drawFitnessScore( pos );

        if( isFitnessScoreReady() ) {
            mGenesButton.setVisible( true );
            }

        // YummyLife: Add fade affect, since the tip now covers this text
        if(mTip == NULL && mLastTipFade < 0.5){
            // YumLife: show window title with version info
            pos = mServicesButton.getPosition();
            pos.y += 50;
            pos.x = 0;
            float fade = 1.0 - (mLastTipFade * 2);
            setDrawColor( 0.5, 1, 0.5, fade );
            mainFont->drawString( getWindowTitle(), pos, alignCenter );
            }
        }

    if(showEditAccountWindow || showChangeAccountWindow) { 
        // Display background for both edit and change account windows
        setDrawColor(0.2, 0.2, 0.3, 0.95);
        doublePair backGroundStart = {-610, 300};
        doublePair backGroundEnd = {-150, -240};
        drawRect(backGroundStart.x, backGroundStart.y, backGroundEnd.x, backGroundEnd.y);

        mAcntCloseButton.base_draw(inViewCenter, inViewSize);

        if(showEditAccountWindow) {
            // Draw the edit account fields and buttons over the top of the window
            mAcntEmailField.base_draw(inViewCenter, inViewSize);
            mAcntKeyField.base_draw(inViewCenter, inViewSize);
            mAcntNotesField.base_draw(inViewCenter, inViewSize);
            if(!inSaveAccountProcess) mAcntConfirmButton.base_draw(inViewCenter, inViewSize);
            mAcntCancelButton.base_draw(inViewCenter, inViewSize);
            if(shouldAcntPasteButtonBeVisable) mAcntPasteButton.base_draw(inViewCenter, inViewSize);

            if(inSaveAccountProcess) {
                // Draw saving message
                doublePair msgPos1 = { (backGroundStart.x + backGroundEnd.x) / 2, backGroundEnd.y + 190 };
                doublePair msgPos2 = {  msgPos1.x, msgPos1.y - 40 };
                setDrawColor(1, 1, 1, 1);
                mainFont->drawString( translateWithDefault("yummyLifeSavingAccountMessage", "Leaderboard Name..."), msgPos1, alignCenter);
                char* secondsStr = autoSprintf("Timeout: %.1f/%.0fs", game_getCurrentTime() - accountSaveStartTime, SAVE_ACCOUNT_TIMEOUT_SECONDS);
                mainFont->drawString( secondsStr, msgPos2, alignCenter);
                delete [] secondsStr;
            }
        }

        if(showChangeAccountWindow) {

            int maxNumberLinesInView = 6;

            // Draw the account management buttons over the top of the window
            mAcntNewButton.base_draw(inViewCenter, inViewSize);
            mAcntEditButton.base_draw(inViewCenter, inViewSize);
            mAcntSelectButton.base_draw(inViewCenter, inViewSize);
            if(confirmDeleteAccount) mAcntDeleteConfirmButton.base_draw(inViewCenter, inViewSize);
            else mAcntDeleteButton.base_draw(inViewCenter, inViewSize);
            mAcntPrevButton.base_draw(inViewCenter, inViewSize);
            mAcntNextButton.base_draw(inViewCenter, inViewSize);

            // Draw account list (scrollable window)
            int totalAccounts = (int)YummyLife::AccountManager::accounts.size();
            int visible = std::min(maxNumberLinesInView, totalAccounts);

            int startIndex = 0;
            if (hoveredAccountIndex != -1) {
                startIndex = hoveredAccountIndex - visible / 2;
                if (startIndex < 0) startIndex = 0;
                if (startIndex + visible > totalAccounts) startIndex = std::max(0, totalAccounts - visible);
            }

            for (int i = 0; i < visible; ++i) {
                int idx = startIndex + i;
                YummyLife::AccountManager::Account &acnt = YummyLife::AccountManager::accounts[idx];

                doublePair lb_name_pos = {backGroundStart.x + 10, backGroundStart.y - 120 - (i * 75)};
                doublePair notes_pos = {backGroundStart.x + 10, lb_name_pos.y - 30};

                // Draw selection indicator
                if(idx == selectedAccountIndex) {
                    doublePair selectPos = {backGroundEnd.x - 50, lb_name_pos.y + 10};
                    setDrawColor(0, 1, 0, 1);
                    mainFont->drawString("*", selectPos, alignCenter);
                }

                if(idx == hoveredAccountIndex) {
                    doublePair highlightPosStart = {backGroundStart.x + 5, lb_name_pos.y + 30};
                    doublePair highlightPosEnd = {backGroundEnd.x - 5, notes_pos.y - 15};
                    setDrawColor(1, 1, 0, 0.5);
                    drawRect(highlightPosStart.x, highlightPosStart.y, highlightPosEnd.x, highlightPosEnd.y);
                    setDrawColor(1, 1, 0, 1); // Highlight text in yellow
                } else {
                    setDrawColor(1, 1, 1, 1);
                }

                if(idx == selectedAccountIndex) {
                    setDrawColor(0, 1, 0, 1); // Selected account text in green
                }

                // Draw fields
                const char* lbName = (acnt.leaderboardName == "") ? "N/A" : acnt.leaderboardName.c_str();
                mainFont->drawString(lbName, lb_name_pos, alignLeft);
                const char* notes = (acnt.notes == "") ? "-----" : acnt.notes.c_str();
                mainFont->drawString(notes, notes_pos, alignLeft);
            }

            // Optional: draw small up/down indicators
            setDrawColor(1, 1, 1, 1);
            if (startIndex > 0) {
                doublePair arrowPos = {backGroundEnd.x - 30, backGroundStart.y - 90};
                mainFont->drawString("^", arrowPos, alignCenter);
            }
            if (startIndex + visible < totalAccounts) {
                doublePair arrowPos = {backGroundEnd.x - 30, backGroundEnd.y + 30};
                mainFont->drawString("v", arrowPos, alignCenter);
            }
        }
    }
}