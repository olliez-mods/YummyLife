#include "GamePage.h"

#include "TextField.h"
#include "TextButton.h"
#include "KeyEquivalentTextButton.h"

#include "yumRebirthComponent.h"

#include "minorGems/ui/event/ActionListener.h"


class ExistingAccountPage : public GamePage, public ActionListener {
        
    public:
        
        ExistingAccountPage();
        
        virtual ~ExistingAccountPage();
        
        void clearFields();


        // defaults to true
        void showReviewButton( char inShow );
        
        // defaults to false
        void showDisableCustomServerButton( char inShow );
        

        
        virtual void actionPerformed( GUIComponent *inTarget );

        
        virtual void makeActive( char inFresh );
        virtual void makeNotActive();

        virtual void step();
        

        // for TAB and ENTER (switch fields and start login)
        virtual void keyDown( unsigned char inASCII );
        
        // for arrow keys (switch fields)
        virtual void specialKeyDown( int inKeyCode );
        
        virtual void draw( doublePair inViewCenter, 
                           double inViewSize );


    protected:
        
        TextField mEmailField;
        TextField mKeyField;

        TextField *mFields[2];

        TextButton mAtSignButton;

        KeyEquivalentTextButton mPasteButton;
        KeyEquivalentTextButton mPasteEmailButton;

        TextButton mDisableCustomServerButton;
        
        TextButton mLoginButton;
        TextButton mFriendsButton;
        TextButton mGenesButton;
        TextButton mFamilyTreesButton;
        TextButton mClearAccountButton;
        TextButton mChangeAccountButton;
        TextButton mCancelButton;

        TextButton mSettingsButton;
        TextButton mReviewButton;
        
        TextButton mRetryButton;
        TextButton mRedetectButton;

        TextButton mViewAccountButton;
        
        TextButton mTutorialButton;
        TextButton mTutOneButton;
        TextButton mTutTwoButton;

        TextButton mServicesButton;
        
        TextButton mAHAPSettingsButton;

        TextButton mNextImageButton;
        TextButton mPrevImageButton;

        TextButton mUpdateYummyLifeButton;

        // Account management buttons
        TextButton mAcntPrevButton;
        TextButton mAcntNextButton;
        TextButton mAcntSelectButton;
        TextButton mAcntEditButton;
        TextButton mAcntDeleteButton;
        TextButton mAcntDeleteConfirmButton;
        TextButton mAcntNewButton;
        TextButton mAcntCloseButton;

        TextButton mAcntConfirmButton;
        TextButton mAcntCancelButton;
        TextField mAcntEmailField;
        TextField mAcntKeyField;
        TextField mAcntNotesField;
        KeyEquivalentTextButton mAcntPasteButton;

        yumRebirthComponent mYumRebirth;
        

        double mPageActiveStartTime;
        int mFramesCounted;
        char mFPSMeasureDone;

        char showChangeAccountWindow = false;
        char showEditAccountWindow = false;
        int editAccountIndex; // -1 to add new account

        int selectedAccountIndex = -1;
        int hoveredAccountIndex = -1;

        char inSaveAccountProcess = false; // Are we currently trying to save an account?
        double accountSaveStartTime = 0.0; // For request timeout

        char shouldAcntPasteButtonBeVisable;
        char confirmDeleteAccount = false;

        char mHideAccount;

        void hideLeftScreenItems(bool hide);
        void setChangeAccountWindow(bool open, int hoveredAccountIndexOverride = -1);
        void setEditAccountWindow(bool open, int accountIndex = -1); // -1 to add new account

        void onAccountDeleteConfirmed();
        void onAccountDeleteCancelled();
        void onAccountDeleteStarted();

        void selectAccountAtIndex(int index);
        void updateLoginInfo(const char* inEmail, const char* inKey, bool saveToSettings = false);

        void switchFields();
        
        void processLogin( char inStore, const char *inSignal );

    };

