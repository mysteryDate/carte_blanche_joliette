/*
 *  Path.h
 *  CarteBlancheJoliette
 *
 *  Created by Jonathan Chomko on 12-11-01.
 *  
 *
 */

#ifndef _PATH
#define _PATH

#include "ofMain.h"
 
class Path{
public:
	Path();
	
	ofPolyline points;
	float radius;
	
	void addPoint(float x, float y);
	void display();
	
private:
};

#endif