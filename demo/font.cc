#include <pgamecc/window.h>
#include <pgamecc/gl.h>
#include <pgamecc/fonts.h>

#include <memory>
#include <string>
#include <vector>

using std::string;
using std::unique_ptr;
using std::vector;

using namespace std::string_literals;

using namespace pgamecc::key;


const char* vertex = R"(
    #version 330

    uniform mat4 V;
    uniform mat4 P;

    in vec4 position;
    in vec2 offset;
    in vec4 glyph; // p0=.xy p1=.zw

    out vec2 t;

    void main() {
        t = mix(glyph.xy, glyph.zw, position.xy);
        gl_Position = P * V *
            vec4(position.xy * (glyph.zw - glyph.xy) + offset, 0, 1);
    }
)";

const char* fragment = R"(
    #version 330

    uniform sampler2D font;
    uniform vec4 c0, c1;

    in vec2 t;

    out vec4 fragColor;

    void main() {
        fragColor = mix(c0, c1, texture(font, t).r);
    }
)";


class DemoWindow : public pgamecc::WindowBase {
public:
    struct Renderer {
        pgamecc::gl::Font font;
        pgamecc::gl::Program program;
        pgamecc::gl::Sampler nearest;

        Renderer() :
            font(pgamecc::fonts["DejaVuSans.ttf"], 64),
            program(pgamecc::gl::Font::simple_program())
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glSamplerParameteri(nearest.sampler(),
                                GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        }

        void render(pgamecc::ivec2 size) {
            glClear(GL_COLOR_BUFFER_BIT);

            auto text =
                U"Hello from pgamecc!\nWoo — kerning.\nИ даже по-русски."s;

            auto aspect = (double)size.x / size.y;
            auto P = glm::ortho(-1., 1., -1/aspect, 1/aspect);
            auto V = glm::scale(glm::dvec3(.07));

            program.use();
            program.uniform("background").set(glm::vec4());
            program.uniform("color").set(glm::vec4(1, .5, 0, 1));
            program.uniform("transform").set(glm::mat4(P * V));
            program.unuse();
            font.render(program, text);

            // show entire font texture on the left for debugging
            program.use();
            font.bind_texture(0);
            nearest.bind(0);
            program.uniform("background").set(glm::vec4(0, .5, 0, 1));
            program.uniform("color").set(glm::vec4(1, 1, 1, 1));
            program.uniform("transform").set(glm::mat4(
                P * glm::translate(glm::dvec3(-1, -.5, 0))));
            program.attrib("t0t1").uninstanced().set(glm::vec4(0, 0, 1, 1));
            program.attrib("p0p1").uninstanced().set(glm::vec4(0, 0, 1, 1));
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            nearest.unbind(0);
            font.unbind_texture(0);
            program.unuse();
        }
    };
    unique_ptr<Renderer> renderer;

    DemoWindow() {
        set_title("pgamecc demo");
        set_samples(4);
    }

    void render_init() { renderer.reset(new Renderer); }
    void render_done() { renderer.reset(); }
    void render() { renderer->render(size()); }

    void input_key(bool press, int key, int mods) {
        bool ctrl = mods & mod_control,
              alt = mods & mod_alt;

        if (press) {
            if (key == key_escape || ctrl && key == 'Q' || key == key_close)
                quit();
            if (alt && key == key_enter || key == key_f11)
                fullscreen();
        }
    }
};


int main()
{
    return pgamecc::pgamecc_main<DemoWindow>();
}
