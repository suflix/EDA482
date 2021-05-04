void startup(void) __attribute__((naked)) __attribute__((section(".start_section")));

void startup(void) {
    __asm volatile(" LDR R0,=0x2001C000\n" /* set stack */
                   " MOV SP,R0\n"
                   " BL main\n"   /* call main */
                   "_exit: B .\n" /* never return */
    );
}

#define STK_CTRL ((volatile unsigned int*)(0xE000E010))
#define STK_LOAD ((volatile unsigned int*)(0xE000E014))
#define STK_VAL ((volatile unsigned int*)(0xE000E018))

#define B_E 0x40
#define B_SELECT 4
#define B_RW 2
#define B_RS 1

#define PORT_BASE ((volatile unsigned int*)0x40021000)
#define portModer ((volatile unsigned int*)(PORT_BASE))
#define portOtyper ((volatile unsigned short*)(0x40021004))
#define portOspeedr ((volatile unsigned int*)(0x40021008))
#define portPupdr ((volatile unsigned int*)(0x4002100C))

#define portIdrLow ((volatile unsigned char*)(0x40021010))
#define portIdrHigh ((volatile unsigned char*)(0x40021011))
#define portOdrLow ((volatile unsigned char*)(0x40021014))
#define portOdrHigh ((volatile unsigned char*)(0x40021015))

#define B_RST 0x20
#define B_CS2 0x10
#define B_CS1 8

#define LCD_ON 0x3F
#define LCD_OFF 0x3E
#define LCD_SET_ADD 0x40
#define LCD_SET_PAGE 0xB8
#define LCD_DISP_START 0xC0
#define LCD_BUSY 0x80
#define MAX_POINTS (unsigned char)20
#define SIMULATOR

typedef unsigned char uint8_t;
typedef struct tPoint {
    int x;
    int y;
} POINT;

typedef struct line {
    POINT startpoint;
    POINT endpoint;
} LINE, *PLINE;

// alt

// typedef struct line {
//    int startx;
//    int starty;
//    int endx;
//    int endy;
//} LINE, *PLINE;

void app_init(void) {
    *portModer = 0x55555555;
    *portOtyper = 0x00000000;
    *portOspeedr = 0x55555555;
    *portPupdr = 0x55550000;
}
void delay_250ns(void) {
    /* SystemCoreClock = 168*10^6 */
    *STK_CTRL = 0;
    *STK_LOAD = ((168 / 4) - 1);
    *STK_VAL = 0;
    *STK_CTRL = 5;
    while((*STK_CTRL & 0x10000) == 0)
	;
    *STK_CTRL = 0;
}

void delay_milli(unsigned int ms) {
#ifdef SIMULATOR     // kollar om det körs i simulator eller ej
    ms = ms / 1000;  // gör ms en faktor 1000 mindre om det körs i simulator
    ms++;            // gör att ms blir 1
#endif
    for(int i = 0; i < ms * 4000; i++) {  // loopar delay_250ns tillräckligt många gånger för att få önskad tid
	delay_250ns();
    }
}
void delay_mikro(unsigned int us) {  //överflödig, men kan behövas nångång, funkar på samma sätt som delay_milli
#ifdef SIMULATOR
    us = us / 1000;
    us++;
#endif
    for(int i = 0; i < us * 4; i++) {
	delay_250ns();
    }
}
static void graphic_ctrl_bit_set(uint8_t x) {
    // uint8_t c;
    // c = *portOdrLow;
    // c |=(B_SELECT|x);
    //*portOdrLow = c;
    *portOdrLow |= (~B_SELECT & x);
}
static void graphic_ctrl_bit_clear(uint8_t x) {
    // uint8_t c;
    // c = *portOdrLow;
    // c&=(B_SELECT| -x);
    //*portOdrLow = c;
    *portOdrLow &= (~B_SELECT & ~x);
}

void graphic_wait_rdy(void) {
    uint8_t c;
    graphic_ctrl_bit_clear(B_E);
    *portModer = 0x00005555;
    graphic_ctrl_bit_clear(B_RS);
    graphic_ctrl_bit_set(B_RW);
    delay_250ns;
    delay_250ns;
    while(1) {
	graphic_ctrl_bit_set(B_E);
	delay_250ns;
	delay_250ns;
	c = *portIdrHigh & LCD_BUSY;
	graphic_ctrl_bit_clear(B_E);
	delay_250ns;
	delay_250ns;
	if(c == 0)
	    break;
    }
    // graphic_ctrl_bit_set(B_E);
    *portModer = 0x55555555;
}

uint8_t graphic_read(controller) {
    graphic_ctrl_bit_clear(B_E);
    *portModer = 0x00005555;
    graphic_ctrl_bit_set(B_RS | B_RW);
    select_controller(controller);
    delay_250ns;
    delay_250ns;
    graphic_ctrl_bit_set(B_E);
    delay_250ns;
    delay_250ns;

    uint8_t c;
    c = *portIdrHigh;
    graphic_ctrl_bit_clear(B_E);
    *portModer = 0x55555555;
    if(controller & B_CS1) {
	select_controller(B_CS1);
	graphic_wait_rdy();
    }
    if(controller & B_CS2) {
	select_controller(B_CS2);
	graphic_wait_rdy();
    }
    return c;
}

uint8_t graphic_read_data(uint8_t controller) {
    graphic_read(controller);
    return graphic_read(controller);
}
void graphic_write(uint8_t value, uint8_t controller) {
    *portOdrHigh = value;
    select_controller(controller);
    delay_250ns;
    delay_250ns;
    graphic_ctrl_bit_set(B_E);
    delay_250ns;
    delay_250ns;
    graphic_ctrl_bit_clear(B_E);
}

