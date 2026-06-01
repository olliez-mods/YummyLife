#include "GamePage.h"



#include "minorGems/ui/event/ActionListener.h"


#include "TextButton.h"


class ExtendedMessagePage : public GamePage, public ActionListener {
        
    public:
        ExtendedMessagePage();
        ~ExtendedMessagePage();
        

        void setMessageKey( const char *inMessageKey );
        
        // destroyed by caller
        void setSubMessage( const char *inMessage );

        void showMenuButton( char inShow );

        virtual void actionPerformed( GUIComponent *inTarget );

        virtual void draw( doublePair inViewCenter, 
                           double inViewSize );
        
    protected:
        
        TextButton mOKButton;
        TextButton mMenuButton; // YummyLife

        const char *mMessageKey;
        char *mSubMessage;


    };
