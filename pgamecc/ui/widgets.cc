#include "widgets.h"

#include <pgamecc/gl/font.h>

using std::string;

using namespace pgamecc::ui;


void
Widget::place_at(dvec2 at, dvec2 at_position)
{
    position = at_position - at * size;
}



void
Label::render(Primitives& p) const
{
    auto font_size = layer.em;
    auto line = p.get_line_metrics(true, font_size);
    auto offset = line.ascender * font_size;
    p.glyphs(position + dvec2(0, offset),
             text, false, font_size, color::RGB::gray(1).srgb());
}


void
detail::Text::set_text(string new_text)
{
    set_text(gl::Font::from_ascii(new_text));
}

void
detail::Text::set_text(std::u32string new_text)
{
    text = new_text;
}


void
Button::render(Primitives& p) const
{
    p.rect(position, size);
    auto font_size = layer.em * 1.5;
    auto line = p.get_line_metrics(true, font_size);
    auto offset = (line.ascender + line.descender) * font_size;
    p.glyphs(position + dvec2(size.x, size.y + offset) / 2.,
             text, true, font_size, color::RGB::gray(1).srgb(), true);
}
