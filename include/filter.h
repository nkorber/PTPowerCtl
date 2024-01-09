/*
 * filter.h
 *
 *  Created on: 27 août 2011
 *      Author: nk
 */

#ifndef FILTER_H_
#define FILTER_H_

#include <inttypes.h>	//uint8_t

class BWFilter_LP_2_05_50 {
protected:
	short int mIn[3];
	long int mOut[2];
public:
	BWFilter_LP_2_05_50 (void);
	virtual short int execute (const short int in);
	virtual short int getIn(uint8_t index);
	virtual long int getOut(uint8_t index);
	virtual short int getsOut(uint8_t index);
	virtual short int result(void);


};

#endif /* FILTER_H_ */
