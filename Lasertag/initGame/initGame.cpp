#include "initGame.hpp"

initGame::initGame(displayTask& displayControl, ir_transmitter & transmitterControl):
	task("initGameTask"),
	keypadChannel(this, "keyPadChannel"),
	displayControl(displayControl),
	initTimer(this, "initTimer"),
	transmitterControl(transmitterControl)
	{}

void initGame::buttonPressed(const char c){
	displayControl.showTime(time);
	displayControl.showCommand(c);
	displayControl.showState(s);
	keypadChannel.write(c);
};


void initGame::main() {
	//enum voor de states. 
	enum state_t {IDLE, ADJUST_TIME, BUTTON_PRESSED_TWO, SEND_IR_TIME, START_GAME};
	state_t state = IDLE;
	buttonPressed(' ');
	//de loop
	while(1){
		switch(state){
		case IDLE:														//IDLE state  (program starts here
		{
			s = 1;
			wait(keypadChannel);
			temp = keypadChannel.read();
			//hwlib::cout << "character read from channel: " << temp << '\n';
			if (temp == 'C'){											//Wait for a C character. 
				//s=4;
				state= ADJUST_TIME; 
				
				
				break; 
			}
			//break;
		}
		case ADJUST_TIME: 												// Waits for time from keypad input.  
		{
			hwlib::cout << "state: adjust_time " << '\n';
			s = 4;
			initTimer.set(15'000'000);
			auto event = wait(initTimer + keypadChannel);
			if (event == keypadChannel){
				//hwlib::cout << "state Adjust time, event == keypadChannel \n";
				minute_tens = int(keypadChannel.read() - 48);			// Char to int minus the assci value gives a number between 0 and 9 
				hwlib::cout << "min_tiental: " << minute_tens << '\n';
				if (minute_tens >= 0 && minute_tens < 3)			//Checks for right input
				{
					time= (minute_tens *= 10);								//Makes the first input 10 times bigger because the first given nummer represents time per 10 minuts
					//time = minute_tens;
					hwlib::cout << "minute_tens: " << minute_tens << '\n';
					//displayControl.showTime(minute_tens); 
					hwlib::wait_ms(500);
					s=5;
					state = BUTTON_PRESSED_TWO; 
					break;
					
				}else{
					hwlib::cout << "Input a number.\n";
					s=6;
					hwlib::wait_ms(2000);
					state = ADJUST_TIME;
					break;
				}
				//break;
				
			}
			else if (event == initTimer){ 							// When the timer and the input is timedout and program returns to IDLE
				hwlib::cout << "returning to IDLE. \n";
				state=IDLE;
				break;
			}
			//break;
		}
		case BUTTON_PRESSED_TWO: 									// This state waits for the second time input in minutes 
		{
			s=5;
 			hwlib::cout << "state: Button_pressed_two \n";					
			auto event = wait(initTimer + keypadChannel);
			if(event == keypadChannel){								//checks for write input
				min = int(keypadChannel.read() - 48); 				// Char to int minus the assci value gives a number between 0 and 9 
				if (min >= 0 && min <10)
				{
					time = minute_tens + min; 					//adds the two given time variabeles togheter 
					if(time <= 31 && time >0){						
						hwlib::cout <<"time: " << time << '\n';
						//displayControl.showTime(time);
						s=3; 
						state= SEND_IR_TIME;
						break; 
					}else{
						hwlib::cout << "Time to big(over 31), returning to ADJUST_TIME.\n";
						s=6;
						hwlib::wait_ms(2000);
						state = ADJUST_TIME;
						break;
					}
				}//break; 
			}
			else if (event == initTimer) 							// When the timer and the input is timedout and program returns to IDLE
			{
				s = 1;
				state=IDLE;
				break;
			} 
			//break; 
		}
		case SEND_IR_TIME:										//Sends the time to players when # pressed
			{
				s = 2;
				hwlib::cout << "state = SEND_IR_TIME\n";
				wait(keypadChannel);
				auto given= (keypadChannel.read());
				if (given=='#')
				{
					hwlib::cout << "sending time.\n" <<time;
					transmitterControl.send(0,time);
					break;
				}
				else if (given=='*')						//If * is pressed go to startgame state
				{
					transmitterControl.send(0,31); 
					hwlib::cout << "state = start_game.\n";
					state= START_GAME; 
					break; 
				}
				else if(given=='D')
				{
					hwlib::cout << "returning to IDLE";
					//displayControl.showTime(0); 
					state = IDLE;
					
					break;
				}else{
					break;
				}
			}
			
		case START_GAME:									//State to start the game
			{
				//transmitterControl.send(0,31); 
				s = 3;
				hwlib::cout << "state = START_GAME\n";
				wait(keypadChannel);
				auto in = (keypadChannel.read()); 
				if(in=='*')				// if * is pressed the start message will be send again. 
				{
					hwlib::cout << "sending start_message...\n";
					transmitterControl.send(0,31);
					//hwlib::wait_ms(500);
					break;
				}
				else if(in=='D')					// With D pressed go back to IDLE
				{
					hwlib::cout << "D pressed. returning to IDLE.\n";
					displayControl.showTime(0); 
					state=IDLE; 
					break; 
				}
				
				else{
					s = 9;
					break;
				}
			}
		}
	}
}
