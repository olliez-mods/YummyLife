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

#include "hetuwmod.h"
#include "yumRebirthComponent.h"
#include "yummyLife.h"

static JenkinsRandomSource randSource;


extern Font *mainFont;


extern char gamePlayingBack;

extern char *userEmail;
extern char *accountKey;
extern char isAHAP;


extern SpriteHandle instructionsSprite;

extern char loginEditOverride;

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
          mAtSignButton( mainFont, 480, 73, "@" ),
          mPasteButton( mainFont, 380, 73, translate( "paste" ), 'v', 'V' ),
          mPasteEmailButton( mainFont, 380, 73, translate( "paste" ), 'v', 'V' ),
          mDisableCustomServerButton( mainFont, 0, 220, 
                                      translate( "disableCustomServer" ) ),
          mLoginButton( mainFont, 535, -280, translate( "loginButton") ),
          mFriendsButton( mainFont, 385, -280, translateWithDefault( "yummyLifeTwinsButton", "TWINS") ),
          mGenesButton( mainFont, 0, -65, translateWithDefault( "yummyLifeGenesButton", "LIFE DATA" ) ),
          mFamilyTreesButton( mainFont, 0, -140, translate( "familyTrees" ) ),
          mClearAccountButton( mainFont, 380, 135, translate( "clearAccount" ) ),
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

    setButtonStyle( &mDisableCustomServerButton );
    
    mFields[0] = &mEmailField;
    mFields[1] = &mKeyField;

    
    addComponent( &mLoginButton );
    addComponent( &mFriendsButton );
    addComponent( &mGenesButton );
    addComponent( &mFamilyTreesButton );
    addComponent( &mClearAccountButton );
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

    //addComponent( &mYumRebirth ); Yummylife: Disabled for now
    
    mLoginButton.addActionListener( this );
    mFriendsButton.addActionListener( this );
    mGenesButton.addActionListener( this );
    mFamilyTreesButton.addActionListener( this );
    mClearAccountButton.addActionListener( this );
    
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
    
    mDisableCustomServerButton.addActionListener( this );

    mRetryButton.setVisible( false );
    mRedetectButton.setVisible( false );
    mDisableCustomServerButton.setVisible( false );
    mTutOneButton.setVisible( false );
    mTutTwoButton.setVisible( false );
    
    mAtSignButton.setMouseOverTip( translate( "atSignTip" ) );

    mLoginButton.setMouseOverTip( translateWithDefault( "yummyLifeLoginButtonTip", "PLAY ONLINE - GOOD LUCK <3") );
    mClearAccountButton.setMouseOverTip( translate( "clearAccountTip" ) );
    
    mFriendsButton.setMouseOverTip( translate( "friendsTip" ) );
    mGenesButton.setMouseOverTip( translate( "genesTip" ) );
    mFamilyTreesButton.setMouseOverTip( translate( "familyTreesTip" ) );

    mTutorialButton.setMouseOverTip( translateWithDefault("yummyLifeTutorialButtonTip", "CHOOSE A TUTORIAL"));
    mTutOneButton.setMouseOverTip( translateWithDefault("yummyLifeTutOneButtonTip", "PLAY TUTORIAL #1"));
    mTutTwoButton.setMouseOverTip( translateWithDefault("yummyLifeTutTwoButtonTip", "PLAY TUTORIAL #2"));
    
    

    int reviewPosted = SettingsManager::getIntSetting( "reviewPosted", 0 );
    
    if( reviewPosted ) {
        mReviewButton.setLabelText( translate( "updateReviewButton" ) );
        }
    

    // to dodge quit message
    setTipPosition( true );

    // Overrides above code, places on same height of "YummyLife Vx - Oliver"
    setCustomTipHeight(mServicesButton.getPosition().y + 50);
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
    
    // YummyLife: Make tut always active
    mTutorialButton.setVisible( true );

    mFramesCounted = 0;
    mPageActiveStartTime = game_getCurrentTime();    
    
    // don't re-measure every time we return to this screen
    // it slows the player down too much
    // re-measure only at first-startup
    //mFPSMeasureDone = false;
    
    mLoginButton.setVisible( false );
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
    

    int pastSuccess = SettingsManager::getIntSetting( "loginSuccess", 0 );

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



void ExistingAccountPage::step() {
    mPasteButton.setVisible( isClipboardSupported() &&
                             mKeyField.isFocused() );
    mPasteEmailButton.setVisible( isClipboardSupported() &&
                                  mEmailField.isFocused() );
    mAtSignButton.setVisible( mEmailField.isFocused() );
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
    

    doublePair pos = { -9, -225 };
    
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
        pos.y = 100;

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
    }

