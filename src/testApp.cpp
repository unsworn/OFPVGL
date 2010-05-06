#include "testApp.h"

using namespace std;

//------------------------------------------------------------------------------------------------------------


#define CYLINDER_RADIUS     1200
#define CYLINDER_SLICES     32
#define CYLINDER_STACKS     1

#define NUM_SECTIONS        4       // number of quad strips that forms the cylinder

#define ZOOM_RATIO			0.3		// ratio of cylinder radius
#define VIEWPORT_HEIGHT     450

#define USE_SENSOR			1		// not implemented yet

//------------------------------------------------------------------------------------------------------------

void
testApp::setup()
{
    bUseArbTex = false;
	ofDisableArbTex();			// Intel's gfx driver on EEE doesn't support GL_ARB_Texture_Rectangle so we use GL_TEXTURE_2D

	ofDisableSetupScreen();
	ofSetFrameRate(60);         // as smooth as possible
	ofSetVerticalSync(true);
    glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	ofBackground(0, 0, 0);

	loadSettings();             // load settings from XML
	setupScreen();              // we do it ourselves

	cylinderHeight = ((CYLINDER_RADIUS * 2) * PI) / 4;		// too keep 4:1 ratio
    tilt			= 0.0;
    pan				= 0.0;
    zoom			= 0.0;
    panOffset		= 0.0;
    tiltOffset		= 0.0;
	zoomOffset		= CYLINDER_RADIUS * -ZOOM_RATIO;

	view			= 1;							// 1 = standard, 2 = manual camera for debug
	eyeX			= (float) ofGetWidth() / 2;
	eyeY			= (float) ofGetHeight() / 2;
	eyeZ			= 5000;

	interfaceZ		= -665.0;	// -665! This should probably be calculated dynamically, but what the hey
    interfaceW		= 1024.0;
	interfaceH		= 448.0;

	centerX = ofGetWidth() / 2;
    centerY = ofGetHeight() / 2;
	bShowInfo = false;

    //infoImg.loadImage("info.png");
    loadPanorama(0);            // load first pano
}

//------------------------------------------------------------------------------------------------------------

void
testApp::loadSettings()
{
    if(!xml.loadFile("settings.xml")) {
        ofLog(OF_LOG_ERROR, "loadSettings: Couldn't load XML settings!\n");
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
        cout << "loadSettings: URL #" << i << ": " << panoUrls[i] << endl;
    }
    xml.popTag(); // PANOS
    xml.popTag(); // PVGL

    //printf("panOffset = %f\n", panOffset);
    //printf("tiltOffset = %f\n", tiltOffset);
    //printf("viewPortVerticalOffset = %f\n", viewportVerticalOffset);
}

//------------------------------------------------------------------------------------------------------------

void
testApp::setupScreen()
{
	float fov = 60.0;
	float nearDist = 10.0;			// near/far clipping planes
	float farDist = 10000.0;
	float aspect = (float) ofGetWidth() / (float) ofGetHeight();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fov, aspect, nearDist, farDist);
	//glOrtho(-1000.0, 1000.0, -1000.0, 1000.0, 10.0, 10000.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}
//------------------------------------------------------------------------------------------------------------

void
testApp::loadPanorama(int newPanoId)
{
    cout << "loadPanorama: " << panoUrls[newPanoId] << endl;

    // TODO: unload currentPanorama!
    // TODO: Error checking!
   	panorama.loadImage(panoUrls[newPanoId]);			// 8000x2000 -- TODO: Hmm: glError says "invalid value caught"
	currPanoId = newPanoId;

	currPanoW = (int)panorama.getWidth();
	currPanoH = (int)panorama.getHeight();

	// TODO: if panoW > GL_MAX_TEXTURE_SIZE (normally 2048) split it into sections
	sectionW = currPanoW / NUM_SECTIONS;		// 8000 / 4 = 2000
    setupTextures();			// load images and create textures

	// TODO: Info-image hack
	//infoImg.loadImage("info.png");
    //int imgW = infoImg.getWidth();
	//int imgH = infoImg.getHeight();
	//infoTexture.allocate(imgW, imgH, GL_RGBA, true);
	//infoTexture.loadData(infoImg.getPixels(), imgW, imgH, GL_RGBA);
}

