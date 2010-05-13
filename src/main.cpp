#include "ofMain.h"
#include "ofAppGlutWindow.h"
#include "PVGL.h"
//========================================================================
int main( ){

    ofAppGlutWindow window;
	ofSetupOpenGL(&window, 1024,600, OF_WINDOW);			// <-------- setup the GL context

    ofSetLogLevel(OF_LOG_VERBOSE);

	ofRunApp( new PVGL() );
}
