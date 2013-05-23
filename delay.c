/*
 * delay.c
 *
 *  Created on: 2011/10/31
 *      Author: Tonsuke
 */


//=== Delay ms time define ===//
void Delayms(int i)
{
	volatile int j,k;
	for(j = 0;j < i;j++)
		for(k = 0;k < 200;k++);
}

void Delay(int i)
{
	volatile int j,k;
	for(j = 0;j < i;j++)
		for(k = 0;k < 8900;k++);
}
