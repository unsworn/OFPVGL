#include "PVGL.h"

using namespace std;

//------------------------------------------------------------------------------------------------------------



#define CYLINDER_SLICES     4
#define CYLINDER_STACKS     1

#define NUM_SECTIONS        4       // number of quad strips that forms the cylinder

#define ZOOM_RATIO			0.2		// ratio of cylinder radius

//------------------------------------------------------------------------------------------------------------

void
PVGL::setup()
{
	cylinderHeight = ((PVGL::CYLINDER_RADIUS * 2) * PI) / 4;		// too keep 4:1 ratio
    tilt			= 0.0;
    pan				= 0.0;
    zoom			= 0.0;
	zoomOffset		= PVGL::CYLINDER_RADIUS * -ZOOM_RATIO;
	view			= 1;							// 1 = standard, 2 = manual camera for debug
	eyeX			= (float) ofGetWidth() / 2;
	eyeY			= (float) ofGetHeight() / 2;
	eyeZ			= 5000;
    bShowInfo       = false;
    bUseArbTex      = false;
    bLoading        = false;
    bTexturesReady  = false;

	ofDisableArbTex();		// Intel's gfx driver on EEE doesn't support GL_ARB_Texture_Rectangle so we use GL_TEXTURE_2D
    currPanoIdx     = 0;

    defaultInterfaceZ = -800.0;	// -665! This should probably be calculated dynamically, but what the hey
    currInterfaceZ = defaultInterfaceZ;
    interfaceW		= 1024.0;
	interfaceH		= 448.0;
	admin           = new PVGLAdmin(interfaceW, interfaceH);

    ofHideCursor();
	ofDisableSetupScreen();
	ofSetFrameRate(1);         // WARNING: temp hack! should be 60 as smooth as possible
	ofSetVerticalSync(true);
    glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	ofBackground(0, 0, 0);

    refreshPanoramaList();

	setupScreen();              // setup perspective stuff

    //////////////

    imgLoader.start();

    //loadPanorama(0);        // this works - but not the threaded load... hmmm...
    //////////////

    loadInfoImage();

    bUseSensor      = false;

    if (bUseSensor)
        sensor = sensor->Create();


}

//------------------------------------------------------------------------------------------------------------
// Public methods
//------------------------------------------------------------------------------------------------------------
float
PVGL::getInterfaceW()
{
    return interfaceW;
}

float
PVGL::getInterfaceH()
{
    return interfaceH;
}

float
PVGL::getInterfaceZ()
{
    return currInterfaceZ;
}

//------------------------------------------------------------------------------------------------------------

// prepare for drawing
void
PVGL::update()
{

    if (imgLoader.imgLoaded && !bTexturesReady)
    {
        imgLoader.stop();

        // TODO: how to copy images correctly?
        currPanoImg = new ofImage();
        currPanoImg = imgLoader.getPanorama();
        panoSections = imgLoader.getPanoramaSections();

        currPanoW = (int)currPanoImg->getWidth();
        currPanoH = (int)currPanoImg->getHeight();
        sectionW = currPanoW / NUM_SECTIONS;		// 8000 / 4 = 2000

        setupTextures();    // TODO: this should also happen in the PVGLPanoLoader thread
    }
    if (bUseSensor)
    {
        if (sensor != NULL)
        {
            sensor->Poll();
            tilt = (sensor->ReadTilt());   // reverse values to correspond with parascope tilt
            pan = sensor->ReadPan();
            checkButtons();
        }
        else
            printf("Sensor is NULL\n");
    }
}

//------------------------------------------------------------------------------------------------------------

