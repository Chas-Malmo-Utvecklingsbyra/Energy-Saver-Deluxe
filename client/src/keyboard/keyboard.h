#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <termios.h>
#include <functional>

class Keyboard
{
private:
    struct termios orig_termios;
    bool force_close;
    void Close();

public:
    ~Keyboard();
    Keyboard();
    void ForceClose();

    // Reads pressed Keys from the keyboard
    void Read(std::function<bool(char)> callback);
};

#endif