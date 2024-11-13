#include "GeneticHistoryPage.h"

#include "fitnessScore.h"


#include "buttonStyle.h"

#include "message.h"


#include "minorGems/game/Font.h"

#include "minorGems/game/game.h"


#include "minorGems/util/stringUtils.h"
#include "minorGems/util/SettingsManager.h"
#include "minorGems/formats/encodingUtils.h"

#include "hetuwmod.h"
#include "yummyLife.h"


extern Font *mainFont;

extern char isAHAP;



GeneticHistoryPage::GeneticHistoryPage() 
        : mBackButton( mainFont, -522, 300, translate( "backButton" ) ),
          mRefreshButton( mainFont, 542, 300, translate( "refreshButton" ) ),
          mLeaderboardButton( mainFont, 322, 300, translate( "leaderboard" ) ),
          // YummyLife: Moved form main menu
          mOneTechButton( mainFont, -240, 300, translateWithDefault("yummyLifeTechButton", "TECH TREE")),
          mOholCurseButton( mainFont, -10, 300, translateWithDefault("yummyLifeOholCurseButton", "OHOL CURSE")),

          mRefreshTime( 0 ),
          mSkip( 0 ) {

    setButtonStyle( &mBackButton );
    setButtonStyle( &mRefreshButton );
    setButtonStyle( &mLeaderboardButton );
    setButtonStyle(&mOneTechButton);
    setButtonStyle(&mOholCurseButton);

    mBackButton.setVisible( true );
    mRefreshButton.setVisible( false );
    mLeaderboardButton.setVisible( false );
    mOneTechButton.setVisible(true);
    mOholCurseButton.setVisible(true);
    
    mBackButton.addActionListener( this );
    mRefreshButton.addActionListener( this );
    mLeaderboardButton.addActionListener( this );
    mOneTechButton.addActionListener( this );
    mOholCurseButton.addActionListener( this );
    
    addComponent( &mBackButton );
    addComponent( &mRefreshButton );
    addComponent(&mOneTechButton);
    addComponent(&mOholCurseButton);
    
    if( !isHardToQuitMode() ) {
        addComponent( &mLeaderboardButton );
        }
    }


GeneticHistoryPage::~GeneticHistoryPage() {
    }

        

void GeneticHistoryPage::actionPerformed( GUIComponent *inTarget ) {

    if( inTarget == &mBackButton ) {
        setSignal( "done" );
        }
    else if( inTarget == &mRefreshButton ) {
        triggerFitnessScoreDetailsUpdate();
        mRefreshButton.setVisible( false );
        mRefreshTime = game_getCurrentTime();
        mSkip = 0;
        }
    else if( inTarget == &mLeaderboardButton ) {
        char *url;
        if( isAHAP ) {
            url = SettingsManager::getStringSetting( 
                "ahapFitnessServerURL", "" );
            }
        else {
            url = SettingsManager::getStringSetting( "fitnessServerURL", "" );
            }
        
        if( strcmp( url, "" ) != 0 ) {
            
            char *fullURL = autoSprintf( "%s?action=show_leaderboard", url );
            
            launchURL( fullURL );
            delete [] fullURL;
            }
        
        delete [] url;
        }
    // YummyLife: Functionality for web buttons
    else if(inTarget == &mOneTechButton) {
        char *url;
        
        if( isAHAP ) {
            url = SettingsManager::getStringSetting( "ahapTechTreeURL", "" );
        } else {
            url = SettingsManager::getStringSetting( "techTreeURL", "" );
        }
        
        if( strcmp( url, "" ) != 0 ) {
            launchURL( url );
        }
        delete [] url;
    } else if (inTarget == &mOholCurseButton) {
        const char *url = "https://oholcurse.com/redirect/profile";

        const char *leaderboardName = getLeaderboardName();
        char *encodedLeaderboardName = hexEncode((unsigned char *)leaderboardName, strlen(leaderboardName));

        char *fullURL = autoSprintf( "%s/%s", url, encodedLeaderboardName);
                                    
        launchURL( fullURL );
        delete [] encodedLeaderboardName;
        delete [] fullURL;
    }
    }



void GeneticHistoryPage::draw( doublePair inViewCenter, 
                     double inViewSize ) {
    
    if( ! mRefreshButton.isVisible() &&
        game_getCurrentTime() - mRefreshTime > 10 ) {
        // enforce 10 seconds between refreshes
        mRefreshButton.setVisible( true );
        }

    if( isFitnessScoreReady() ){
        mLeaderboardButton.setVisible( true );
        
        doublePair pos = { 570, 0 };
        
        if( canFitnessScroll() ) {
            drawMessage( "scrollTip", pos );
            }
        }
    

    doublePair pos = { 0, 300 };
    drawFitnessScoreDetails( pos, mSkip );

    // YummyLife: Copied from ExistingAccountPage.cpp
    const char *leaderboardName = getLeaderboardName();
    if (leaderboardName != NULL && !HetuwMod::privateModeEnabled) {
        mOholCurseButton.setVisible( true );
    }
    }




void GeneticHistoryPage::makeActive( char inFresh ) {

    // YummyLife: Shouldn't be avaliable into leaderboardname is known
    mOholCurseButton.setVisible( false );

    if( ! inFresh ) {
        return;
        }

    
    if( game_getCurrentTime() - mRefreshTime > 10 ) {
        triggerFitnessScoreDetailsUpdate();
        mRefreshButton.setVisible( false );
        mLeaderboardButton.setVisible( false );
        mRefreshTime = game_getCurrentTime();
        mSkip = 0;
        }
    }


void GeneticHistoryPage::specialKeyDown( int inKeyCode ) {
    if( ! isFitnessScoreReady() ) {
        return;
        }
    
    if( canFitnessScroll() ) {
        
        if( inKeyCode == MG_KEY_UP ) {
            mSkip --;
            if( mSkip < 0 ) {
                mSkip = 0;
                }
            }
        else if( inKeyCode == MG_KEY_DOWN ) {
            mSkip ++;
            
            int max = getMaxFitnessListSkip();
            
            if( mSkip > max ) {
                mSkip = max;
                }
            }
        }
    
    };
