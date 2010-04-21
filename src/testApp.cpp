#include "testApp.h"

using namespace std;

//------------------------------------------------------------------------------------------------------------

// height = ((radius*2) * pi) / 4   -- too keep 4:1 aspect ratio
#define CYLINDER_RADIUS     200
#define CYLINDER_HEIGHT     314
#define CYLINDER_SLICES     32
#define CYLINDER_STACKS     1

#define NUM_SECTIONS        4       // number of quad strips that forms the cylinder

#define ZOOM_OFFSET         100
#define VIEWPORT_HEIGHT     450

//------------------------------------------------------------------------------------------------------------

void
testApp::setup()
{
    loadSettings();             // load settings from XML

	ofDisableSetupScreen();
	setupScreen();              // we do it ourselves

	ofSetFrameRate(60);         // as smooth as possible
	ofSetVerticalSync(true);

    loadPanorama(0);            // load first pano

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	ofBackground(0, 0, 0);

    tilt = 0.0;
    pan = 0.0;
    zoom = 0.0;
    panOffset = 0.0;
    tiltOffset = 0.0;
    ang = 0.0;
    test = false;
    centerX = ofGetWidth() / 2;
    centerY = ofGetHeight() / 2;

    // do not use ARB textures, this way it is easier to use the GLU objects
    /*ofDisableArbTex();
    img.loadImage("tri_tb_2048x512.jpg");    // ska vara "tri_tb_8000x2000" men det vill sig inte
	ofEnableArbTex();
    */
    // init sensor
	sensor = Sensor::Create();
}

//------------------------------------------------------------------------------------------------------------

void
testApp::loadSettings()
{
    if(!xml.loadFile("settings.xml")) {
        printf("Couldn't load XML settings!\n");
    }

    panOffset = xml.getValue("PVGL:PANOFFSET", 0);      // 0 is default value if settings file doesn't exist
    tiltOffset = xml.getValue("PVGL:TILTOFFSET", 0);
    viewportVerticalOffset = xml.getValue("PVGL:VIEVPORTVERTICALOFFSET", 0);    // not implemented yet

    xml.pushTag("PVGL");
    xml.pushTag("PANOS");
    int numPanos = xml.getNumTags("PANO");
    for (int i = 0; i < numPanos; i++ )
    {
        panoUrls[i] = xml.getValue("PANO", "*** invalid pano url ***", i);
        cout << "URL #" << i << ": " << panoUrls[i] << endl;
    }
    xml.popTag(); // PANOS
    xml.popTag(); // PVGL

    printf("panOffset = %f\n", panOffset);
    printf("tiltOffset = %f\n", tiltOffset);
    printf("viewPortVerticalOffset = %f\n", viewportVerticalOffset);
}

//------------------------------------------------------------------------------------------------------------

void
testApp::setupScreen()
{
    int w, h;

	w = ofGetWidth();
	h = ofGetHeight();

	float halfFov, theTan, screenFov, aspect;
	screenFov 		= 60.0f;
	float eyeX 		= (float)w / 2.0;
	float eyeY 		= (float)h / 2.0;
	halfFov 		= PI * screenFov / 360.0;
	theTan 			= tanf(halfFov);
	float dist 		= eyeY / theTan;
	float nearDist 	= dist / 10.0;	// near / far clip plane
	float farDist 	= dist * 10.0;
	aspect 			= (float)w / (float)h;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(screenFov, aspect, nearDist, farDist);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//gluLookAt(eyeX, eyeY, dist, eyeX, eyeY, 0.0, 0.0, 1.0, 0.0);

	//glScalef(1, -1, 1);			// invert Y axis so increasing Y goes down
  	//glTranslatef(0, -h, 0);       // shift origin up to upper-left corner.
}
//------------------------------------------------------------------------------------------------------------

