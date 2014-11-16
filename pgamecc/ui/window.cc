#include "window.h"

#include <utility>

#include <boost/range/adaptors.hpp>

using std::move;
using std::unique_ptr;

using namespace pgamecc::ui;


void
WindowBase::insert_layer(unique_ptr<Layer> layer)
{
    layers.push_back(move(layer));
}

void
WindowBase::step()
{
    for (auto& layer: layers)
        if (layer->active) {
            // set size here in case new layers added after resized()
            layer->size = size();
            layer->step();
            layer->step_controls();
            layer->layout();
            layer->layout_widgets();
        }
    background_step();
}

void
WindowBase::input_key(bool press, int key, int mods)
{
    for (auto& layer: layers | boost::adaptors::reversed)
        if (layer->active && layer->input_key(press, key, mods))
            return;
    background_input_key(press, key, mods);
}

void
WindowBase::input_char(char32_t codepoint)
{
}

void
WindowBase::input_button(bool press, int button, int mods)
{
}

void
WindowBase::input_cursor(dvec2 position)
{
}

void
WindowBase::input_mouse(dvec2 relative)
{
}

void
WindowBase::resized()
{
}

void
WindowBase::render_init()
{
    background_render_init();
    renderer.reset(new Renderer);
}

void
WindowBase::render_done()
{
    background_render_done();
    renderer.reset();
}

void
WindowBase::render()
{
    background_render();
    for (auto& layer: layers)
        if (layer->active)
            renderer->render(*layer);
}

void
WindowBase::background_render()
{
    glClear(GL_COLOR_BUFFER_BIT);
}



WindowControlLayer::WindowControlLayer(pgamecc::WindowBase& window)
{
    using namespace pgamecc::key;

    auto quit = [&] { window.quit(); };
    create<Release>(key_close)->bind(quit);
    create<Release>(key_escape)->bind(quit);
    create<Release>('Q', mod_control)->bind(quit);

    auto fullscreen = [&] { window.fullscreen(); };
    create<Release>(key_enter, mod_alt)->bind(fullscreen);
    create<Release>(key_f11)->bind(fullscreen);
}
