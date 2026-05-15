#include "TwinPage.h"

#include "message.h"
#include "buttonStyle.h"


#include "minorGems/game/Font.h"

#include "minorGems/util/stringUtils.h"
#include "minorGems/util/SettingsManager.h"
#include "minorGems/util/crc32.h"

#include "minorGems/game/game.h"

#include "yummyLife.h"
#include "accountHmac.h"
#include "minorGems/crypto/hashes/sha1.h"
extern char *userEmail;


#include <stdio.h>



extern Font *mainFont;
extern char *userEmail;

extern char *userTwinCode;
extern int userTwinCount;



TwinPage::TwinPage()
  : mCodeField( mainFont, 0, 128, 13, false, 
                translate( "twinCode" ),
                NULL,
                NULL ),
    mGenerateButton( mainFont, 0, 49, translate( "generate") ),
    mOpenFriendCodeShareButton( mainFont, -235, 49, translateWithDefault( "yummyOpenFriendShared", "SHARE CODES" ) ),
    mCopyButton( mainFont, 400, 170, translate( "copy" ) ),
    mPasteButton( mainFont, 400, 86, translate( "paste" ) ),
    mLoginButton( mainFont, 400, -280, translate( "loginButton" ) ),
    mYumRebirth( mainFont, 110, -40 ),
    mCancelButton( mainFont, -400, -280, 
                   translate( "cancel" ) ) {
    

    setButtonStyle( &mGenerateButton );
    setButtonStyle( &mOpenFriendCodeShareButton );
    setButtonStyle( &mCopyButton );
    setButtonStyle( &mPasteButton );
    setButtonStyle( &mLoginButton );
    setButtonStyle( &mCancelButton );
    
    addComponent( &mGenerateButton );
    addComponent( &mOpenFriendCodeShareButton );
    addComponent( &mCopyButton );
    addComponent( &mPasteButton );
    addComponent( &mLoginButton );
    addComponent( &mCancelButton );

    addComponent( &mYumRebirth );

    addComponent( &mCodeField );
    
    mGenerateButton.addActionListener( this );
    mOpenFriendCodeShareButton.addActionListener( this );
    mCopyButton.addActionListener( this );
    mPasteButton.addActionListener( this );
    mLoginButton.addActionListener( this );
    mCancelButton.addActionListener( this );

    
    mCodeField.addActionListener( this );
    
    mCodeField.setFireOnAnyTextChange( true );
    
    mLoginButton.setVisible( false );
    mOpenFriendCodeShareButton.setVisible( false );
    

    const char *choiceList[4] = { translate( "twins" ),
                                  translate( "triplets" ),
                                  translate( "quadruplets" ),
                                  translate( "sameFam" ) };
    
    mPlayerCountRadioButtonSet = 
        new RadioButtonSet( mainFont, 0, -100,
                            4, choiceList,
                            false, 4 );
    addComponent( mPlayerCountRadioButtonSet );


    if( ! isClipboardSupported() ) {
        mCopyButton.setVisible( false );
        mPasteButton.setVisible( false );
        }


    FILE *f = fopen( "wordList.txt", "r" );
    
    if( f != NULL ) {
    
        int numRead = 1;
        
        char buff[100];
        
        while( numRead == 1 ) {
            numRead = fscanf( f, "%99s", buff );
            
            if( numRead == 1 ) {
                mWordList.push_back( stringDuplicate( buff ) );
                }
            }
        fclose( f );
        }
    
    if( mWordList.size() < 20 ) {
        mGenerateButton.setVisible( false );
        }

    if( userEmail != NULL ) {    
        unsigned int timeSeed = 
            (unsigned int)fmod( game_getCurrentTime(), UINT_MAX );
        unsigned int emailSeed =
            crc32( (unsigned char *)userEmail, strlen( userEmail ) );
        
        mRandSource.reseed( timeSeed + emailSeed );
        }

    char oldSet = false;
    char *oldCode = SettingsManager::getSettingContents( "twinCode", "" );
    
    if( oldCode != NULL ) {
        if( strcmp( oldCode, "" ) != 0 ) {
            mCodeField.setText( oldCode );
            oldSet = true;
            actionPerformed( &mCodeField );
            }
        delete [] oldCode;
        }

    if( !oldSet ) {
        // generate first one automatically
        actionPerformed( &mGenerateButton );
        }
    }


        
