#ifndef _TEST_APP
#define _TEST_APP


#include "ofMain.h"
#include "Sensor.h"

class testApp : public ofBaseApp{

	public:

		void setup();
		void update();
		void draw();

		//ofTrueTypeFont franklin;
		ofImage img;

		Sensor* sensor;

		int rotation;

		void keyReleased(int key);

		/*
		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);

		float 	counter;
		float	spin;
		float	spinPct;
		int		prevMX;
		int		prevMY;
		bool 	bFirstMouseMove;

        */
};

#endif
