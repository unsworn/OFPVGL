#ifndef _TEST_APP
#define _TEST_APP

#include "ofMain.h"
//#include "Sensor.h"
#include "ofxXMLSettings.h"

class testApp : public ofBaseApp{

public:

	void setup();
	void update();
	void draw();

	//ofTrueTypeFont franklin;
	ofTexture textures[4];
	ofImage panorama;
	ofImage images[4];          // panoSections

	ofImage infoImg;
	ofTexture infoTexture;

	float ang;

	//Sensor* sensor;

	float tiltOffset;
	float panOffset;
	float viewportVerticalOffset;

	float centerX;
	float centerY;

	void keyReleased(int key);
	void keyPressed(int key);

	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);

	//ButtonState button;



    //////////////////////////////////////////////////////////

private:

	float pan;
	float tilt;
	float zoom;
	float eyeX, eyeY, eyeZ;				// camera position
	float cylinderHeight;

	float zoomOffset;

	int view;

	float interfaceZ;
	float interfaceW, interfaceH;		// size of the interface quad

	bool bUseArbTex;
	bool bShowInfo;

	ofxXmlSettings xml;
	string panoUrls[99];        // hack!

	int currPanoId;
	int currPanoW;
	int currPanoH;

	bool btnPrevDown, btnNextDown, btnZoomDown, btnInfoDown;

	int sectionW;

	void loadSettings();
	void loadPanorama(int newPanoId);
	void setupScreen();
	void setupTextures();
	void drawReferenceLines(float len);
	void drawCylinder(double radius, double height, int slices);
	void drawInfoScreen();
	void circleTable(double **sint, double **cost, const int n);
	void checkButtons();
	void showInfo();
	void hideInfo();
	void glError();
};

#endif
