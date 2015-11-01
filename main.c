#include "main.h"

/* Private variables ---------------------------------------------------------*/
//UART connection  
static __IO uint32_t TimingDelay;
void UARTSend(const unsigned char * pucBuffer, unsigned long ulCount);
volatile char received_string[MAX_STRLEN + 1]; // this will hold the received string
void USART1_IRQHandler(void);

int speed = 0;
int main(void) {
	unsigned char data[] = "1";
	u8 loop = 1;

	init_USART1(BT_BAUD);
	SystemInit();
	GPIOinit();
	GPIOinit_Enable();
	GPIO_SetBits(GPIOB, GPIO_Pin_4);
	TimerInit();

	while (loop) {
		UARTSend(data, sizeof(data));
	}
}

/**
 * @brief  This function handles USARTx global interrupt request
 * @param  None
 * @retval None
 * this is the interrupt request handler (IRQ) for ALL USART1 interrupts
 */
void GPIOinit_Enable() {
	GPIO_InitTypeDef gpio_init2;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	gpio_init2.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
	gpio_init2.GPIO_Mode = GPIO_Mode_OUT;
	gpio_init2.GPIO_Speed = GPIO_Speed_100MHz;
	gpio_init2.GPIO_OType = GPIO_OType_PP;
	gpio_init2.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOB, &gpio_init2);
}
void GPIOinit() {
	GPIO_InitTypeDef gpio_init;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	gpio_init.GPIO_Pin = GPIO_Pin_6;
	gpio_init.GPIO_Mode = GPIO_Mode_AF;
	gpio_init.GPIO_Speed = GPIO_Speed_100MHz;
	gpio_init.GPIO_OType = GPIO_OType_PP;
	gpio_init.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOB, &gpio_init);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_TIM4);

}
void TimerInit() {
	TIM_TimeBaseInitTypeDef time_init;
	TIM_OCInitTypeDef oc_init;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	uint16_t PrescalerValue = (uint16_t)((SystemCoreClock / 2) / 1000000)-1;
	time_init.TIM_Period = 65535;
	time_init.TIM_Prescaler = PrescalerValue;
	time_init.TIM_ClockDivision = 0;
	time_init.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM4, &time_init);
	oc_init.TIM_OCMode = TIM_OCMode_PWM1;
	oc_init.TIM_OutputState = TIM_OutputState_Enable;
	oc_init.TIM_Pulse = 0;
	oc_init.TIM_OCPolarity = TIM_OCPolarity_High;

	TIM_OC1Init(TIM4, &oc_init);
	TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Enable);
	oc_init.TIM_OutputState = TIM_OutputState_Enable;
	oc_init.TIM_Pulse = 0;

	TIM_ARRPreloadConfig(TIM4, ENABLE);
	TIM_Cmd(TIM4, ENABLE);
}

void USART1_IRQHandler(void) {
	static uint16_t RxByte = 0x00;

	if (USART_GetITStatus(USART1, USART_IT_TC) == SET) {
		if (USART_GetFlagStatus(USART1, USART_FLAG_TC)) {
			USART_SendData(USART1, RxByte);
			USART_ITConfig(USART1, USART_IT_TC, DISABLE);
		}
		USART_ClearITPendingBit(USART1, USART_IT_TC);
	}

	if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET) {
		if (USART_GetFlagStatus(USART1, USART_FLAG_RXNE)) {
			RxByte = USART_ReceiveData(USART1);
			USART_ITConfig(USART1, USART_IT_TC, ENABLE);
		}
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
	}
	if(RxByte ==130) speed =70;
	if(RxByte ==140) speed =10;
	if(RxByte ==142) speed =50;
	if(RxByte ==176) speed =90;
	TIM4->CCR1 = speed * 65535 / 100; // 10% Duty cycle
	//Wait some time before ending the loop
	Delay(100000);
}
/*- Normal method ------------------------------------------------------------*/

/**
 * Method that send a string to the UART.
 * @param *pcBuffer buffers to be printed.
 *@param ulCount the buffer's length
 */