TwinPage::~TwinPage() {
    delete mPlayerCountRadioButtonSet;

    mWordList.deallocateStringElements();
    }



void TwinPage::actionPerformed( GUIComponent *inTarget ) {
    if( inTarget == &mCodeField ) {
        char *text = mCodeField.getText();
        
        char *trimText = trimWhitespace( text );

        if( strcmp( trimText, "" ) == 0 ) {
            mLoginButton.setVisible( false );
            }
        else {
            mLoginButton.setVisible( true );
            }
        delete [] text;
        delete [] trimText;
        }
    else if( inTarget == &mCancelButton ) {
        if( userTwinCode != NULL ) {
            delete [] userTwinCode;
            userTwinCode = NULL;
            }
        
        setSignal( "cancel" );
        }
    else if( inTarget == &mGenerateButton ) {
        
        char *pickedWord[3];
        
        for( int i=0; i<3; i++ ) {
            pickedWord[i] = 
                mWordList.getElementDirect( mRandSource.getRandomBoundedInt( 
                                                0, mWordList.size() - 1 ) );
            }
        char *code = autoSprintf( "%s %s %s",
                                  pickedWord[0],
                                  pickedWord[1],
                                  pickedWord[2] );
        
        mCodeField.setText( code );
        actionPerformed( &mCodeField );
        delete [] code;
        }
    else if( inTarget == &mOpenFriendCodeShareButton ) {
        char url[] = "http://phex.antinoid.com/friendscode/";
        launchURL( url );
        }
    else if( inTarget == &mCopyButton ) {
        char *text = mCodeField.getText();
        setClipboardText( text );
        delete [] text;
        }
    else if( inTarget == &mPasteButton ) {
        char *text = getClipboardText();

        mCodeField.setText( text );
        actionPerformed( &mCodeField );

        delete [] text;
        }
    else if( inTarget == &mLoginButton ) {
        if( userTwinCode != NULL ) {
            delete [] userTwinCode;
            userTwinCode = NULL;
            }
        
        char *text = mCodeField.getText();
        
        userTwinCode = trimWhitespace( text );
        delete [] text;

        SettingsManager::setSetting( "twinCode", userTwinCode );

        int pickedItem = mPlayerCountRadioButtonSet->getSelectedItem();

        if( pickedItem == 3 ) {
            userTwinCount = 0;
            }
        else {
            userTwinCount = mPlayerCountRadioButtonSet->getSelectedItem() + 2;
            }
        
        // YummyLife: If we are playing with friends mode, check if it's a shared friend token, if so start third party login flow
        if( userTwinCount == 0 && YummyLife::FriendCodeSharing::isSharedFriendToken(userTwinCode) ) {
            const char* challenge = YummyLife::FriendCodeSharing::begin(userTwinCode);
            if(challenge == nullptr) {
                // TODO: Make error messages more specific
                displayTipMessage( "Could not start SharedFriendCode login", "red", 5000 );
                return;
            }
            // Hash the challenge and send it on
            char *pureKey = getPureAccountKey();
            std::string keyHash = hmac_sha1( pureKey, challenge );
            bool res = YummyLife::FriendCodeSharing::complete(userEmail, keyHash.c_str());
            delete [] pureKey;
            if(!res) {
                displayTipMessage( "Failed to complete SharedFriendCode login", "red", 5000 );
                return;
            }
            displayTipMessage( "SharedFriendCode login successful! Starting game...", "green", 5000 );
        }

        // YummyLife: Always login to main game UNLESS Shift or Ctl is held down
        SettingsManager::setSetting( "tutorialDone", 2 );
        if(isShiftKeyDown()) {        setSignal("tutorial1"); } // Tut 1
        else if(isControlKeyDown()) { setSignal("tutorial2"); } // Tut 2
        else {                        setSignal( "done" );    } // Main Game
        }
    }



void TwinPage::makeActive( char inFresh ) {
    mYumRebirth.onMakeActive();
    }

        

void TwinPage::draw( doublePair inViewCenter, 
                     double inViewSize ) {
    doublePair pos = { 0, 278 };
    
    drawMessage( translate( "twinTip" ), pos );

    int pickedItem = mPlayerCountRadioButtonSet->getSelectedItem();

    mOpenFriendCodeShareButton.setVisible( pickedItem == 3 );

    if( pickedItem == 3 ) {
        pos.y = -280;
        drawMessage( translate( "sameFamExplain" ), pos );
        }

    }

