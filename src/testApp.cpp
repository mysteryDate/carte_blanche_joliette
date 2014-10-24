#include "testApp.h"
#include <list>


//--------------------------------------------------------------
void testApp::setup(){
	
	
	//ofEnableSmoothing();	
	ofEnableAlphaBlending();  
	ofSetFrameRate(30);
	
	//ofSetDataPathRoot("./data/");
	//if we need to package up program
	
	//Syphon
	mClient.setup();
    mClient.setApplicationName("Simple Server");
    mClient.setServerName("");

	backgroundServer.setName("Background");
	//foregroundServer.setName("Foreground");
	textServer.setName("Text");
	
	backgroundTex.allocate(ofGetWidth(), ofGetHeight(),GL_RGBA);  
	//foregroundTex.allocate(ofGetWidth(), ofGetHeight()/4,GL_RGBA);
	textTex.allocate(ofGetWidth(),300, GL_RGBA);
	
	//UI
	smsTriggerIP = "hello hi";
	
	smsIP = "127.0.0.1";
	drawFill = true;
	float dim = 16;
	float xInit = OFX_UI_GLOBAL_WIDGET_SPACING; 
    float length = 320-xInit; 
	
	gui = new ofxUICanvas(0,0,length+xInit*2.0,ofGetHeight());   
	gui->addWidgetDown(new ofxUILabel("JOLIETTE INTERACTIVE", OFX_UI_FONT_LARGE)); 
	gui->addSpacer(length, 2); 
	
	gui->addWidgetDown(new ofxUILabel("BOID CONTROL", OFX_UI_FONT_MEDIUM));
	gui->addMinimalSlider("STATIC FLYING PUSH", 0, 50, setSeparation, 95, dim);
	gui->addMinimalSlider("STATIC FLYING PULL", 0, 10, setCohesion, 95, dim);
	gui->addMinimalSlider("MAXSPEED", 0, 5, setMaxSpeed, 95, dim);
	gui->addMinimalSlider("MAXFORCE", 0, 1, setForce, 95, dim);
    gui->addMinimalSlider("FLAP MAGNITUDE", 0, 1, flapMagnitude, 95, dim);
	gui->addMinimalSlider("LINE FOLLOW", 0, 200, setNeighbordist, 95, dim);
    
    gui->addWidgetDown(new ofxUILabel("VIDEO SETTINGS", OFX_UI_FONT_MEDIUM));
	gui->addMinimalSlider("VIDEO SCALE", 0, 640, scale, 95, dim);
	gui->addMinimalSlider("SCALE MAGNITUDE", 0, 300, scaleMagnitude, 95, dim);
	
    gui->addWidgetDown(new ofxUILabel(" CV SETTINGS ", OFX_UI_FONT_MEDIUM));
	gui->addMinimalSlider("THRESHOLD", 0, 100, threshold, 95, dim);
    
	gui->addSpacer(length, 2);
	gui->addWidgetDown(new ofxUILabel("RECORD FLYING", OFX_UI_FONT_MEDIUM));
	gui->addMinimalSlider("UP MULTIPLIER", 0,10, downMult, 95, dim);
	gui->addMinimalSlider("DOWN MULTIPLIER", 0, 1, upMult, 95, 0);
	gui->addMinimalSlider("FLAP THRESHOLD", 0, 1, flapThresh, 95, dim);
	gui->addMinimalSlider("END RECORD SEQUENCE DELAY", 0, 100 , endRecordSequenceDelay, 95, dim);
    gui->addMinimalSlider("END SPEED", 0, 2, endSpeed, 95, dim);

    gui->addSpacer(length, 2);
	gui->addWidgetDown(new ofxUILabel("TEXT", OFX_UI_FONT_MEDIUM));
	
	gui->addWidgetDown( new ofxUITextInput("UDP_TARGET_IP", "", 95, dim)); //("UDP_TARGET_IP",smsTriggerIP, 95,dim); //smsIP
	
	gui->addMinimalSlider("TEXT SPEED", 0, 10, textSpeed, 95, dim);
		
	ofAddListener(gui->newGUIEvent,this,&testApp::guiEvent);
	gui->loadSettings("GUI/guiSettings.xml"); 

	
	//UDP Communication
	
	textMessageConnection.Create();
	textMessageConnection.Bind(7303);
	textMessageConnection.SetNonBlocking(true);
	
	triggerInConnection.Create();
	triggerInConnection.Bind(7304);
	triggerInConnection.SetNonBlocking(true);
	
	triggerOutConnection.Create();
	triggerOutConnection.Connect("127.0.0.1",7305);   //"192.168.7.254" MAC pro IP in Joliette:
	triggerOutConnection.SetNonBlocking(true);
	
	
	// Messages
	shimmer.loadFont("shimmerbold_opentype.ttf", 30);
	sendTrigger = false;
		
	
	//Video
	camWidth 	= 640; 
	camHeight 	= 360; 
	
	vidGrabber.setVerbose(true);
	vidGrabber.listDevices();
	
	vidGrabber.setDeviceID(0);
	vidGrabber.initGrabber(camWidth,camHeight,true);
    
	camWidthScale = ofGetWidth();
	camHeightScale = camWidthScale *0.562; //should give proportions of 16:9

	cout << camHeightScale;
	cout << camWidthScale;
	
	// OpenCV
	getBackground = false; // true is auto, false is manual
	
	highBlob = 9999999;
	lowBlob = 100;
	
	diffMode  = 0;
	flap	  = 0;
    mappedFlap = 0;
    
	
	cvColor.allocate(camWidth,camHeight);
	cvGray.allocate(camWidth,camHeight);
	cvBackground.allocate(camWidth,camHeight);
	cvThresh.allocate(camWidth,camHeight);
	lastGray.allocate(camWidth,camHeight);
	frameDiff.allocate(camWidth,camHeight);
	
	
	
	
	// Recording
	
	playIndex = 0;
	lastPlayIndex = 0;
	
	nrDisplaySequences = 12;
	bufferSize = 24;
   
	cutoutTex.allocate(camWidth,camHeight,GL_RGBA);
	cutoutPixels = new unsigned char[camWidth*camHeight*4];
	record = 0;
	index = 0;  
	
	videoPos = 0;
	videoVel = 0;
	videoProgressBar = 100;
	
	endRecordSequence = false;
	endRecordSequenceTime = 0;
	
	bufferFull = false;
    bufferFullDuringShow = false;
  	
	//Playback
	play = 0;
	playbackIndex = 0;
	playSequenceIndex = 0;
	sequenceIndex = 0;
	startIndex = 0;
	
	gui->toggleVisible();
	
	
	//XML Settings
	message = "loading mySettings.xml";
	
	if (XML.loadFile("mySettings.xml")) {
		message = "mySettings.xml loaded";
	}else{
		message = "unable to load mySettings.xml check data/ folder";
	}
		int numDragTags = XML.getNumTags("STROKE:PT");
		
		if(numDragTags > 0 ){
		
			XML.pushTag("STROKE", numDragTags-1);
			
			int numPtTags = XML.getNumTags("PT");
			
			if (numPtTags > 0) {
				
				int totalToRead = numPtTags;
				
				for (int i = 0; i < totalToRead; i ++) {
					
					int x = XML.getValue("PT:X", 0,i);
					int y = XML.getValue("PT:Y", 0,i);
					pth.addPoint(x, y);
					
				}
				
			}
			
			XML.popTag();
		
		}
	
	
	//Debug 
	cvImgDisp = false;

	
	// Show Management
	end = 0;
	pan = 0;
	showState = 0;
	hold = 0;
	
    
	//Flock
	
	for(int i = 0; i < bufferSize; i ++) {
		flock.addBoid(ofRandom(-100, -500),ofRandom(100,2*ofGetHeight()/3));
	}
    
	showBoidsHead = 0;
	showBoidsTail = 0;
	showBoids = false;
	removeLastBoid = false;
    
	// LOAD BACKGROUNS IMAGE
	//bgImg.loadImage("imgBg/bgParallax-1.png");
	bgImg.loadImage("imgBg/bgCabine.jpg");
	progressBar.loadImage("progressBarCandy.png");
	
    // LOAD BACKGROUND VIDEO
	//bgVideo.loadMovie("videoBg/bg_cabanRotate.mov");
	//bgVideoEndRec.loadMovie("videoBg/blue-sparks.mov");
    
    posBgImg = -960;
}