void UARTSend(const unsigned char *pucBuffer, unsigned long ulCount) {
	// Loop while there are more characters to send.
	while (ulCount--) {
		while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET) {
		}
		USART_SendData(USART1, (uint16_t) * pucBuffer++);
		/* Loop until the end of transmission */
	}
}
/* This function initializes the USART1 peripheral
 *
 * Arguments: baud rate --> the baud rate at which the USART is
 * 						   supposed to operate
 */
void init_USART1(uint32_t baudrate) {

	/* This is a concept that has to do with the libraries provided by ST
	 * to make development easier the have made up something similar to
	 * classes, called TypeDefs, which actually just define the common
	 * parameters that every peripheral needs to work correctly
	 *
	 * They make our life easier because we don't have to mess around with
	 * the low level stuff of setting bits in the correct registers
	 */
	GPIO_InitTypeDef GPIO_InitStruct; // this is for the GPIO pins used as TX and RX
	USART_InitTypeDef USART_InitStruct; // this is for the USART1 initialization
	NVIC_InitTypeDef NVIC_InitStructure; // this is used to configure the NVIC (nested vector interrupt controller)

	/* enable APB2 peripheral clock for USART1
	 * note that only USART1 and USART6 are connected to APB2
	 * the other USARTs are connected to APB1
	 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	/* enable the peripheral clock for the pins used by
	 * USART1, PB6 for TX and PB7 for RX
	 */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	/* This sequence sets up the TX and RX pins
	 * so they work correctly with the USART1 peripheral
	 */
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10; // Pins 6 (TX) and 7 (RX) are used
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF; // the pins are configured as alternate function so the USART peripheral has access to them
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz; // this defines the IO speed and has nothing to do with the baud rate!
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP; // this defines the output type as push pull mode (as opposed to open drain)
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL; // this activates the pull up resistors on the IO pins

	GPIO_Init(GPIOA, &GPIO_InitStruct); // now all the values are passed to the GPIO_Init() function which sets the GPIO registers

	/* The RX and TX pins are now connected to their AF
	 * so that the USART1 can take over control of the
	 * pins
	 */
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);

	/* Now the USART_InitStruct is used to define the
	 * properties of USART1
	 */
	USART_InitStruct.USART_BaudRate = baudrate; // the baud rate is set to the value we passed into this init function
	USART_InitStruct.USART_WordLength = USART_WordLength_8b; // we want the data frame size to be 8 bits (standard)
	USART_InitStruct.USART_StopBits = USART_StopBits_1; // we want 1 stop bit (standard)
	USART_InitStruct.USART_Parity = USART_Parity_No; // we don't want a parity bit (standard)
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // we don't want flow control (standard)
	USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx; // we want to enable the transmitter and the receiver
	USART_Init(USART1, &USART_InitStruct); // again all the properties are passed to the USART_Init function which takes care of all the bit setting

	/* Here the USART1 receive interrupt is enabled
	 * and the interrupt controller is configured
	 * to jump to the USART1_IRQHandler() function
	 * if the USART1 receive interrupt occurs
	 */
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE); // enable the USART1 receive interrupt

	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn; // we want to configure the USART1 interrupts
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; // this sets the priority group of the USART1 interrupts
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0; // this sets the sub priority inside the group
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; // the USART1 interrupts are globally enabled
	NVIC_Init(&NVIC_InitStructure); // the properties are passed to the NVIC_Init function which takes care of the low level stuff

	// finally this enables the complete USART1 peripheral
	USART_Cmd(USART1, ENABLE);
}

/*- Timing methods -----------------------------------------------------------*/

/**
 * @brief  Decrements the TimingDelay variable.
 * @param  None
 * @retval None
 */
void TimingDelay_Decrement(void) {
	if (TimingDelay != 0x00) {
		TimingDelay--;
	}
}

/**
 *@brief Method used to wait a certain amount of time
 *@param nCount the time you want to wait
 */
void Delay(__IO uint32_t nCount) {
	while (nCount--)
		;
}