// the place for efficiency
void
PVGL::draw()
{
    ofLog(OF_LOG_VERBOSE, "PVGL - drawing frame %i", ofGetFrameNum());

    //if (sensor == NULL)
    //    return ;

    /*
	 if (!sensor->HasChanged())
	 return ;
	 */
    setupScreen();                                                      // viewport settings (do we have to do it every time?!)

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if (view == 1)
		gluLookAt(0.0, 0.0, zoom, 0.0, 0.0, -10000.0, 0.0, 1.0, 0.0);	// real deal
	else if (view == 2)
		gluLookAt(eyeX, eyeY, eyeZ, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);		// overview

	ofPushMatrix();														// store camera matrix

	drawReferenceLines(2000.0);

	glColor3f(1.0, 1.0, 1.0);
    glRotatef(tilt + admin->getTiltOffset(), 1000.0, 0.0, 0.0);						// tilt
	glRotatef(pan + admin->getPanOffset(), 0.0, 1000.0, 0.0);						// pan

    if (bTexturesReady)
        drawCylinder(PVGL::CYLINDER_RADIUS, cylinderHeight, CYLINDER_SLICES);

	ofPopMatrix();

    //if (ofGetFrameNum()%10 == 0)

	if (bShowInfo) {				// use alpha blending
		/*
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		drawInfoScreen();
		glDisable(GL_BLEND);
		*/
	}

    glError();

    if (bAdminMode)
        admin->draw();
}

//------------------------------------------------------------------------------------------------------------

