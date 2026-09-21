#include "YumSettingsPage.h"

#include <algorithm>

#include "buttonStyle.h"
#include "hetuwmod.h"

#include "minorGems/game/Font.h"
#include "minorGems/game/game.h"
#include "minorGems/game/drawUtils.h"


extern Font *mainFont;
extern Font *smallFont;

// relaunchGame lives in game.cpp alongside the other pages that offer a restart
extern char relaunchGame();

// who is on screen right now, owned by game.cpp
extern GamePage *currentGamePage;


YumSettingsPage *YumSettingsPage::sActivePage = NULL;


// list geometry, in page units
static const double sListTop = 258;
static const double sRowHeight = 27;
static const double sNameX = -604;
static const double sValueX = 210;
static const double sMarkerX = 596;
static const double sListBottom = -222;


YumSettingsPage::YumSettingsPage()
        : mEditingRow( -1 ), mScroll( 0 ), mHoverRow( -1 ),
          mValueField( mainFont, sValueX + 150, 0, 18, false, NULL, NULL, NULL ),
          mFilterField( mainFont, 330, 312, 16, false, "FILTER", NULL, NULL ),
          mExitButton( mainFont, -500, -300, "EXIT" ),
          mSaveButton( mainFont, 400, -300, "SAVE + RESTART" ),
          mResetButton( mainFont, -180, -300, "REVERT" ) {

    setButtonStyle( &mExitButton );
    setButtonStyle( &mSaveButton );
    setButtonStyle( &mResetButton );

    addComponent( &mExitButton );
    addComponent( &mSaveButton );
    addComponent( &mResetButton );
    addComponent( &mValueField );
    addComponent( &mFilterField );

    mExitButton.addActionListener( this );
    mSaveButton.addActionListener( this );
    mResetButton.addActionListener( this );
    mValueField.addActionListener( this );
    mFilterField.addActionListener( this );

    // retype the filter and the list narrows as you go
    mFilterField.setFireOnAnyTextChange( true );

    mValueField.setVisible( false );
    }


YumSettingsPage::~YumSettingsPage() {
    if( sActivePage == this ) sActivePage = NULL;
    }



void YumSettingsPage::rebuildRows() {
    mRows.clear();

    char *filterRaw = mFilterField.getText();
    std::string needle( filterRaw != NULL ? filterRaw : "" );
    delete [] filterRaw;
    for( size_t i = 0; i < needle.length(); i++ ) {
        needle[i] = tolower( needle[i] );
        }

    std::vector<yumConfig::SettingInfo> all = yumConfig::listSettings();

    std::string lastSection;

    for( size_t i = 0; i < all.size(); i++ ) {
        if( !needle.empty() ) {
            std::string hay = all[i].name + " " + all[i].hint;
            for( size_t c = 0; c < hay.length(); c++ ) {
                hay[c] = tolower( hay[c] );
                }
            if( hay.find( needle ) == std::string::npos ) continue;
            }

        if( all[i].section != lastSection ) {
            Row header;
            header.isHeader = true;
            header.header = all[i].section;
            mRows.push_back( header );
            lastSection = all[i].section;
            }

        Row row;
        row.isHeader = false;
        row.info = all[i];
        mRows.push_back( row );
        }

    if( mScroll > (int)mRows.size() - 1 ) mScroll = 0;
    if( mScroll < 0 ) mScroll = 0;
    }



int YumSettingsPage::visibleRowCount() {
    return (int)( ( sListTop - sListBottom ) / sRowHeight ) + 1;
    }


double YumSettingsPage::rowY( int inIndex ) {
    return sListTop - ( inIndex - mScroll ) * sRowHeight;
    }


int YumSettingsPage::rowAtY( float inY ) {
    if( inY > sListTop + sRowHeight / 2 ) return -1;
    if( inY < sListBottom - sRowHeight / 2 ) return -1;

    int offset = (int)( ( sListTop + sRowHeight / 2 - inY ) / sRowHeight );
    int index = mScroll + offset;

    if( index < 0 || index >= (int)mRows.size() ) return -1;
    return index;
    }



std::string YumSettingsPage::displayValue( const Row &inRow ) {
    auto it = mPending.find( inRow.info.name );
    if( it != mPending.end() ) return it->second;
    return inRow.info.value;
    }


