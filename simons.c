#include <stdio.h>
#include <stdlib.h>
#include <LPC23xx.H>
#include "LCD.h"

#define BUTTON1_PIN (1 << 10)
#define BUTTON2_PIN (1 << 11)
#define BUTTON3_PIN (1 << 12)
#define BUTTON4_PIN (1 << 13)

#define LED1_PIN    (1 << 0)
#define LED2_PIN    (1 << 1)
#define LED3_PIN    (1 << 3)
#define LED4_PIN    (1 << 4)

#define MAX_SEQUENCE_LENGTH 49


void GPIO_Init(void);
void Play_Sequence(void);
void Reset_Game(void);
void Delay(int count);
void Light_LED(int led_pin);
void Clear_LEDs(void);
void Enable_Interrupts(void);
__irq void eint0_irq(void);
__irq void eint1_irq(void);
__irq void eint2_irq(void);
__irq void eint3_irq(void);
__irq void Timer0_IRQHandler(void);

short AD_last;
int i, z, j, k;
int sequence[49];
char cVal[49];
int sequence[MAX_SEQUENCE_LENGTH];
int sequence_length = 0;
int player_index = 0;
volatile int button_press = -1;
volatile int timeout_flag = 0;  // Flag to indicate if timeout occurred
volatile int timer_count = 0;


void buzzer2()
 { int count4=0;
	 int n=0;
	 int val_decent[12] = {0,0,1,1,3,3,7,7,15,15,31,31};
	 
		T2MR0 = 1200 - 1;         				/* TC0 Match Value 0 */
		T2MCR = 3;                                /* TCO Interrupt and Reset on MR0 */
	 T2TCR = 1;                                /* TC0 Enable */
	 while(count4<5000){
	 if (T2IR & (1 << 0)) {             /* Check if MR0 match occurred (0.1ms) */ 
					T2IR = (1 << 0);
					DACR = 0xAA *(val_decent[n] << 6);             /* Set Speaker Output to DAC*/
					n++;
					if (n == 12) n = 0;
					count4++;
					}}
	T2TCR = 0;			
}
void buzzer(int n)
{	int a[12];
	int x;
	int m=0,count=0;
	switch (n) {
        case 1:{ int a[12] = {21,50,11,80,71,92,02,24,01,00,40,400};
					x=170;
					break;
}
        case 2: {int a[12]={512, 612, 712, 812, 912, 1012, 1112, 1212,53,00,00,53};
					x=255;
					break;
				}
        case 3:{int a[12]={75,100,50,75,100,50,75,100,50,75,100,50}; 
					x=119;
				break;
				}
				case 4: {int a[12]={51,100,51,100,51,100,51,100,51,100,51,100}; 
					x=136;
					break;
				}
    }
	T1MR0 = 1200 - 1;         				/* TC0 Match Value 0 */
  T1MCR = 3;                                /* TCO Interrupt and Reset on MR0 */
  T1TCR = 1;                                /* TC0 Enable */
   while(count<1000){
	 if (T1IR & (1 << 0)) {             /* Check if MR0 match occurred (0.1ms) */ 
					T1IR = (1 << 0);
					DACR = x *(a[m] << 6);             /* Set Speaker Output to DAC*/
					m++;
					if (m == 12) m = 0;
					count++;
					}}
	T1TCR = 0;			
		 
}


void init_adc()
{
    PCONP |= (1 << 12) | (1 << 1)|(1<<22);
    PINSEL1 = 0x254000;
	  DACR    = 0x0; 
}

void read_adc()
{
    int i;
    AD0CR = 0x00200004;
    AD0CR |= (1 << 24);
    for (i = 0; i < 99999; i++);
    AD_last = (AD0DR2 >> 6) & 0x3FF;
}

/* Initialize GPIO for Buttons and LEDs */
void GPIO_Init(void) {
    PINSEL4 = 0x05500000;
    SCS = 0X00000001;
    FIO0DIR |= (LED1_PIN | LED2_PIN | LED3_PIN | LED4_PIN);
    FIO0CLR = (LED1_PIN | LED2_PIN | LED3_PIN | LED4_PIN);
}

/* Generate a random LED sequence for each round */
void Generate_Sequence(void) {
    for (j = 0; j < 48; j++) {
        read_adc();
        sprintf(cVal, "%f", ((AD_last / 10.0) * (3.2 / 1023)));
        sequence[j] = cVal[7] % 4;
    }
}

/* Play the sequence of LEDs for the player to memorize */
void Play_Sequence(void) {
	int i;
    for ( i = 0; i < sequence_length; i++) {
        Light_LED(sequence[i]);
       // Delay(2000000);
      //  Clear_LEDs();
        //Delay(2000000);
    }
}


void Light_LED(int led_index) {
	int m=0;
	if(led_index==0)
	{FIO0SET = LED1_PIN;
		buzzer(1);
Clear_LEDs();}
	else if(led_index==1)
	{FIO0SET = LED2_PIN;
		buzzer(2);
Clear_LEDs();}
	else if(led_index==2)
	{FIO0SET = LED3_PIN;
	buzzer(3);
Clear_LEDs();}
	else if(led_index==3)
	{FIO0SET = LED4_PIN;
	buzzer(4);
Clear_LEDs();}
}

/* Turn off all LEDs */
void Clear_LEDs(void) {
    FIO0CLR = (LED1_PIN | LED2_PIN | LED3_PIN | LED4_PIN);
}

