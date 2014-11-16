#ifndef PGAMECC_GL_TEXTURE_H
#define PGAMECC_GL_TEXTURE_H

#include "common.h"

#include <pgamecc/color.h>
#include <pgamecc/image.h>


namespace pgamecc {
namespace gl {

class Framebuffer;

class Texture : public detail::Object<Texture> {
public:
    Texture();
    void clamp_to_edge(); // default is repeat

    void bind(int unit) const;
    static void unbind(int unit);

    void load(const Image<color::RGB>& image);
    void load_at(const Image<color::RGB>& image, ivec2 at);
    void load_at(const Image<double>& image, ivec2 at);
    void clear_red(ivec2 size, double color = 0);
    void reset_rgb(ivec2 size); // contents undefined
    void reset_rgba(ivec2 size); // contents undefined
    void reset_depth(ivec2 size); // contents undefined

    friend class Framebuffer;
};


class Sampler : public detail::Object<Sampler> {
public:
    Sampler();
    GLuint sampler() const { return object; }

    void bind(int unit) const;
    static void unbind(int unit);
};


class Renderbuffer : public detail::Object<Renderbuffer> {
public:
    Renderbuffer();

    void bind() const;
    static void unbind();

    void reset_depth(ivec2 size);

    friend class Framebuffer;
};


class Framebuffer : public detail::Object<Framebuffer> {
public:
    Framebuffer();
    Framebuffer(const Texture&, const Renderbuffer&);

    void bind() const;
    static void unbind();

    void attach_color(const Texture&);
    void attach_depth(const Texture&);
    void attach_depth(const Renderbuffer&);
    void check() const;
};


} // gl
} // pgamecc

#endif
