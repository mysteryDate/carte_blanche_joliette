#ifndef _TEST_APP
#define _TEST_APP


#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxSyphon.h"
#include "ofxFTGLFont.h"
#include "ofxUI.h"
#include "ofxFlocking.h"
#include "Boid.h"
#include "Sequence.h"
#include "Path.h"
#include "ofxNetwork.h"






class testApp : public ofBaseApp{
	
public:
	
		
	void setup();
	void update();
	void draw();
	void exit();
	void drawBoids();
	void drawSnow();
	void drawText();
	void updateVideo();
	void drawSnowFromContour();
	void cvImages();
	
	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void windowResized(int w, int h);
	
	
	//Syphon
	ofTexture backgroundTex; 
	ofTexture foregroundTex;	
	ofTexture textTex;
	
	ofxSyphonServer backgroundServer;
	ofxSyphonServer textServer;
	
	ofxSyphonClient mClient;
	
	
	// UI 
	
	ofxUICanvas *gui;
	ofxUITextArea *textbox;
	void guiEvent(ofxUIEventArgs &e);
	bool drawFill; 
	
	
	
	// UDP Communication
	ofxUDPManager textMessageConnection;
	ofxUDPManager triggerInConnection;
	ofxUDPManager triggerOutConnection;

	string UDPmessage;
	const char *smsIP;
	string smsTriggerIP;
	
			
	
	
	//Messages
	
	ofxFTGLFont shimmer;
	vector <string> messages;
	vector <int> messageSent;
    vector <float> messagePositions;
    float textSpeed;
	bool sendTrigger;
	
	/*
	//Box2d 
	ofPolyline			drawing;
	ofPoint				p;
	vector <ofPoint>	outside;
	float boun;
	float frict;
	float dampening;
	
	//Snow
	
	ofImage snow;
	ofTexture snowTex;
	float snowMax;
	float snowMin;
	float snowLeave;
	float snowReturn;
	int snowOpacityMin;
	*/
	
	// Background Texture
	ofImage bgImg;
	ofVideoPlayer bgVideo;
	ofVideoPlayer bgVideoEndRec;
	float posBgImg; 
	
	//Video 
	int 				camWidth;
	int 				camHeight;
	
	ofVideoGrabber 		vidGrabber;
	ofVideoPlayer		video;
	
	
	//OpenCV
	
	int	highBlob;
	int	lowBlob;
	int threshold;
	int diffMode;
	ofxCvGrayscaleImage cvBackground;
	ofxCvGrayscaleImage cvGray;
	ofxCvGrayscaleImage cvThresh;
	ofxCvColorImage		cvColor;
	ofxCvContourFinder 	contourFinder;
	ofxCvGrayscaleImage lastGray;
	ofxCvGrayscaleImage frameDiff;
	int blockSize;
	bool getBackground;
	
	
	//Recording
	Sequence bufferSequences[24];
	float endSpeed;
	float videoPos;
	float videoVel;
	float videoProgressBar;
	float videoGravity;
	bool newSequence;
	int record;
	int index;
	float flap;
	float lastFlap;
    float mappedFlap;
	float flapThresh;
	float downMult;
	float upMult;
	int playIndex;
	int lastPlayIndex;
	int endRecordSequenceTime;
	int endRecordSequenceDelay;
	bool endRecordSequence;
    bool bufferFull;
	bool bufferFullDuringShow;
	ofImage progressBar;
   
	    

	//Playback
	int showBoidsHead;
	int showBoidsTail;
	bool showBoids;
	bool removeLastBoid;
	float staticFlyingPush;
	float staticFlyingPull;
	unsigned char *  cutoutPixels;
	ofTexture cutoutTex;
	int nrDisplaySequences;
	int bufferSize;
	int play;
	int playbackIndex;  
	int sequenceIndex;
	int playSequenceIndex;
	int startIndex;
	int scale;
	float flapMagnitude;
	float scaleMagnitude; 
	int camHeightScale;
	int camWidthScale;
	

	//Path
	Path pth;
	
	//Settings
	ofxXmlSettings XML;
	string message;
	string xmlStructure;
	int lastTagNumber;
		
	
	//Flocking
	ofxFlocking flock;
	float setSeparation;
	float setAlignment;
	float setCohesion;
	float setMaxSpeed;
	float setForce;
	float setDesiredSeparation;
	int nrBoids;
	float setNeighbordist;
	float pushMaxSpeed;
	

//	int x;
//	int y;
//	float z;
//int start;

	
	//Show Managment
	int showState;
	int hold;
	int end;
	float pan;
	bool cvImgDisp;
	
	

		
	
};

#endif


