/*
 * keyboard.cpp
 *
 *  Created on: 2 Nov 2024
 *      Author: jole
 */

#include <iostream>
#include <fstream>
#include <string>

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <time.h>
#include <ctime>

#include "keymap.h"
#include "keyboard.h"

//struct termios orig_termios;


int addLogEntry(const std::string _fileName, const std::string _logEntry)
{
	// Specify the log file name
	//const std::string logFileName = "logfile.txt";

	// Open the log file in append mode
	std::ofstream logFile(_fileName, std::ios::app);

	// Check if the file opened successfully
	if (!logFile.is_open()) {
		std::cerr << "Error: Could not open log file." << std::endl;
		return 1;
	}

	// Optional: include a timestamp (requires <chrono> and <ctime>)
	std::time_t currentTime = std::time(nullptr);
	logFile << std::asctime(std::localtime(&currentTime)) << std::flush;


	// Append a message to the log file
	logFile << " - " << _logEntry << std::flush;

	// Close the file
	logFile.close();

	//std::cout << "Log entry added successfully." << std::endl;
	return 0;
}



void keyboard::die(const char* _s)
{
	perror(_s);
	exit(1);
}



void keyboard::disableRawMode(void)
{
	if (tcsetattr(fd, TCSAFLUSH, &originalTermios) == -1)
		die("tcsetattr");
}



void  keyboard::enableRawMode(void)
{
	if (tcgetattr(fd, &originalTermios) == -1)
		die("tcgetattr");

	//atexit(disableRawMode);

	struct termios raw = originalTermios;
	raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
	raw.c_oflag &= ~(OPOST);
	raw.c_cflag |= (CS8);
	raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
	raw.c_cc[VMIN] = 0;
	raw.c_cc[VTIME] = 1;
	if (tcsetattr(fd, TCSAFLUSH, &raw) == -1)
		die("tcsetattr");
}



// Function to check if a key has been pressed
bool keyboard::keyPressed(void)
{
	//addLogEntry("keyboard.log", "isKeyPressed()");

	struct timeval tv = {0, 0};
	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(fd, &readfds);

	return select(fd + 1, &readfds, nullptr, nullptr, &tv) == 1;
}



int keyboard::readkey(void)
{
	char	buf[4];
	//short	i = 0;

	char c = '\0';

	if(read(fd, &c, 1) == -1 && errno != EAGAIN)
		die("read");

	state = keyState::NORMAL;

	if(c != '\0'){
		if(iscntrl(c)){
			state = keyState::ESCAPE_SEQ_0;
			printf("Control - Level 1: %d\r\n", c);
			while(keyPressed()){
				if(read(fd, &buf[0], 1) > /*!=*/ 0){
					state = keyState::ESCAPE_SEQ_1;
					printf("Control - Level 2: %d\r\n", buf[0]);
					if(read(fd, &buf[1], 1) > /*!=*/ 0){
						state = keyState::ESCAPE_SEQ_2;
						printf("Control - Level 3: %d\r\n", buf[1]);
						if(read(fd, &buf[2], 1) > /*!=*/ 0){
							state = keyState::ESCAPE_SEQ_3;
							printf("Control - Level 4: %d\r\n", buf[2]);
						} // if(read(STDIN_FILENO, &buf[2], 1) != 0){
					} // if(read(STDIN_FILENO, &buf[1], 1) != 0)
				} // if(read(STDIN_FILENO, &buf[0], 1) != 0)
			} // while(keyPressed())
		}
		else{
			printf("Regular: %d ('%c')\r\n", c, c);
			return c;
		}
	}

	return c;

}



int main()
{
	int key;
	int file_descriptor = STDIN_FILENO;
	bool quit = false;

	keyboard kb(file_descriptor);

	while(!quit){
		key = kb.readkey();

		if(key != '\0'){
			switch(key){
				case 'q':	quit = true;
							break;

				default:	std::cout << "switch default: key: " << key << "\r\n" << std::flush;
							break;
			}
		}

		/*
		if(key == 'q')
			break;
		*/

	} // while(1)
  return 0;
}
