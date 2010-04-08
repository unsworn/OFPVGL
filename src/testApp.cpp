#include "testApp.h"

using namespace std;

//------------------------------------------------------------------------------------------------------------

// height = ((radius*2) * pi) / 4   -- too keep 4:1 aspect ratio
#define CYLINDER_RADIUS     200
#define CYLINDER_HEIGHT     314
#define CYLINDER_SLICES     32
#define CYLINDER_STACKS     1

#define VIEWPORT_HEIGHT     450

//------------------------------------------------------------------------------------------------------------
void testApp::setup()
{
	ofBackground(20, 20, 20);

	//for smooth animation, set vertical sync if we can
	// ofSetVerticalSync(true);
	// also, frame rate:
	// ofSetFrameRate(60);

	// todo: load offset values from XML
    panOffset = 0;
    tiltOffset = 0;
    viewportVerticalOffset = 0;

    tilt = 0;
    pan = 0;

    x = 0;
    y = 0;
    z = 0;

    test = false;

    centerX = ofGetWidth() / 2;
    centerY = ofGetHeight() / 2;

    // do not use ARB textures, this way it is easier to use the GLU objects
    ofDisableArbTex();
    img.loadImage("tri_tb_2048x512.jpg");    // ska vara "tri_tb_8000x2000" men det vill sig inte
	ofEnableArbTex();

	sensor = Sensor::Create();
}

//------------------------------------------------------------------------------------------------------------

void testApp::update()
{
    // do sensor polling here
    if (sensor != NULL)
    {
        sensor->Poll();
        tilt = sensor->ReadTilt();
        pan = sensor->ReadPan();
    }
    else
        cout << "Sensor is NULL" << endl;
}

//------------------------------------------------------------------------------------------------------------

void testApp::draw()
{
    if (sensor == NULL)
        return ;

    /*
    if (!sensor->HasChanged())
        return ;
    */

ofRect(100, 100, 300, 400);

    drawCylinder();

    // draw viewport edges
    ofSetColor(0xFF0000);
    ofLine(0, (centerY - VIEWPORT_HEIGHT / 2) + viewportVerticalOffset, ofGetWidth(), (centerY - VIEWPORT_HEIGHT / 2) + viewportVerticalOffset);
    ofLine(0, (centerY + VIEWPORT_HEIGHT / 2) + viewportVerticalOffset, ofGetWidth(), (centerY + VIEWPORT_HEIGHT / 2) + viewportVerticalOffset);


}

//------------------------------------------------------------------------------------------------------------

void testApp::drawCylinder()
{
    ofPushMatrix();

        // draw the cylinder in the center of the screen
//        ofTranslate(ofGetWidth() / 2, ofGetHeight() / 2 - (CYLINDER_HEIGHT / 2), 150);

        ofSetColor(0xffffff);

        // center the cylinder and rotate it to make it stand up properly
        ofTranslate(centerX, centerY - (CYLINDER_HEIGHT / 2), CYLINDER_HEIGHT + CYLINDER_RADIUS);
        ofRotateX(90.0f);
        ofRotateY(180.0f);

        // ofRotateX(tilt + tiltOffset);   // doesn't work - rotates the cylinder with top as reference
        ofRotateZ(pan + panOffset);

        // create a new quadric to hold our cylinder
        GLUquadric* quad = gluNewQuadric();

        // tell GLU how to create the cylinder
        gluQuadricNormals(quad, GLU_SMOOTH);
        gluQuadricDrawStyle(quad, GLU_FILL);
//        gluQuadricDrawStyle(quad, GLU_LINE);
        gluQuadricTexture(quad, GL_TRUE);
        gluQuadricOrientation(quad, GLU_OUTSIDE);

        // enable depth test, so we only see the front
        glEnable(GL_DEPTH_TEST);

        // use our texture to draw the cylinder
        img.getTextureReference().bind();

        // draw cylinder
        // void gluCylinder	(	GLUquadric* quad , GLdouble base , GLdouble top , GLdouble height , GLint slices , GLint stacks );
        gluCylinder(quad, CYLINDER_RADIUS, CYLINDER_RADIUS, CYLINDER_HEIGHT, CYLINDER_SLICES, CYLINDER_STACKS);

        // stop using our texture
        img.getTextureReference().unbind();

        // delete the cylinder from memory
        gluDeleteQuadric(quad);

    ofPopMatrix();
}
//------------------------------------------------------------------------------------------------------------

void testApp::keyReleased(int key)
{
	switch(key)
	{
		case OF_KEY_LEFT:
			x += 4;
			break;
		case OF_KEY_RIGHT:
            x -= 4;
			break;
        case OF_KEY_UP:
            y += 4;
			break;
        case OF_KEY_DOWN:
            y -= 4;
			break;
        case OF_KEY_F5:
            z += 4;
            break;
        case OF_KEY_F6:
            z -= 4;
            break;
		case OF_KEY_RETURN:
			cout << "framerate: " << ofGetFrameRate() << std::endl;
			//cout << "key: " << key << endl;
            cout << "Tilt: " << tilt << " Pan: " << pan << endl;
            cout << "X=" << x << " Y=" << y << " Z=" << z << endl;

//			GLint texSize;
//			glGetIntegerv(GL_MAX_TEXTURE_SIZE, &texSize);
//			cout << "GL_MAX_TEXTURE_SIZE: " << texSize << endl;

			break;

        case OF_KEY_ESC:
            OF_EXIT_APP(1);
            break;
	}

}

void testApp::mousePressed(int x, int y, int button)
{
        test = true;
}

void testApp::mouseReleased(int x, int y, int button)
{
        test = false;
}

