#include "font.h"

#include <pgamecc/image.h>

#include <algorithm>
#include <exception>
#include <iterator>
#include <limits>
#include <sstream>

#include <glm/gtx/component_wise.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

using std::string;
using std::u32string;
using std::vector;
using std::max;
using std::swap;
using std::transform;
using std::runtime_error;
using std::begin;
using std::end;
using std::back_inserter;
using std::numeric_limits;
using std::ostringstream;

using namespace pgamecc::gl;


static void
check(const string function, FT_Error error)
{
    if (error) {
        // no API function to get error details
        // (but see fterrors.h to build a lookup table)
        ostringstream err;
        err << "FreeType error in " << function;
        throw runtime_error(err.str());
    }
}

static FT_Library& library() {
    static FT_Library library = [] {
        FT_Library library;
        check("initialization",
              FT_Init_FreeType(&library));
        return library;
    }();
    return library;
}


struct Font::impl {
    FT_Face face;
};


Font::Font(std::string data, int em_texels) :
    p(new impl),
    face_data(data)
{
    static_assert(sizeof(FT_Byte) == 1, "");
    check("Font constructor",
          FT_New_Memory_Face(
              library(), reinterpret_cast<FT_Bytes>(data.data()), data.size(),
              0, &p->face));
    check("Font constructor",
          FT_Set_Pixel_Sizes(p->face, 0, em_texels));
}

Font::~Font()
{
    check("Font destructor",
          FT_Done_Face(p->face));
}


u32string
Font::from_ascii(string text)
{
    u32string result;
    transform(begin(text), end(text), back_inserter(result), [] (char c) {
        if (c < 0 || c >= 128)
            throw runtime_error("error in Font: string must be ASCII");
        return static_cast<char32_t>(c);
    });
    return result;
}


Font::line_metrics
Font::get_line_metrics() const
{
    auto& m = p->face->size->metrics;
    auto d = 64. * m.y_ppem;
    return { m.ascender / d, m.descender / d, m.height / d };
}

vector<Font::glyph_metrics>
Font::prepare(string text)
{
    return prepare(from_ascii(text));
}

vector<Font::glyph_metrics>
Font::prepare(u32string text)
{
    vector<glyph_metrics> r;
    dvec2 pen;
    glyph_index previous = 0;

    auto& metrics = p->face->size->metrics;
    dvec2 ppem(metrics.x_ppem, metrics.y_ppem);

    for (auto codepoint: text) {
        if (codepoint == '\n') {
            pen.x = 0;
            pen.y -= get_line_metrics().height;
            previous = 0;
            continue;
        }

        // use larger type if fails
        static_assert(numeric_limits<FT_UInt>::max() <=
                      numeric_limits<glyph_index>::max(), "");
        glyph_index index = FT_Get_Char_Index(p->face, codepoint);

        glyph_info glyph;
        auto it = glyphs.find(index);
        if (it != glyphs.end())
            glyph = it->second;
        else {
            auto slot = p->face->glyph;

            check("Font::prepare() loading and rendering",
                  FT_Load_Glyph(p->face, index, FT_LOAD_RENDER));

            // TODO: FT_Bitmap_Convert if this isn't true (e.g. bitmap fonts)
            assert(slot->bitmap.pixel_mode == FT_PIXEL_MODE_GRAY &&
                   slot->bitmap.num_grays == 256);

            ivec2 b(slot->bitmap.width, slot->bitmap.rows);

            // Include 1-pixel space above and to the right of glyphs.
            // Texture wraps, so don't need a border below and to the left.
            if (shelf.p.x + b.x + 1 > texture_size) {
                shelf.p.x = 0;
                shelf.p.y += shelf.height;
                shelf.height = 0;
            }

            if (glm::compMax(shelf.p + b + 1) > texture_size) {
                texture_size = max(256, texture_size * 2);
                texture.clear_red(ivec2(texture_size));
                // Probably errors out if size >= GL_MAX_TEXTURE_SIZE or uses
                // too much memory, so this recursion is bounded.

                shelf = {};
                glyphs.clear();
                return prepare(text);
                // WARNING: infinite recursion if texture is too small
            }

            auto& bitmap = slot->bitmap;
            Image<double> image(b, [&] (ivec2 p) {
                // TODO: verify that FreeType doesn't use a gamma curve
                return bitmap.buffer[bitmap.pitch * p.y + p.x] / 255.;
            });
            texture.load_at(image, shelf.p);

            auto& m = glyph.metrics;
            double ts = texture_size;
            m.t0 = dvec2(shelf.p) / ts;
            m.t1 = m.t0 + dvec2(b) / ts;
            swap(m.t0.y, m.t1.y); // flip
            m.p0 = dvec2(slot->bitmap_left,
                         slot->bitmap_top - bitmap.rows) / ppem;
            m.p1 = m.p0 + dvec2(b) / ppem;
            glyph.advance = dvec2(slot->advance.x,
                                  slot->advance.y) / 64. / ppem;
            glyphs[index] = glyph;

            shelf.p.x += b.x + 1;
            shelf.height = max(shelf.height, b.y + 1);
        }

        if (FT_HAS_KERNING(p->face) && previous) {
            FT_Vector kerning;
            check("Font::prepare() kerning",
                  FT_Get_Kerning(p->face, previous, index, FT_KERNING_DEFAULT,
                                 &kerning));
            pen += dvec2(kerning.x, kerning.y) / 64. / ppem;
        }
        previous = index;

        glyph.metrics.p0 += pen;
        glyph.metrics.p1 += pen;
        r.push_back(glyph.metrics);
        pen += glyph.advance;
    }

    return r;
}


static const char* vertex = R"(
    #version 330

    uniform mat4 transform; // x-axis is baseline, y-axis is up, in em units

    in vec4 t0t1;
    in vec4 p0p1;

    out vec2 t;

    void main() {
        vec2 t0 = t0t1.xy, t1 = t0t1.zw;
        vec2 p0 = p0p1.xy, p1 = p0p1.zw;
        vec2 p = vec2(gl_VertexID % 2, gl_VertexID / 2);
        t = mix(t0, t1, p);
        gl_Position = transform * vec4(mix(p0, p1, p), 0, 1);
    }
)";

static const char* fragment = R"(
    #version 330

    uniform sampler2D font;
    uniform vec4 background, color = vec4(1);

    in vec2 t;

    out vec4 fragColor;

    void main() {
        fragColor = mix(background, color, texture(font, t).r);
    }
)";

Program
Font::simple_program()
{
    return Program(vertex, fragment);
}


void
Font::render(Program& program, string text)
{
    render(program, from_ascii(text));
}

void
Font::render(Program& program, u32string text)
{
    vector<glm::vec4> t0t1;
    vector<glm::vec4> p0p1;
    for (auto m: prepare(text)) {
        t0t1.emplace_back(m.t0, m.t1);
        p0p1.emplace_back(m.p0, m.p1);
    }
    StreamArray<glm::vec4> t0t1_array(t0t1);
    StreamArray<glm::vec4> p0p1_array(p0p1);

    program.use();
    int unit = program.uniform("font").get_int();
    bind_texture(unit);

    program.attrib("t0t1").instanced().array(t0t1_array);
    program.attrib("p0p1").instanced().array(p0p1_array);
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, t0t1.size());
    program.attrib("p0p1").uninstanced().unarray();
    program.attrib("t0t1").uninstanced().unarray();
    // TODO: properly restore instanced state

    unbind_texture(unit);
    program.unuse();
}
