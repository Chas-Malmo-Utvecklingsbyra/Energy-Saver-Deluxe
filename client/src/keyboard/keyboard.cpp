#include "keyboard.h"

#include <unistd.h>
#include <iostream>

void Keyboard::Start()
{
    struct termios raw;
    tcgetattr(STDIN_FILENO, &this->orig_termios);
    raw = *(&orig_termios);
    raw.c_lflag &= ~(ECHO | ICANON);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

Keyboard::Keyboard()
{
    this->force_close = false;
    this->is_active = true;
    Start();
}

void Keyboard::Read(std::function<bool(Keyboard_Code)> callback)
{
    char c;
    while (read(STDIN_FILENO, &c, 1) == 1 ) {
        if (c == '\x1b')
        {
            char seq[2];
            read(STDIN_FILENO, &seq[0], 1);
            read(STDIN_FILENO, &seq[1], 1);

            if (seq[0] == '[')
            {
                switch (seq[1])
                {
                    case 'A': if (!callback(Keyboard_Code::ARROW_UP)) break; break;
                    case 'B': if (!callback(Keyboard_Code::ARROW_DOWN)) break; break;
                    case 'C': if (!callback(Keyboard_Code::ARROW_RIGHT)) break; break;
                    case 'D': if (!callback(Keyboard_Code::ARROW_LEFT)) break; break;
                }
            }
            continue;
        }

        if (!callback((Keyboard_Code)c)) break;
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

void Keyboard::Toggle()
{
    if (is_active)
        Close();
    else
        Start();
    
    is_active = !is_active;
}

std::ostream& operator<<(std::ostream& os, Keyboard_Code code)
{
    switch (code)
    {
        case Keyboard_Code::ARROW_DOWN:
        {
            os << "ARROW_DOWN";
            break;
        }
        case Keyboard_Code::ARROW_LEFT:
        {
            os << "ARROW_LEFT";
            break;
        }
        case Keyboard_Code::ARROW_RIGHT:
        {
            os << "ARROW_RIGHT";
            break;
        }
        case Keyboard_Code::ARROW_UP:
        {
            os << "ARROW_UP";
            break;
        }
        default:
        {
            os << (char)code;
            break;
        }
    }

    return os;
}