//--------------------------------------------------------------
void testApp::update(){
	 
	//bgVideo.update();
	//bgVideoEndRec.update();
    
    
    //Check for new text message
	
	char textMessage[100000];
	textMessageConnection.Receive(textMessage, 100000);
	
	if (textMessage[0] != 0) {
        
			string grt = textMessage;
            messages.push_back(grt);
		    messageSent.push_back(0);
            messagePositions.push_back(0);
			
	}
	
	//Check for new trigger message
	char triggerInMessage[10];
	triggerInConnection.Receive(triggerInMessage, 10);
	int tm;
	
	tm = atoi(triggerInMessage);  //Char to Int

	if (tm == 1 || tm == 3 || tm == 4 || tm == 5) { //Show State Trigger
		//showState += 1;
	    showState = tm;
     }
	
    
	if (tm == 6){  //Record Trigger
		record = 1;
		
	}
    
	if (tm == 7){ 
        getBackground = true;
    }
	
    
	
	//Show management
	if (showState > 4) {  // Check at beginning of update because multiple functions use this
		showState = 0;
	}
		
	
	if (showState == 0 ) {
		
		hold = 0;   
		end = 0;	
		play = 0;
		
		messages.clear();
        messagePositions.clear();
		messageSent.clear();
		
    }
    
    
	//Reset show variables
	if (showState == 1) {
        
        
        
        messages.clear();
        messagePositions.clear();
        
        for(int i = 0; i < flock.boids.size(); i ++){
			
			flock.boids[i].setLoc(ofVec2f(ofRandom(-100, -700),ofRandom(200,700)));
			
		}
		
		if (showBoidsHead > nrDisplaySequences) {
		
			showBoidsTail = showBoidsHead - nrDisplaySequences;
		
		}
		
		showBoids = true;
		
		showState += 1;          
		
	
	}
	
	//Bring flock of video onscreen
	if (showState == 2) {  
		
		play = 1;
		hold=1;
		
		//setNeighbordist = 0;
		
		
		if (pan < 0) {
			pan -= pan/100;
		}
		
        
	}
	
	//Flock flies around
	if (showState == 3) {  
		
        hold = 1;
        //setNeighbordist = 21.05;
	}
	
	//Flock leaves screen
	if (showState == 4) {  
		
        if(pan < ofGetWidth()){
            pan += pan/100;
        }
        
        if (pan > ofGetWidth()) {
            
            showState+= 1;
            
        }
        
        showBoids = false;
        
	}
	
	updateVideo();
	
}


