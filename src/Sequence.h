/*
 *  Sequence.h
 *  CarteBlancheJolietteNewBoids
 *
 *  Created by Jonathan Chomko on 12-11-08.
 *
 */

#ifndef SEQUENCE
#define	 SEQUENCE

#include "ofMain.h"


class Sequence{
public:
	
	Sequence();
	float playBack(int index, int scale);
	ofPixels pixels[100];
	float flaps[100];
	ofTexture dispTex;
	

private:
};

#endif