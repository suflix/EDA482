
__attribute__((naked)) __attribute__((section (".start_section")) )
void startup(void){
__asm__ volatile(" LDR R0,=0x2001C000\n");		/* set stack */
__asm__ volatile(" MOV SP,R0\n");
__asm__ volatile(" BL main\n");					/* call main */
__asm__ volatile(".L1: B .L1\n");				/* never return */
	#define GPIO_E         0x40021000
	#define GPIO_E_MODER    ((volatile unsigned int *)(GPIO_E))
	#define GPIO_E_OTYPER   ((volatile unsigned short *)(GPIO_E+0x4))
	#define GPIO_E_OSPEEDR  ((volatile unsigned int *)(GPIO_E+0x8))
	#define GPIO_E_PUPDR	  ((volatile unsigned int *)(GPIO_E+0xC))
	#define GPIO_E_IDR_LOW  ((volatile unsigned char *)(GPIO_E+0x10))
	#define GPIO_E_IDR_HIGH ((volatile unsigned char *)(GPIO_E+0x11))
	#define GPIO_E_ODR_LOW  ((volatile unsigned char *)(GPIO_E+0x14))
	#define GPIO_E_ODR_HIGH ((volatile unsigned char *)(GPIO_E+0x15)) 
	#define GPIO_D         0x40020C00
	#define GPIO_D_MODER    ((volatile unsigned int *)(GPIO_D))
	//#define GPIO_OTYPER   ((volatile unsigned short *)(GPIO_D+0x4))			behövs ej
	#define GPIO_D_PUPDR	  ((volatile unsigned int *)(GPIO_D+0xC))
	#define GPIO_D_IDR_LOW  ((volatile unsigned char *)(GPIO_D+0x10))
	#define GPIO_D_IDR_HIGH ((volatile unsigned char *)(GPIO_D+0x11))
	#define GPIO_D_ODR_LOW  ((volatile unsigned char *)(GPIO_D+0x14))
	#define GPIO_D_ODR_HIGH ((volatile unsigned char *)(GPIO_D+0x15)) 

//#define stick (*((volatile systick *) 0xE000E010))
//#define GPIO_E (*((volatile GPIO *) 0x40021000))
#define B_E (unsigned char) 0x40
#define B_SELECT (unsigned char) 0x04
#define B_RW (unsigned char) 0x02
#define B_RS (unsigned char) 0x01
//#define B_CS1 (unsigned char) 0x08
//#define B_CS2 (unsigned char) 0x10
//#define B_RST (unsigned char) 0x20
//#define LCD_ON (unsigned char) 0x3F
//#define LCD_OFF (unsigned char) 0x3E
//#define LCD_SET_ADD (unsigned char) 0x40
//#define LCD_SET_PAGE (unsigned char) 0xB8
//#define LCD_DISP_START (unsigned char) 0xC0
//#define LCD_BUSY (unsigned char) 0x80 

}
	#define INIT 0					// definierar en init
	#define WAIT 1					// definierar en wait

	keyb_state = INIT;				// sätter state till init som initialvärde
	
	

void app_init(void){

	// hela E som utport
	*GPIO_E_MODER = 0x55555555;
	*GPIO_E_OTYPER = 0x0000;
	
	// förbered för input (pull-up)
	*GPIO_E_PUPDR = 0x55555555;
	
	 *GPIO_D_MODER  = 0x55005555; /* PD0-7 ut,  PD8-15 in */
	 *GPIO_D_PUPDR  = 0x00AA0000;          /* Ingångar ”flytande” */ 
	
	*GPIO_E_ODR_LOW |= B_SELECT;			// sätter ett initialt ODR värde
}

#define STK_CTRL ((volatile unsigned int *)(0xE000E010))
#define STK_LOAD ((volatile unsigned int *)(0xE000E014))
#define STK_VAL ((volatile unsigned int *)(0xE000E018))

void delay_250ns( void){
		/* SystemCoreClock = 168*10^6 */
		*STK_CTRL = 0;
		*STK_LOAD = ((168/4)-1);
		*STK_VAL = 0;
		*STK_CTRL = 5;
		while((*STK_CTRL & 0x10000) == 0 );
			*STK_CTRL = 0;
}