void testApp::updateVideo(){
		
	
	vidGrabber.update();	
	
	
	if (vidGrabber.isFrameNew()){
		
		
		playIndex++;
		
		cvColor.setFromPixels(vidGrabber.getPixels(), camWidth, camHeight);
		cvGray = cvColor;
		
		
		//Grab background after ten frames
		if (getBackground == true && ofGetFrameNum() > 10) {
			
			cvBackground = cvGray;
			getBackground = false;
			
		}
		
		//Background subtraction options
		if(diffMode == 0){
			cvThresh = cvGray;
			cvThresh.absDiff(cvBackground);
		}else if(diffMode == 1){
			cvThresh = cvGray;
			cvThresh -= cvBackground;
		}else if(diffMode == 2){
			cvThresh = cvBackground;
			cvThresh -= cvGray;
		}
		
		cvThresh.threshold(threshold);
        
		frameDiff.absDiff(cvGray, lastGray);
		
		contourFinder.findContours(cvThresh, lowBlob, highBlob, 1, true, true);
		
        //Blurring the threshold image before using it as a cutout softens edges
        cvThresh.blur();
       
		
		//Removing Background
		unsigned char * colorPix = cvColor.getPixels();
		unsigned char * grayPix = cvThresh.getPixels();
		unsigned char * frameDiffPix = frameDiff.getPixels();
		
		
		for(int i = 0; i < (camWidth*camHeight); i++){
			
			//If pixel has content
			if(grayPix[i] > 1){ 
				
				cutoutPixels[(i*4)+0] = colorPix[(i*3)+0];
				cutoutPixels[(i*4)+1] = colorPix[(i*3)+1];
				cutoutPixels[(i*4)+2] = colorPix[(i*3)+2];
				cutoutPixels[(i*4)+3] = 255;
				
			}else
			//If no content, set pixel to be translucent
				if(grayPix[i]  < 1){
					
					cutoutPixels[(i*4)+0] =	0;
					cutoutPixels[(i*4)+1] = 0;
					cutoutPixels[(i*4)+2] = 0;
					cutoutPixels[(i*4)+3] = 0;
					
				}
			
		}
        
	    
		
		
		//Check to see if all 24 spots are full
		if(showBoids == true && showBoidsHead-showBoidsTail >= bufferSize){
			record = 0;
			bufferFull = true;
		}else {
			bufferFull= false;
		}
        
		
		//Record a sequence
       if (record == 1 ) 
		{
			//Accessing the buffer directly was the quickest method I could find
		    bufferSequences[showBoidsHead%bufferSize].pixels[index].setFromPixels(cutoutPixels, camWidth, camHeight, 4);
			bufferSequences[showBoidsHead%bufferSize].flaps[index] = mappedFlap;
			index ++;
			
			//Size of a Sequence is hard-coded here and in Sequence.h
			if (index == 100) 
			{	
				
				index = 0;
				record = 0;
				play = 1;
				
				//Increment display int
				showBoidsHead ++;
				
				//Put that boid offscreen
				
				// ligne bug
                flock.boids[showBoidsHead%bufferSize].setLoc(ofVec2f(ofRandom(-100, -400),ofRandom(100,2*ofGetHeight()/3)));
				
				
				//Start end record sequence
				endRecordSequence = true;
				endRecordSequenceTime = ofGetElapsedTimeMillis();
								
					
			}
			
		}//End Record	
		
		//Load background subtracted image for displaying 
		cutoutTex.loadData(cutoutPixels,camWidth,camHeight,GL_RGBA); 
        
		
	}//End of New Frame Check
	
	
	//Automatic Recording original
	
	if (videoPos > 200 && !endRecordSequence ) {
		record = 1;
	}
	
	/*
	if (videoProgressBar == 0 && !endRecordSequence ) {
		record = 1;
	}
	*/
	//If the sequence is done recording
	if (endRecordSequence) {
		//Pull the image offscreen
		if (videoPos < ofGetHeight()) {
			videoVel += endSpeed;
		}
		
		//Once the allotted time as passed, reset the values & stop showing the text
		if (ofGetElapsedTimeMillis() - endRecordSequenceTime >= endRecordSequenceDelay*1000) {
			endRecordSequence = false;
			videoPos = 0;
			videoVel = 0;
		}
		
	}
	
	//Getting Contour Height Data (Flap)
	if (contourFinder.nBlobs > 0) {
		
		flap = contourFinder.blobs[0].boundingRect.height;
		mappedFlap = ofMap(flap, 0, camHeight, 0, 1);
		
	}
	
	
	
	//If a user as raised their arms, and lowered them
	if (mappedFlap < lastFlap-flapThresh) { //+0.01
		//Set velocity to flap value
		videoVel = mappedFlap*upMult;
		
	}
	
	//Add velocity to positon
	videoPos += videoVel;
	
	
	//If the video position is above zero
	if(videoPos > 0) { 
		//Add gravity
		videoVel -=  downMult;
	}
	
	//If we're below the ground
	if(videoPos <= 0) {
		//Set postion to 0
		videoVel  = 0;
	
	}
	
	//Get last flap to compare with new flap
	lastFlap = mappedFlap;
	
	
	mClient.draw(50, 50);    
	
	

}
//--------------------------------------------------------------
void testApp::draw(){
	
	
	ofClear(255, 255, 255, 0);
	ofSetColor(255, 255, 255,255);
	
	//Draw Flying Videos
	drawBoids();
	
	//Clear Background
	ofClear(255, 255, 255, 0); //Tranparent Background)
	ofBackground(255, 255, 255, 0); //Tranparent Background
	
	//Text 
	ofFill();
	
    if (cvImgDisp) {
         ofSetColor(255, 0, 0); // RED FOR DEBUG
    }else{
        ofSetColor(255, 255, 255,255); // White for display color
    }
   
	drawText();
	
	
	ofSetColor(210, 229, 247, 255);  //BACKGROUND FOR FLYERS
	ofRect(0,0, ofGetWidth(), ofGetHeight()); 
	
    //bgImg.draw(0,0); // TEXTURE FOR FLYERS
	// OR VIDEO TEXTURE
	
	
	//bgVideo.play();
	
	
	/*
	bgVideo.draw(0,0,ofGetWidth(), ofGetHeight());
	*/
	
	
	// MULTIPLE SEQUENCE
	/*
	if(endRecordSequence == true){
		
		bgVideoEndRec.play();
		bgVideoEndRec.draw(0,0,ofGetWidth(), ofGetHeight());
		bgVideo.stop();
	
	}else {
		
		bgVideo.play();
		bgVideo.draw(0,0,ofGetWidth(), ofGetHeight());
		bgVideoEndRec.stop();
	}
	*/
	
	
		if(posBgImg >= 0 && endRecordSequence){
			posBgImg = 0;
		}else{
			posBgImg = videoPos*1-960;
		} 
	
	
	bgImg.draw(posBgImg,0 , 1920, ofGetHeight()); // TEXTURE FOR FLYERS
	
	ofSetColor(255, 255, 255,255);
	////COVER TEXT
	
	
	//Cutout
	
    ofSetColor(255, 255, 255,255); //ofSetColor can change the tint of an image, this resets is
	cutoutTex.draw((0-videoPos*1.5)+200,(ofGetHeight()/2)-(camHeightScale/2) , camWidthScale, camHeightScale);  // correct proportions
	
    
    
        
	//Show the 'End Record Sequence'
	if (endRecordSequence == true) {
		
		string goodbye = "Tu voles maintenant sur la Carte Blanche!";
		ofPushStyle();
		//ofSetColor(28, 20, 255);
		ofPushMatrix();
		
		ofTranslate(900, (ofGetHeight()/2)+350);
		ofRotateZ(-90);
		
		ofSetColor(255, 255, 255);
		shimmer.drawString(goodbye, 0, 0);
		
		ofSetColor(28, 20, 255);
		shimmer.drawString(goodbye, 1, 1);
		
		ofPopMatrix();
		ofPopStyle();
		
	}
    
	//Tell people that the buffer is full if that is true
    if (bufferFull && !endRecordSequence) {
        
        string full = "La carte est pleine! L'enregistrement reprendra lors du prochain spectacles";
		ofPushStyle();
		ofSetColor(28, 20, 255);
		
		ofPushMatrix();
		ofTranslate(900, (ofGetHeight()/2)+480);
		ofRotateZ(-90);
		shimmer.drawString(full, 0, 0);
		ofPopMatrix();
		ofPopStyle();
        
    }
	
	//Progress bar for recording
	//ofSetColor(255-(100-index)*2,0,0,255);
	
	//ofSetColor(0, 229, 247, 255); 
	
	
	
	if(endRecordSequence == false){
		
		ofSetColor(255, 0, 0, 50); 
		ofRect(800,(ofGetHeight()/2), 15, 200); 
		ofRect(800,(ofGetHeight()/2), 15, -200);
		
		ofSetColor(255, 0, 0, 255); 
		/*
		ofRect(800,(ofGetHeight()/2), 15, (100-index)*2); 
		ofRect(800,(ofGetHeight()/2), 15, -(100-index)*2);
		*/
		
		
		//videoProgessBar = videoProgressBar - videoPos;
		/*
		if(videoProgressBar ==  0){
			endRecordSequence =true;
		}else {
			endRecordSequence=false;
		}
        */
		
		
		ofRect(800,(ofGetHeight()/2), 15, (100-videoPos/4)*2); 
		ofRect(800,(ofGetHeight()/2), 15, -(100-videoPos/4)*2);
		
		// test circle progress
		// ofCircle((ofGetHeight()/2),(150+index)*2,20);
		
		ofSetColor(255, 255, 255, 255); 
		
		//posBgImg = -1280;
		
		/*
		progressBar.draw(800,(ofGetHeight()/2), 20, 200);
		progressBar.draw(800,(ofGetHeight()/2), 20, -200); 
		*/
		
		/*
		progressBar.draw(800,(ofGetHeight()/2), 20, (100-index)*2);
		progressBar.draw(800,(ofGetHeight()/2), 20, -(100-index)*2); 
		*/
		
		ofSetColor(210, 229, 247, 120); 
		ofNoFill();
		ofRect(794,(ofGetHeight()/2)-205,25,410);
		
		
	}else{
		ofRect(800,0, 15, 0); 
		ofRect(800,0, 15, 0);
		progressBar.draw(800,0, 15, 0);
		progressBar.draw(800,0, 15, 0);

	}
	
	
	
	
	/* OLD MECANISM FOR THE PROGRESS BAR
	ofSetColor(255-(100-index)*2,0,0,255);
	ofRect(800,(ofGetHeight()/2), 20, (100-index)*2); 
	ofRect(800,(ofGetHeight()/2), 20, -(100-index)*2);
	*/
	
	//Debug Functions
	// shimmer.drawString(ofToString(index), ofGetWidth()-50, ofGetHeight()-50);
	
	ofSetColor(0,0,0, 120);
	//shimmer.drawString(ofToString(showState), ofGetWidth()-50, ofGetHeight()-50);
	//shimmer.drawString(ofToString(posBgImg), ofGetWidth()-150, ofGetHeight()-100);
	
	//shimmer.drawString(ofToString(lowBlob), ofGetWidth()-300, ofGetHeight()-100);
	//shimmer.drawString(ofToString(highBlob), ofGetWidth()-300, ofGetHeight()-150);
	
	
	/*
	shimmer.drawString(ofToString(showBoidsHead-showBoidsTail), ofGetWidth()-180, 50);
	shimmer.drawString("de" , ofGetWidth()-120, 50);
	shimmer.drawString(ofToString(bufferSize), ofGetWidth()-70, 50);
	shimmer.drawString(ofToString(posBgImg), ofGetWidth()-200, 100);
	*/

	//shimmer.drawString(ofToString(videoPos), ofGetWidth()-70, 50);
	//shimmer.drawString(ofToString(highBlob), ofGetWidth()-200, 100);
	
	if (cvImgDisp) {
		cvImages();
	}
    
	//get the last playIndex before the program loops back 
    lastPlayIndex = playIndex; 
	
	 
}



