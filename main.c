#include	<system_stm32f4xx.c>
#include 	<stdio.h>

//PORT1 |=  0x01  /* Set P1.0 */
//PORT1 &= ~0x01  /* Clr P1.0 */
//PORT1 ^=  0x01  /* Toggle P1.0 */

unsigned int EXTI1_counter, EXTI2_counter;
unsigned int counter_static;
unsigned int prescaler;
unsigned int EXTI1_clock, EXTI2_clock, divider, difference;
long Timer_Frequency = 1000000;										//1 Mhz
long Strobe_Frequency = 1000;
char counterBuff[16];

void TextOut(const char *str)											//ITM Stimulus port 0 -> 1-be
{
	do {
		if(*str=='\n') ITM_SendChar('\r');
		ITM_SendChar(*str);														//ITM Send char printf helyett
	} while (*str++);
}

void LED_Init(void)
{
		//LED init PD12
			GPIOD	->	MODER			|=	(1<<24);						//PD12 = output
			GPIOD ->	OTYPER		&=	(1<<12);						//Output Push/pull
			GPIOD ->	OSPEEDR		&=	~(3<<24);						//50Mhz fast speed
			GPIOD ->	OSPEEDR		|=	(2<<24);		
			GPIOD ->	PUPDR			&=	~(3<<24);						//PD12 pull up
			GPIOD	->	PUPDR			|=	(1<<24);
		//LED init PD12	

		//LED init PD14
			GPIOD	->	MODER			|=	(1<<28);						//PD12 = output
			GPIOD ->	OTYPER		&=	(1<<14);						//Output Push/pull
			GPIOD ->	OSPEEDR		&=	~(3<<28);						//50Mhz fast speed
			GPIOD ->	OSPEEDR		|=	(2<<28);		
			GPIOD ->	PUPDR			&=	~(3<<28);						//PD12 pull up
			GPIOD	->	PUPDR			|=	(1<<28);
		//LED init PD14	
	
		//LED init PD13
			GPIOD	->	MODER			|=	(1<<26);						//PD12 = output
			GPIOD ->	OTYPER		&=	(1<<13);						//Output Push/pull
			GPIOD ->	OSPEEDR		&=	~(3<<26);						//50Mhz fast speed
			GPIOD ->	OSPEEDR		|=	(2<<26);		
			GPIOD ->	PUPDR			&=	~(3<<26);						//PD12 pull up
			GPIOD	->	PUPDR			|=	(1<<26);
		//LED init PD13
}

void EXTI1_IRQHandler(void)__irq //Kezeli a PA1 lábra kapcsolt számlálást
	{
		TIM2	->	CR1				^=	(1<<0);							//Timer Disable
		NVIC->ICER[0] 			&= ~(1<<7);							//Disable this interrupt during processing
			//Itt fogsz azt csinálni amit szeretnél
		EXTI1_counter = TIM2->CNT;													//STM Studio debughoz
		EXTI1_clock = ((SystemCoreClock/2)/TIM2->CNT);
		TIM2	->	CNT = 0;
		TIM2	->	SR				 =	0;													//Clear UIF bit
		TIM2	->	CR1				|=	(1<<0);								//Timer Enable
		EXTI->PR 	|=(1<<1);													//Pending request clear
		NVIC->ICPR[0] 			|= (1<<7);								//Clear the Pending Interrupt
		NVIC->ISER[0] |= (1 << 7) | (1 << 8);										//Reenable the EXTI1 Interrupt
	}

void EXTI2_IRQHandler(void)__irq //Kezeli a PA2 lábra kapcsolt számlálást
	{
		TIM5	->	CR1				^=	(1<<0);							//Timer Disable
		NVIC->ICER[0] 			&= ~(1<<8);							//Disable this interrupt during processing
			//Itt fogsz azt csinálni amit szeretnél
		EXTI2_counter = TIM5->CNT;													//STM Studio debughoz
		EXTI2_clock = ((SystemCoreClock/2)/TIM5->CNT);
		TIM5	->	CNT = 0;
		TIM5	->	SR		=	0;													//Clear UIF bit
		TIM5	->	CR1				|=	(1<<0);								//Timer Enable
		EXTI->PR 	|= (1<<3);													//Pending request clear
		NVIC->ICPR[0] 			|= (1<<8);								//Clear the Pending Interrupt
		NVIC->ISER[0] |= (1 << 8) | (1 << 7);										//Reenable the EXTI2 Interrupt
	}
	
int	main (void)
	{	//RCC init
			RCC		->	AHB1ENR		|=	(1<<0);							//Enable PortA
			RCC		->	AHB1ENR		|=	(1<<1);							//Enable PortB
			RCC		->	AHB1ENR		|=	(1<<3);							//Enable PortD
			RCC		->	APB1ENR		|=	(1<<0);							//Enable Timer2 clock
			RCC		->	APB1ENR		|=	(1<<3);							//Enable Timer5 clock
		
		//Timer2 init
			prescaler = ((SystemCoreClock/2)/Timer_Frequency)-1;
			TIM2	->	PSC				=	prescaler;					//Prescaler 840
		//
		
		//Timer5 init
			prescaler = ((SystemCoreClock/2)/Timer_Frequency)-1;
			TIM5	->	PSC 			= prescaler;	
		//
		
			LED_Init();
			
		/*EXTI1 interrupt
			PA1 lábra megy a magasabb frekvencia
			EXTI1 kezeli
			Timer2 számlálja
		*/
			EXTI->IMR 	|= (1<<1);											//Interrupt mask register
			EXTI->PR 	|= (1<<1);												//Pending request
			EXTI->FTSR 	|= (1<<1);										  //Falling Trigger selection register
			NVIC->ISER[0] |= (1<<7);										//Set enable 
			NVIC->ICPR[0] |= (1<<7);										//Clear pending (fake) if needed
		
		/*EXTI2 interrupt
			PA2 lábra megy a magasabb frekvencia
			EXTI2 kezeli
			Timer5 számlálja
		*/
			EXTI->IMR 	|= (1<<2);											//Interrupt mask register
			EXTI->PR 	|= (1<<2);												//Pending request
			EXTI->FTSR 	|= (1<<2);										  //Falling Trigger selection register
			NVIC->ISER[0] |= (1<<8);										//Set enable 
			NVIC->ICPR[0] |= (1<<8);										//Clear pending (fake) if needed
	
		//Timer enable
			TIM2	->	CR1				|=	(1<<0);							//Timer2 Enable
			TIM5	->	CR1 			|=  (1<<0);								//TIMER5 enable
			
			divider = 0;
			while (1)
			{
				if ( TIM2 -> SR & 1)											//Ha a TIMER2 túlcsordul, piros led felkapcsol
				{
					GPIOD -> ODR |= (1<<14);									
				}
				else divider = (EXTI2_clock*10) / (EXTI1_clock);
				//counter_static = (TIM2->CNT/1000);					//debug: STM Studiohoz
			}
}//main