void graphic_write_cmd(uint8_t cmd, uint8_t ctrl) {
    graphic_ctrl_bit_clear(B_E);
    select_controller(ctrl);
    graphic_ctrl_bit_clear(B_RS | B_RW);
    graphic_write(cmd, ctrl);
}

void graphic_write_data(uint8_t data, uint8_t ctrl) {
    graphic_ctrl_bit_clear(B_E);
    select_controller(ctrl);
    graphic_ctrl_bit_set(B_RS);
    graphic_ctrl_bit_clear(B_RW);
    graphic_write(data, ctrl);
}

void select_controller(uint8_t controller) {
    switch(controller) {
    case 0:
	graphic_ctrl_bit_clear(B_CS1 | B_CS2);
	break;
    case B_CS1:
	graphic_ctrl_bit_set(B_CS1);
	graphic_ctrl_bit_clear(B_CS2);
	break;
    case B_CS2:
	graphic_ctrl_bit_set(B_CS2);
	graphic_ctrl_bit_clear(B_CS1);
	break;
    case B_CS1 | B_CS2:
	graphic_ctrl_bit_set(B_CS1 | B_CS2);
	break;
    }
}

void graphic_initialize(void) {
    graphic_ctrl_bit_set(B_E);
    delay_mikro(10);
    graphic_ctrl_bit_clear(B_CS1 | B_CS2 | B_RST | B_E);
    delay_milli(30);
    graphic_ctrl_bit_set(B_RST);
    delay_milli(100);
    graphic_write_cmd(LCD_OFF, B_CS1 | B_CS2);
    graphic_write_cmd(LCD_ON, B_CS1 | B_CS2);
    graphic_write_cmd(LCD_DISP_START, B_CS1 | B_CS2);
    graphic_write_cmd(LCD_SET_ADD, B_CS1 | B_CS2);
    graphic_write_cmd(LCD_SET_PAGE, B_CS1 | B_CS2);
    select_controller(0);
}

void graphic_clear_screen(void) {
    for(uint8_t page = 0; page < 8; page++)
	graphic_write_cmd(LCD_SET_PAGE | page, B_CS1 | B_CS2);
    graphic_write_cmd(LCD_SET_ADD | 0, B_CS1 | B_CS2);
    for(uint8_t add = 0; add < 64; add++) {
	graphic_write_data(0, B_CS1 | B_CS2);
    }
}

void pixel(int x, int y, int set) {
    int index;
    uint8_t mask, temp, controller;

    // bitmask för pixel
    if(x > 128 || x < 1) {
	return;
    }
    if(y > 64 || y < 1) {
	return;
    }
    index = (y - 1) / 8;
    switch((y - 1) % 8) {
    case 0:
	mask = 1;
	break;
    case 1:
	mask = 2;
	break;
    case 2:
	mask = 4;
	break;
    case 3:
	mask = 8;
	break;
    case 4:
	mask = 0x10;
	break;
    case 5:
	mask = 0x20;
	break;
    case 6:
	mask = 0x40;
	break;
    case 7:
	mask = 0x80;
	break;
    }
    if(set == 0) {
	mask = ~mask;
    }
    // bestämmelse av koordinater
    if(x > 64) {
	controller = B_CS2;
	x = x - 65;  // x fast 0-index pga 2 skärmar
    } else {
	controller = B_CS1;
	x = x - 1;
    }
    graphic_write_cmd(LCD_SET_ADD | x, controller);
    graphic_write_cmd(LCD_SET_PAGE | index, controller);

    temp = graphic_read_data(controller);
    graphic_write_cmd((LCD_SET_ADD | x), controller);
    if(set == 1) {
	mask = mask | temp;
    } else {
	mask = mask & temp;
    }
    graphic_write_data(mask, controller);
}

int draw_line(PLINE l) {
    int steep, temp;
    int y;
    int ystep;
    int error;
    int deltax, deltay;
    int x0, y0, x1, y1;
    for(int i = 0; i <= sizeof(l); i++) {
	x0 = l[i].startpoint.x;
	y0 = l[i].startpoint.y;
	x1 = l[i].endpoint.x;
	y1 = l[i].endpoint.y;

	if(abs(y1 - y0) > abs(x1 - x0)) {
	    steep = 1;
	} else
	    steep = 0;
    }
    if(steep == 1) {
	temp = x0;
	x0 = y0;
	y0 = temp;
    }
    if(x0 > x1) {
	temp = x0;
	x0 = x1;
	x1 = temp;
	temp = y0;
	y0 = y1;
	y1 = temp;
    }
    deltax = x1 - x0;
    deltay = abs(y1, y0);
    error = 0;
    y = y0;

    if(y0 < y1) {
	ystep = 1;
    } else
	ystep = -1;
    for(int x = x0; x <= x1; x++) {
	if(steep == 1) {
	    pixel(y, x, 1);
	} else
	    pixel(x, y, 1);

	error = error = error + deltay;
	if(2 * error >= deltax) {
	    y = y + ystep;
	    error = error - deltax;
	}
    }
}

void main(void) {
    unsigned i;
    LINE lines[] = {{40, 10, 100, 10}, {40, 10, 100, 20}, {40, 10, 100, 30}, {40, 10, 100, 40},
                    {40, 10, 100, 50}, {40, 10, 100, 60}, {40, 10, 90, 60},  {40, 10, 80, 60},
                    {40, 10, 70, 60},  {40, 10, 60, 60},  {40, 10, 50, 60},  {40, 10, 40, 60}};

    app_init();
    graphic_initialize();
#ifdef SIMULATOR
    graphic_clear_screen();
#endif
    while(1) {
	for(int i = 0; i < sizeof(lines) / sizeof(LINE); i++) {
	    draw_line(&lines[i]);
	    delay_milli(500);
	}
	graphic_clear_screen();
    }
}
