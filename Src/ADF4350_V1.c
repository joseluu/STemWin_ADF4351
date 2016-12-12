/*==========================================================
//	function:	ADF4530/51 ����ģ��
//	date:		2015/09/18
//	E_Mail:		technicxie@126.com	
==========================================================*/


#include "stm32f7xx_hal.h"
#include <math.h>
#include <string.h>  // for memcpy
#include "ADF4350_V1.h"


void WriteToADF4350(unsigned char count, unsigned char *buf);
void ReadFromADF4350(unsigned char count, unsigned char *buf);

static unsigned int Fraction, Integer;
static unsigned char Reg_Buf[6];
static double RF_Fre_Value;
static unsigned long Start_Fre_value;
static unsigned long Stop_Fre_value;
static double Delta_Fre_value;
static unsigned long Sweep_Time_value;
static double Current_Fre;



unsigned long Register_Buf[12]; //to communicate with menu

extern void SweepTimerStart(void); 
extern void SweepTimerStop(void);

double GetCurrentFrequency(){
	return Current_Fre;
}


/* Delay routine for SPI signaling */
/* does not work in optimized (release) mode because of compiler optimization */
/* in debug mode, the 100 multiplier leads to a 10us delay and a 2ms ADF4351 programming time */
/* in debug mode a 10 multiplier leads to a 2us delay and a 500us programming time */
/* in optimized mode, the delay is 300ns and th e ADF4351 program time is 50us*/
//#define FAST 1
#ifdef FAST
#define DELAY(n)  
#else#define DELAY(n) delay_dds(n*10)
#endif
void delay_dds(int length)
{
	while (length >0)
    	length--;
}

#if 0
void *memcpy(void *s1, const void *s2, unsigned char n)
{
	unsigned char *cs1 = s1; 
	const unsigned char *cs2 = s2;

	while (n--)
		*cs1++ = *cs2++;
	return s1;
}
#endif

//---------------------------------
//void WriteToADF4350(unsigned char count,unsigned char *Reg_Buf);
//---------------------------------
//Function that writes to the ADF4350 via the SPI port. 
//--------------------------------------------------------------------------------
void WriteToADF4350(unsigned char count, unsigned char *buf)
{
	unsigned	char	ValueToWrite = 0;
    unsigned	char	i = 0;
	unsigned	char	j = 0;
	
	DELAY(2);
	CLR_SCL();
	CLR_LE();
	DELAY(2);

	for(i=count;i>0;i--)
 	{
	 	ValueToWrite = *(buf + i - 1);
		for(j=0; j<8; j++)
		{
			if(0x80 == (ValueToWrite & 0x80))
			{
				SET_DATA();	  //Send one to SDO pin
			}
			else
			{
				CLR_DATA();	  //Send zero to SDO pin
			}
			DELAY(1);
			SET_SCL();
			DELAY(2);
			ValueToWrite <<= 1;	//Rotate data
			CLR_SCL();
			DELAY(1);
		}
	}
	CLR_DATA();
	DELAY(2);
	SET_LE();
	DELAY(2);
	CLR_LE();
}

unsigned int div_i_f(double num)
{
	 unsigned long op;
   unsigned char x;	
	 unsigned int i_v,f_v;
	 num *=1000;
	 op = (unsigned long)num;
	
	 x= op%10;
	 op-=x;
	 op/=10;
	 f_v =x;
	  
	 x= op%10;
	 op-=x;
	 op/=10;	
	 f_v +=x*10;
	
	 x= op%10;
	 op-=x;
	 op/=10;	
	 f_v += x*100;	
	 
	 x= op%10;
	 op-=x;
	 op/=10;
	 i_v =x;
	  
	 x= op%10;
	 op-=x;
	 op/=10;	
	 i_v +=x*10;
	
	 x= op%10;
	 op-=x;
	 op/=10;	
	 i_v += x*100;	 
	
	 x= op%10;
	 op-=x;
	 op/=10;	
	 i_v += x*1000;	

	 Integer = i_v;
	 
	 return f_v;
}

