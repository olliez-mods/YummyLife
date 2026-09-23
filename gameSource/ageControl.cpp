#include "ageControl.h"
#include "settingsToggle.h"

#include <math.h>
#include <stdio.h>


static double babyHeadDownFactor = 0.6;

static double babyBodyDownFactor = 0.75;

static double oldHeadDownFactor = 0.35;

static double oldHeadForwardFactor = 2;


#include "minorGems/util/SettingsManager.h"
#include "minorGems/util/SimpleVector.h"
#include "minorGems/util/stringUtils.h"


// YummyLife:  one (real age, drawn age) pair from the ageScaling setting.
typedef struct AgeScalePoint {
        double realAge;
        double displayAge;
    } AgeScalePoint;


// YummyLife:  2HOL runs a 120-year life, but draws its people with art laid
// out on the same 0-60 scale as OneLife, so it maps one age onto the other
// before drawing.

// That map is the ageScaling setting, one
// "<realAge> <displayAge> <label>" line per milestone.  There is no such
// setting on OneLife or AHAP, where the two scales already agree, and
// getDisplayAge is then the identity.
static SimpleVector<AgeScalePoint> ageScalePoints;

// YummyLife:  these numbers live in contentSettings on OneLife, but 2HOL
// ships no such folder and keeps them in settings instead.
static double getContentOrMainSetting( const char *inSettingName,
                                       double inDefaultValue ) {
    char found = false;

    useContentSettings();
    double value = SettingsManager::getDoubleSetting( inSettingName, &found );

    if(!found) {
        useMainSettings();
        value = SettingsManager::getDoubleSetting( inSettingName, &found );
    }

    useMainSettings();

    if(!found) return inDefaultValue;
    return value;
}

static char *getContentOrMainContents( const char *inSettingName ) {
    useContentSettings();
    char *contents = SettingsManager::getSettingContents( inSettingName );

    if(contents == NULL) {
        useMainSettings();
        contents = SettingsManager::getSettingContents( inSettingName );
    }

    useMainSettings();
    return contents;
}

static void initAgeScaling() {
    ageScalePoints.deleteAll();

    char *contents = getContentOrMainContents( "ageScaling" );

    if(contents == NULL) return;

    int numLines;
    char **lines = split( contents, "\n", &numLines );

    for( int i=0; i<numLines; i++ ) {
        AgeScalePoint p;
        if( sscanf( lines[i], "%lf %lf", &( p.realAge ), &( p.displayAge ) ) == 2 && p.realAge > 0 ) {
            ageScalePoints.push_back( p );
        }
        delete [] lines[i];
    }
    delete [] lines;
    delete [] contents;


    // getDisplayAge walks these in order, so sort by real age rather than
    // trusting the file to be written that way.  Only a handful of points.
    for( int i=1; i<ageScalePoints.size(); i++ ) {
        AgeScalePoint p = ageScalePoints.getElementDirect( i );

        int j = i - 1;

        while( j >= 0 && ageScalePoints.getElementDirect( j ).realAge > p.realAge ) {
            *( ageScalePoints.getElement( j + 1 ) ) = ageScalePoints.getElementDirect( j );
            j --;
        }
        *( ageScalePoints.getElement( j + 1 ) ) = p;
    }

    if( ageScalePoints.size() > 0 ) {
        AgeScalePoint last = ageScalePoints.getElementDirect( ageScalePoints.size() - 1 );
        printf( "Loaded %d age scaling points, "
                "drawing a real age of %.0f as %.0f\n",
                ageScalePoints.size(), last.realAge, last.displayAge );
    }
}


void initAgeControl() {
    babyHeadDownFactor = getContentOrMainSetting( "babyHeadDownFactor", 0.6 );
    babyBodyDownFactor = getContentOrMainSetting( "babyBodyDownFactor", 0.75 );
    oldHeadDownFactor = getContentOrMainSetting( "oldHeadDownFactor", 0.35 );
    oldHeadForwardFactor = getContentOrMainSetting( "oldHeadForwardFactor", 2 );
    initAgeScaling();
}

double getDisplayAge( double inRealAge ) {
    // -1 means "not a person" throughout the drawing code, so it, and
    // anything else before birth, passes straight through
    if( inRealAge < 0 ) return inRealAge;

    int numPoints = ageScalePoints.size();

    // nothing configured, so the two scales are the same
    if( numPoints == 0 ) return inRealAge;

    // birth is the same moment on both scales, and anchors the first segment
    double prevReal = 0;
    double prevDisplay = 0;

    for( int i=0; i<numPoints; i++ ) {
        AgeScalePoint p = ageScalePoints.getElementDirect( i );

        if( inRealAge <= p.realAge ) {
            double span = p.realAge - prevReal;

            if( span <= 0 ) return p.displayAge;
            return prevDisplay +
                ( inRealAge - prevReal ) *
                ( p.displayAge - prevDisplay ) / span;
        }

        prevReal = p.realAge;
        prevDisplay = p.displayAge;
    }

    // past the last point, which is death - hold there
    return prevDisplay;
}
// YummyLife: changes end here




doublePair getAgeHeadOffset( double inAge, doublePair inHeadSpritePos,
                             doublePair inBodySpritePos,
                             doublePair inFrontFootSpritePos ) {
    if( inAge == -1 ) {
        return (doublePair){ 0, 0 };
        }
    
    if( inAge < 20 ) {
        
        double maxHead = inHeadSpritePos.y - inBodySpritePos.y;
        
        double yOffset = ( ( 20 - inAge ) / 20 ) * babyHeadDownFactor * maxHead;
        
        
        return (doublePair){ 0, round( -yOffset ) };
        }
    

    if( inAge >= 40 ) {
        
        if( inAge > 60 ) {
            // no worse after 60
            inAge = 60;
            }

        double maxHead = inHeadSpritePos.y - inBodySpritePos.y;
        
        double vertOffset = 
            ( ( inAge - 40 ) / 20 ) * oldHeadDownFactor * maxHead;
        
        double footOffset = inFrontFootSpritePos.x - inHeadSpritePos.x;
        
        double forwardOffset = 
            ( ( inAge - 40 ) / 20 ) * oldHeadForwardFactor * footOffset;

        return (doublePair){ round( forwardOffset ), round( -vertOffset ) };
        }

    return (doublePair){ 0, 0 };
    }



doublePair getAgeBodyOffset( double inAge, doublePair inBodySpritePos ) {
    if( inAge == -1 ) {
        return (doublePair){ 0, 0 };
        }
    
    if( inAge < 20 ) {
        
        double maxBody = inBodySpritePos.y;
        
        double yOffset = ( ( 20 - inAge ) / 20 ) * babyBodyDownFactor * maxBody;
        
        
        return (doublePair){ 0, round( -yOffset ) };
        }

    return (doublePair){ 0, 0 };
    }

