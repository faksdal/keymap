#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

struct termios orig_termios;

void disableRawMode() {
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enableRawMode() {
  tcgetattr(STDIN_FILENO, &orig_termios);
  atexit(disableRawMode);

  struct termios raw = orig_termios;
  raw.c_iflag &= ~(ICRNL | IXON);
  raw.c_oflag &= ~(OPOST);
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int main() {
  enableRawMode();

  char c;
  int i=0;
  while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q') {

    if (iscntrl(c)) {
      printf("[ESC-SEQ]: %d\r\n", c);
      i = 0;
    } else {
      printf("   seq[%d]: %d ('%c')\r\n", i, c, c);
      i++;
    }
  }

  return 0;
}
