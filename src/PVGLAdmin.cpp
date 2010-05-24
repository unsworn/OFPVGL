#include "PVGLAdmin.h"

using namespace std;

//------------------------------------------------------------------------------------------------------------
// Public methods
//------------------------------------------------------------------------------------------------------------

PVGLAdmin::PVGLAdmin()
{
    /*
    ofLog(OF_LOG_VERBOSE, "PVGLAdmin created");
    SETTINGS_FILE = "settings.xml";

    tiltOffset = 0.0;
	panOffset = 0.0;
    viewportVerticalOffset = 0.0;

    loadSettings();

    font.loadFont("frabk.ttf", FONT_SIZE);
    */
}

PVGLAdmin::PVGLAdmin(float w, float h)
{
    ofLog(OF_LOG_VERBOSE, "PVGLAdmin created");
    SETTINGS_FILE = "settings.xml";

    width = w;
    height = h;

    tiltOffset = 0.0;
	panOffset = 0.0;
    viewportVerticalOffset = 0.0;

    loadSettings();

    font.loadFont("frabk.ttf", FONT_SIZE);
}

//------------------------------------------------------------------------------------------------------------

float
PVGLAdmin::getPanOffset()
{
    return panOffset;
}

float
PVGLAdmin::getTiltOffset()
{
    return tiltOffset;
}

float
PVGLAdmin::getViewportVerticalOffset()
{
    return viewportVerticalOffset;
}

//------------------------------------------------------------------------------------------------------------
// Private methods
//------------------------------------------------------------------------------------------------------------

bool
PVGLAdmin::loadSettings()
{
    if(!xml.loadFile(SETTINGS_FILE)) {
        ofLog(OF_LOG_ERROR, "PVGLAdmin: Couldn't load XML settings");
        return false;
    }

    panOffset = xml.getValue("PVGL:PANOFFSET", 0);      // 0 is default value if settings file doesn't exist
    tiltOffset = xml.getValue("PVGL:TILTOFFSET", 0);
    viewportVerticalOffset = xml.getValue("PVGL:VIEWPORTVERTICALOFFSET", 0);

    /*
    xml.pushTag("PVGL");
    xml.pushTag("PANOS");
    int numPanos = xml.getNumTags("PANO");
    for (int i = 0; i < numPanos; i++ )
    {
        panoUrls[i] = xml.getValue("PANO", "*** invalid pano url ***", i);
        cout << "loadSettings: URL #" << i << ": " << panoUrls[i] << endl;
    }
    xml.popTag(); // PANOS
    xml.popTag(); // PVGL
    */

    ofLog(OF_LOG_VERBOSE, "PVGLAdmin: Settings loaded");
    ofLog(OF_LOG_VERBOSE, "PVGLAdmin: panOffset = %f", panOffset);
    ofLog(OF_LOG_VERBOSE, "PVGLAdmin: tiltOffset = %f", tiltOffset);
    ofLog(OF_LOG_VERBOSE, "PVGLAdmin: vieportVerticalOffset = %f", viewportVerticalOffset);

    return true;
}

//-------------------------------------------------------------------------------------------------------------

void
PVGLAdmin::saveSettings()
{
    xml.setValue("PVGL:PANOFFSET", panOffset);
    xml.setValue("PVGL:TILTOFFSET", tiltOffset);
    xml.setValue("PVGL:VIEPORTVERTICALOFFSET", viewportVerticalOffset);
    xml.saveFile(SETTINGS_FILE);
    ofLog(OF_LOG_VERBOSE, "PVGLAdmin: saved settings");
}

//--------------------------------------------------------------------------------------------------------------
void
PVGLAdmin::buttonPressed(int button)
{
    switch(button)
    {
        case Sensor::BUTTON_ZOOM:                       // ZOOM = ok!
            saveSettings();
            break;
        case Sensor::BUTTON_INFO:                       // INFO = menu/back
            break;
        case Sensor::BUTTON_PREV:                       // PREV = -
            panOffset--;
            ofLog(OF_LOG_VERBOSE, "PVGLAdmin: Changed panOffset to %f", panOffset);
            break;
        case Sensor::BUTTON_NEXT:                       // NEXT = +
            panOffset++;
            ofLog(OF_LOG_VERBOSE, "PVGLAdmin: Changed panOffset to %f", panOffset);
            break;
    }
}

//------------------------------------------------------------------------------------------------------------

void
PVGLAdmin::draw()
{
    glPushMatrix();

    glScalef(1.0, -1.0, 1.0);           // use OF coordinate system
    glTranslatef(-width / 2, -height / 2, -665);        // TODO: interfaceZ should be read from PVGL

    ofEnableAlphaBlending();	// turn on alpha blending
    ofSetColor(255,0,0,127);	// red, 50% transparent
    ofRect(0, 0, width, height);
    ofDisableAlphaBlending();	// turn it back off, if you don't need it

    ofSetColor(255, 255, 255);
    font.drawString("Pan offset: " + ofToString(panOffset), 0, 0);

    glPopMatrix();
}
//------------------------------------------------------------------------------------------------------------


//------------------------------------------------------------------------------------------------------------