void testApp::drawText(){
    
		//only advance text when there is a new frame
		//keeps the text flowing smoothly
     if (playIndex > lastPlayIndex) 
		{ 
		 
		for(int i = 0; i < messages.size(); i ++)
			{
        
			messagePositions[i] += textSpeed; 
			
			//Get the actual width of the string
			float w = shimmer.stringWidth(messages[i]);
				
			//If the string has left the edge of the screen,
		    //and no request for a new message has been sent
			if (ofGetWidth() - messagePositions[i] + w < ofGetWidth() && messageSent[i] == 0){
					//Trigger the UDP message send
					sendTrigger = true;
					//Only send once
					messageSent[i] = 1;
				}
			//If the message is offscreen
			if (ofGetWidth() - messagePositions[i] + w < 0){
					//Erase that message and its variables
					messages.erase(messages.begin()+i);
					messagePositions.erase(messagePositions.begin()+i);
					messageSent.erase(messageSent.begin()+i);
				}
			}
		}
	
	//Send a 5 to get a new message from the MAX patch
	if (sendTrigger == true)
		{
		int sig = 5;
		char tmp[32];    
		memset(tmp,0,32);
		sprintf(tmp, "%i",sig );
		int sent = triggerOutConnection.Send(tmp, strlen(tmp));
		cout << sent;
		cout << sig;
		sendTrigger = false;
		}
		 
	//Draw messages 
	for(int i = 0; i < messages.size(); i ++)
		{
		shimmer.drawString(messages[i], ofGetWidth() - messagePositions[i], 100);
		}
	
	//Grab an area of the screen & publish on syphon
	textTex.loadScreenData(0, 0, 1280, 300);
	textServer.publishTexture(&textTex);
}


