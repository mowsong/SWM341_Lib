#include "SWM341.h"
#include "CircleBuffer.h"


/* ADC0 demo program, using DMA Scatter-Gather mode */


CircleBuffer_t CirBuf;

#define ADC_LEN	  250	// ADC can convert a maximum of 256 times consecutively
uint16_t ADC_Buffer[2][ADC_LEN] = {0};	// ADC_Buffer[0]: the destination buffer for the first half of the DMA transfer
										// ADC_Buffer[1]: the destination buffer for the second half of the DMA transfer

void SerialInit(void);

int main(void)
{
	DMA_InitStructure DMA_initStruct;
	ADC_InitStructure ADC_initStruct;
	ADC_SEQ_InitStructure ADC_SEQ_initStruct;
	
	SystemInit();
	
	SerialInit();
	
	PORT_Init(PORTC, PIN6,  PORTC_PIN6_ADC0_CH0,   0);	//PC.6  => ADC0.CH0
	PORT_Init(PORTC, PIN5,  PORTC_PIN5_ADC0_CH1,   0);	//PC.5  => ADC0.CH1
	PORT_Init(PORTC, PIN4,  PORTC_PIN4_ADC0_CH2,   0);	//PC.4  => ADC0.CH2
	PORT_Init(PORTC, PIN3,  PORTC_PIN3_ADC0_CH3,   0);	//PC.3  => ADC0.CH3
	PORT_Init(PORTC, PIN2,  PORTC_PIN2_ADC0_CH4,   0);	//PC.2  => ADC0.CH4
	PORT_Init(PORTC, PIN1,  PORTC_PIN1_ADC0_CH5,   0);	//PC.1  => ADC0.CH5
	PORT_Init(PORTC, PIN0,  PORTC_PIN0_ADC0_CH6,   0);	//PC.0  => ADC0.CH6
	PORT_Init(PORTA, PIN15, PORTA_PIN15_ADC0_CH7,  0);	//PA.15 => ADC0.CH7
	PORT_Init(PORTA, PIN14, PORTA_PIN14_ADC0_CH8,  0);	//PA.14 => ADC0.CH8
	PORT_Init(PORTA, PIN13, PORTA_PIN13_ADC0_CH9,  0);	//PA.13 => ADC0.CH9
	PORT_Init(PORTA, PIN12, PORTA_PIN12_ADC0_CH10, 0);	//PA.12 => ADC0.CH10
	PORT_Init(PORTA, PIN10, PORTA_PIN10_ADC0_CH11, 0);	//PA.10 => ADC0.CH11
	
	ADC_initStruct.clk_src = ADC_CLKSRC_HRC_DIV8;
	ADC_initStruct.samplAvg = ADC_AVG_SAMPLE1;
	ADC_initStruct.EOC_IEn = 0;	
	ADC_initStruct.HalfIEn = 0;
	ADC_Init(ADC0, &ADC_initStruct);
	
	ADC_SEQ_initStruct.channels = ADC_CH7;
	ADC_SEQ_initStruct.trig_src = ADC_TRIGGER_SW;
	ADC_SEQ_initStruct.conv_cnt = ADC_LEN;
	ADC_SEQ_initStruct.samp_tim = ADC_SAMPLE_1CLOCK;
	ADC_SEQ_Init(ADC0, ADC_SEQ0, &ADC_SEQ_initStruct);
	
	ADC_Open(ADC0);
	ADC_Calibrate(ADC0);
	
	ADC0->CR |= (ADC_SEQ0 << ADC_CR_DMAEN_Pos);
	
	
	DMA_initStruct.Mode = DMA_MODE_CIRCLE;
	DMA_initStruct.Unit = DMA_UNIT_HALFWORD;
	DMA_initStruct.Count = ADC_LEN * 2;		// DMA transfer is divided into two sections, each length ADC_LEN
	DMA_initStruct.SrcAddr = (uint32_t)&ADC0->SEQ[0].DR;
	DMA_initStruct.SrcAddrInc = 0;
	DMA_initStruct.DstAddr = (uint32_t)ADC_Buffer;
	DMA_initStruct.DstAddrInc = 2;			// Scatter-Gather mode
	DMA_initStruct.Handshake = DMA_CH0_ADC0;
	DMA_initStruct.Priority = DMA_PRI_LOW;
	DMA_initStruct.INTEn = DMA_IT_DSTSG_HALF | DMA_IT_DSTSG_DONE;
	DMA_CH_Init(DMA_CH0, &DMA_initStruct);
	DMA_CH_Open(DMA_CH0);
	
	ADC_Start(ADC0, ADC_SEQ0);
	
	while(1==1)
	{
		while(CirBuf_Full(&CirBuf) == 0) __NOP();
		
		for(int i = 0; i < 2000; i++)
		{
			uint16_t data;
			
			CirBuf_Read(&CirBuf, &data, 1);
			
			printf("%4d,", data & ADC_DR_VALUE_Msk);
		}
		
		CirBuf_Clear(&CirBuf);
	}
}


void DMA_Handler(void)
{
	ADC_Start(ADC0, ADC_SEQ0);	// The conversion stops after 250 times and needs to be restarted
	
	if(DMA_CH_INTStat(DMA_CH0, DMA_IT_DSTSG_HALF))
	{
		DMA_CH_INTClr(DMA_CH0, DMA_IT_DSTSG_HALF);
		
		CirBuf_Write(&CirBuf, ADC_Buffer[0], ADC_LEN);
	}
	else if(DMA_CH_INTStat(DMA_CH0, DMA_IT_DSTSG_DONE))
	{
		DMA_CH_INTClr(DMA_CH0, DMA_IT_DSTSG_DONE);
		
		CirBuf_Write(&CirBuf, ADC_Buffer[1], ADC_LEN);
	}
}


void SerialInit(void)
{
	UART_InitStructure UART_initStruct;
	
	PORT_Init(PORTM, PIN0, PORTM_PIN0_UART0_RX, 1);
	PORT_Init(PORTM, PIN1, PORTM_PIN1_UART0_TX, 0);
 	
 	UART_initStruct.Baudrate = 57600;
	UART_initStruct.DataBits = UART_DATA_8BIT;
	UART_initStruct.Parity = UART_PARITY_NONE;
	UART_initStruct.StopBits = UART_STOP_1BIT;
	UART_initStruct.RXThresholdIEn = 0;
	UART_initStruct.TXThresholdIEn = 0;
	UART_initStruct.TimeoutIEn = 0;
 	UART_Init(UART0, &UART_initStruct);
	UART_Open(UART0);
}

int fputc(int ch, FILE *f)
{
	UART_WriteByte(UART0, ch);
	
	while(UART_IsTXBusy(UART0));
 	
	return ch;
}