void testApp::loadPanorama(int newPanoId)
{
    cout << "Loading panorama:" << panoUrls[newPanoId] << endl;

    // TODO: unload currentPanorama!
    // TODO: Error checking!
   	panorama.loadImage(panoUrls[newPanoId]);			// 8000x2000
    currPanoId = newPanoId;

	float panoW = panorama.getWidth();

	// TODO: if panoW > GL_MAX_TEXTURE_SIZE (normally 2048) split it into sections
	sectionW = panoW / NUM_SECTIONS;		// 8000 / 4 = 2000

    setupTextures();			// load images and create textures
}

void
testApp::setupTextures()
{
	unsigned char* hugeImagePixels = panorama.getPixels();
	unsigned char* sections[NUM_SECTIONS];
	for (int i = 0; i < NUM_SECTIONS; i++)
	{
		sections[i] = new unsigned char[sectionW * sectionW *3];
		for (int j = 0; j < sectionW; j++ )
		{
			// copy 1 row of pixels from large panorama image to sections
			// this is assuming RGB images - for RGBA change all the 3's into 4's.
			memcpy( &sections[i][j * sectionW * 3], &hugeImagePixels[j * sectionW * 3 * 4 + i * sectionW *3], sectionW * 3 );
		}
		images[i].setFromPixels(sections[i], sectionW, sectionW, OF_IMAGE_COLOR, true);
		delete[] sections[i];
	}

	int j = NUM_SECTIONS;	// 4
	for (int i = 0; i < 4; i++)
	{
		j--;	// reverse image order
		textures[i].allocate(sectionW, sectionW, GL_RGB, true);		// textures are initially bound and enabled here
		textures[i].loadData(images[j].getPixels(), sectionW, sectionW, GL_RGB);
	}
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

    ang += 0.2;
}

//------------------------------------------------------------------------------------------------------------

void
testApp::draw()
{
    // if (sensor == NULL)
    //    return ;

    /*
    if (!sensor->HasChanged())
        return ;
    */

    zoom = 0;

    if (sensor->IsButtonPressed(Sensor::kButtonPrev))
    {
        printf("PREV pressed!\n");
    }
    if (sensor->IsButtonPressed(Sensor::kButtonNext))
    {
        printf("NEXT pressed!\n");
    }
    if (sensor->IsButtonPressed(Sensor::kButtonInfo))
    {
        printf("INFO pressed!\n");
    }
    if (sensor->IsButtonPressed(Sensor::kButtonZoom))
    {
        printf("ZOOM pressed!\n");
        zoom = ZOOM_OFFSET;
    }

    ofPushMatrix();

    glTranslated(0.0, -CYLINDER_HEIGHT / 2, zoom);          // "setup camera"
                                                            // TODO: tilt + zoom = no good
    drawReferenceLines(500.0);
    glColor3f(1.0, 1.0, 1.0);

    glRotatef(tilt + tiltOffset, 1.0, 0.0, 0.0);        // tilt
    glRotatef(pan + panOffset, 0.0, 1.0, 0.0);          // pan

	drawCylinder(CYLINDER_RADIUS, CYLINDER_HEIGHT, 32);

	ofPopMatrix();


    /*
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
    */
}

//------------------------------------------------------------------------------------------------------------

