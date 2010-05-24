#ifndef _PVGL
#define _PVGL

#include "ofMain.h"
#include "Sensor.h"
#include "ofxDirList.h"
#include "PVGLAdmin.h"
#include "PVGLImageLoader.h"

class PVGL : public ofBaseApp
{

public:

    static const float CYLINDER_RADIUS      = 1200.0;
    static const int VIEWPORT_WIDTH         = 1024;
    static const int VIEWPORT_HEIGHT        = 448;      //448;
    static const float FOVY                 = 36.0;     // vertical field of view

    ///////////////////////////////////

	void setup();
	void update();
	void draw();

	//ofTrueTypeFont franklin;
	ofxDirList dirList;

	ofTexture textures[4];
	ofImage* currPanoImg;
	ofImage* panoSections;      // pointer to array

	ofImage images[4];          // panoSections

	ofImage infoImg;
	ofTexture infoTexture;

	float ang;

	float centerX;
	float centerY;

    float getInterfaceH();
    float getInterfaceW();
    float getInterfaceZ();
	void keyReleased(int key);
	void keyPressed(int key);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void windowResized(int w, int h);

	Sensor* sensor;

    //////////////////////////////////////////////////////////

private:

	PVGLAdmin* admin;

	float pan;
	float tilt;
	float zoom;
	float eyeX, eyeY, eyeZ;				// camera position
	float cylinderHeight;
	float zoomOffset;
	int view;

	bool bUseArbTex;
	bool bShowInfo;
    bool bUseSensor;
    bool bAdminMode;
    bool bLoading;
    bool bTexturesReady;

	vector<string> panoUrls;
	int currPanoIdx;
	int currPanoW, currPanoH;

	bool btnPrevDown, btnNextDown, btnZoomDown, btnInfoDown;

	int sectionW;

	float defaultInterfaceZ;            // start value
	float currInterfaceZ;               // might be zoomed
	float interfaceW, interfaceH;		// size of the interface quad

	PVGLImageLoader imgLoader;

    // private methods

	void loadSettings();
	void refreshPanoramaList();
	void gotoPanorama(int newPanoIdx);
	void loadPanorama(int newPanoId);
	void loadInfoImage();
	void setupScreen();
	void setupTextures();

	void drawReferenceLines(float len);
	void drawCylinder(double radius, double height, int slices);
	void circleTable(double **sint, double **cost, const int n);

    void setAdminMode(bool flag);
	void checkButtons();

	void drawInfoScreen();
	void showInfo();
	void hideInfo();
	void zoomIn();
	void zoomOut();

	void glError();
};

#endif
