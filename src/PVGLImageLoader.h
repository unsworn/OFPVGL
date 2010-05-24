#ifndef _PVGL_IMAGELOADER
#define _PVGL_IMAGELOADER

#include "ofMain.h"
#include "ofxThread.h"

// threaded fucntions that share data need to use lock (mutex)
	                // and unlock in order to write to that data
	                // otherwise it's possible to get crashes.
	                //
	                // also no opengl specific stuff will work in a thread...
	                // threads can't create textures, or draw stuff on the screen
	                // since opengl is single thread safe


class PVGLImageLoader : public ofxThread
{

	public:

        bool imgLoaded;

	    ofImage* img;
	    ofImage panoSections[4];
        //vector<ofImage*> panoSections;

		PVGLImageLoader();
		~PVGLImageLoader();

		void start();
		void stop();
		void threadedFunction();
		ofImage* getPanoramaSections();

		void loadPanorama(string url);
		ofImage* getPanorama();

    private:
        void createPanoramaSections();

};

#endif