void
PVGL::setupScreen()
{
	float nearDist = 10.0;			// near/far clipping planes
	float farDist = 10000.0;
	float aspect = PVGL::VIEWPORT_WIDTH / PVGL::VIEWPORT_HEIGHT;// (float)ofGetWidth() / ofGetHeight();
    float viewPortY = ((ofGetHeight() / 2) - PVGL::VIEWPORT_HEIGHT/2) + admin->getViewportVerticalOffset();

    glViewport(0, viewPortY, PVGL::VIEWPORT_WIDTH, PVGL::VIEWPORT_HEIGHT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(PVGL::FOVY, aspect, nearDist, farDist);
}

//------------------------------------------------------------------------------------------------------------

void
PVGL::refreshPanoramaList()
{
    string currUrl;

    if (panoUrls.size() == 0)
        currUrl = "";
    else
        currUrl = panoUrls[currPanoIdx];

    panoUrls.clear();   // reset vector
    currPanoIdx = 0;

    int numPanos = dirList.listDir("panoramas");
    for(int i = 0; i < numPanos; i++)
    {
        panoUrls.push_back(dirList.getPath(i));

        if (dirList.getPath(i) == currUrl)
            currPanoIdx = i;
    }

}

//------------------------------------------------------------------------------------------------------------

void
PVGL::gotoPanorama(int newPanoId)
{
    refreshPanoramaList();

    if (newPanoId <= 0)
        currPanoIdx = panoUrls.size() - 1;
    else if (newPanoId >= panoUrls.size())
        currPanoIdx = 0;
    else
        currPanoIdx = newPanoId;

    //loadPanorama(currPanoIdx);
}

//------------------------------------------------------------------------------------------------------------

// TODO: Deprecated - move to thread
void
PVGL::loadPanorama(int panoIdx)
{
    // WORKS
    ofLog(OF_LOG_VERBOSE, "PVGL - loadPanorama %s", panoUrls[panoIdx].c_str());

    // TODO: unload currentPanorama!
    // TODO: Error checking!
    /*
    currPanoImg = new ofImage();
    currPanoImg->setUseTexture(false);
   	currPanoImg->loadImage(panoUrls[panoIdx]);			// 8000x2000 -- TODO: Hmm: glError says "invalid value caught"
    */

    imgLoader.loadPanorama(panoUrls[panoIdx]);
    currPanoImg = imgLoader.getPanorama();

	currPanoW = (int)currPanoImg->getWidth();
	currPanoH = (int)currPanoImg->getHeight();
	sectionW = currPanoW / NUM_SECTIONS;		// 8000 / 4 = 2000
    //setupTextures();			// load images and create textures
}

//------------------------------------------------------------------------------------------------------------

void
PVGL::loadInfoImage()
{
    // TODO: Info-image hack
	infoImg.loadImage("info1024x448.png");
	int imgW = infoImg.getWidth();
	int imgH = infoImg.getHeight();
	infoTexture.allocate(imgW, imgH, GL_RGBA, bUseArbTex);
	infoTexture.loadData(infoImg.getPixels(), imgW, imgH, GL_RGBA);
}

//------------------------------------------------------------------------------------------------------------

void
PVGL::setupTextures()
{
	ofLog(OF_LOG_VERBOSE, "PVGL - setupTextures");
	int t1 = ofGetElapsedTimeMillis();

	/*
	//unsigned char* hugeImagePixels = panorama.getPixels();
    unsigned char* hugeImagePixels = currPanoImg->getPixels();
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
	bTexturesReady = true;

	int t2 = ofGetElapsedTimeMillis();
	ofLog(OF_LOG_VERBOSE, "PVGL - setupTextures finished in %i ms", t2 - t1);
	 */

    int j = NUM_SECTIONS;	// 4
	for (int i = 0; i < 4; i++)
	{
		j--;	// reverse image order
		textures[i].allocate(sectionW, sectionW, GL_RGB, bUseArbTex);		// textures are initially bound and enabled here
		textures[i].loadData(imgLoader.panoSections[j].getPixels(), sectionW, sectionW, GL_RGB);
	}
	bTexturesReady = true;

	int t2 = ofGetElapsedTimeMillis();
	ofLog(OF_LOG_VERBOSE, "PVGL - setupTextures finished in %i ms", t2 - t1);
}

//------------------------------------------------------------------------------------------------------------

// Draws an open, 1-stacked cylinder around the Y-axis.
// Bottom is Y=(-height/2) and top is Y=(height/2)
void
PVGL::drawCylinder(double radius, double height, int slices)
{
    ofLog(OF_LOG_VERBOSE, "PVGL - Drawing cylinder...");

    int i, j;
	double tx, ty, vx, vy, vz;

    /* Pre-computed circle */
    double *sint, *cost;
    circleTable(&sint, &cost, -slices);

	double quadW = currPanoW / (slices / NUM_SECTIONS);
	int slice = 1;
    glError();
	for (j = 0; j < NUM_SECTIONS; j++)
	{
	    //ofLog(OF_LOG_VERBOSE, "PVGL - Cylinder section #%i", j);
		slice--;				// start at the same line as the last quad

		textures[j].bind();		// bind the right texture

		glBegin(GL_QUAD_STRIP);
		for (i = 0; i <= slices / NUM_SECTIONS; i++)
		{
		    //ofLog(OF_LOG_VERBOSE, "PVGL - Cylinder slice #%i", i);
			tx = (currPanoW - (quadW * i)) / currPanoW;		// clamp texture coords to 0..1
			//cout << "panoH" << currPanoH << endl;
			ty = currPanoH / currPanoH;						// 1.0  // WARNING: possible division by 0
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
            ofLog(OF_LOG_VERBOSE, "PVGL - End of cylinder slice #%i", i);
			slice++;
		}
		glEnd();
	}

    /* Release sin and cos tables */
    free(sint);
    free(cost);

    ofLog(OF_LOG_VERBOSE, "PVGL - Done drawing cylinder!");
}

//------------------------------------------------------------------------------------------------------------

void
PVGL::drawInfoScreen()
{
    float w = getInterfaceW();
    float h = getInterfaceH();
    float z = getInterfaceZ();

	glTranslatef(-w / 2, -h / 2, z);
	//glTranslatef(-w/2, -200.0, z);
	infoTexture.bind();

// 224
	glBegin(GL_QUADS);
        glTexCoord2f(0.0, 0.0); glVertex2f(0.0, 0.0);
        glTexCoord2f(1.0, 0.0); glVertex2f(w, 0.0);
        glTexCoord2f(1.0, 1.0); glVertex2f(w, h);
        glTexCoord2f(0.0, 1.0); glVertex2f(0.0, h);
	glEnd();
}

//------------------------------------------------------------------------------------------------------------

void
PVGL::zoomIn()
{
    zoom = zoomOffset;
    currInterfaceZ = defaultInterfaceZ + zoom;
}

//------------------------------------------------------------------------------------------------------------

void
PVGL::zoomOut()
{
    zoom = 0.0;
    currInterfaceZ = defaultInterfaceZ;
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
void PVGL::circleTable(double **sint, double **cost, const int n)
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
PVGL::drawReferenceLines(float len)
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

void
PVGL::setAdminMode(bool flag)
{
    bAdminMode = flag;

    if (bAdminMode)
        ofLog(OF_LOG_VERBOSE, "PVGL - Admin mode ON");
    else
        ofLog(OF_LOG_VERBOSE, "PVGL - Admin mode OFF");
}

//------------------------------------------------------------------------------------------------------------

void
PVGL::showInfo()
{
	bShowInfo = true;
}

//------------------------------------------------------------------------------------------------------------

void
PVGL::hideInfo()
{
	bShowInfo = false;
}

//------------------------------------------------------------------------------------------------------------

void
PVGL::windowResized(int w, int h)
{
    ofLog(OF_LOG_VERBOSE, "PVGL - windowResized w:%i h:%i", w, h);
}

//------------------------------------------------------------------------------------------------------------

void
PVGL::keyPressed(int key)
{
	//cout << "key: " << key << endl;

	double eyeIncr = 20.0;

	switch(key)
	{
		case 97:							// A -> Zoom
			if (!bAdminMode)
                zoomIn();
            else
                admin->buttonPressed(Sensor::BUTTON_ZOOM);
			break;
		case 115:							// S -> Info
            if (!bAdminMode)
                showInfo();
            else
                admin->buttonPressed(Sensor::BUTTON_INFO);
			break;
		case 122:							// Z -> Previous
			if (!bAdminMode)
                gotoPanorama(currPanoIdx - 1);
            else
                admin->buttonPressed(Sensor::BUTTON_PREV);
			break;
		case 120:							// X -> Next
			if (!bAdminMode)
                gotoPanorama(currPanoIdx + 1);
			else
                admin->buttonPressed(Sensor::BUTTON_NEXT);
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
PVGL::keyReleased(int key)
{

	switch(key)
	{
		case OF_KEY_F2:
            setAdminMode(!bAdminMode);      // F2 -> Toggle Adminmode
        break;

		case 'a':							// A
			zoomOut();						// Zoom out to normal
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
PVGL::checkButtons()
{
    if (sensor->IsButtonPressed(Sensor::BUTTON_PREV))               // PREV button
    {
        if (btnPrevDown)
            return; // not the first click

        btnPrevDown = true;
        ofLog(OF_LOG_VERBOSE, "PREV pressed");

        if (bAdminMode)
            admin->buttonPressed(Sensor::BUTTON_PREV);
        else
            gotoPanorama(currPanoIdx - 1);
    }
    else
    {
        btnPrevDown = false;
    }

    if (sensor->IsButtonPressed(Sensor::BUTTON_NEXT))               // NEXT button
    {
        if (btnNextDown)
            return;

        btnNextDown = true;
        ofLog(OF_LOG_VERBOSE, "NEXT pressed!");

        if (bAdminMode)
            admin->buttonPressed(Sensor::BUTTON_NEXT);
        else
            gotoPanorama(currPanoIdx + 1);
    }
    else
    {
        btnNextDown = false;
    }

    if (sensor->IsButtonPressed(Sensor::BUTTON_INFO))               // INFO button
    {
        btnInfoDown = true;
        ofLog(OF_LOG_VERBOSE, "INFO pressed");
        if (!bShowInfo)
            showInfo();
    }
    else
    {
        btnInfoDown = false;
        if (bShowInfo)
            hideInfo();
    }

    if (sensor->IsButtonPressed(Sensor::BUTTON_ZOOM))               // ZOOM button
    {
        if(!btnZoomDown)
        {
            btnZoomDown = true;
            ofLog(OF_LOG_VERBOSE, "ZOOM pressed");
            zoomIn();
        }
    }
    else
    {
        if(btnZoomDown)
        {
            btnZoomDown = false;
            ofLog(OF_LOG_VERBOSE, "ZOOM released");
            zoomOut();
        }
    }
}

//------------------------------------------------------------------------------------------------------------

void
PVGL::mousePressed(int x, int y, int button)
{
	//zoom = ZOOM_OFFSET;
	//glError();
}

//------------------------------------------------------------------------------------------------------------

void
PVGL::mouseReleased(int x, int y, int button)
{
	//zoom = 0;
}

//------------------------------------------------------------------------------------------------------------

void
PVGL::glError()
{
	GLenum err = glGetError();
	while (err != GL_NO_ERROR) {
		fprintf(stderr, "glError: %s caught at %s:%u\n", (char *)gluErrorString(err), __FILE__, __LINE__);
		err = glGetError();
	}
}

