#ifndef PGAMECC_UI_WINDOW_H
#define PGAMECC_UI_WINDOW_H

#include <pgamecc/window.h>
#include <pgamecc/ui/layer.h>

#include <memory>

namespace pgamecc {
namespace ui {

// Specialization that distributes events to and handles rendering of multiple
// UI layers.

class WindowBase : public pgamecc::WindowBase {
    std::list<std::unique_ptr<Layer>> layers;
    std::unique_ptr<pgamecc::ui::Renderer> renderer;

public:
    void insert_layer(std::unique_ptr<Layer>);

    template<typename T, typename... Args>
    std::enable_if_t<std::is_base_of<Layer, T>::value, T*>
    create_layer(Args&&... args) {
        T* layer = new T(std::forward<Args>(args)...);
        insert_layer(std::unique_ptr<Layer>{layer});
        return layer;
    }

    void input_key(bool press, int key, int mods) override final;
    void input_char(char32_t codepoint) override final;
    void input_button(bool press, int button, int mods) override final;
    void input_cursor(dvec2 position) override final;
    void input_mouse(dvec2 relative) override final;
    void resized() override final;

    void step() override final;

    void render_init() override final;
    void render_done() override final;
    void render() override final;

    // Background layer is like an ordinary WindowBase.
    virtual void background_input_key(bool press, int key, int mods) {}
    virtual void background_step() {}
    virtual void background_render_init() {}
    virtual void background_render_done() {}
    virtual void background_render();
};


struct WindowControlLayer : Layer {
    WindowControlLayer(pgamecc::WindowBase&);
};


} // ui
} // pgamecc

#endif
