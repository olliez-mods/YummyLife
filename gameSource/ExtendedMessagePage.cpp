#include "ExtendedMessagePage.h"

#include "buttonStyle.h"
#include "message.h"

#include "minorGems/game/Font.h"
#include "minorGems/game/game.h"

#include "minorGems/util/stringUtils.h"


extern Font *mainFont;


extern char *userEmail;
extern char *accountKey;





ExtendedMessagePage::ExtendedMessagePage()
        : mOKButton( mainFont, 0, -128, 
                     translate( "okay" ) ),
          mMenuButton( mainFont, 0, -210,
                       "MAIN MENU" ),
          mMessageKey( "" ),
          mSubMessage( NULL ) {

    addComponent( &mOKButton );
    setButtonStyle( &mOKButton );
    mOKButton.addActionListener( this );

    addComponent( &mMenuButton );
    setButtonStyle( &mMenuButton );
    mMenuButton.addActionListener( this );
    mMenuButton.setVisible( false );
    }


ExtendedMessagePage::~ExtendedMessagePage() {
    if( mSubMessage != NULL ) {
        delete [] mSubMessage;
        }
    }




void ExtendedMessagePage::setMessageKey( const char *inMessageKey ) {
    mMessageKey = inMessageKey;
    }

void ExtendedMessagePage::setSubMessage( const char *inMessage ) {
    if( mSubMessage != NULL ) {
        delete [] mSubMessage;
        }
    mSubMessage = stringDuplicate( inMessage );
    }

void ExtendedMessagePage::showMenuButton( char inShow ) {
    mMenuButton.setVisible( inShow );
    }



        
void ExtendedMessagePage::actionPerformed( GUIComponent *inTarget ) {
    if( inTarget == &mOKButton ) {
        setSignal( "done" );
        }
    else if( inTarget == &mMenuButton ) {
        setSignal( "menu" );
        }
    }



void ExtendedMessagePage::draw( doublePair inViewCenter, 
                                  double inViewSize ) {
    
    doublePair pos = { 0, 200 };
    
    drawMessage( mMessageKey, pos );
    
    if( mSubMessage != NULL ) {
        pos.y = 50;
        drawMessage( mSubMessage, pos );
        }
    
    }

