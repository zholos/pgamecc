#include "window.h"

#include "gl.h"

#include <cassert>
#include <chrono>
#include <iostream>
#include <list>
#include <sstream>
#include <stdexcept>
#include <thread>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

using std::runtime_error;
using std::ostringstream;
using std::string;
using std::list;
using std::cerr;

using namespace pgamecc;
using namespace pgamecc::key;


// Most GLFW calls should be made from the main thread, so keep checking this

static std::thread::id main_thread_id;

static bool
is_main_thread()
{
    return std::this_thread::get_id() == main_thread_id;
}

WindowBase::WindowBase()
{
    // assume we're originally called from the main thread
    if (main_thread_id == std::thread::id())
        main_thread_id = std::this_thread::get_id();
    else
        assert(is_main_thread());
}


static WindowBase&
callback_window(GLFWwindow* glfw_window)
{
    WindowBase* window =
        static_cast<WindowBase*>(glfwGetWindowUserPointer(glfw_window));
    assert(window);
    return *window;
}

static void
size_callback(GLFWwindow* glfw_window, int width, int height)
{
    callback_window(glfw_window)._pgamecc_resized(ivec2(width, height));
}

// this is promised in window.h, so better check it
static_assert(
    GLFW_KEY_SPACE == ' ' &&
    GLFW_KEY_APOSTROPHE == '\'' &&
    GLFW_KEY_COMMA == ',' &&
    GLFW_KEY_MINUS == '-' &&
    GLFW_KEY_PERIOD == '.' &&
    GLFW_KEY_SLASH == '/' &&
    GLFW_KEY_0 == '0' && GLFW_KEY_1 == '1' && GLFW_KEY_2 == '2' &&
    GLFW_KEY_3 == '3' && GLFW_KEY_4 == '4' && GLFW_KEY_5 == '5' &&
    GLFW_KEY_6 == '6' && GLFW_KEY_7 == '7' && GLFW_KEY_8 == '8' &&
    GLFW_KEY_9 == '9' &&
    GLFW_KEY_SEMICOLON == ';' &&
    GLFW_KEY_EQUAL == '=' &&
    GLFW_KEY_A == 'A' && GLFW_KEY_B == 'B' && GLFW_KEY_C == 'C' &&
    GLFW_KEY_D == 'D' && GLFW_KEY_E == 'E' && GLFW_KEY_F == 'F' &&
    GLFW_KEY_G == 'G' && GLFW_KEY_H == 'H' && GLFW_KEY_I == 'I' &&
    GLFW_KEY_J == 'J' && GLFW_KEY_K == 'K' && GLFW_KEY_L == 'L' &&
    GLFW_KEY_M == 'M' && GLFW_KEY_N == 'N' && GLFW_KEY_O == 'O' &&
    GLFW_KEY_P == 'P' && GLFW_KEY_Q == 'Q' && GLFW_KEY_R == 'R' &&
    GLFW_KEY_S == 'S' && GLFW_KEY_T == 'T' && GLFW_KEY_U == 'U' &&
    GLFW_KEY_V == 'V' && GLFW_KEY_W == 'W' && GLFW_KEY_X == 'X' &&
    GLFW_KEY_Y == 'Y' && GLFW_KEY_Z == 'Z' &&
    GLFW_KEY_LEFT_BRACKET == '[' &&
    GLFW_KEY_BACKSLASH == '\\' &&
    GLFW_KEY_RIGHT_BRACKET == ']' &&
    GLFW_KEY_GRAVE_ACCENT == '`',
    "non-ASCII execution character set, write code to remap");

static void
key_callback(GLFWwindow* glfw_window,
             int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS || action == GLFW_RELEASE) // ignore GLFW_REPEAT
        callback_window(glfw_window).input_key(action == GLFW_PRESS, key, mods);
}

static void
char_callback(GLFWwindow* glfw_window, unsigned codepoint)
{
    callback_window(glfw_window).input_char(codepoint);
}

