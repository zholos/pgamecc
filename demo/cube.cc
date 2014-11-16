#include <pgamecc/window.h>
#include <pgamecc/gl.h>

#include <algorithm>
#include <memory>

#include <glm/gtx/transform.hpp>

using std::min;
using std::max;
using std::unique_ptr;

using namespace pgamecc::key;


const char* vertex = R"(
    #version 330

    uniform mat4 M;
    uniform mat4 V;
    uniform mat4 P;
    layout(location = 0) in vec4 position;

    out vec4 t;
    out vec4 p;

    void main() {
        t =         M * position;
        p = P * V * M * position;
        gl_Position = p;
    }
)";

const char* fragment = R"(
    #version 330

    in vec4 t;
    in vec4 p;
    out vec4 fragColor;

    uniform vec4 c;

    const float border = .01;

    void main() {
        float b = border * pow(p.w, .75);
        vec4 u = step(1 - b, fract(t - .5*b));
        fragColor = c + (1 - c) * step(2, u.x + u.y + u.z) * .25;
    }
)";


class DemoWindow : public pgamecc::WindowBase {
public:
    struct Renderer {
        pgamecc::gl::Program program;
        pgamecc::gl::Array<glm::vec4> cube_array;

        Renderer() :
            program(vertex, fragment)
        {
            cube_array.load(pgamecc::gl::cube_strip);
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);
            //glEnable(GL_MULTISAMPLE); // seems to be automatic
        }

        void render(pgamecc::ivec2 size, int angle) {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            program.use();
            program.uniform("M").set(glm::mat4());
            program.uniform("V").set(
                glm::translate(glm::vec3(0, 0, -5)) *
                glm::rotate(glm::radians((float)angle), glm::vec3(1, 3, 2)) *
                glm::scale(glm::vec3(2, 2, 2)) *
                glm::translate(glm::vec3(-.5, -.5, -.5)));
            program.uniform("P").set(
                glm::perspective(glm::radians(60.f),
                                 (float)size.x / size.y, 1.f, 1000.f));
            program.uniform("c").set(glm::vec4(1, .5, 0, 0));
            program.attrib(0).array(cube_array);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 14);
            program.attrib(0).unarray();
            program.unuse();
        }
    };
    unique_ptr<Renderer> renderer;

    int angle = 0, speed = 1;

    DemoWindow() {
        set_title("pgamecc demo");
        set_samples(4);
    }

    void step() {
        angle = (angle + speed) % 360;
    }

    void render_init() { renderer.reset(new Renderer); }
    void render_done() { renderer.reset(); }
    void render() { renderer->render(size(), angle); }

    void input_key(bool press, int key, int mods) {
        bool ctrl = mods & mod_control,
              alt = mods & mod_alt;

        if (press) {
            if (key == key_escape || ctrl && key == 'Q' || key == key_close)
                quit();
            if (alt && key == key_enter || key == key_f11)
                fullscreen();
            if (key == key_right)
                speed = min(speed + 1, 10);
            if (key == key_left)
                speed = max(speed - 1, -10);
        }
    }
};


int main()
{
    return pgamecc::pgamecc_main<DemoWindow>();
}
