/*
 * djpeg.h
 *
 *  Created on: 2012/12/25
 *      Author: masayuki
 */

#ifndef DJPEG_H_
#define DJPEG_H_

#define DJPEG_PLAY        (1 << 2)
#define DJPEG_ARROW_LEFT  (1 << 1)
#define DJPEG_ARROW_RIGHT (1 << 0)


extern int dojpeg(int id, uint8_t djpeg_arrows, uint8_t arrow_clicked);


#endif /* DJPEG_H_ */