void RF_OUT(void)
{
	unsigned int mod_v[10]={5,25,50,125,250,500,1000,2000,4000};
	unsigned char div,flag;
	unsigned int frac_value;
	unsigned int mod_value;
	double n_value,op,y;
	double ref_clk=REF_CLK;
	unsigned long temp;
	unsigned int i,x;
	
		if(RF_Fre_Value<69)	{
				Register_Buf[4] &= 0x000FFFFF;
				Register_Buf[4] |= 0x00E00000;
			  div=64;
		}
		else if(RF_Fre_Value<138)
		{
				Register_Buf[4] &= 0x000FFFFF;
				Register_Buf[4] |= 0x00D00000;
			  div=32;
		}
		else if(RF_Fre_Value<275)
		{
				Register_Buf[4] &= 0x000FFFFF;
				Register_Buf[4] |= 0x00C00000;
			  div=16;
		}
		else if(RF_Fre_Value<550)
		{
				Register_Buf[4] &= 0x000FFFFF;
				Register_Buf[4] |= 0x00B00000;
			  div=8;
		}
		else if(RF_Fre_Value<1100)
		{
				Register_Buf[4] &= 0x000FFFFF;
				Register_Buf[4] |= 0x00A00000;
			  div=4;
		}
		else if(RF_Fre_Value<2200)
		{
				Register_Buf[4] &= 0x000FFFFF;
				Register_Buf[4] |= 0x00900000;
			  div=2;
		}
		else
		{
				Register_Buf[4] &= 0x000FFFFF;
				Register_Buf[4] |= 0x00800000;
			  div=1;
		}
		
		op = ref_clk/div;
		n_value = RF_Fre_Value/op;
		Fraction=div_i_f(n_value);
		temp = Integer;
		temp <<=15;
		Register_Buf[0] &= 0x00007FFF;
		Register_Buf[0] |= temp;
	
		if(Fraction==0)
		{
					frac_value=0;
			    mod_value=2;
		}
		else
		{
			flag=1;
			y=Fraction;
			y /= 1000;
			while(flag){
				for(i=0;i<9;i++){
					op=mod_v[i]*y;
					x = div_i_f(op);
					if(Integer<mod_v[i]){
						if(x==0){
								frac_value=Integer;	
								mod_value=mod_v[i];
								i=10;
								flag=0;
						}
					}
				}				
			}
		}
		
		frac_value<<=3;
		Register_Buf[0] &= 0xEfff8007;
		Register_Buf[0] |= frac_value;
		mod_value <<=3;
		Register_Buf[1] &= 0x1FFFE007;
		Register_Buf[1] |= mod_value;
		
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);// debug write to device
		memcpy(&Reg_Buf, &Register_Buf[5], 4);
		WriteToADF4350(4,Reg_Buf);
		memcpy(&Reg_Buf, &Register_Buf[4], 4);
		WriteToADF4350(4,Reg_Buf);
		memcpy(&Reg_Buf, &Register_Buf[3], 4);
		WriteToADF4350(4,Reg_Buf);
		memcpy(&Reg_Buf, &Register_Buf[2], 4);
		WriteToADF4350(4,Reg_Buf);
		memcpy(&Reg_Buf, &Register_Buf[1], 4);
		WriteToADF4350(4,Reg_Buf);
		memcpy(&Reg_Buf, &Register_Buf[0], 4);
		WriteToADF4350(4,Reg_Buf);	
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
}

void reset_all_reg(void)
{
		Register_Buf[5]=0x00580005;
		Register_Buf[4]=0x00cc803c;
	  Register_Buf[3]=0x000004B3;
		Register_Buf[2]=0x00004E42;
	  Register_Buf[1]=0x08008011;
	  Register_Buf[0]=0x00400000;
	
	  Register_Buf[6]=200000;
	  Register_Buf[7]=35;
		Register_Buf[8]=4400;
	  Register_Buf[9]=100;
	  Register_Buf[10]=100;
	
}

