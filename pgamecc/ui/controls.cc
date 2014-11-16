#include "controls.h"

#include <stdexcept>

using std::logic_error;
using std::min;

using namespace pgamecc::ui;


detail::ControlLayer::ControlLayer() :
    layer(*(Layer*)nullptr)
{
    throw std::logic_error("don't create ui::Control directly");
}


bool
Release::input_key(bool press, int key, int mods)
{
    if (key == bound_key && (mods & bound_mods) == bound_mods) {
        if (press)
            presses = min(100, presses + 1);
        return true;
    }
    return false;
}

int
Release::value()
{
    if (presses) {
        presses--;
        return 1;
    } else
        return 0;
}

void
Release::step()
{
    if (callback && value())
        callback();
}


bool
Trigger::input_key(bool press, int key, int mods)
{
    if (key == bound_key) {
        state = press;
        return true;
    }
    return false;
}

int
Trigger::value()
{
    return state;
}


bool
Axis::input_key(bool press, int key, int mods)
{
    if (key == minus_key || key == plus_key) {
        int sign = key == minus_key ? -1 : 1;
        if (press)
            state = sign;
        else {
            if (state == sign)
                state = 0;
        }
        return true;
    }
    return false;
}

int
Axis::value()
{
    return state;
}
