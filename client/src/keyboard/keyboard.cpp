#include "keyboard.h"

#include <unistd.h>
#include <pthread.h>

Keyboard::Keyboard()
{
    this->force_close = false;

    struct termios raw;
    tcgetattr(STDIN_FILENO, &this->orig_termios);
    raw = *(&orig_termios);
    raw.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void Keyboard::Read(std::function<bool(char)> callback)
{
    char c;
    while (read(STDIN_FILENO, &c, 1) == 1 ) {
        if (!callback(c)) break;
    }
}

void Keyboard::Close()
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &this->orig_termios);
}

void Keyboard::ForceClose()
{
    this->force_close = true;
    Close();
}

Keyboard::~Keyboard()
{
    if (!force_close)
        Close();
}