#include <stdlib.h>
#include "./drivers/inc/ISRs.h"
#include "./drivers/inc/LEDs.h"
#include "./drivers/inc/vga.h"
#include "./drivers/inc/audio.h"
#include "./drivers/inc/HPS_TIM.h"
#include "./drivers/inc/int_setup.h"
#include "./drivers/inc/wavetable.h"
#include "./drivers/inc/pushbuttons.h"
#include "./drivers/inc/ps2_keyboard.h"
#include "./drivers/inc/HEX_displays.h"
#include "./drivers/inc/slider_switches.h"

//declare global variables
float frequencies[] = {130.813, 146.832, 164.814, 174.614, 195.998, 220.000, 246.942, 261.626};
float amplitude =1;
char keyReleased = 0;
char keysPressed[8] = {};
double oldValues[320] = {0};
double newValue = 0;
int t = 0;

//declare functions
void readKB();
float getSignal(float f, int t);
double makeWave();
void drawWave(double signal);

int main() {
	int_setup(1, (int []) {199}); //Enable interupts for push buttons and hps timer 0
	
	//declare variables
	double signal = 0;;

	//set up timer for wave feeder
	HPS_TIM_config_t timer; 	//create the structure element
	timer.tim = TIM0;			//set tim to the first enum of the timers
	timer.timeout = 20.83;		//set the timeout (in usec) T = 1/48k =0.0000208s = 20us ish
	timer.LD_en = 1;			//set the flags
	timer.INT_en = 1;			//make timer start immediately
	timer.enable = 1;
	HPS_TIM_config_ASM(&timer);

	while(1) {
		//get pressed keys
		readKB();

		//make signal according to key's frequencies
		signal = makeWave(); 
		
		//draw the wave
		drawWave(signal);
		
	}
	return 0;
}

void drawWave(double signalSum){
	int x = (t/10)%320;
	int y = 120 + signalSum/500000;
	if((t%10 == 0)){ //only draw one point out of 10 samples
		VGA_draw_point_ASM(x, oldValues[x], 0); //clear previous wave at that point
		oldValues[x] = y; //save new value to old array
		VGA_draw_point_ASM(x, y, 63); //write wave value at that point		
	}
}


float getSignal(float f, int t){
	int index = ((int)(f*t)) % 48000; //keep whole values for index
	float interpolation = (f*t) - (int)(f*t); //keep decimal values for interpolation
	float signal = amplitude * ((1.0 - interpolation)*sine[index] + (interpolation)*sine[index+1]); //calculate with interpolation
	//float signal = amplitude * sine[index]; //calculate without interpolation
	return signal;	
}


double makeWave() {
	int i;
	double sample = 0;
	for(i = 0; i < (sizeof(keysPressed)/sizeof(keysPressed[0])); i++){ //go through all the keys
		if(keysPressed[i] == 1){ //if key we are checking is pressed
			sample += getSignal(frequencies[i],t);//sum all the samples of different keys together
		}
	}
	if(hps_tim0_int_flag == 1) { //check if timer is done
		hps_tim0_int_flag = 0; //reset flag
		audio_write_data_ASM(sample, sample); //write audio
		t++; //increment sample nb
		if(t>48000){
			t = 0; //reset t when over max sample
		}
	}
	return sample;
}

void readKB(){
	char value;
	if (read_ps2_data_ASM(&value)) { //there is valid data, then it means a key is pressed
		switch (value){ //compare the key code to identify pressed key
		case 0x1C: //A
		if(keyReleased == 1){ //if the key is getting released
			keysPressed[0] = 0; //update keyPressed array
			keyReleased = 0; //reset flag
		} else{
			keysPressed[0] = 1; //update array to show key is pressed
			VGA_write_char_ASM(15,15,'C'); //write the note we are playing to the screen
		}
		break;
		
		case 0x1B: //S
		if(keyReleased == 1){
			keysPressed[1] = 0;
			keyReleased = 0;
		} else{
			keysPressed[1] = 1;
			VGA_write_char_ASM(15,15,'D');
		}
		break;
		
		case 0x23: //D
		if(keyReleased == 1){
			keysPressed[2] = 0;
			keyReleased = 0;
		} else{
			keysPressed[2] = 1;
			VGA_write_char_ASM(15,15,'E');
		}
		break;
		
		case 0x2B: //F
		if(keyReleased == 1){
			keysPressed[3] = 0;
			keyReleased = 0;
		} else{
			keysPressed[3] = 1;
			VGA_write_char_ASM(15,15,'F');
		}
		break;
		
		case 0x3B: //J
		if(keyReleased == 1){
			keysPressed[4] = 0;
			keyReleased = 0;
		} else{
			keysPressed[4] = 1;
			VGA_write_char_ASM(15,15,'G');
		}
		break;
		
		case 0x42: //K
		if(keyReleased == 1){
			keysPressed[5] = 0;
			keyReleased = 0;
		} else{
			keysPressed[5] = 1;
			VGA_write_char_ASM(15,15,'A');
		}
		break;
		
		case 0x4B: //L
		if(keyReleased == 1){
			keysPressed[6] = 0;
			keyReleased = 0;
		} else{
			keysPressed[6] = 1;
			VGA_write_char_ASM(15,15,'B');
		}
		break;

		case 0x4C: //;
		if(keyReleased == 1){
			keysPressed[7] = 0;
			keyReleased = 0;
		}else{
			keysPressed[7] = 1;
			VGA_write_char_ASM(15,15,'C');
		}
		break;

		case 0x41: //,
		amplitude -=0.5; //change volume down
		keyReleased = 0; //reset flag
		break;	

		case 0x49: //.
		amplitude +=0.5; //change volume up
		keyReleased = 0; //reset flag
		break;

		case 0xF0: //code from PS2 when a key is released
		keyReleased = 1; //flag toggled
		break;

		default:
		keyReleased = 0;
		}
	}
	if (amplitude < 0){ //min
		amplitude = 0;
	}
	else if(amplitude > 10){ //max
		amplitude = 10;
	}
}


