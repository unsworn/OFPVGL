#ifndef _TEST_APP
#define _TEST_APP

#include "ofMain.h"
#include "Sensor.h"
#include "ofxXMLSettings.h"

class testApp : public ofBaseApp{

	public:

		void setup();
		void update();
		void draw();

		//ofTrueTypeFont franklin;
		ofImage img;

		Sensor* sensor;

		float tiltOffset;
		float panOffset;
		float viewportVerticalOffset;

		float centerX;
		float centerY;

		float pan;
		float tilt;
		float zoom;
		float x, y, z;        // only for testing purposes

		void keyReleased(int key);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);

		bool test;
		//ButtonState button;


    //////////////////////////////////////////////////////////

    private:

        ofxXmlSettings xml;

        void drawReferenceLines();
        void drawCylinder();
        void autoRotate();

};

#endif