//------------------------------------------------------------------------------------------------------------

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
		textures[i].allocate(sectionW, sectionW, GL_RGB, bUseArbTex);		// textures are initially bound and enabled here
		textures[i].loadData(images[j].getPixels(), sectionW, sectionW, GL_RGB);
	}
}

//------------------------------------------------------------------------------------------------------------
void
testApp::update()
{
	/*
    // do sensor polling here
    if (sensor != NULL)
    {
        //sensor->Poll();
        //tilt = sensor->ReadTilt();
        //pan = sensor->ReadPan();
    }
    else
        printf("Sensor is NULL\n");
	*/

	if (!USE_SENSOR)
		printf("Not using sensor");
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

    checkButtons();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if (view == 1)
		gluLookAt(0.0, 0.0, zoom, 0.0, 0.0, -10000.0, 0.0, 1.0, 0.0);	// real deal
	else if (view == 2)
		gluLookAt(eyeX, eyeY, eyeZ, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);		// overview


	ofPushMatrix();														// store camera matrix

	drawReferenceLines(2000.0);

	glColor3f(1.0, 1.0, 1.0);
    glRotatef(tilt + tiltOffset, 1000.0, 0.0, 0.0);						// tilt
	glRotatef(pan + panOffset, 0.0, 1000.0, 0.0);						// pan

	drawCylinder(CYLINDER_RADIUS, cylinderHeight, CYLINDER_SLICES);
	ofPopMatrix();

	if (bShowInfo) {				// use alpha blending
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		drawInfoScreen();
		glDisable(GL_BLEND);
	}
    glError();
}

//------------------------------------------------------------------------------------------------------------

// Draws an open, 1-stacked cylinder around the Y-axis.
// Bottom is Y=(-height/2) and top is Y=(height/2)
void
testApp::drawCylinder(double radius, double height, int slices)
{
    int i, j;
	double tx, ty, vx, vy, vz;

    /* Pre-computed circle */
    double *sint, *cost;
    circleTable(&sint, &cost, -slices);

	double quadW = currPanoW / (slices / NUM_SECTIONS);
	int slice = 1;

	for (j = 0; j < NUM_SECTIONS; j++)
	{
		//printf("---------------------------\n");
		slice--;				// start at the same line as the last quad

		textures[j].bind();		// bind the right texture

		glBegin(GL_QUAD_STRIP);
		for (i = 0; i <= slices / NUM_SECTIONS; i++)
		{
			tx = (currPanoW - (quadW * i)) / currPanoW;		// clamp texture coords to 0..1
			ty = currPanoH / currPanoH;						// 1.0
			vx = cost[slice] * radius;
			vy = -height/2;
			vz = sint[slice] * radius;
			glTexCoord2d(tx, ty);
			glVertex3d(vx, vy , vz);
//			printf("Tex: X=%f Y=%f  ", tx, ty);
//			printf("Vertex: X=%f Y=%f Z=%f \n", vx, vy, vz);

			//tx = (imgW - (quadW * i)) / imgW;		// tx is the same
			ty = 0.0 / currPanoH;						// 0.0
			//vx = cost[slice] * radius;			// vx is the same
			vy = height/2;
			//vz = sint[slice]*radius;				// vz as well
			glTexCoord2d(tx, ty);
			glVertex3d(vx, vy , vz);
//			printf("Tex: X=%f Y=%f  ", tx, ty);
//			printf("Vertex: X=%f Y=%f Z=%f \n", vx, vy, vz);

			slice++;
		}
		glEnd();
	}

    /* Release sin and cos tables */
    free(sint);
    free(cost);
}


//------------------------------------------------------------------------------------------------------------

void
testApp::drawInfoScreen()
{
	float infoW = 1024.0;
	float infoH = 448.0;

	glTranslatef(-interfaceW / 2, -interfaceH / 2, interfaceZ + zoom);
	infoTexture.bind();

	glBegin(GL_QUADS);
        glTexCoord2f(0.0, 0.0); glVertex2f(0.0, 0.0);
        glTexCoord2f(infoW, 0.0); glVertex2f(interfaceW, 0.0);
        glTexCoord2f(infoW, infoH); glVertex2f(interfaceW, interfaceH);
        glTexCoord2f(0.0, infoH); glVertex2f(0.0, interfaceH);
	glEnd();
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

	//glClear(GL_COLOR_BUFFER_BIT);
}

//------------------------------------------------------------------------------------------------------------

void
testApp::showInfo()
{
	bShowInfo = true;
}