/* Delay for a simple game pace */
void Delay(int count) {
	int i;
    for ( i = 0; i < count; i++);
}

int Check_Player_Input(int led_index) {
    if (sequence[player_index] == led_index) {
        player_index++;
        if (player_index == sequence_length) {
            player_index = 0; // Reset for the next round
            return 1; // Player successfully completed the sequence
        }
    } else {
        return -1; // Player failed
    }
    return 0; // Waiting for the player to complete the sequence
}

/* Reset the game variables to restart */
void Reset_Game(void) {
    sequence_length = 0;
    player_index = 0;
    button_press = -1;
    timeout_flag = 0;  // Reset timeout flag
    Clear_LEDs();
}

/* Enable interrupts and configure button pins */
void Enable_Interrupts(void) {
    EXTMODE = 0x0F;
    EXTPOLAR = 0x00;
    VICIntEnable = 0x0003C000;
    VICVectAddr14 = (unsigned long)eint0_irq;
    VICVectAddr15 = (unsigned long)eint1_irq;
    VICVectAddr16 = (unsigned long)eint2_irq;
    VICVectAddr17 = (unsigned long)eint3_irq;
}

void enable_timer(void) {
//     T0PR=11;                 // Set prescaler for 1 ms increments
//     T0MR0 = 3999999;                // Set match register for 4-second timeout
//     T0MCR = 3;                         // Interrupt and reset on MR0
//     T0TCR = 1;                         // Start Timer0
//     VICVectAddr4 = (unsigned long)Timer0_IRQHandler; // Set timer interrupt
//     VICIntEnable |= (1 << 4);          // Enable Timer0 interrupt
// 	
	int count3=0;
	
	T1MR0 = 1200 - 1;         				/* TC0 Match Value 0 */
	T1MCR = 3;                                /* TCO Interrupt and Reset on MR0 */
  T0TCR = 1;                                /* TC0 Enable */
	while(count3<8000){
	if (T0IR & (1 << 0)) {             /* Check if MR0 match occurred (0.1ms) */ 
					T0IR = (1 << 0);
					count3++;}
//if(button_press!=-1) break;
}
	if(count3==8000){ 
	 timeout_flag = 1; // Set timeout flag
	 count3=0;}
	T0TCR=0;

}
/* IRQ Handlers for Button Presses */
__irq void eint0_irq(void) {
    button_press = 0;
	
    EXTINT = 0x01;
    VICVectAddr = 0;
    T0TCR = 2; // Reset timer for next input
	// enable_timer();
}

__irq void eint1_irq(void) {
    button_press = 1;
    EXTINT = 0x02;
    VICVectAddr = 0;
    T0TCR = 2; // Reset timer for next input
	//  enable_timer();
}

__irq void eint2_irq(void) {
    button_press = 2;
    EXTINT = 0x04;
    VICVectAddr = 0;
    T0TCR = 2; // Reset timer for next input
	 // enable_timer();
}

__irq void eint3_irq(void) {
    button_press = 3;
    EXTINT = 0x08;
    VICVectAddr = 0;
    T0TCR = 2; // Reset timer for next input
	 // enable_timer();
}

/* Timer0 IRQ Handler for timeout */
// __irq void Timer0_IRQHandler(void) {
//     timeout_flag = 1; // Set timeout flag
//     T0IR = 1;         // Clear interrupt flag
//     VICVectAddr = 0;  // Acknowledge interrupt
//     T0TCR = 0;        // Stop Timer0 to avoid repeated timeouts
// }

int main(void) {
    int result;
    int round_complete;
    int score;
    char score_msg[16];
	

    while (1) {
				result = 0;
        round_complete = 0;
        score = 0;

        init_adc();
        read_adc();
        Generate_Sequence();
        sequence_length = 1;
        GPIO_Init();
        Enable_Interrupts();
        lcd_init();
        lcd_clear();
        lcd_print("Let's Start!! ");
        buzzer2();
        lcd_clear();

        while (1) {
            Play_Sequence();
            player_index = 0;
            round_complete = 0;
            timeout_flag = 0;
           // enable_timer();  // Start 4-second timer

            while (!round_complete) {
                if (timeout_flag) {
                    Reset_Game();
                    lcd_clear();
									  set_cursor(0,0);
                    lcd_print("Time Out!");
										set_cursor(0,1);
                    lcd_print("Game Over");
                    lcd_clear();
                    sprintf(score_msg, "Score: %d", score);
                    lcd_print(score_msg);
                    Delay(4000000);
                    break;
                }
                
                if (button_press != -1) {
                    result = Check_Player_Input(button_press);
                    Light_LED(button_press);
                    Delay(1000000);
                    Clear_LEDs();
                    button_press = -1;

                    if (result == 1) {
                        round_complete = 1;
                        score++;
                        sequence_length++;
                        lcd_clear();
                        sprintf(score_msg, "Score: %d", score);
                        lcd_print(score_msg);
                        Delay(2000000);
                    } else if (result == -1) {
                        Reset_Game();
                        lcd_clear();
                        lcd_print("Oops Game Over. ");
                        buzzer2();
                        lcd_clear();
                        sprintf(score_msg, "Score: %d", score);
                        lcd_print(score_msg);
                        Delay(4000000);
                        break;
                    }
                }
            }
            if (timeout_flag || result == -1) break;
        }
    }
}
