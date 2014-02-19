/*
 * icon.c
 *
 *  Created on: 2013/03/11
 *      Author: Tonsuke
 */


#include "icon.h"
#include "lcd.h"
#include "sound.h"

void Update_Navigation_Loop_Icon(uint8_t index)
{
	LCDPutBuffToBgImg(_drawBuff->navigation_loop.x, _drawBuff->navigation_loop.y, \
			_drawBuff->navigation_loop.width, _drawBuff->navigation_loop.height, _drawBuff->navigation_loop.p);

	switch(index){
	case NAV_ONE_PLAY_EXIT: // 1 play exit
		LCDPutIcon(_drawBuff->navigation_loop.x, _drawBuff->navigation_loop.y, _drawBuff->navigation_loop.width, _drawBuff->navigation_loop.height, \
				navigation_bar_24x18, navigation_bar_24x18_alpha);
		break;
	case NAV_PLAY_ENTIRE: // play entire in directry
		LCDPutIcon(_drawBuff->navigation_loop.x, _drawBuff->navigation_loop.y, _drawBuff->navigation_loop.width, _drawBuff->navigation_loop.height, \
				navigation_entire_loop_24x18, navigation_entire_loop_24x18_alpha);
		break;
	case NAV_INFINITE_PLAY_ENTIRE: // infinite play entire in directry
		LCDPutIcon(_drawBuff->navigation_loop.x, _drawBuff->navigation_loop.y, _drawBuff->navigation_loop.width, _drawBuff->navigation_loop.height, \
				navigation_infinite_entire_loop_24x18, navigation_infinite_entire_loop_24x18_alpha);
		break;
	case NAV_INFINITE_ONE_PLAY: // infinite 1 play
		LCDPutIcon(_drawBuff->navigation_loop.x, _drawBuff->navigation_loop.y, _drawBuff->navigation_loop.width, _drawBuff->navigation_loop.height, \
				navigation_one_loop_24x18, navigation_one_loop_24x18_alpha);
		break;
	case NAV_SHUFFLE_PLAY: // shuffle
		LCDPutIcon(_drawBuff->navigation_loop.x, _drawBuff->navigation_loop.y, _drawBuff->navigation_loop.width, _drawBuff->navigation_loop.height, \
				navigation_shuffle_24x18, navigation_shuffle_24x18_alpha);
		break;
	default:
		break;
	}

}

void Update_Bass_Boost_Icon(uint8_t index)
{
	LCDPutBuffToBgImg(_drawBuff->bass_boost.x, _drawBuff->bass_boost.y, \
			_drawBuff->bass_boost.width, _drawBuff->bass_boost.height, _drawBuff->bass_boost.p);

	switch(index){
	case 0:
		LCDPutIcon(_drawBuff->bass_boost.x, _drawBuff->bass_boost.y, _drawBuff->bass_boost.width, _drawBuff->bass_boost.height, \
				bass_base_24x18, bass_base_24x18_alpha);
		break;
	case 1:
		LCDPutIcon(_drawBuff->bass_boost.x, _drawBuff->bass_boost.y, _drawBuff->bass_boost.width, _drawBuff->bass_boost.height, \
				bass_level1_24x18, bass_level1_24x18_alpha);
		break;
	case 2:
		LCDPutIcon(_drawBuff->bass_boost.x, _drawBuff->bass_boost.y, _drawBuff->bass_boost.width, _drawBuff->bass_boost.height, \
				bass_level2_24x18, bass_level2_24x18_alpha);
		break;
	case 3:
		LCDPutIcon(_drawBuff->bass_boost.x, _drawBuff->bass_boost.y, _drawBuff->bass_boost.width, _drawBuff->bass_boost.height, \
				bass_level3_24x18, bass_level3_24x18_alpha);
		break;
	default:
		break;
	}
}

void Update_Reverb_Effect_Icon(uint8_t index)
{
	LCDPutBuffToBgImg(_drawBuff->reverb_effect.x, _drawBuff->reverb_effect.y, \
			_drawBuff->reverb_effect.width, _drawBuff->reverb_effect.height, _drawBuff->reverb_effect.p);

	switch(index){
	case 0:
		LCDPutIcon(_drawBuff->reverb_effect.x, _drawBuff->reverb_effect.y, _drawBuff->reverb_effect.width, _drawBuff->reverb_effect.height, \
				reverb_base_24x18, reverb_base_24x18_alpha);
		break;
	case 1:
		LCDPutIcon(_drawBuff->reverb_effect.x, _drawBuff->reverb_effect.y, _drawBuff->reverb_effect.width, _drawBuff->reverb_effect.height, \
				reverb_level1_24x18, reverb_level1_24x18_alpha);
		break;
	case 2:
		LCDPutIcon(_drawBuff->reverb_effect.x, _drawBuff->reverb_effect.y, _drawBuff->reverb_effect.width, _drawBuff->reverb_effect.height, \
				reverb_level2_24x18, reverb_level2_24x18_alpha);
		break;
	case 3:
		LCDPutIcon(_drawBuff->reverb_effect.x, _drawBuff->reverb_effect.y, _drawBuff->reverb_effect.width, _drawBuff->reverb_effect.height, \
				reverb_level3_24x18, reverb_level3_24x18_alpha);
		break;
	default:
		break;
	}

}

void Update_Vocal_Canceler_Icon(uint8_t index)
{
	LCDPutBuffToBgImg(_drawBuff->vocal_cancel.x, _drawBuff->vocal_cancel.y, \
			_drawBuff->vocal_cancel.width, _drawBuff->vocal_cancel.height, _drawBuff->vocal_cancel.p);

	switch(index){
	case 0:
		LCDPutIcon(_drawBuff->vocal_cancel.x, _drawBuff->vocal_cancel.y, _drawBuff->vocal_cancel.width, _drawBuff->vocal_cancel.height, \
				vocal_base_24x18, vocal_base_24x18_alpha);
		break;
	case 1:
		LCDPutIcon(_drawBuff->vocal_cancel.x, _drawBuff->vocal_cancel.y, _drawBuff->vocal_cancel.width, _drawBuff->vocal_cancel.height, \
				vocal_canceled_24x18, vocal_canceled_24x18_alpha);
		break;
	default:
		break;
	}

}


