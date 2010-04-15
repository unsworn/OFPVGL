#include "testApp.h"

using namespace std;

//------------------------------------------------------------------------------------------------------------

// height = ((radius*2) * pi) / 4   -- too keep 4:1 aspect ratio
#define CYLINDER_RADIUS     200
#define CYLINDER_HEIGHT     314
#define CYLINDER_SLICES     32
#define CYLINDER_STACKS     1

#define ZOOM_OFFSET         100
#define VIEWPORT_HEIGHT     450

int dir = 1;

//------------------------------------------------------------------------------------------------------------
void
testApp::setup()
{
	ofBackground(0, 0, 0);

    // load settings
	if(!xml.loadFile("settings.xml"))
        printf("Couldn't load XML settings!");
    panOffset = xml.getValue("PVGL:PANOFFSET", 0);      // 0 is default value if settings file doesn't exist
    tiltOffset = xml.getValue("PVGL:TILTOFFSET", 0);
    viewportVerticalOffset = xml.getValue("PVGL:VIEVPORTVERTICALOFFSET", 0);    // not implemented yet

    printf("panOffset = %f\n", panOffset);
    printf("tiltOffset = %f\n", tiltOffset);
    printf("viewPortVerticalOffset = %f\n", viewportVerticalOffset);

	//for smooth animation, set vertical sync if we can
	ofSetVerticalSync(true);
	ofSetFrameRate(60);

    tilt = 0.0;
    pan = 0.0;
    zoom = 0.0;

    x = 0.0;
    y = 0.0;
    z = 0.0;

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

void
testApp::update()
{
    // do sensor polling here
    if (sensor != NULL)
    {
        sensor->Poll();
        tilt = sensor->ReadTilt();
        pan = sensor->ReadPan();
    }
    else
        printf("Sensor is NULL\n");
}

//------------------------------------------------------------------------------------------------------------

void
testApp::draw()
{

    //if (sensor == NULL)
    //    return ;

    /*
    if (!sensor->HasChanged())
        return ;
    */
    zoom = 0;

    if (sensor->IsButtonPressed(sensor->kButtonPrev))
    {
        printf("PREV pressed!\n");
    }
    if (sensor->IsButtonPressed(sensor->kButtonNext))
    {
        printf("NEXT pressed!\n");
    }
    if (sensor->IsButtonPressed(sensor->kButtonInfo))
    {
        printf("INFO pressed!\n");
    }
    if (sensor->IsButtonPressed(sensor->kButtonZoom))
    {
        printf("ZOOM pressed!\n");
        zoom = ZOOM_OFFSET;
    }

    ofTranslate(centerX, centerY, CYLINDER_RADIUS * 2);             // position the "camera"
    ofTranslate(0.0, 0.0, zoom);                                    // zoom?
    drawReferenceLines();

    ofPushMatrix();
        ofSetColor(0xffffff);
        ofRotateX(90.0);                                            // från rör till velodrom
        ofRotateX(tilt + tiltOffset);                               // tilt the cylinder
        ofRotateZ(-pan + panOffset);                                 // spin it around
        ofRotateY(180.0);                                             // for some reason it's upside down - turn it!
		ofTranslate(0.0, 0.0, -CYLINDER_HEIGHT / 2);				// center the cylinder around "origin

        GLUquadric* quad = gluNewQuadric();
        gluQuadricNormals(quad, GLU_SMOOTH);
        gluQuadricDrawStyle(quad, GLU_FILL);
//      gluQuadricDrawStyle(quad, GLU_LINE);
        gluQuadricTexture(quad, GL_TRUE);
        glEnable(GL_DEPTH_TEST);

        img.getTextureReference().bind();
        gluCylinder(quad, CYLINDER_RADIUS, CYLINDER_RADIUS, CYLINDER_HEIGHT, CYLINDER_SLICES, CYLINDER_STACKS);
        img.getTextureReference().unbind();

        gluDeleteQuadric(quad);
    ofPopMatrix();

}

//------------------------------------------------------------------------------------------------------------

void
testApp::drawReferenceLines()
{
	float len = 200.0;
	glBegin(GL_LINES);
	glColor3f(1.0, 0.0, 0.0);		// x = red
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(len, 0.0, 0.0);
	glColor3f(0.0, 1.0, 0.0);		// y = green
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(0.0, len, 0.0);
	glColor3f(0.0, 0.0, 1.0);		// z = blue
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(0.0, 0.0, len);
	glEnd();
}

//------------------------------------------------------------------------------------------------------------

void
testApp::autoRotate()
{
   	if (dir == 1) {
		tilt += 0.1;
		if (tilt > 10.0)
			dir = 0;
	}
	else {
		tilt -= 0.1;
		if (tilt < -10.0)
			dir = 1;
	}

	pan += 0.3;
	if (pan > 360.0)
		pan = 0;
}

//------------------------------------------------------------------------------------------------------------

void
testApp::keyReleased(int key)
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
        zoom = ZOOM_OFFSET;
}
void testApp::mouseReleased(int x, int y, int button)
{
        zoom = 0;
}

