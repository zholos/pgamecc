#ifndef PGAMECC_GL_FONT_H
#define PGAMECC_GL_FONT_H

#include <pgamecc/gl/texture.h>
#include <pgamecc/gl/program.h>

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace pgamecc {
namespace gl {

class Font {
    Texture texture;
    int texture_size = 0;
    struct {
        ivec2 p;
        int height;
    } shelf = {};

    typedef unsigned long glyph_index;
    struct glyph_metrics_ {
        dvec2 t0, t1; // glyph rect in texture
        dvec2 p0, p1; // glyph rect relative to baseline and origin, em units
    };
    struct glyph_info {
        glyph_metrics_ metrics;
        dvec2 advance;
    };
    std::map<glyph_index, glyph_info> glyphs;

    struct impl;
    std::unique_ptr<impl> p; // face member
    std::string face_data; // owned copy, referenced by FreeType after load

public:
    Font(std::string data, int em_texels);
    Font(Source data, int em_texels) : Font(data.source, em_texels) {}
    ~Font();

    static std::u32string from_ascii(std::string text);

    struct line_metrics {
        double ascender, descender, height; // in em units
    };
    line_metrics get_line_metrics() const;

    typedef glyph_metrics_ glyph_metrics;
    std::vector<glyph_metrics> prepare(std::string text); // ascii
    std::vector<glyph_metrics> prepare(std::u32string text);

    static Program simple_program();
    void render(Program& program, std::string text); // ascii
    void render(Program& program, std::u32string text);

    void bind_texture(int unit) { texture.bind(unit); }
    static void unbind_texture(int unit) { Texture::unbind(unit); }
};


} // gl
} // pgamecc

#endif
