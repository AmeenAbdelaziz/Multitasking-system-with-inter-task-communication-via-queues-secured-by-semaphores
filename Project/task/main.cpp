 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>

// Libraries
#include "LCD.h"
#include "DigiPort.h"
#include "OSKernel.h"
#include "Timer.h"
#include "BoundedQueue.h"
#include "BinarySemaphor.h"


volatile uint16_t counter1 = 0;
volatile uint16_t counter2 = 0;

volatile uint16_t counter1_limit = 0;
volatile uint16_t counter2_limit = 0;

volatile uint8_t task_id [6];							// array that holds task_ids
volatile uint8_t task_toggle_request[4] = {0,0,0,0};

uint8_t m = 0;
uint8_t n = 0;
uint8_t p = 0; 
uint8_t q = 0;

char string_1 [] = "1Aufgabe";
char string_2 [] = "2Aufgabe";
char string_3 [] = "3Aufgabe";
char string_4 [] = "4Aufgabe";

 

// Function declarations
void Viewer_1	(void);									
void Viewer_2	(void);									
void Task_1		(void);									
void Task_2	    (void);								
void Task_3		(void);									
void Task_4		(void);									
void Task_read_keys(void);								// Keys einlesen
void Blinker_1  ();										// Timer ISR 1
void Blinker_2  ();										// Timer ISR 2

void string_write_loop(char *input, BoundedQueue *pointer);	// Funktion zum Schreiben der Strings im Loop
void toggle_task (uint8_t task_id);

// Initializations
LCD mein_display   (PC, LCD_Type_40x4, DISPLAY_ON | CURSOR_ON | BLINK_ON | WRAPPING_ON);
DigiPortIRPT keys(PK, SET_IN_PORT);
DigiPortRaw	 leds   (PA, SET_OUT_PORT);
Timer16	tick_1	    (TC1, Blinker_1);
Timer16 tick_2	    (TC3, Blinker_2);

BinarySemaphor semaphor1;
BinarySemaphor semaphor2;
BoundedQueue   queue1;
BoundedQueue   queue2;
int main(void)
{
	task_id [0] = task_insert(Task_1,High);
	task_id [1] = task_insert(Task_2,Medium);
	task_id [2] = task_insert(Task_3,Low);
	task_id [3] = task_insert(Task_4,Low);
	task_id [4] = task_insert(Viewer_1);
	task_id [5] = task_insert(Viewer_2);
	
	task_insert(Task_read_keys);
	
	tick_1.start_ms(100);
	tick_2.start_ms(200);
	
	kernel();
	
}

// Implementation for the first task
void Task_1() {
	do {
		semaphor1.wait_aquire();
		queue1.write('1');
		string_write_loop(string_1, &queue1);
		semaphor1.release();
		if (task_toggle_request[0]){toggle_task(0);}
		yield();
		
	} while(1);
}


// Implementation for the second task
void Task_2() {
	do {
		semaphor1.wait_aquire();
		queue1.write('2');
		string_write_loop(string_2, &queue1);
		semaphor1.release();
		if (task_toggle_request[1]){toggle_task(1);}
		yield();
		
	} while(1);
}

// Implementation for the third task
void Task_3() {
	do {
		semaphor2.wait_aquire();
		queue2.write('3');
		string_write_loop(string_3, &queue2);
		semaphor2.release();
		if (task_toggle_request[2]){toggle_task(2);}
		yield();
		
	} while(1);
}

// Implementation for the fourth task
void Task_4() {
	do {
		semaphor2.wait_aquire();
		queue2.write('4');
		string_write_loop(string_4, &queue2);
		semaphor2.release();
		if (task_toggle_request[3]){toggle_task(3);}
		yield();
		
	} while(1);
}


void Viewer_1 () {				
	
	do {
		char c = 0;
		while(queue1.get_used_size()) {
			c = queue1.read();
			if(c == '1') {
				m = 0;
				n = 0;
			} else if (c == '2') {
				m = 1;
				n = 0;
			}
			mein_display.set_pos(m,n);
			mein_display.write_char(c);
			n++;			
		}
		yield();		
	} while(1);
}


void Viewer_2 () {					
	do {
		char c = 0;
		while(queue2.get_used_size()) {
			c = queue2.read();
			
			
			if(c == '3') {
				p = 2;
				q = 0;
				} else if (c == '4') {
				p = 3;
				q = 0;
				} 				
			mein_display.set_pos(p,q);
			mein_display.write_char(c);
			q++;			
		}
		yield();
	} while(1);
}

void Task_read_keys()
{
	while (1){
		switch(keys.wait_falling_edge()){			//Keys einlesen
			case 0b00000001: toggle_task(0); break;
			case 0b00000010: toggle_task(1); break;
			case 0b00000100: toggle_task(2); break;
			case 0b00001000: toggle_task(3); break;
			case 0b00010000: counter1_limit += 2; break;
			case 0b00100000: if (counter1_limit > 0) counter1_limit -= 2; break;
			case 0b01000000: counter2_limit += 2; break;
			case 0b10000000: if (counter2_limit > 0) counter2_limit -= 2; break;
			default:;
		}
		yield();
	}
}


void string_write_loop (char *input, BoundedQueue *pointer) {
	uint8_t i = 0;
	while(i < strlen(input) && pointer -> write(input[i])){
			i++;
			yield();
	} 
}


void toggle_task (uint8_t task_nr)
{
	if (!is_active(task_id[task_nr]))	{											
		activate(task_id[task_nr]);									
	}else if (task_toggle_request[task_nr] == true){
		deactivate(task_id[task_nr]);
		task_toggle_request[task_nr] = false;
	}else if (task_toggle_request[task_nr] == false){
		task_toggle_request[task_nr] = true;
	}							
}

// Callback function for the Timer 1:
void Blinker_1() {
	if (counter1 < counter1_limit) counter1++;
	
	if(counter1 >= counter1_limit) {
		leds.toggle(0b00001111);
		counter1 = 0;
	}
}

// Callback function for the Timer 2:
void Blinker_2() {
	if (counter2 < counter2_limit) counter2++;
	
	if(counter2 >= counter2_limit) {
		leds.toggle(0b11110000);
		counter2 = 0;
	}
}