/*
 *  Path.cpp
 *  Carte Blanche Joliette
 *
 *  Created by Jonathan Chomko on 12-11-01.
 *
 */

#include "Path.h"

Path::Path()
{

}


void Path::addPoint(float x, float y){

	ofPoint p;
	p.set(x,y);
	
	points.addVertex(p);
	
	
}

void Path::display(){
	
	ofPushMatrix();
	
	ofSetColor(255, 0, 0);
	
	points.draw();
	
	ofPopMatrix();
}


