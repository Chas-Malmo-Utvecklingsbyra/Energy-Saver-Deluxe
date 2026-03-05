#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <termios.h>
#include <functional>
#include <ostream>

enum class Keyboard_Code
{
    ENTER = 10,

    ZERO = 48,
    ONE,
    TWO,
    THREE,
    FOUR,
    FIVE,
    SIX,
    SEVEN,
    EIGHT,
    NINE,

    a = 97,
    b,
    c,
    d,
    e,
    f,
    g,
    h,
    i,
    j,
    k,
    l,
    m,
    n,
    o,
    p,
    q,
    r,
    s,
    t,
    u,
    v,
    w,
    x,
    y,
    z,

    A = 65,
    B,
    C,
    D,
    E,
    F,
    G,
    H,
    I,
    J,
    K,
    L,
    M,
    N,
    O,
    P,
    Q,
    R,
    S,
    T,
    U,
    V,
    W,
    X,
    Y,
    Z,

    ARROW_UP,
    ARROW_RIGHT,
    ARROW_DOWN,
    ARROW_LEFT,
};
std::ostream& operator<<(std::ostream& os, Keyboard_Code code);

class Keyboard
{
private:
    struct termios orig_termios;
    bool force_close;
    bool is_active;
    void Close();
    void Start();

public:
    ~Keyboard();
    Keyboard();
    void ForceClose();
    void Toggle();

    // Reads pressed Keys from the keyboard
    void Read(std::function<bool(Keyboard_Code)> callback);
};

#endif