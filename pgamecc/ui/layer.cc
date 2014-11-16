#include "layer.h"

#include "widgets.h"

#include <pgamecc/fonts.h>

#include <algorithm>
#include <utility>

using std::forward_as_tuple;
using std::max;
using std::move;
using std::list;
using std::piecewise_construct;
using std::unique_ptr;

using namespace pgamecc::ui;


void
Layer::insert(unique_ptr<Control> control)
{
    controls.push_back(move(control));
}

void
Layer::step_controls()
{
    for (auto& c: controls)
        c->step();
}

void
Layer::layout_widgets()
{
    for (auto& c: controls)
        if (auto* w = dynamic_cast<Widget*>(c.get()))
            w->layout();
}

void
Layer::render_widgets(Primitives& p)
{
    for (auto& c: controls)
        if (auto* w = dynamic_cast<Widget*>(c.get()))
            w->render(p);
}

bool
Layer::input_key(bool press, int key, int mods)
{
    for (auto& c: controls)
        if (c->input_key(press, key, mods))
            return true;
    return false;
}



static const char* rect_vertex = R"(
    #version 330
    uniform mat4 transform;
    in vec4 p0p1;
    void main() {
        vec2 p0 = p0p1.xy, p1 = p0p1.zw;
        vec2 p = vec2(gl_VertexID % 2, gl_VertexID / 2);
        gl_Position = transform * vec4(mix(p0, p1, p), 0, 1);
    }
)";

static const char* rect_fragment = R"(
    #version 330
    uniform vec4 color = vec4(1);
    out vec4 fragColor;
    void main() {
        fragColor = color;
    }
)";


Renderer::Renderer() :
    glyph_program(gl::Font::simple_program()),
    rect_program(rect_vertex, rect_fragment)
{
}

pgamecc::gl::Font&
Renderer::font(bool bold, double size)
{
    // TODO: size-dependent em_texels
    int em_texels = 64;
    decltype(fonts)::key_type key{bold, em_texels};
    auto it = fonts.find(key);
    if (it == fonts.end())
        it = fonts.emplace(piecewise_construct, key, forward_as_tuple(
            pgamecc::fonts[bold ? "DejaVuSans-Bold.ttf" : "DejaVuSans.ttf"],
            em_texels)).first;
    return it->second;
}

void
Renderer::render(Layer& layer)
{
    glEnable(GL_BLEND); // for font rendering
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // TODO: restore settings later

    Primitives p{*this, layer.size};
    layer.render_widgets(p);
    p.flush();
}



Primitives::Primitives(Renderer& renderer, ivec2 size) :
    renderer(renderer),
    projection(glm::ortho(0., 0.+size.x, 0.+size.y, 0.))
{
}

pgamecc::gl::Font::line_metrics
Primitives::get_line_metrics(bool bold, double size) const
{
    return renderer.font(bold, size).get_line_metrics();
}


void
Primitives::glyphs(dvec2 position, std::u32string text, bool bold, double size,
                   color::sRGBA color, bool center)
{
    auto& font = renderer.font(bold, size);

    if (center) {
        double right = 0;
        for (auto m: font.prepare(text))
            right = max(right, m.p1.x);
        position.x -= right * size / 2;
    }

    auto M =
        glm::translate(dvec3(position, 0)) * glm::scale(dvec3(size, -size, 1));
    auto& program = renderer.glyph_program;
    program.use();
    program.uniform("transform").set(glm::mat4(projection * M));
    program.unuse();
    font.render(program, text);
}

void
Primitives::rect(dvec2 position, dvec2 size)
{
    auto& program = renderer.rect_program;
    program.use();
    program.uniform("transform").set(glm::mat4(projection));
    program.uniform("color").set(glm::vec4(1, .5, 0, 1));
    program.attrib("p0p1").set(glm::vec4(position, position + size));
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    program.unuse();
}

void
Primitives::flush()
{
}
