#ifndef YUM_SETTINGS_PAGE_INCLUDED
#define YUM_SETTINGS_PAGE_INCLUDED

#include <string>
#include <vector>
#include <map>

#include "GamePage.h"
#include "TextButton.h"
#include "TextField.h"

#include "yumConfig.h"

#include "minorGems/ui/event/ActionListener.h"


// YummyLife: every row on this page is generated from the yumConfig registry,
// so adding a registerSetting call is the only work a new setting needs.
//
// Rows are drawn as plain text and a single TextField is moved onto whichever
// row is being edited - with ninety-odd settings, one field that travels is a
// great deal less machinery than ninety fields that mostly sit off-screen.
class YumSettingsPage : public GamePage, public ActionListener {

    public:

        YumSettingsPage();
        ~YumSettingsPage();

        virtual void draw( doublePair inViewCenter, double inViewSize );
        virtual void step();

        virtual void actionPerformed( GUIComponent *inTarget );

        virtual void makeActive( char inFresh );
        virtual void makeNotActive();

        virtual void pointerDown( float inX, float inY );
        virtual void pointerMove( float inX, float inY );
        virtual void keyDown( unsigned char inASCII );
        virtual void specialKeyDown( int inKeyCode );

        // the mouse wheel never reaches a GamePage, so HetuwMod hands it over
        // while this page is the one on screen
        static bool handleScroll( int inDir );

    protected:

        // one line of the list: either a section banner or an actual setting
        struct Row {
            bool isHeader;
            std::string header;
            yumConfig::SettingInfo info;
        };

        std::vector<Row> mRows;

        // name -> edited value, held here until Save so Exit can walk away
        std::map<std::string, std::string> mPending;

        std::string mError;       // complaint about the row being edited
        int mEditingRow;          // -1 when nothing is being edited
        int mScroll;              // index of the top visible row
        int mHoverRow;

        TextField mValueField;
        TextField mFilterField;

        TextButton mExitButton;
        TextButton mSaveButton;
        TextButton mResetButton;

        void rebuildRows();
        void scrollBy( int inLines );
        void beginEdit( int inRow );
        bool commitEdit();          // false when the value doesn't validate
        void cancelEdit();

        int visibleRowCount();
        double rowY( int inIndex );
        int rowAtY( float inY );

        // what a row should show: the pending edit if there is one
        std::string displayValue( const Row &inRow );
        bool isChanged( const Row &inRow );

        int changedCount();

        void applyAll();            // pending -> live variables -> config file

        static YumSettingsPage *sActivePage;
    };

#endif
