#include <string.h>

#include "SWM341.h"

#include "ugui.h"

#define LCD_HDOT	480		// 水平点数
#define LCD_VDOT	272		// 垂直点数
#define LCD_DIRH	1		// 水平显示？


UG_GUI gui;


uint32_t *LCD_Buffer = (uint32_t *)SDRAMM_BASE;

void _HW_DrawPoint(UG_S16 x, UG_S16 y, UG_COLOR c)
{	
#if LCD_DIRH
	LCD_Buffer[y * LCD_HDOT + x] = c;
#else
	LCD_Buffer[(LCD_VDOT - x) * LCD_HDOT + y] = c;
#endif
}


void SerialInit(void);
void MemoryInit(void);
void RGBLCDInit(void);

int main(void)
{
	uint32_t i, j;
	uint32_t color[3] = {C_GREEN, C_BLACK, C_BLUE};
	
	SystemInit();
	
	SerialInit();
		
	MemoryInit();
	
	RGBLCDInit();
	
	LCD_Start(LCD);

#if LCD_DIRH
	UG_Init(&gui,(void(*)(UG_S16,UG_S16,UG_COLOR))_HW_DrawPoint, LCD_HDOT, LCD_VDOT);
#else
	UG_Init(&gui,(void(*)(UG_S16,UG_S16,UG_COLOR))_HW_DrawPoint, LCD_VDOT, LCD_HDOT);
#endif
	
	UG_FillScreen(C_WHITE);
	
	UG_DrawLine(40, 50, 240, 50, C_GREEN);
	
	UG_DrawLine(40, 36, 40, 236, C_BLUE);
	
	UG_DrawCircle(200, 210, 50, C_GREEN);
	UG_DrawCircle(200, 210, 51, C_GREEN);
	UG_DrawCircle(200, 210, 52, C_GREEN);

	while(1==1)
	{		
		for(i = 0; i < 3; i++)
		{
			for(j = 0; j < SystemCoreClock/20; j++) __NOP();
			
			UG_FillFrame(60, 75, 200, 150, color[i]);
		}
	}
}


