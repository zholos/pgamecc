#ifndef PGAMECC_UI_CONTROLS_H
#define PGAMECC_UI_CONTROLS_H

#include <pgamecc/window.h>

#include <functional>

namespace pgamecc {
namespace ui {

class Layer;

namespace detail {
struct ControlLayer {
    ControlLayer(); // should not be called
    ControlLayer(Layer& layer) : layer(layer) {}
    Layer& layer;
};
}

struct Control : virtual detail::ControlLayer {
    virtual ~Control() = default;
    virtual void step() {}
    virtual bool input_key(bool press, int key, int mods) { return false; }
};


struct Input : Control {
    virtual int value() { return 0; }
};

struct Release : Input {
    int bound_key, bound_mods;
    int presses = 0;
    std::function<void()> callback;

public:
    Release(int key, int mods = 0) : bound_key(key), bound_mods(mods) {}
    void bind(std::function<void()> f) { callback = f; }

    bool input_key(bool press, int key, int mods) override;
    int value() override;
    void step() override;
};

struct Trigger : Input {
    int bound_key;
    int state = 0;

public:
    Trigger(int key) : bound_key(key) {}

    bool input_key(bool press, int key, int mods) override;
    int value() override;
};

struct Axis : Input {
    int minus_key, plus_key;
    int state = 0;

public:
    Axis(int minus_key, int plus_key) :
        minus_key(minus_key), plus_key(plus_key) {}
    bool input_key(bool press, int key, int mods) override;
    int value() override;
};


} // ui
} // pgamecc

#endif