void testApp::drawBoids(){
	
	//Set background to be transparent
	ofBackground(255, 255, 255, 0); 
	
	//Show Debug?
	if (cvImgDisp) {  
		pth.display();
	}
	
	//If the number of displayed boids is greater than the desired number
	if ( showBoidsHead-showBoidsTail > nrDisplaySequences ){
		removeLastBoid = true;
	}
	
	//Color affects image tint, set to full opacity
	ofSetColor(255, 255, 255, 255);  
	
	//Variables for boid displaying
	ofPoint pathPoint;
	ofVec3f loc;
	
	float zScale;
	float lFlap;
    float lLastFlap;
	
	//Boid display boolean
    if(showBoids)
	{
        
	for (int j = showBoidsTail; j < showBoidsHead; j ++)
		{
            
			//Loop i around the size of the buffer using modulo
            int i = j % bufferSize;
			
			/*
			//This function can be removed once the preferred values are set
			flock.boids[i].updateValues(setSeparation,setAlignment,setCohesion,
										setForce*0.001,setMaxSpeed,setDesiredSeparation,setNeighbordist);
            */
			
			
			
			if(showState==2){
				flock.boids[i].updateValues(setSeparation,10,setCohesion,
											setForce*0.001,setMaxSpeed-1.40,10, 15.26);
			} else {
				
				//This function can be removed once the preferred values are set
				flock.boids[i].updateValues(setSeparation,setAlignment,setCohesion,
											setForce*0.001,setMaxSpeed,setDesiredSeparation, 200);
			
			}
			
			
			
			
			flock.boids[i].update(flock.boids,hold);
			 
			
			//Get boids location
			loc.set(flock.boids[i].getLoc());
			
		
						
			if(hold == 1)
			{ 
				
				
				//If the boids are not flying in
				//Get the boid's predicted location
				ofVec2f pl(flock.boids[i].getPredictLoc());
				//Get the point on the path closest to that predicted location
				pathPoint = pth.points.getClosestPoint(pl);
				//Seek that point
				flock.boids[i].seek(pathPoint);
			}
			
		
		
	
		
		//Draw the Sequence 
        ofPushMatrix();
		
		//Get the dynamic scale of the boids
		//This perspective effet could be replaced by having 3d boids
		zScale = flock.boids[i].getScale();
		//Multiply this value by the scaleMagnitude slider value
		zScale *= scaleMagnitude;

		//Set coordinates
		ofTranslate(loc.x+pan, loc.y);  	

			
		//Add 90 degrees of rotation to account for the camera's orientation
		//this may need to change depending on where the camera is placed in future
		ofRotate(loc.z+90); 
		
		//Draw the desired sequence, and retrieve the flap data
		lFlap = bufferSequences[(i) % bufferSize].playBack(playbackIndex, scale+zScale);
        
        //Add flap data to that boid
			//Can also be done with a threshold
		flock.boids[i].push(lFlap*flapMagnitude);
		
		
		ofPopMatrix();
			
		//If we/re looking to remove the last boid the last boid goes offscreen
            if(removeLastBoid &&
               flock.boids[showBoidsTail%bufferSize].getLoc().x  > ofGetWidth()-100)
			{
				//Increase the show buffer
				showBoidsTail ++;
				//Stop removing the last boid
                removeLastBoid = false;
                
            }
		} 
    }
	
	
	if(play == 1) {
		
		//Check for new frame and increase index if frame is new
		if(playIndex > lastPlayIndex){ 
			
			// Dont go out of bounds
			if(playbackIndex == 99){   
				playbackIndex = 0;
			}else {
				playbackIndex++;
			}
		}
	}
	
	//Load screen data
	backgroundTex.loadScreenData(0, 0, ofGetWidth(),ofGetHeight());
	//Publish screen data to syphon 
	backgroundServer.publishTexture(&backgroundTex);
	
	
	
}


