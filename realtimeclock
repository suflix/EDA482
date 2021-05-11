void startup(void) __attribute__((naked)) __attribute__((section (".start_section")) );

void startup ( void )
{
	__asm volatile(
	    " LDR R0,=0x2001C000\n"		/* set stack */
	    " MOV SP,R0\n"
	    " BL main\n"				/* call main */
	    "_exit: B .\n"				/* never return */
	) ;
}
#ifdef  SIMULATOR
#define DELAY_COUNT  100
#else
#define DELAY_COUNT  1000000
#endif

#define SCB_VTOR (volatile unsigned int *) 0xE000ED08
#define GPIO_D 0x40020C00
#define GPIO_D_MODER ((volatile unsigned int*)(GPIO_D))
#define GPIO_D_OTYPER ((volatile unsigned short*)(GPIO_D + 0x4))
#define GPIO_D_PUPDR ((volatile unsigned int*)(GPIO_D + 0xC))
#define GPIO_D_IDR_LOW ((volatile unsigned char*)(GPIO_D + 0x10))
#define GPIO_D_IDR_HIGH ((volatile unsigned char*)(GPIO_D + 0x11))
#define GPIO_D_ODR_LOW ((volatile unsigned char*)(GPIO_D + 0x14))
#define GPIO_D_ODR_HIGH ((volatile unsigned char*)(GPIO_D + 0x15))
#define     STK_CTRL ((volatile unsigned int*)(0xE000E010))
#define     STK_LOAD ((volatile unsigned int*)(0xE000E014))
#define     STK_VAL ((volatile unsigned int*)(0xE000E018))
#define TIM6_CR1 ((volatile unsigned short *)0x40001000)
#define TIM6_DIER ((volatile unsigned short *)0x4000100C)
#define TIM6_SR ((volatile unsigned short *)0x400010010)
#define TIM6_CNT ((volatile unsigned short *)0x40001024)
#define TIM6_PSC ((volatile unsigned short *)0x40001028)
#define TIM6_ARR ((volatile unsigned short *)0x4000102C)
#define TIM6_DAC ((volatile unsigned short *)0x2001C118)

#define UDIS (0x0)
#define CEN (0x1)
#define UIF (0x1)
#define UIE (0x1)
#define NVIC_TIM6_IRQ_BPOS (1<<22)
#define NVIC_TIM6_ISER ((volatile unsigned int *)0xE000E104)

volatile int ticks;
volatile int seconds;

void app_init(void)
{
	*GPIO_D_MODER=0x55555555;
	*SCB_VTOR = 0x2001C000;
	//initiera port D och undantagsvektor
}
void timer6_interrupt(void)
{
	// 100 ms period
	*TIM6_SR &= ~UIF;
	if(ticks > 9) {
		ticks = 0;
		seconds++;
	}
}
void timer6_init(void)
{
	ticks = 0;
	seconds = 0;
	*TIM6_CR1 &= ~CEN;
	*TIM6_DAC = timer6_interrupt;
	*NVIC_TIM6_ISER |= NVIC_TIM6_IRQ_BPOS;
	// 100 ms tidbas
	*TIM6_PSC = 83;
	*TIM6_ARR = 999;
	*TIM6_DIER |=UIE;
	*TIM6_CR1 |= CEN;
}



void main(void)
{
	char nbcd;
	app_init();
	timer6_init();
	while(1) {
		nbcd = ((seconds/100)<<4);
		nbcd |= ((seconds %	100) & 0xF);
		*GPIO_D_ODR_LOW = nbcd;
	}

}
