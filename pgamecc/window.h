#ifndef PGAMECC_WINDOW_H
#define PGAMECC_WINDOW_H

#include <pgamecc/types.h>

#include <iostream>
#include <stdexcept>
#include <string>

namespace pgamecc {

// keys wrapped to avoid having to include glfw3.h
namespace key {
    enum {
        // printable characters are represented as is, e.g. 'A', '0', '+'

        // the rest are copied from GLFW
        key_unknown = -1,
        key_world_1 = 161,
        key_world_2 = 162,
        key_escape = 256,
        key_enter = 257,
        key_tab = 258,
        key_backspace = 259,
        key_insert = 260,
        key_delete = 261,
        key_right = 262,
        key_left = 263,
        key_down = 264,
        key_up = 265,
        key_page_up = 266,
        key_page_down = 267,
        key_home = 268,
        key_end = 269,
        key_caps_lock = 280,
        key_scroll_lock = 281,
        key_num_lock = 282,
        key_print_screen = 283,
        key_pause = 284,
        key_f1 = 290,
        key_f2 = 291,
        key_f3 = 292,
        key_f4 = 293,
        key_f5 = 294,
        key_f6 = 295,
        key_f7 = 296,
        key_f8 = 297,
        key_f9 = 298,
        key_f10 = 299,
        key_f11 = 300,
        key_f12 = 301,
        key_f13 = 302,
        key_f14 = 303,
        key_f15 = 304,
        key_f16 = 305,
        key_f17 = 306,
        key_f18 = 307,
        key_f19 = 308,
        key_f20 = 309,
        key_f21 = 310,
        key_f22 = 311,
        key_f23 = 312,
        key_f24 = 313,
        key_f25 = 314,
        key_kp_0 = 320,
        key_kp_1 = 321,
        key_kp_2 = 322,
        key_kp_3 = 323,
        key_kp_4 = 324,
        key_kp_5 = 325,
        key_kp_6 = 326,
        key_kp_7 = 327,
        key_kp_8 = 328,
        key_kp_9 = 329,
        key_kp_decimal = 330,
        key_kp_divide = 331,
        key_kp_multiply = 332,
        key_kp_subtract = 333,
        key_kp_add = 334,
        key_kp_enter = 335,
        key_kp_equal = 336,
        key_left_shift = 340,
        key_left_control = 341,
        key_left_alt = 342,
        key_left_super = 343,
        key_right_shift = 344,
        key_right_control = 345,
        key_right_alt = 346,
        key_right_super = 347,
        key_menu = 348,
        key_last = key_menu,

        // extras
        key_close = 1000, // click on close button in window decoration

        // user values starting here
        key_user = 2000
    };

    enum {
        // copied entirely from GLFW:
        mod_shift = 1,
        mod_control = 2,
        mod_alt = 4,
        mod_super = 8
    };
}


// The WindowBase is the interface between player and game.
// Derive from this class and implement the event functions and write main() as:
//     int main() { return pgamecc::pgamecc_main<MyWindow>(); }

class WindowBase {
    struct {
        // current status
        ivec2 size[2] = { ivec2(640, 480), ivec2(0, 0) };
        bool fullscreen = false;
        bool grabbed = false;
        dvec2 cursor;

        // requests
        bool reset = false; // new window, OpenGL context reset
        bool next_fullscreen = false;
        bool next_grab = false;
        bool quit = false;
        int samples = 0;
        std::string title;

        // statistics
        bool have_info;
        double fps;
        double step_load;
        double render_load;
    } _pgamecc;

public:
    void _pgamecc_cycle();
    void _pgamecc_resized(ivec2 size);
    void _pgamecc_cursor(dvec2 position);

public:
    WindowBase();
    virtual ~WindowBase() = default;

    // can be called directly, but usually called by main()
    void run();

    // requests
    void quit();
    void fullscreen();
    void set_samples(int samples);
    void set_title(const std::string);
    void grab_mouse();
    void release_mouse();

    // queries
    ivec2 size() const { return _pgamecc.size[_pgamecc.fullscreen]; }

    double fps() const { return _pgamecc.fps; };
    double step_load() const { return _pgamecc.step_load; }
    double render_load() const { return _pgamecc.render_load; }

    void print_info() const;

    // events
    virtual void input_key(bool press, int key, int mods) {}
    virtual void input_char(char32_t codepoint) {}
    virtual void input_button(bool press, int button, int mods) {}
    virtual void input_cursor(dvec2 position) {}
    virtual void input_mouse(dvec2 relative) {}
    virtual void resized() {}

    virtual void step() {} // called each frame after input and before rendering

    virtual void render_init() {}
    virtual void render_done() {}
    virtual void render() {}
};


template<typename Window, typename... Args>
int pgamecc_main(Args&&... args) {
    try {
        Window window{std::forward<Args>(args)...};
        window.run();
        return 0;
    } catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
}

}

#endif