bool YumSettingsPage::isChanged( const Row &inRow ) {
    auto it = mPending.find( inRow.info.name );
    if( it == mPending.end() ) return false;
    return it->second != inRow.info.value;
    }


int YumSettingsPage::changedCount() {
    int count = 0;
    for( auto &pair : mPending ) {
        if( pair.second != yumConfig::getSettingValue( pair.first ) ) count++;
        }
    return count;
    }


void YumSettingsPage::scrollBy( int inLines ) {
    int maxScroll = (int)mRows.size() - visibleRowCount();
    if( maxScroll < 0 ) maxScroll = 0;

    mScroll += inLines;
    if( mScroll < 0 ) mScroll = 0;
    if( mScroll > maxScroll ) mScroll = maxScroll;
    }


bool YumSettingsPage::handleScroll( int inDir ) {
    if( sActivePage == NULL ) return false;

    // nothing in game.cpp calls base_makeNotActive, so leaving this page never
    // tells us we left.  Ask who is actually on screen instead of trusting a
    // flag that only ever gets set.
    if( currentGamePage != (GamePage *)sActivePage ) return false;

    sActivePage->scrollBy( -inDir * 3 );
    return true;
    }



void YumSettingsPage::beginEdit( int inRow ) {
    if( inRow < 0 || inRow >= (int)mRows.size() ) return;
    if( mRows[inRow].isHeader ) return;

    mEditingRow = inRow;
    mError = "";

    mValueField.setText( displayValue( mRows[inRow] ).c_str() );
    mValueField.setPosition( sValueX + 150, rowY( inRow ) );
    mValueField.setVisible( true );
    mValueField.focus();
    }


bool YumSettingsPage::commitEdit() {
    if( mEditingRow < 0 ) return true;

    Row &row = mRows[mEditingRow];

    char *raw = mValueField.getText();
    std::string value( raw != NULL ? raw : "" );
    delete [] raw;

    // trim, so a stray space doesn't fail an otherwise fine value
    size_t start = value.find_first_not_of( " \t" );
    if( start == std::string::npos ) value = "";
    else {
        size_t end = value.find_last_not_of( " \t" );
        value = value.substr( start, end - start + 1 );
        }

    std::string error;
    if( !yumConfig::validateSetting( row.info.name, value, error ) ) {
        mError = row.info.name + ": " + error;
        return false;
        }

    mPending[ row.info.name ] = value;
    mError = "";

    mEditingRow = -1;
    mValueField.setVisible( false );
    mValueField.unfocus();
    return true;
    }


void YumSettingsPage::cancelEdit() {
    mEditingRow = -1;
    mError = "";
    mValueField.setVisible( false );
    mValueField.unfocus();
    }



void YumSettingsPage::applyAll() {
    for( auto &pair : mPending ) {
        std::string error;
        // already validated when it was typed, but the registry is the authority
        yumConfig::applySetting( pair.first, pair.second, error );
        }

    // a few settings are copied into working variables at startup; hetuw
    // knows which, so it redoes that pass rather than this page guessing.
    // The restart makes this moot, except when the relaunch fails and the
    // player carries on in the process that is already running.
    HetuwMod::onSettingsChanged();

    yumConfig::saveSettings( yummylifeSettingsFileName );

    mPending.clear();
    rebuildRows();
    }



void YumSettingsPage::makeActive( char inFresh ) {
    sActivePage = this;

    if( !inFresh ) return;

    mPending.clear();
    mError = "";
    mEditingRow = -1;
    mScroll = 0;
    mHoverRow = -1;
    mValueField.setVisible( false );
    mFilterField.setText( "" );

    rebuildRows();
    }


void YumSettingsPage::makeNotActive() {
    if( sActivePage == this ) sActivePage = NULL;
    cancelEdit();
    }



void YumSettingsPage::step() {
    }



void YumSettingsPage::pointerDown( float inX, float inY ) {
    int row = rowAtY( inY );

    // a click inside the open editor belongs to the text field
    if( mEditingRow >= 0 && row == mEditingRow && inX > sValueX ) return;

    if( mEditingRow >= 0 ) {
        // moving away commits what was typed, unless it doesn't validate -
        // then we keep the user where the problem is
        if( !commitEdit() ) return;
        }

    if( row >= 0 && !mRows[row].isHeader ) {
        beginEdit( row );
        }
    }


