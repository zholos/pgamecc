#ifndef PGAMECC_UI_LAYER_H
#define PGAMECC_UI_LAYER_H

#include "controls.h"

#include <pgamecc/types.h>
#include <pgamecc/gl/font.h>

#include <list>
#include <map>
#include <memory>
#include <tuple>

namespace pgamecc {
namespace ui {

class Control;
class Primitives;

class Layer {
    std::list<std::unique_ptr<Control>> controls;

public:
    virtual ~Layer() = default;

    void insert(std::unique_ptr<Control>);

    template<typename T, typename... Args>
    std::enable_if_t<std::is_base_of<Control, T>::value, T*>
    create(Args&&... args) {
        auto& layer = *this;

        struct WithLayer : T, virtual detail::ControlLayer {
            WithLayer(Layer& layer, Args&&... args) :
                detail::ControlLayer(layer),
                T(std::forward<Args>(args)...) {}
        };

        T* control = new WithLayer(layer, std::forward<Args>(args)...);
        insert(std::unique_ptr<Control>{control});
        return control;
    }

    bool active = true;
    dvec2 size;
    double em = 20;

    dvec2 at(dvec2 at) const { return size * at; }

    virtual void step() {}
    virtual void layout() {}

    void step_controls();
    void layout_widgets();
    void render_widgets(Primitives&);

    bool input_key(bool press, int key, int mods);
};

// Persists as long as the OpenGL context.
// Caches OpenGL objects such as textures.
class Renderer {
    std::map<std::tuple<bool, int>, gl::Font> fonts;
    gl::Font& font(bool bold, double size); // size = em * scale

    gl::Program glyph_program, rect_program;

public:
    Renderer();
    void render(Layer&);

    friend class Primitives;
};

// Persists for one frame.
// Aggregates vertex data from multiple primitives for instanced rendering.
struct Primitives {
private:
    Renderer& renderer;
    glm::dmat4 projection;
    Primitives(Renderer&, ivec2 size);
    void flush();

    struct metrics_result {
        ivec2 p0, p1;
    };

public:
    gl::Font::line_metrics get_line_metrics(bool bold, double size) const;
    void glyphs(dvec2 position, std::u32string, bool bold, double size,
                color::sRGBA, bool center = false);
    void rect(dvec2 position, dvec2 size);

    friend class Renderer;
};


} // ui
} // pgamecc

#endif
