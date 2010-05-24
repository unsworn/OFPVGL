#include "PVGLImageLoader.h"

//------------------------------------------------------------------------------------------------------------

PVGLImageLoader::PVGLImageLoader()
{
	imgLoaded = false;
}

//------------------------------------------------------------------------------------------------------------


PVGLImageLoader::~PVGLImageLoader()
{
	//delete [] textures;
    //textures = NULL;
}

//------------------------------------------------------------------------------------------------------------

void
PVGLImageLoader::start()
{
    startThread(true, true);   // blocking, verbose
}

//------------------------------------------------------------------------------------------------------------

void
PVGLImageLoader::stop()
{
    ofLog(OF_LOG_VERBOSE, "PVGLImageLoader - stop() called");
    stopThread();
}

//------------------------------------------------------------------------------------------------------------

void
PVGLImageLoader::threadedFunction()
{
    while (isThreadRunning() != 0)
    {
        if (!imgLoaded) {
            loadPanorama("data/panoramas/operan8192.jpg");
            createPanoramaSections();
            imgLoaded = true;
        }
    }
    ofLog(OF_LOG_VERBOSE, "PVGLImageLoader - Exiting threadedFunction() in 30 seconds...");
    ofSleepMillis(30000);
    ofLog(OF_LOG_VERBOSE, "PVGLImageLoader - Exiting threadedFunction() NOW!");
}

//------------------------------------------------------------------------------------------------------------

void
PVGLImageLoader::loadPanorama(string url)
{

    ofLog(OF_LOG_VERBOSE, "PVGLImageLoader - Loading \"%s...\"", url.c_str());
    int t1 = ofGetElapsedTimeMillis();

    // TODO: unload currentPanorama!
    // TODO: Error checking!
    img = new ofImage();
    img->setUseTexture(false);
   	img->loadImage(url);			// 8000x2000 -- TODO: Hmm: glError says "invalid value caught"

    int t2 = ofGetElapsedTimeMillis();
    ofLog(OF_LOG_VERBOSE, "PVGLImageLoader - Loaded \"%s\" in %i ms!", url.c_str(), t2 - t1);
}

//------------------------------------------------------------------------------------------------------------

void
PVGLImageLoader::createPanoramaSections()
{
    ofLog(OF_LOG_VERBOSE, "PVGLImageLoader - Slicing up panorama...");
    int t1 = ofGetElapsedTimeMillis();
    ////////////

    int numSections = 4;
    int currPanoW = (int)img->getWidth();
	int currPanoH = (int)img->getHeight();
	int sectionW = currPanoW / numSections;		// 8000 / 4 = 2000

    unsigned char* hugeImagePixels = img->getPixels();
	unsigned char* sections[numSections];

	for (int i = 0; i < numSections; i++)
	{
		sections[i] = new unsigned char[sectionW * sectionW *3];
		for (int j = 0; j < sectionW; j++ )
		{
			// copy 1 row of pixels from large panorama image to sections
			// this is assuming RGB images - for RGBA change all the 3's into 4's.
			memcpy( &sections[i][j * sectionW * 3], &hugeImagePixels[j * sectionW * 3 * 4 + i * sectionW *3], sectionW * 3 );
		}
		panoSections[i].setFromPixels(sections[i], sectionW, sectionW, OF_IMAGE_COLOR, true);
		delete[] sections[i];
	}

	// delete[] hugeImagePixels; ?

    ////////////
    int t2 = ofGetElapsedTimeMillis();
    ofLog(OF_LOG_VERBOSE, "PVGLImageLoader - Sliced it up in %i ms!", t2 - t1);
}

//------------------------------------------------------------------------------------------------------------

ofImage*
PVGLImageLoader::getPanorama()
{
    if(imgLoaded)
    {
        //if(currTexture)
        //    delete currTexture;
        //currTexture=nextTexture;
        //imgLoaded = false;
        return img;
    }
    else
    {
        return NULL;
    }
}

//------------------------------------------------------------------------------------------------------------

ofImage*
PVGLImageLoader::getPanoramaSections()
{
    if(imgLoaded)
    {
        //if(currTexture)
        //    delete currTexture;
        //currTexture=nextTexture;
        //imgLoaded = false;
        ofSleepMillis(1000);
        return panoSections;
    }
    else
    {
        return NULL;
    }
}