void YumSettingsPage::keyDown( unsigned char inASCII ) {
    if( inASCII == 27 ) { // escape
        if( mEditingRow >= 0 ) cancelEdit();
        else setSignal( "back" );
        }
    }


void YumSettingsPage::specialKeyDown( int inKeyCode ) {
    if( mValueField.isFocused() ) return;

    switch( inKeyCode ) {
        case MG_KEY_UP:        scrollBy( -1 ); break;
        case MG_KEY_DOWN:      scrollBy( 1 ); break;
        case MG_KEY_PAGE_UP:   scrollBy( -visibleRowCount() + 1 ); break;
        case MG_KEY_PAGE_DOWN: scrollBy( visibleRowCount() - 1 ); break;
        }
    }



void YumSettingsPage::actionPerformed( GUIComponent *inTarget ) {

    if( inTarget == &mFilterField ) {
        cancelEdit();
        mScroll = 0;
        rebuildRows();
        }
    else if( inTarget == &mValueField ) {
        commitEdit();
        }
    else if( inTarget == &mExitButton ) {
        setSignal( "back" );
        }
    else if( inTarget == &mResetButton ) {
        cancelEdit();
        mPending.clear();
        rebuildRows();
        }
    else if( inTarget == &mSaveButton ) {
        // saving always restarts, so the game comes back up having read every
        // setting and there is never a half-applied state to explain
        if( mEditingRow >= 0 && !commitEdit() ) return;
        applyAll();

        // this is meant to replace the process; returning at all means it
        // didn't, and the player has to restart by hand
        relaunchGame();
        setSignal( "relaunchFailed" );
        }
    }


void YumSettingsPage::pointerMove( float inX, float inY ) {
    mHoverRow = rowAtY( inY );
    if( mHoverRow >= 0 && mRows[mHoverRow].isHeader ) mHoverRow = -1;
    }