void delay_mikro(unsigned int us){		
	#ifdef	 SIMULATOR						//kollar om det körs i simulator eller ej
		us = us/1000;                   	// gör ms en faktor 1000 mindre om det körs i simulator
		us++;                           	// gör att us blir 1
	#endif                              
	for(int i = 0; i<us*4;i++){         	//loopar delay_250ns tillräckligt många gånger för att få önskad tid
		delay_250ns();
	}
}

void delay_milli(unsigned int ms){
	#ifdef	 SIMULATOR						//kollar om det körs i simulator eller ej
		ms = ms / 1000;						// gör ms en faktor 1000 mindre om det körs i simulator
		ms++;								// gör att ms blir 1
	#endif
		for(int i = 0; i<ms*4000;i++){		//loopar delay_250ns tillräckligt många gånger för att få önskad tid
			delay_250ns();					
		}
}


/*Addressera ascii-display och ettställ bitar som är i x*/
void ascii_ctrl_bit_set(unsigned char x){
	*GPIO_E_ODR_LOW |= x;
	*GPIO_E_ODR_LOW |= B_SELECT;
}

/*Addressera ascii-display och nollställ bitar som är i x*/
void ascii_ctrl_bit_clear(unsigned char x){
	*GPIO_E_ODR_LOW &= ~x;
	*GPIO_E_ODR_LOW |= B_SELECT;
}
void ascii_write_controller(unsigned char byte){
	ascii_ctrl_bit_set(B_E);							//kallar på ascii_ctrl_Bit_set med B_E som input
	*GPIO_E_ODR_HIGH = byte;								//sätter byte till pointern
	delay_250ns();										//kallar på en delay
	ascii_ctrl_bit_clear(B_E);							//kallar på ascii_ctr_bit_clear med B_E som input
}
unsigned char ascii_read_controller(void){
	ascii_ctrl_bit_set(B_E);							//kallar på ascii_ctrl_Bit_set med B_E som input		
	delay_250ns();										//kallar på en delay
	delay_250ns();										//kallar på en delay
	unsigned char rv = *GPIO_E_IDR_HIGH;					//sätter in pointern på en variabel
	ascii_ctrl_bit_clear(B_E);							//kallar på ascii_ctr_bit_clear med B_E som input
	return rv;											//returnar variablen
}

void ascii_write_cmd(unsigned char command){
	ascii_ctrl_bit_clear(B_RS);							//kallar på ascii_ctr_bit_clear med B_RS som input
	ascii_ctrl_bit_clear(B_RW);							//kallar på ascii_ctr_bit_clear med B_RW som input
	ascii_write_controller(command);					//kallar på ascii_write_controller med command som input
}
void ascii_write_data(unsigned char data){
	ascii_ctrl_bit_set(B_RS);							//kallar på ascii_ctrl_Bit_set med B_RS som input			
	ascii_ctrl_bit_clear(B_RW);							//kallar på ascii_ctr_bit_clear med B_RW som input
	ascii_write_controller(data);						//kallar på ascii_write_controller med data som input
}
unsigned char ascii_read_status(void){
	*GPIO_E_MODER &= 0x0000FFFF;							//omkonfigurera till ingång temporärt
	ascii_ctrl_bit_clear(B_RS);							//kallar på ascii_ctr_bit_clear med B_RS som input
	ascii_ctrl_bit_set(B_RW);							//kallar på ascii_ctrl_Bit_set med B_RW som input
	unsigned char rv = ascii_read_controller();			//sätter ascii_read_controller värde till en variabel
	*GPIO_E_MODER |= 0x55550000;							//tillbaka till utgång
	return rv;											//returnar variabeln
}

