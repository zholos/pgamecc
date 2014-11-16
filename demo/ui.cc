#include <pgamecc/window.h>
#include <pgamecc/gl.h>
#include <pgamecc/ui.h>

using pgamecc::dvec2;

using namespace pgamecc::key;


class DemoLayer : public pgamecc::ui::Layer {
    pgamecc::ui::Axis* axis;
    pgamecc::ui::Label* label;
    pgamecc::ui::Button* button;

public:
    DemoLayer() {
        axis = create<pgamecc::ui::Axis>(key_down, key_up);

        label = create<pgamecc::ui::Label>();
        label->set_text("I'm a text box");

        button = create<pgamecc::ui::Button>();
        button->set_text("Energize");
    }

    void layout() override {
        label->place_at(dvec2(0, 0), dvec2(1, 1) * em);
        button->size = dvec2(10, 2) * em;
        button->place_at(dvec2(0, 1), at({0, 1}) + dvec2(1, -1) * em);
    }
};


class DemoWindow : public pgamecc::ui::WindowBase {
public:
    DemoWindow() {
        set_title("pgamecc demo");

        create_layer<DemoLayer>();
        create_layer<pgamecc::ui::WindowControlLayer>(*this);
    }
};


int main()
{
    return pgamecc::pgamecc_main<DemoWindow>();
}