void YumSettingsPage::draw( doublePair inViewCenter, double inViewSize ) {

    doublePair pos;

    // ---- heading ----
    setDrawColor( 1, 1, 1, 1 );
    pos.x = sNameX;
    pos.y = 312;
    mainFont->drawString( "YUMMYLIFE SETTINGS", pos, alignLeft );

    // ---- the list well ----
    doublePair wellCenter = { 0, ( sListTop + sListBottom ) / 2 };
    setDrawColor( 0, 0, 0, 0.30 );
    drawRect( wellCenter, 630, ( sListTop - sListBottom ) / 2 + 18 );

    int visible = visibleRowCount();
    int last = mScroll + visible;
    if( last > (int)mRows.size() ) last = (int)mRows.size();

    for( int i = mScroll; i < last; i++ ) {
        Row &row = mRows[i];
        double y = rowY( i );

        if( row.isHeader ) {
            // a section banner, lifted out of the ==== rules in the cfg file
            setDrawColor( 0.98, 0.80, 0.32, 1 );
            pos.x = sNameX;
            pos.y = y;
            smallFont->drawString( row.header.c_str(), pos, alignLeft );

            double titleWidth = smallFont->measureString( row.header.c_str() );
            setDrawColor( 0.98, 0.80, 0.32, 0.35 );
            doublePair ruleCenter = { sNameX + titleWidth + 14 + 290, y + 4 };
            drawRect( ruleCenter, 290, 0.5 );
            continue;
            }

        if( i == mHoverRow && i != mEditingRow ) {
            setDrawColor( 1, 1, 1, 0.07 );
            doublePair rowCenter = { 0, y + 4 };
            drawRect( rowCenter, 626, sRowHeight / 2 - 1 );
            }

        bool changed = isChanged( row );

        // an edited row is called out on the left, where the eye starts
        if( changed ) {
            setDrawColor( 0.55, 0.95, 0.55, 1 );
            pos.x = sNameX - 14;
            pos.y = y;
            smallFont->drawString( "*", pos, alignLeft );
            }

        if( changed ) setDrawColor( 0.62, 0.98, 0.62, 1 );
        else setDrawColor( 0.85, 0.85, 0.85, 1 );

        pos.x = sNameX;
        pos.y = y;
        smallFont->drawString( row.info.name.c_str(), pos, alignLeft );

        // while a row is being edited the text field sits on top of its value
        if( i != mEditingRow ) {
            std::string value = displayValue( row );
            if( value.empty() ) {
                setDrawColor( 0.5, 0.5, 0.5, 1 );
                value = "-";
                }
            pos.x = sValueX;
            pos.y = y;
            smallFont->drawString( value.c_str(), pos, alignLeft );
            }
        }

    // ---- scrollbar ----
    if( (int)mRows.size() > visible ) {
        double trackTop = sListTop + 12;
        double trackBottom = sListBottom - 12;
        double trackHeight = trackTop - trackBottom;

        setDrawColor( 1, 1, 1, 0.10 );
        doublePair trackCenter = { 624, ( trackTop + trackBottom ) / 2 };
        drawRect( trackCenter, 3, trackHeight / 2 );

        double fraction = (double)visible / (double)mRows.size();
        double thumbHeight = trackHeight * fraction;
        if( thumbHeight < 20 ) thumbHeight = 20;

        int maxScroll = (int)mRows.size() - visible;
        double progress = maxScroll > 0 ? (double)mScroll / maxScroll : 0;

        double thumbTop = trackTop - progress * ( trackHeight - thumbHeight );
        doublePair thumbCenter = { 624, thumbTop - thumbHeight / 2 };
        setDrawColor( 1, 1, 1, 0.45 );
        drawRect( thumbCenter, 3, thumbHeight / 2 );
        }

    // ---- the line under the list: an error, or help for the row in play ----
    pos.x = sNameX;
    pos.y = -250;

    if( !mError.empty() ) {
        setDrawColor( 1, 0.45, 0.4, 1 );
        smallFont->drawString( mError.c_str(), pos, alignLeft );
        }
    else {
        int helpRow = mEditingRow >= 0 ? mEditingRow : mHoverRow;

        if( helpRow >= 0 && helpRow < (int)mRows.size() &&
            !mRows[helpRow].isHeader ) {

            yumConfig::SettingInfo &info = mRows[helpRow].info;
            std::string help = info.hint;

            // for the constrained types, what is allowed beats any prose
            if( info.kind == yumConfig::SETTING_CHOICE ) {
                std::string choices;
                for( size_t c = 0; c < info.choices.size(); c++ ) {
                    if( c > 0 ) choices += " / ";
                    choices += info.choices[c];
                    }
                help = choices + ( help.empty() ? "" : "   -   " + help );
                }
            else if( info.kind == yumConfig::SETTING_SCALED ) {
                char range[64];
                snprintf( range, sizeof(range), "%d to %d",
                          info.scaledMin, info.scaledMax );
                help = std::string( range ) + ( help.empty() ? "" : "   -   " + help );
                }
            else if( info.kind == yumConfig::SETTING_BOOL && help.empty() ) {
                help = "yes / no";
                }
            else if( info.kind == yumConfig::SETTING_KEY && help.empty() ) {
                help = "one key, or empty to unbind";
                }

            if( help.length() > 118 ) help = help.substr( 0, 115 ) + "...";

            setDrawColor( 0.72, 0.72, 0.72, 1 );
            smallFont->drawString( help.c_str(), pos, alignLeft );
            }
        }

    // ---- what saving is going to do ----
    int changed = changedCount();

    pos.x = sMarkerX;
    pos.y = -250;

    if( changed > 0 ) {
        char summary[128];
        snprintf( summary, sizeof(summary), "%d changed", changed );
        setDrawColor( 0.62, 0.98, 0.62, 1 );
        smallFont->drawString( summary, pos, alignRight );
        }
    else {
        setDrawColor( 0.55, 0.55, 0.55, 1 );
        smallFont->drawString( "no changes", pos, alignRight );
        }

    // ---- saving always restarts, so say so ----
    pos.x = 400;
    pos.y = -333;
    setDrawColor( 0.72, 0.72, 0.72, 0.95 );
    smallFont->drawString( "the game restarts to pick the settings up",
                           pos, alignCenter );
    }
