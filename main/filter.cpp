/*
 * filter.cpp
 *
 *  Created on: 27 ao�t 2011
 *      Author: nk
 *
 *      2022/04/10 NK Added method result()
 */

#include "filter.h"

BWFilter_LP_2_05_50::BWFilter_LP_2_05_50 (void){
	mIn[0] = 0;
	mIn[1] = 0;
	mIn[2] = 0;
	mOut[0] = 0;
	mOut[1] = 0;
}

BWFilter_LP_2_05_50::~BWFilter_LP_2_05_50(){

}

short int BWFilter_LP_2_05_50::execute (short int in) {
	// Shift mIn along with compute (256*In[2]+512*In[1]+256*In[0]...
	long int temp = (mIn[2] = mIn[1]);
	temp += (mIn[1] = mIn[0]);
	temp += mIn[1];
	temp += (mIn[0] = in);
	temp <<= 8;
	// Compute ... -(937*Out[1]-1957*Out[0]))/2^10 along with shift mOut
	temp -= mOut[1]*937;
	temp += (mOut[1] = mOut[0])*1957;
	temp >>= 10;
	mOut[0] = temp;
	return result();
//	// Output with rounding
//	short int out = (temp + 128)>>8;
//	return(out > 0 ? out : 0);
}

short int BWFilter_LP_2_05_50::getIn(uint8_t index){
	return mIn[index];
}
short int BWFilter_LP_2_05_50::getsOut(uint8_t index){
	return ((short int *)mOut)[index];
}

long int BWFilter_LP_2_05_50::getOut(uint8_t index){
	return mOut[index];
}

short int BWFilter_LP_2_05_50::result(void){
	long int temp = mOut[0];
	// Output with rounding
	short int out = (temp + 128)>>8;
	return(out > 0 ? out : 0);
}
