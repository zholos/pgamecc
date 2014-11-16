#ifndef PGAMECC_UI_WIDGETS_H
#define PGAMECC_UI_WIDGETS_H

#include <pgamecc/ui/controls.h>
#include <pgamecc/ui/layer.h>

namespace pgamecc {
namespace ui {

struct Widget : Control {
public:
    dvec2 position, size;

    dvec2 at(dvec2 at) const { return position + size * at; }
    void place_at(dvec2 at, dvec2 at_position);

    virtual void layout() {}
    virtual void render(Primitives&) const {}
};


namespace detail {
struct Text : Widget {
protected:
    std::u32string text;

public:
    void set_text(std::string text);
    void set_text(std::u32string text);
};
}


struct Label : detail::Text {
    void render(Primitives&) const;
};


struct Button : detail::Text {
    void render(Primitives&) const;
};


} // ui
} // pgamecc

#endif