void RGBLCDInit(void)
{
	uint32_t i;
	LCD_InitStructure LCD_initStruct;
	
	GPIO_Init(GPIOA, PIN6, 1, 0, 0, 0);		//屏幕背光
	GPIO_SetBit(GPIOA, PIN6);
	GPIO_Init(GPIOC, PIN6, 1, 0, 0, 0);		//屏幕复位
	GPIO_ClrBit(GPIOC, PIN6);
	for(i = 0; i < 1000000; i++) __NOP();
	GPIO_SetBit(GPIOC, PIN6);
	
	PORT_Init(PORTB, PIN1,  PORTB_PIN1_LCD_B0,  0);
	PORT_Init(PORTB, PIN11, PORTB_PIN11_LCD_B1, 0);
	PORT_Init(PORTB, PIN13, PORTB_PIN13_LCD_B2, 0);
	PORT_Init(PORTB, PIN15, PORTB_PIN15_LCD_B3, 0);
	PORT_Init(PORTA, PIN2,  PORTA_PIN2_LCD_B4,  0);
	PORT_Init(PORTA, PIN9,  PORTA_PIN9_LCD_B5,  0);
	PORT_Init(PORTA, PIN10, PORTA_PIN10_LCD_B6, 0);
	PORT_Init(PORTA, PIN11, PORTA_PIN11_LCD_B7, 0);
	PORT_Init(PORTA, PIN12, PORTA_PIN12_LCD_G0, 0);
	PORT_Init(PORTA, PIN13, PORTA_PIN13_LCD_G1, 0);
	PORT_Init(PORTA, PIN14, PORTA_PIN14_LCD_G2, 0);
	PORT_Init(PORTA, PIN15, PORTA_PIN15_LCD_G3, 0);
	PORT_Init(PORTC, PIN0,  PORTC_PIN0_LCD_G4,  0);
	PORT_Init(PORTC, PIN1,  PORTC_PIN1_LCD_G5,  0);
	PORT_Init(PORTC, PIN2,  PORTC_PIN2_LCD_G6,  0);
	PORT_Init(PORTC, PIN3,  PORTC_PIN3_LCD_G7,  0);
	PORT_Init(PORTC, PIN4,  PORTC_PIN4_LCD_R0,  0);
	PORT_Init(PORTC, PIN5,  PORTC_PIN5_LCD_R1,  0);
	PORT_Init(PORTC, PIN8,  PORTC_PIN8_LCD_R2,  0);
	PORT_Init(PORTC, PIN9,  PORTC_PIN9_LCD_R3,  0);
	PORT_Init(PORTC, PIN10, PORTC_PIN10_LCD_R4, 0);
	PORT_Init(PORTC, PIN11, PORTC_PIN11_LCD_R5, 0);
	PORT_Init(PORTC, PIN12, PORTC_PIN12_LCD_R6, 0);
	PORT_Init(PORTC, PIN13, PORTC_PIN13_LCD_R7, 0);
	PORT_Init(PORTB, PIN2,  PORTB_PIN2_LCD_VSYNC, 0);
	PORT_Init(PORTB, PIN3,  PORTB_PIN3_LCD_HSYNC, 0);
	PORT_Init(PORTB, PIN4,  PORTB_PIN4_LCD_DEN,   0);
	PORT_Init(PORTB, PIN5,  PORTB_PIN5_LCD_DCLK,  0);
	
	LCD_initStruct.ClkDiv = 8;
	LCD_initStruct.Format = LCD_FMT_RGB888;
	LCD_initStruct.HnPixel = LCD_HDOT;
	LCD_initStruct.VnPixel = LCD_VDOT;
	LCD_initStruct.Hfp = 5;
	LCD_initStruct.Hbp = 40;
	LCD_initStruct.Vfp = 8;
	LCD_initStruct.Vbp = 8;
	LCD_initStruct.HsyncWidth = 5;
	LCD_initStruct.VsyncWidth = 5;
	LCD_initStruct.DataSource = (uint32_t)LCD_Buffer;
	LCD_initStruct.Background = 0xFFFF;
	LCD_initStruct.SampleEdge = LCD_SAMPLE_FALL;	// ATK-4342 RGBLCD 下降沿采样
	LCD_initStruct.IntEOTEn = 1;
	LCD_Init(LCD, &LCD_initStruct);
}

void LCD_Handler(void)
{
	LCD_INTClr(LCD);
	
	LCD_Start(LCD);
}


