#include <stdio.h>
#include "GUI.h"
#include "stm32746g_discovery_ts.h"
#include "k_touch.h"

void k_TouchInit(uint16_t ts_SizeX, uint16_t ts_SizeY)
{
	BSP_TS_Init( ts_SizeX,  ts_SizeY);
	}
//extern uint8_t SelLayer;
/**
  * @brief  Read the coordinate of the point touched and assign their
  *         value to the variables u32_TSXCoordinate and u32_TSYCoordinate
  * @param  None
  * @retval None
  */
void k_TouchUpdate(void)
{
	static GUI_PID_STATE TS_State = { 0, 0, 0, 0 };
	__IO TS_StateTypeDef  ts;
	uint16_t xDiff, yDiff;  

	BSP_TS_GetState((TS_StateTypeDef *)&ts);

	if ((ts.touchX[0] >= LCD_GetXSize()) || (ts.touchY[0] >= LCD_GetYSize())) {
		ts.touchX[0] = 0;
		ts.touchY[0] = 0;
		ts.touchDetected = 0;
	}

	xDiff = (TS_State.x > ts.touchX[0]) ? (TS_State.x - ts.touchX[0]) : (ts.touchX[0] - TS_State.x);
	yDiff = (TS_State.y > ts.touchY[0]) ? (TS_State.y - ts.touchY[0]) : (ts.touchY[0] - TS_State.y);
  
  
	if ((TS_State.Pressed != ts.touchDetected) ||
	   (xDiff > 30) ||
	    (yDiff > 30)) {
		TS_State.Pressed = ts.touchDetected;
		TS_State.Layer = 0;//SelLayer;
		if (ts.touchDetected) {
			TS_State.x = ts.touchX[0];
			TS_State.y = ts.touchY[0];
			GUI_TOUCH_StoreStateEx(&TS_State);
		} else {
			GUI_TOUCH_StoreStateEx(&TS_State);
			TS_State.x = 0;
			TS_State.y = 0;
		}
	}
}