void testApp::guiEvent(ofxUIEventArgs &e){
	
	//Remove some of these
	
	string name = e.widget->getName();
	int kind = e.widget ->getKind();
    
   
	//Boid Control
	
	if( name == "STATIC FLYING PUSH"){
		
		ofxUISlider *slider = (ofxUISlider *) e.widget;
		setSeparation = slider -> getScaledValue();
	}
	else if( name == "STATIC FLYING PULL"){
		
		ofxUISlider *slider = (ofxUISlider *) e.widget;
		setCohesion = slider -> getScaledValue();
	}
	else if( name == "MAXSPEED"){
		
		ofxUISlider *slider = (ofxUISlider *) e.widget;
		setMaxSpeed = slider -> getScaledValue();
	}
	else if( name == "MAXFORCE"){
		
		ofxUISlider *slider = (ofxUISlider *) e.widget;
		setForce = slider -> getScaledValue();
	}
	else if( name == "FLAP MAGNITUDE"){
		
		ofxUISlider *slider = (ofxUISlider *) e.widget;
		flapMagnitude = slider -> getScaledValue();
	}
	else if( name == "LINE FOLLOW"){
		
		ofxUISlider *slider = (ofxUISlider *) e.widget;
		setNeighbordist = slider -> getScaledValue();
	}
	
	//Video Settings
	else if( name == "VIDEO SCALE"){
		
		ofxUISlider *slider = (ofxUISlider *) e.widget;
		scale = slider -> getScaledValue();
	}
	else if( name == "SCALE MAGNITUDE"){
		
		ofxUISlider *slider = (ofxUISlider *) e.widget;
		scaleMagnitude = slider -> getScaledValue();
	}
	
	//CV Settings
	else if( name == "THRESHOLD"){
		
		ofxUISlider *slider = (ofxUISlider *) e.widget;
		threshold = slider -> getScaledValue();
	}
	
	//Record Flying
	else if(name == "UP MULTIPLIER" ){
		
		ofxUISlider *slider = (ofxUISlider *) e.widget;
		upMult = slider->getScaledValue();
	}
	else if(name == "DOWN MULTIPLIER" ){
		
		ofxUISlider *slider = (ofxUISlider *) e.widget;
		downMult = slider->getScaledValue();
	}
	else if(name == "FLAP THRESHOLD" ){
		
		ofxUISlider *slider = (ofxUISlider *) e.widget;
		flapThresh = slider->getScaledValue();
	}
	
	else if(name == "END RECORD SEQUENCE DELAY" ){
		
		ofxUISlider *slider = (ofxUISlider *) e.widget;
		endRecordSequenceDelay = slider->getScaledValue();
	}
	
	else if(name == "END SPEED"){
		
		ofxUISlider *slider = (ofxUISlider *) e.widget;
		endSpeed =slider -> getScaledValue();
		
	}
	
	//Text Speed
	else if(name == "TEXT SPEED" ){
		
		ofxUISlider *slider = (ofxUISlider *) e.widget;
		textSpeed = slider->getScaledValue();
	
	}else if(name == "UDP_TARGET_IP"){
		ofxUITextInput *text = (ofxUITextInput *) e.widget;
		smsIP = text->getTextString().c_str();
		//cout << smsIP;
	}
		
	
			
}