void MemoryInit(void)
{
	SDRAM_InitStructure SDRAM_InitStruct;
	
	PORT_Init(PORTM, PIN13, PORTM_PIN13_SDR_CLK, 0);
	PORT_Init(PORTM, PIN14, PORTM_PIN14_SDR_CKE, 0);
	PORT_Init(PORTB, PIN7,  PORTB_PIN7_SDR_WE,   0);
	PORT_Init(PORTB, PIN8,  PORTB_PIN8_SDR_CAS,  0);
	PORT_Init(PORTB, PIN9,  PORTB_PIN9_SDR_RAS,  0);
	PORT_Init(PORTB, PIN10, PORTB_PIN10_SDR_CS,  0);
	PORT_Init(PORTE, PIN15, PORTE_PIN15_SDR_BA0, 0);
	PORT_Init(PORTE, PIN14, PORTE_PIN14_SDR_BA1, 0);
	PORT_Init(PORTN, PIN14, PORTN_PIN14_SDR_A0,  0);
	PORT_Init(PORTN, PIN13, PORTN_PIN13_SDR_A1,  0);
	PORT_Init(PORTN, PIN12, PORTN_PIN12_SDR_A2,  0);
	PORT_Init(PORTN, PIN11, PORTN_PIN11_SDR_A3,  0);
	PORT_Init(PORTN, PIN10, PORTN_PIN10_SDR_A4,  0);
	PORT_Init(PORTN, PIN9,  PORTN_PIN9_SDR_A5,   0);
	PORT_Init(PORTN, PIN8,  PORTN_PIN8_SDR_A6,   0);
	PORT_Init(PORTN, PIN7,  PORTN_PIN7_SDR_A7,   0);
	PORT_Init(PORTN, PIN6,  PORTN_PIN6_SDR_A8,   0);
	PORT_Init(PORTN, PIN3,  PORTN_PIN3_SDR_A9,   0);
	PORT_Init(PORTN, PIN15, PORTN_PIN15_SDR_A10, 0);
	PORT_Init(PORTN, PIN2,  PORTN_PIN2_SDR_A11,  0);
	PORT_Init(PORTM, PIN15, PORTM_PIN15_SDR_A12, 0);
	PORT_Init(PORTE, PIN7,  PORTE_PIN7_SDR_D0,   1);
	PORT_Init(PORTE, PIN6,  PORTE_PIN6_SDR_D1,   1);
	PORT_Init(PORTE, PIN5,  PORTE_PIN5_SDR_D2,   1);
	PORT_Init(PORTE, PIN4,  PORTE_PIN4_SDR_D3,   1);
	PORT_Init(PORTE, PIN3,  PORTE_PIN3_SDR_D4,   1);
	PORT_Init(PORTE, PIN2,  PORTE_PIN2_SDR_D5,   1);
	PORT_Init(PORTE, PIN1,  PORTE_PIN1_SDR_D6,   1);
	PORT_Init(PORTE, PIN0,  PORTE_PIN0_SDR_D7,   1);
	PORT_Init(PORTE, PIN8,  PORTE_PIN8_SDR_D8,   1);
	PORT_Init(PORTE, PIN9,  PORTE_PIN9_SDR_D9,   1);
	PORT_Init(PORTE, PIN10, PORTE_PIN10_SDR_D10, 1);
	PORT_Init(PORTE, PIN11, PORTE_PIN11_SDR_D11, 1);
	PORT_Init(PORTE, PIN12, PORTE_PIN12_SDR_D12, 1);
	PORT_Init(PORTE, PIN13, PORTE_PIN13_SDR_D13, 1);
	PORT_Init(PORTC, PIN14, PORTC_PIN14_SDR_D14, 1);
	PORT_Init(PORTC, PIN15, PORTC_PIN15_SDR_D15, 1);
	PORT_Init(PORTB, PIN6,  PORTB_PIN6_SDR_LDQM, 0);
	PORT_Init(PORTM, PIN12, PORTM_PIN12_SDR_UDQM,0);
	
	SDRAM_InitStruct.Size = SDRAM_SIZE_8MB;
	SDRAM_InitStruct.ClkDiv = SDRAM_CLKDIV_1;
	SDRAM_InitStruct.CASLatency = SDRAM_CASLATENCY_2;
	SDRAM_InitStruct.TimeTRP  = SDRAM_TRP_3;
	SDRAM_InitStruct.TimeTRCD = SDRAM_TRCD_3;
	SDRAM_InitStruct.TimeTRFC  = SDRAM_TRFC_15;
	SDRAM_Init(&SDRAM_InitStruct);
}


void SerialInit(void)
{
	UART_InitStructure UART_initStruct;
	
	PORT_Init(PORTM, PIN0, PORTM_PIN0_UART0_RX, 1);		//GPIOM.0配置为UART0输入引脚
 	PORT_Init(PORTM, PIN1, PORTM_PIN1_UART0_TX, 0);		//GPIOM.1配置为UART0输出引脚
 	
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

/****************************************************************************************************************************************** 
* 函数名称: fputc()
* 功能说明: printf()使用此函数完成实际的串口打印动作
* 输    入: int ch		要打印的字符
*			FILE *f		文件句柄
* 输    出: 无
* 注意事项: 无
******************************************************************************************************************************************/
int fputc(int ch, FILE *f)
{
	UART_WriteByte(UART0, ch);
	
	while(UART_IsTXBusy(UART0));
 	
	return ch;
}
