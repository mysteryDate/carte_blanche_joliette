/*
 *  Sequence.cpp
 *  CarteBlancheJolietteNewBoids
 *
 *  Created by Jonathan Chomko on 12-11-08.
 *  
 *
 */

#include "Sequence.h"
Sequence::Sequence(){
	
	dispTex.allocate(640,360,GL_RGBA);
	
}

float Sequence::playBack(int index, int scale){
	
	dispTex.loadData(pixels[index]);
	//1.779 is the ratio of the camera used at Joliette 2012/2013
	//This should change if a new camera is used
	dispTex.setAnchorPoint((scale*1.779)/2, scale/3);
	
	//Draw Image
	dispTex.draw(0, 0,scale *1.779, scale);
    
	//Return the flap value
	return(flaps[index]);

}