static_assert(
    GLFW_MOUSE_BUTTON_1 + 1 == GLFW_MOUSE_BUTTON_2 &&
    GLFW_MOUSE_BUTTON_2 + 1 == GLFW_MOUSE_BUTTON_3 &&
    GLFW_MOUSE_BUTTON_3 + 1 == GLFW_MOUSE_BUTTON_4 &&
    GLFW_MOUSE_BUTTON_4 + 1 == GLFW_MOUSE_BUTTON_5 &&
    GLFW_MOUSE_BUTTON_5 + 1 == GLFW_MOUSE_BUTTON_6 &&
    GLFW_MOUSE_BUTTON_6 + 1 == GLFW_MOUSE_BUTTON_7 &&
    GLFW_MOUSE_BUTTON_7 + 1 == GLFW_MOUSE_BUTTON_8,
    "non-contiguous GLFW mouse button codes");

static_assert(
    GLFW_MOUSE_BUTTON_8 == GLFW_MOUSE_BUTTON_LAST,
    "update static_assert above");

static void
button_callback(GLFWwindow* glfw_window, int button, int action, int mods)
{
    callback_window(glfw_window).input_button(
        action == GLFW_PRESS, button - GLFW_MOUSE_BUTTON_1 + 1, mods);
}

static void
cursor_callback(GLFWwindow* glfw_window, double xpos, double ypos)
{
    callback_window(glfw_window)._pgamecc_cursor(dvec2(xpos, ypos));
}

static void
close_callback(GLFWwindow* glfw_window)
{
    callback_window(glfw_window).input_key(true, key_close, 0);
}

#ifdef PGAMECC_DEBUG
static void APIENTRY
opengl_debug_message(GLenum source, GLenum type, GLuint id, GLenum severity,
                     GLsizei length, const GLchar* message,
                     const void* userParam)
{
    cerr << "OpenGL debug message: " << string(message, length) << '\n';
}
#endif

