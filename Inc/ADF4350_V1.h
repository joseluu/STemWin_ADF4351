#pragma once
#ifndef __ADF4350_H
#define __ADF4350_H

#include <WM.h>   // for WM_MESSAGE
/*
/* MASTER CLOCK
*/

#define REF_CLK   10


/*
/* CONNECTION PINS DEFINITION
*/

// chip enable: must be high
#define CE_PORT GPIOF
#define CE_PIN  GPIO_PIN_6
#define SET_CE()		HAL_GPIO_WritePin(CE_PORT,CE_PIN,GPIO_PIN_SET)
#define CLR_CE()		HAL_GPIO_WritePin(CE_PORT,CE_PIN,GPIO_PIN_RESET)

// serial data
#define MOSI_PORT GPIOF
#define MOSI_PIN  GPIO_PIN_8
#define SET_DATA()		HAL_GPIO_WritePin(MOSI_PORT, MOSI_PIN,GPIO_PIN_SET)
#define CLR_DATA()		HAL_GPIO_WritePin(MOSI_PORT, MOSI_PIN,GPIO_PIN_RESET)

// load serial data on rising edge, data must be present 10ns before and after
// min clock half period: 25ns
#define SCL_PORT GPIOF
#define SCL_PIN  GPIO_PIN_9
#define SET_SCL()		HAL_GPIO_WritePin(SCL_PORT, SCL_PIN,GPIO_PIN_SET)
#define CLR_SCL()		HAL_GPIO_WritePin(SCL_PORT, SCL_PIN,GPIO_PIN_RESET)


// load 32 bit register on rising edge
#define LE_PORT GPIOF
#define LE_PIN  GPIO_PIN_7
#define SET_LE()		HAL_GPIO_WritePin(LE_PORT, LE_PIN,GPIO_PIN_SET)
#define CLR_LE()		HAL_GPIO_WritePin(LE_PORT, LE_PIN,GPIO_PIN_RESET)


void OnEditClicked_RF_Out(WM_MESSAGE * pMsg);
void OnButtonClicked_RF_OUT(WM_MESSAGE * pMsg);
void OnButtonClicked_Start_F(WM_MESSAGE * pMsg);
void OnButtonClicked_Stop_F(WM_MESSAGE * pMsg);
void OnButtonClicked_Delta_F(WM_MESSAGE * pMsg);
void OnButtonClicked_Ramp_Clk(WM_MESSAGE * pMsg);
void OnButtonClicked_Start_SW(WM_MESSAGE * pMsg);
void OnButtonClicked_Stop_SW(WM_MESSAGE * pMsg);
void OnEditClicked_Start_F(WM_MESSAGE * pMsg);
void OnEditClicked_Stop_F(WM_MESSAGE * pMsg);
void OnEditClicked_Delta_F(WM_MESSAGE * pMsg);
void OnEditClicked_Ramp_Clk(WM_MESSAGE * pMsg);


void ADF4351_Init(void);
void RF_OUT(void);
void SweepTimerTick(void);
double GetCurrentFrequency(void);
void reset_all_reg(void);

extern unsigned long Register_Buf[12];


#endif