//Used as a debug function
void testApp::cvImages()
{
	camWidth  = camWidth/2;
	camHeight = camHeight/2;
	
	vidGrabber.draw(100,  100, camWidth, camHeight);
	cvColor.draw(100+camWidth, 100, camWidth, camHeight);
	cvGray.draw(100, 100+camHeight, camWidth, camHeight);
	cvBackground.draw(100+camWidth, 100+camHeight, camWidth, camHeight);
	cvThresh.draw(100, 100+camHeight*2, camWidth, camHeight);
	
	camWidth = camWidth*2;
	camHeight = camHeight*2;
	
	ofFill();
	ofSetColor(255, 255, 255,255);
	
	string info = "";
		
	info += "number of boids displayed : " + ofToString(showBoidsHead-showBoidsTail) + "\n";
	info += "showState: " + ofToString(showState) + "\n";
	
	info += "showBoidsHead : " + ofToString(showBoidsHead) + "\n";
	info += "showBoidsTail : " + ofToString(showBoidsTail) + "\n";
	
	info += "showBoidsHead % bufferSize " +ofToString(showBoidsHead % bufferSize) + "\n";
	info += "showBoidsTail % bufferSize: " +ofToString(showBoidsTail % bufferSize) + "\n";
	
	info += "Number of Messages : " +ofToString(messages.size()) + "\n";
	
	info += "Record: " +ofToString(record) + "\n";
	info += "Index: " + ofToString(index) + "\n";
	
	info += message +"\n";
	
	ofSetHexColor(0x444342);
	ofDrawBitmapString(info, 30, 30);
	
	ofSetColor(255, 0,0);
	ofLine(ofGetWidth()-100, 0, ofGetWidth()-100, ofGetHeight());
    pth.display();
	
}