void
WindowBase::_pgamecc_cycle()
{
    _pgamecc.reset = false;
    _pgamecc.fullscreen = _pgamecc.next_fullscreen;

    GLFWmonitor* monitor = nullptr;
    if (_pgamecc.fullscreen) {
        monitor = glfwGetPrimaryMonitor();
        if (!monitor)
            throw runtime_error("glfwGetPrimaryMonitor() failed");

        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        if (!mode)
            throw runtime_error("glfwGetVideoMode() failed");
        _pgamecc.size[_pgamecc.fullscreen] = ivec2(mode->width, mode->height);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#ifdef PGAMECC_DEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif
    glfwWindowHint(GLFW_SAMPLES, _pgamecc.samples);
    GLFWwindow* window = glfwCreateWindow(size().x, size().y,
                                          _pgamecc.title.c_str(),
                                          monitor, NULL);
    if (!window)
        throw runtime_error("glfwCreateWindow() failed");
    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE; // needed to work around bug with core profile
    auto glew_status = glewInit();
    if (glew_status != GLEW_OK) {
        ostringstream err;
        err << "glewInit() failed: " << glewGetErrorString(glew_status);
        throw runtime_error(err.str());
    }
    glGetError(); // needed to work around bug with core profile

#ifdef PGAMECC_DEBUG
    glDebugMessageCallback(opengl_debug_message, nullptr);
#endif

    // In core profile there is no default VAO.
    // TODO: wrap VAOs and require user code to bind one
    GLuint default_vao;
    glGenVertexArrays(1, &default_vao);
    glBindVertexArray(default_vao);

    // TODO: can this fail if the window is iconified?
    // TODO: this can be overridden by the user
    glfwSwapInterval(1); // vsync

    glfwSetWindowUserPointer(window, this);
    glfwSetWindowSizeCallback(window, size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCharCallback(window, char_callback);
    glfwSetMouseButtonCallback(window, button_callback);
    glfwSetCursorPosCallback(window, cursor_callback);
    glfwSetWindowCloseCallback(window, close_callback);

    gl::detail::current_context_iteration++;
    render_init();
    _pgamecc_resized(size());

    typedef std::chrono::steady_clock clock_type;
    static_assert(std::ratio_less_equal<clock_type::period, std::milli>::value,
                  "clock resolution too low");
    struct RunningSum {
        list<clock_type::duration> samples;
        clock_type::duration sum = clock_type::duration::zero();

        void push(clock_type::duration d) {
            samples.push_back(d);
            sum += d;
            if (samples.size() > 10) {
                sum -= samples.front();
                samples.pop_front();
            }
        }

        double mean() const {
            assert(!samples.empty());
            return std::chrono::duration_cast<
                std::chrono::duration<double>>(sum).count() / samples.size();
        }
    };
    RunningSum sum_frame, sum_step, sum_render;
    _pgamecc.have_info = false;
    auto clock_start = clock_type::now();

    for (;;) {
        glfwPollEvents();
        if (_pgamecc.reset)
            break;

        step();
        if (_pgamecc.reset)
            break;

        if (_pgamecc.grabbed != _pgamecc.next_grab) {
            _pgamecc.grabbed = _pgamecc.next_grab;
            glfwSetInputMode(
                window, GLFW_CURSOR,
                _pgamecc.grabbed ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
            if (_pgamecc.grabbed)
                glfwGetCursorPos(window,
                                 &_pgamecc.cursor.x, &_pgamecc.cursor.y);
        }
        if (_pgamecc.grabbed) {
            glfwSetCursorPos(window, 0, 0); // recenter to avoid precision loss
            glfwGetCursorPos(window, // may not have changed
                             &_pgamecc.cursor.x, &_pgamecc.cursor.y);
        }

        auto clock_step = clock_type::now();

        render();
        glFinish(); // for better timing information
        auto clock_render = clock_type::now();

        glfwSwapBuffers(window);
        auto clock_swap = clock_type::now();

        sum_frame. push(clock_swap - clock_start);
        sum_step.  push(clock_step - clock_start);
        sum_render.push(clock_render - clock_step);
        clock_start = clock_swap;

        _pgamecc.fps = 1 / sum_frame.mean();
        _pgamecc.step_load   = sum_step.mean()   / sum_frame.mean();
        _pgamecc.render_load = sum_render.mean() / sum_frame.mean();
        _pgamecc.have_info = true;
    }

    // Must destroy all old OpenGL objects here, because if they get destroyed
    // automatically in some destructor when render_start() is called, they will
    // delete some newly created OpenGL objects that share the same IDs.
    render_done();
    gl::detail::current_context_iteration++;

    glfwDestroyWindow(window);
}

void
WindowBase::_pgamecc_resized(ivec2 size)
{
    _pgamecc.size[_pgamecc.fullscreen] = size;

    glViewport(0, 0, size.x, size.y);

    resized();
}


void
WindowBase::_pgamecc_cursor(dvec2 position)
{
    if (_pgamecc.grabbed) {
        input_mouse(position - _pgamecc.cursor);
        _pgamecc.cursor = position;
    } else
        input_cursor(position);
}


void
WindowBase::run()
{
    assert(is_main_thread());

    if (!glfwInit())
        throw runtime_error("glfwInit() failed");

    while (!_pgamecc.quit)
        _pgamecc_cycle();

    glfwTerminate();
}


void
WindowBase::quit()
{
    _pgamecc.quit = true;
    _pgamecc.reset = true;
}

void
WindowBase::fullscreen()
{
    _pgamecc.next_fullscreen ^= 1;
    _pgamecc.reset = true;
}

void
WindowBase::set_samples(int samples)
{
    _pgamecc.samples = samples;
    _pgamecc.reset = true;
}

void
WindowBase::set_title(const string title)
{
    _pgamecc.title = title;
    // TODO: update current title
}

void
WindowBase::grab_mouse()
{
    _pgamecc.next_grab = true;
}

void
WindowBase::release_mouse()
{
    _pgamecc.next_grab = false;
}

void
WindowBase::print_info() const
{
    if (_pgamecc.have_info)
        std::cout << "FPS: " << fps() <<
            " load: " << step_load() << " " << render_load() << '\n';
}
