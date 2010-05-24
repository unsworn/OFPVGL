#ifndef _PVGLAdmin
#define _PVGLAdmin

#include "Sensor.h"
#include "ofxXMLSettings.h"
#include <stdio.h>

class PVGLAdmin
{

  public:

    PVGLAdmin();
    PVGLAdmin(float width, float height);

    void draw();
    void buttonPressed(int btn);

    float getTiltOffset();
    float getPanOffset();
    float getViewportVerticalOffset();

  private:

    static const int FONT_SIZE              = 64;
    static const int LINE_HEIGHT            = 32;

    ofTrueTypeFont font;

    float width;
    float height;

    string SETTINGS_FILE;               // hack: should be a constant
    ofxXmlSettings xml;
    float tiltOffset;
	float panOffset;
	float viewportVerticalOffset;

	bool loadSettings();                // returns false if something went wrong during load
	void saveSettings();

};

#endif
