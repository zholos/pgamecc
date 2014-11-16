#include <pgamecc.h>

#include <memory>

#include <glm/gtx/transform.hpp>

using std::unique_ptr;

using namespace pgamecc;
using namespace pgamecc::key;


const char* vertex = R"(
    #version 330

    uniform mat4 V;
    uniform mat4 P;

    layout(location = 0) in vec4 position;

    out vec4 p;
    out vec2 t;

    void main() {
        t = vec2(position);
        p = P * V * position;
        gl_Position = p;
    }
)";

const char* fragment = R"(
    #version 330

    uniform sampler2D planet;

    in vec4 p;
    in vec2 t;
    out vec4 fragColor;

    void main() {
        vec2 u = t*2-1;
        if (length(u) > 1)
            discard;

        vec2 v = u.yx;
        vec2 a1 = u*u - v*v + 2;
        vec2 a2 = a1.yx;
        vec2 x = u / sqrt(a1 + sqrt(a2*a2 - v*v*8));

        fragColor = texture(planet, x+.5);
    }
)";


class DemoWindow : public WindowBase {
public:
    struct Renderer {
        DemoWindow& window;

        gl::Program program;
        gl::Array<glm::vec4> quad_array;
        gl::Texture planet_texture;

        Renderer(DemoWindow& window) :
            window(window),
            program(vertex, fragment)
        {
            quad_array.load(gl::quad_strip);
            update();
        }

        void update() {
            planet_texture.load(window.planet_image);
        }

        void render(ivec2 size) {
            glClear(GL_COLOR_BUFFER_BIT);

            program.use();
            program.uniform("planet").set(0);
            program.uniform("V").set(
                glm::scale(glm::vec3(.9)) *
                glm::translate(glm::vec3(-.5, -.5, 0.)));
            float aspect = (float)size.x / size.y;
            program.uniform("P").set(
                glm::ortho(-aspect/2, aspect/2, -.5f, .5f));
            planet_texture.bind(0);
            program.attrib("position").array(quad_array);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }
    };
    unique_ptr<Renderer> renderer;

    static auto make_planet_image() {
        Gradient<color::RGB> gradient;
        gradient[0] = gradient[1] = color::YCH{
            .8 - entropy::trunc_exp(3, .4),
            entropy::normal()*.3 + .3,
            entropy::normal()*.01 + .2
        }.srgb().rgb();

        int subdivs = entropy::poisson(4);
        for (int i = 0; i < subdivs; i++) {
            auto s = i ? entropy::uniform() : entropy::coin();
            gradient[s] = (gradient(s).srgb().ych() + color::YCH{
                entropy::normal()*.2,
                entropy::normal()*.3,
                entropy::normal()*.05
            }).srgb().rgb();
        }

        PerlinNoise noise;
        noise.reseed();

        return make_image(ivec2{1000, 1000},
            [&](dvec2 p) { return gradient((noise(p)+1)*.5); });
    }

    Image<color::RGB> planet_image;
    DemoWindow() :
        planet_image(make_planet_image())
    {
        set_title("pgamecc demo");
    }

    void render_init() { renderer.reset(new Renderer(*this)); }
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
            if (key == ' ') {
                planet_image = make_planet_image();
                renderer->update();
            }
        }
    }
};


int main()
{
    return pgamecc_main<DemoWindow>();
}