unsigned char ascii_read_data(void){
	*GPIO_E_MODER &= 0x0000FFFF;							//omkonfigurera till ingång temporärt
	ascii_ctrl_bit_set(B_RS);							//kallar på ascii_ctrl_Bit_set med B_RS som input
	ascii_ctrl_bit_set(B_RW);							//kallar på ascii_ctrl_Bit_set med B_RW som input
	unsigned char rv = ascii_read_controller();			//sätter ascii_read_controller värde till en variabel
	*GPIO_E_MODER |= 0x55550000;							//tillbaka till utgång
	return rv;											//returnar variabeln
}
void ascii_init(void){
	// clear display
	while(ascii_read_status() & 0x80)
	{}
	delay_mikro(8);
	ascii_write_cmd(0x01);
	delay_milli(2);
	
	// function set
	while(ascii_read_status() & 0x80)
	{}
	delay_mikro(8);
	ascii_write_cmd(0x38);
	delay_mikro(39);
	
	// display control
	while(ascii_read_status() & 0x80)
	{}
	delay_mikro(8);
	ascii_write_cmd(0x0e);
	delay_mikro(39);
	
	// entry mode set
	while(ascii_read_status() & 0x80)
	{}
	delay_mikro(8);
	ascii_write_cmd(0x04);
	delay_mikro(39);
}
void ascii_write_char(unsigned char c){
	while(ascii_read_status() & 0x80)
	{}
	delay_mikro(8);
	ascii_write_data(c);
	delay_mikro(43);
}
void ascii_gotoxy(int x, int y){
	int address = x-1;
	if (y == 2){
		address = address + 0x40;
	}
	
	while(ascii_read_status() & 0x80)
	{}
	delay_mikro(8);
	ascii_write_cmd(0x80|address);
	delay_mikro(39);
}
void kbdActivate(unsigned int row){
	switch(row){															// kollar vilken rad den befinner sig i
				case 1: *GPIO_D_ODR_HIGH = 0x10; break;                       
				case 2: *GPIO_D_ODR_HIGH = 0x20; break;                       
				case 3: *GPIO_D_ODR_HIGH = 0x40; break;                       
				case 4: *GPIO_D_ODR_HIGH = 0x80; break;                       
				case 5: *GPIO_D_ODR_HIGH = 0xF0; break;                       
				default: *GPIO_D_ODR_HIGH = 0x00;                              // default läge, ingen rad
	}                                                                       
}                                                                           
int kbdGetCol(void){                                                         // kollar vilken column den befinner sig i
		unsigned char c;                                                    
		c = *GPIO_D_IDR_HIGH;                                                 
		if(c & 8)return 4;                                                  
		if(c & 4)return 3;                                                  
		if(c & 2)return 2;                                                  
		if(c & 1)return 1;                                                   
		return 0;															 // returnar 0 om ingen column
}
unsigned char keyb_enhanced(void)
{
	unsigned char keyValue[] = {1,2,3,10,4,5,6,11,7,8,9,12,14,0,15,13};		// sätter en array med de olika värdena
	int row = 1;                                                            // sätter en row som int, som är lika med 1
	int column;                                                             // sätter em column som int
	
	while(keyb_state == WAIT){												// medans state är i wait läge
		kbdActivate(5);														// sätt in case 5 i kbdActivate funktionen
		if (!kbdGetCol()) 													// om inte kbdGetKol
		{
			keyb_state = INIT;												// ändra state till init
		}
	}
	while(keyb_state == INIT){												// medans state är i init läge
			for(row = 1; row <= 4; row ++){									// för row (initlial = 1) till 4 eller mindre, med row increment
			kbdActivate(row);												// kör kbdActivate funktion med row som input
				if(column = kbdGetCol()){									// om column lika med kbdGetCol funktionen
				keyb_state = WAIT;											// sätt state till wait
				return keyValue[(4*(row-1)+column)-1];						// returna keyvalue med row och column pusition
				}
			}
	}
}

void main(void){
		unsigned char c;
		int cursor_pos;
		cursor_pos = 1; 
		while( 1 ) {
		  c = keyb_enhanced(); 
			switch( c ){
			    case 0xD: return 0;  
				case 0xB: cursor_pos --; 
				ascii_gotoxy(cursor_pos,2); 
				ascii_write_char( ' ' ); 
				ascii_gotoxy(cursor_pos,2); 
				break;
				default:
				if( c < 10 ){
					ascii_write_char( c+'0'); 
					cursor_pos ++; 
				}
			} 	
		}
}