//--------------------------------------------------------------
void testApp::keyPressed  (int key){
	
	// in fullscreen mode, on a pc at least, the
	// first time video settings the come up
	// they come up *under* the fullscreen window
	// use alt-tab to navigate to the settings
	// window. we are working on a fix for this...
	
	if (key == 'f' || key == 'F') {
		ofToggleFullscreen();
	}
	
	//Grab Background
	if(key == 'b' || key == 'B'){
		getBackground = true;
	}
	
	//Show debug 
	if(key == 'u' || key == 'U'){
		if(cvImgDisp){
			cvImgDisp = false;
		}else {
			cvImgDisp = true;
		}
	}
    
	//Clear path
    if (key == 'x' || key == 'X') {
		pth.points.clear();
		XML.clear();
		lastTagNumber	= XML.addTag("STROKE");
		xmlStructure	= "<STROKE>\n";
	}
	
	
	if(key == 'g' || key == 'G'){
		gui->toggleVisible();
        
	}
	
    //Increase showstate
	if (key == 's') { 
		showState += 1;
	}
	if (key == 'S') { 
		showState -= 1;
	}
	
	
	//Testing Text Display
	/*
	if (key == 'd'){
	
		string text = "Ca c'èst un meéssagçe de test";
         messages.push_back(text);
         messagePositions.push_back(0);
		messageSent.push_back(0);
        
        		
	}
	if (key == 'v') {
		
		string text = " a,b, c,d,e,f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z, A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z";
		messages.push_back(text);
        messagePositions.push_back(0);
		messageSent.push_back(0);
	}
	
	if (key == 'k'){
		
		string text = ",, ., :,  , !, @, #, $, %, ?, &, (, ), -, +, ;, 0, 1,2, 3, 4, 5, 6, 7, 8, 9, é, É, è, È, à, À, ç, Ç, ê, Ê, ë , Ë, ù, û, Ù, Û, =, ô, Ô";
        messages.push_back(text);
        messagePositions.push_back(0);
        messageSent.push_back(0);
		
	}
	if (key == 'n') {
		
        
		
       string text = "Bonjour tout le monde, hôtel, môtel, hôliday inn. À demain. Être en santè. Ça va. ";
        messages.push_back(text);
        messagePositions.push_back(0);
		messageSent.push_back(0);
        
    
    }
    
	if (key == 'y') {
		string text = " Joyeux Noël charmante Joliette! ❤";
        
        messages.push_back(text);
        messagePositions.push_back(0);
		messageSent.push_back(0);
	}
	*/
    
	if(key == 'p'){
		vidGrabber.videoSettings();
	}
	
	if(key =='1'){
		diffMode = 0;
	}else if(key =='2'){
		diffMode = 1;
	}else if(key == '3'){
		diffMode = 2;
	}
    
    if(key == OF_KEY_RIGHT){
		highBlob += 10000;
	}else if(key == OF_KEY_LEFT){
		highBlob -= 10000;
	}
	
	if(key == '='){
		lowBlob += 10000;
	}else if(key == '-'){
		lowBlob -= 10000;
	}
	
	if(key == OF_KEY_UP){
		threshold += 2;
	}else if(key == OF_KEY_DOWN){
		threshold -= 2;
		if(threshold < 0)threshold = 0;
	}
     
	if(key == 'r'){
		 
     record = 1;
          
	}
	

	
	
	
	 
}


//--------------------------------------------------------------
void testApp::keyReleased(int key){
	
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
	
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

		
	if(button == 2){
	
		//This mapping allows for offscreen drawing, which enables removing of the boids
		int mx = (int)ofMap(x,0, ofGetWidth(),-100,  ofGetWidth()+100);
		int my = (int)ofMap(y,0, ofGetHeight(),   0, ofGetHeight());
		
		pth.addPoint(mx, my);
		
	if (XML.pushTag("STROKE", lastTagNumber)) {
		
		int tagNum = XML.addTag("PT");
		XML.setValue("PT:X", mx, tagNum);
		XML.setValue("PT:Y", my, tagNum);
		XML.popTag();
		
		}
	}
	
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
	
	if(button == 1){
		flock.addBoid(x,y);
	}
	
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){
	
						
}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){
	
}

void testApp::exit(){
	
	mClient.unbind();
	XML.saveFile("mySettings.xml");
	gui->saveSettings("GUI/guiSettings.xml");
    delete gui; 
	

}