//------------------------------------------------------------------------------------------------------------

void
testApp::hideInfo()
{
	bShowInfo = false;
}

//------------------------------------------------------------------------------------------------------------

void
testApp::keyPressed(int key)
{
	//cout << "key: " << key << endl;

	double eyeIncr = 20.0;

	switch(key)
	{
		case 97:							// A -> Zoom
			zoom = zoomOffset;
			break;
		case 115:							// S -> Info
			showInfo();
			break;
		case 122:							// Z -> Previous
			loadPanorama(--currPanoId);
			break;
		case 120:							// X -> Next
			loadPanorama(++currPanoId);
			break;

		case OF_KEY_LEFT:					// Arrow keys -> manual pan and tilt
			pan -= 5;
			break;
		case OF_KEY_RIGHT:
            pan += 5;
			break;
        case OF_KEY_UP:
            tilt += 5;
			break;
        case OF_KEY_DOWN:
            tilt -= 5;
			break;
											// manual camera control
		case 103:							// G
			eyeX += eyeIncr;
			break;
		case 98:							// B
			eyeX -= eyeIncr;
			break;
		case 104:							// H
			eyeY += eyeIncr;
			break;
		case 110:							// N
			eyeY -= eyeIncr;
			break;
		case 106:							// J
			eyeZ += eyeIncr;
			break;
		case 109:							// M
			eyeZ -= eyeIncr;
			break;

		// debug
		case OF_KEY_RETURN:
			cout << "framerate: " << ofGetFrameRate() << std::endl;
            cout << "Tilt: " << tilt << " Pan: " << pan << endl;
            cout << "eyeX=" << eyeX << " eyeY=" << eyeY << " eyeZ=" << eyeZ << endl;

			GLint texSize;
			glGetIntegerv(GL_MAX_TEXTURE_SIZE, &texSize);
			cout << "GL_MAX_TEXTURE_SIZE: " << texSize << endl;

			GLint maxArbRectSize;
			glGetIntegerv(GL_MAX_RECTANGLE_TEXTURE_SIZE_ARB, &maxArbRectSize);
			cout << "GL_MAX_RECTANGLE_TEXTURE_SIZE_ARB: " << maxArbRectSize << endl;

			cout << "VERSION: " << (GLubyte*) glGetString(GL_VERSION) << endl;
            cout << "RENDERER: " << (GLubyte*) glGetString(GL_RENDERER) << endl;
            cout << "VENDOR: " << (GLubyte*) glGetString(GL_VENDOR) << endl;
            //cout << "EXTENSIONS: " << (GLubyte*) glGetString(GL_EXTENSIONS) << endl;

			break;

        case OF_KEY_ESC:
            OF_EXIT_APP(1);
            break;
	}

}

//------------------------------------------------------------------------------------------------------------

void
testApp::keyReleased(int key)
{

	switch(key)
	{
		case 'a':							// A
			zoom = 0.0;						// Zoom out to normal
			break;
		case 's':							// S -> Info
			hideInfo();
			break;

		case '1':							// Views
			view = 1;
			break;
		case '2':
			view = 2;
			break;
	}
}

//------------------------------------------------------------------------------------------------------------

void
testApp::checkButtons()
{
/*
    if (sensor->IsButtonPressed(Sensor::kButtonPrev))               // PREV button
    {
        if (btnPrevDown)
            return; // not the first click

        btnPrevDown = true;
        // save to buttonHistory
        loadPanorama(currPanoId - 1);
        printf("PREV pressed!\n");
    }
    else
    {
        btnPrevDown = false;
    }

    if (sensor->IsButtonPressed(Sensor::kButtonNext))
    {
        if (btnNextDown)
            return;

        btnNextDown = true;
        loadPanorama(currPanoId + 1);
        printf("NEXT pressed!\n");
    }
    else
    {
        btnNextDown = false;
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
	*/

}

void testApp::mousePressed(int x, int y, int button)
{
	//zoom = ZOOM_OFFSET;
	//glError();
}
void testApp::mouseReleased(int x, int y, int button)
{
	//zoom = 0;
}

void
testApp::glError()
{
	GLenum err = glGetError();
	while (err != GL_NO_ERROR) {
		fprintf(stderr, "glError: %s caught at %s:%u\n", (char *)gluErrorString(err), __FILE__, __LINE__);
		err = glGetError();
	}
}