// Draws an open, 1-stacked cylinder around the X-axis. Top is X=0
void testApp::drawCylinder(double radius, double height, int slices)
{
    int i, j;

    /* Pre-computed circle */
    double *sint, *cost;
    circleTable(&sint, &cost, -slices);

	double imgW = 2000.0;
	double imgH = 2000.0;

	double quadW = imgW / (slices / NUM_SECTIONS);
	int slice = 1;

	for (j = 0; j < NUM_SECTIONS; j++)
	{
		slice--;		// start at the same line as the last quad

		textures[j].bind();		// bind the right texture

		glBegin(GL_QUAD_STRIP);
		for (i = 0; i <= slices / NUM_SECTIONS; i++)
		{
			glTexCoord2d(imgW - (quadW * i), imgH);
			glVertex3d(cost[slice]*radius, 0.0, sint[slice]*radius);
			//printf("Vertex: X=%f Y=%f Z=%f \n", cost[i]*radius, y0, sint[i]*radius);

			//glVertex3d(cost[j]*radius, sint[j]*radius, z1  );
			glTexCoord2d(imgW - (quadW * i), 0.0);
			glVertex3d(cost[slice]*radius, height, sint[slice]*radius);
			//printf("Vertex: X=%f Y=%f Z=%f \n", cost[j]*radius, y1, sint[j]*radius);

			slice++;
		}
		glEnd();
	}

    /* Release sin and cos tables */
    free(sint);
    free(cost);
}

//------------------------------------------------------------------------------------------------------------

/*
 * Compute lookup table of cos and sin values forming a cirle
 *
 * Notes:
 *    It is the responsibility of the caller to free these tables
 *    The size of the table is (n+1) to form a connected loop
 *    The last entry is exactly the same as the first
 *    The sign of n can be flipped to get the reverse loop
 */
// TODO: should be calculated only once! static?
// TODO: do the right thing when out of memory
void testApp::circleTable(double **sint, double **cost, const int n)
{
    int i;

    /* Table size, the sign of n flips the circle direction */
    const int size = abs(n);

    /* Determine the angle between samples */
    const double angle = 2*M_PI/(double)n;

    /* Allocate memory for n samples, plus duplicate of first entry at the end */
    *sint = (double *) calloc(sizeof(double), size+1);
    *cost = (double *) calloc(sizeof(double), size+1);

    /* Bail out if memory allocation fails, fgError never returns */
    if (!(*sint) || !(*cost))
    {
        free(*sint);
        free(*cost);
        //fgError("Failed to allocate memory in circleTable");
    }

    /* Compute cos and sin around the circle */
    for (i=0; i<size; i++)
    {
        (*sint)[i] = sin(angle*i);
        (*cost)[i] = cos(angle*i);
    }

    /* Last sample is duplicate of the first */
    (*sint)[size] = (*sint)[0];
    (*cost)[size] = (*cost)[0];
}

//------------------------------------------------------------------------------------------------------------

void
testApp::drawReferenceLines(float len)
{
	glBegin(GL_LINES);
	glColor3f(1.0, 0.0, 0.0);		// x = red
	glVertex3f(-len, 0.0, 0.0);
	glVertex3f(len, 0.0, 0.0);
	glVertex3f(len, 0.0, 0.0);		// arrow
	glVertex3f(len - len/10, -len/20, 0.0);
	glVertex3f(len, 0.0, 0.0);
	glVertex3f(len- len/10, len/20, 0.0);

	glColor3f(0.0, 1.0, 0.0);		// y = green
	glVertex3f(0.0, -len, 0.0);
	glVertex3f(0.0, len, 0.0);
	glVertex3f(0.0, len, 0.0);
	glVertex3f(-len/20, len - len/10, 0.0);
	glVertex3f(0.0, len, 0.0);
	glVertex3f(len/20, len - len/10, 0.0);

	glColor3f(0.0, 0.0, 1.0);		// z = blue
	glVertex3f(0.0, 0.0, -len);
	glVertex3f(0.0, 0.0, len);
	glVertex3f(0.0, 0.0, len);
	glVertex3f(0.0, -len/20, len - len/10);
	glVertex3f(0.0, 0.0, len);
	glVertex3f(0.0, len/20, len - len/10);
	glEnd();

	glClear(GL_COLOR_BUFFER_BIT);
}


//------------------------------------------------------------------------------------------------------------

void
testApp::keyReleased(int key)
{
	switch(key)
	{
		case OF_KEY_LEFT:
			loadPanorama(--currPanoId);
			break;
		case OF_KEY_RIGHT:
            loadPanorama(++currPanoId);
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

