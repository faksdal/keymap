/*
 * keymap.cpp
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

struct termios orig_termios;


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



void die(const char *s) {
  perror(s);
  exit(1);
}



void disableRawMode() {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
    die("tcsetattr");
}



void enableRawMode() {
  if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) die("tcgetattr");
  atexit(disableRawMode);
  struct termios raw = orig_termios;
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  raw.c_oflag &= ~(OPOST);
  raw.c_cflag |= (CS8);
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}



// Function to check if a key has been pressed
bool isKeyPressed()
{
	addLogEntry("keyboard.log", "isKeyPressed()");
	struct timeval tv = {0, 0};
	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(STDIN_FILENO, &readfds);
	return select(STDIN_FILENO + 1, &readfds, nullptr, nullptr, &tv) == 1;
}


int main()
{
	char buf[4];

	enableRawMode();
	while(1){

		char c = '\0';

		if(read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN)
			die("read");
		if(c != '\0'){
			if (iscntrl(c)){
				//while(isKeyPressed()){
					printf("Control - Level 1: %d\r\n", c);
					while(isKeyPressed()){					// this might work!
					if(read(STDIN_FILENO, &buf[0], 1) != -1){
						printf("Control - Level 2: %d\r\n", buf[0]);
						if(read(STDIN_FILENO, &buf[1], 1) != -1){
							printf("Control - Level 3: %d\r\n", buf[1]);
							if(read(STDIN_FILENO, &buf[2], 1) != -1){
								printf("Control - Level 4: %d\r\n", buf[2]);
							}
						}
					}
				}
			}
		else{
			printf("Regular: %d ('%c')\r\n", c, c);
		}

			if (c == 'q') break;
		}
	} // while(1)

  return 0;
}