//*********************************************************************
void ADF4351_Init(void)
{
	SET_CE();
	reset_all_reg();

	Reg_Buf[3] = 0x00;
	Reg_Buf[2] = 0x58;
	Reg_Buf[1] = 0x00;				//write communication register 0x00580005 to control the progress 
 	Reg_Buf[0] = 0x05;				//to write Register 5 to set digital lock detector
	WriteToADF4350(4,Reg_Buf);		

	Reg_Buf[3] = 0x00;
	Reg_Buf[2] = 0xCC;				//(DB23=1)The signal is taken from the VCO directly;(DB22-20:4H)the RF divider is 16;(DB19-12:50H)R is 80
	Reg_Buf[1] = 0x80;				//(DB11=0)VCO powerd up;
 	Reg_Buf[0] = 0x3C;				//(DB5=1)RF output is enabled;(DB4-3=3H)Output power level is 5
	WriteToADF4350(4,Reg_Buf);		

	Reg_Buf[3] = 0x00;
	Reg_Buf[2] = 0x00;
	Reg_Buf[1] = 0x04;				//(DB14-3:96H)clock divider value is 150.
 	Reg_Buf[0] = 0xB3;
	WriteToADF4350(4,Reg_Buf);	

	Reg_Buf[3] = 0x00;
	Reg_Buf[2] = 0x00;				//(DB6=1)set PD polarity is positive;(DB7=1)LDP is 6nS;
	Reg_Buf[1] = 0x4E;				//(DB8=0)enable fractional-N digital lock detect;
 	Reg_Buf[0] = 0x42;				//(DB12-9:7H)set Icp 2.50 mA;
	WriteToADF4350(4,Reg_Buf);		//(DB23-14:1H)R counter is 1

	Reg_Buf[3] = 0x08;
	Reg_Buf[2] = 0x00;
	Reg_Buf[1] = 0x80;			   //(DB14-3:6H)MOD counter is 6;
 	Reg_Buf[0] = 0x11;			   //(DB26-15:6H)PHASE word is 1,neither the phase resync 
	WriteToADF4350(4,Reg_Buf);	   //nor the spurious optimization functions are being used
							   //(DB27=1)prescaler value is 8/9

	Reg_Buf[3] = 0x00;
	Reg_Buf[2] = 0x40;
	Reg_Buf[1] = 0x00;
 	Reg_Buf[0] = 0x00;				//(DB14-3:0H)FRAC value is 0;
	WriteToADF4350(4,Reg_Buf);		//(DB30-15:140H)INT value is 320;
}


static unsigned long Sweep_Time_Counter;
static unsigned char Sweep_DIR_Flag;

void SweepTimerTick(void){ // interrupt processing routine
	unsigned long temp = Register_Buf[10] / 100; // timer is every 100 us
	if (Sweep_Time_Counter++ >= temp) {
		Sweep_Time_Counter = 0;
		if (Sweep_DIR_Flag == 0) {
			Current_Fre += Delta_Fre_value;
			if (Current_Fre >= Stop_Fre_value) {
				Current_Fre = Stop_Fre_value;
				Sweep_DIR_Flag = 1;
			}
		} else {
			Current_Fre -= Delta_Fre_value;
			if (Current_Fre <= Start_Fre_value) {
				Current_Fre = Start_Fre_value;
				Sweep_DIR_Flag = 0;
			}
		}
		RF_Fre_Value = 	Current_Fre;
		RF_OUT();
	}
}

void OnEditClicked_RF_Out(WM_MESSAGE * pMsg)
{
}

void OnButtonClicked_RF_OUT(WM_MESSAGE * pMsg)
{
	unsigned long temp = Register_Buf[6], op;
	double n;
	op = temp % 100;
	temp -= op;
	n = temp;
	n /= 1000;
	RF_Fre_Value = n;
	Current_Fre = RF_Fre_Value;
	RF_OUT();//���
}

void OnButtonClicked_Start_F(WM_MESSAGE * pMsg)
{
}

void OnButtonClicked_Stop_F(WM_MESSAGE * pMsg)
{
}

void OnButtonClicked_Delta_F(WM_MESSAGE * pMsg)
{
}

void OnButtonClicked_Ramp_Clk(WM_MESSAGE * pMsg)
{
}

int bStarted;

void OnButtonClicked_Start_SW(WM_MESSAGE * pMsg)
{
	unsigned long temp = Register_Buf[9], op;
	double n;

	if (!bStarted) {
		bStarted = 1;
		op = temp % 100;
		temp -= op;
		n = temp;
		n /= 1000;
		Delta_Fre_value = n;
	
		Start_Fre_value = Register_Buf[7];
		Stop_Fre_value = Register_Buf[8];
		Sweep_Time_value = Register_Buf[10];
		if (Start_Fre_value < Stop_Fre_value) {
			SweepTimerStart();
			Sweep_Time_Counter = 0;
			Current_Fre = Start_Fre_value;
			Sweep_DIR_Flag = 0;
			RF_Fre_Value = Start_Fre_value;
			RF_OUT();
		} else {
			SweepTimerStop();
		}
	}
}

void OnButtonClicked_Stop_SW(WM_MESSAGE * pMsg)
{
	if (bStarted) {
	  SweepTimerStop();	
	  bStarted = 0;
	}
}

void OnEditClicked_Start_F(WM_MESSAGE * pMsg)
{
}

void OnEditClicked_Stop_F(WM_MESSAGE * pMsg)
{
}

void OnEditClicked_Delta_F(WM_MESSAGE * pMsg)
{
}

void OnEditClicked_Ramp_Clk(WM_MESSAGE * pMsg)
{
}
