#include "texture.h"

#include <iterator>
#include <memory>
#include <stdexcept>
#include <vector>

using std::begin;
using std::end;
using std::unique_ptr;
using std::logic_error;
using std::vector;

using namespace pgamecc::gl;


//
// gl::Texture
//

Texture::Texture()
{
    error_check ec("Texture constructor");
    glGenTextures(1, &object);
    bind(0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    unbind(0);
}

void
Texture::clamp_to_edge()
{
    bind(0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    unbind(0);
}

namespace pgamecc { namespace gl { namespace detail {
template<> void
Object<Texture>::destroy()
{
    error_check ec("Texture destructor");
    glDeleteTextures(1, &object);
}
}}}


static void
active_texture(int unit) {
    // minimum is 80 units
    if (unit < 0 || unit >= GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS)
        throw logic_error("texture unit out of range");
    glActiveTexture(GL_TEXTURE0 + unit);
}

void
Texture::bind(int unit) const
{
    error_check ec("Texture::bind");
    active_texture(unit);
    glBindTexture(GL_TEXTURE_2D, object);
}

void
Texture::unbind(int unit)
{
    error_check ec("Texture::unbind");
    active_texture(unit);
    glBindTexture(GL_TEXTURE_2D, 0);
}


static unique_ptr<GLfloat[]>
image_data(const pgamecc::Image<pgamecc::color::RGB>& image)
{
    glPixelStorei(GL_UNPACK_ALIGNMENT, sizeof(GLfloat));
    size_t size = (size_t)image.size().x * image.size().y;
    unique_ptr<GLfloat[]> data(new GLfloat[size * 3]);
    GLfloat* d = &data[0];
    for (auto c: image.pixels()) {
        *d++ = c.r;
        *d++ = c.g;
        *d++ = c.b;
    }
    return move(data);
}

void
Texture::load(const Image<color::RGB>& image)
{
    error_check ec("Texture::load");
    bind(0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image.size().x, image.size().y, 0,
                 GL_RGB, GL_FLOAT, &image_data(image)[0]);
    unbind(0);
}

void
Texture::load_at(const Image<color::RGB>& image, ivec2 at)
{
    error_check ec("Texture::load_at");
    bind(0);
    glTexSubImage2D(GL_TEXTURE_2D, 0, at.x, at.y,
                    image.size().x, image.size().y,
                    GL_RGB, GL_FLOAT, &image_data(image)[0]);
    unbind(0);
}

void
Texture::load_at(const Image<double>& image, ivec2 at)
{
    error_check ec("Texture::load_at");
    bind(0);
    // TODO: use dynarray instead
    auto pixels = image.pixels();
    vector<GLfloat> data(begin(pixels), end(pixels));
    glTexSubImage2D(GL_TEXTURE_2D, 0, at.x, at.y,
                    image.size().x, image.size().y,
                    GL_RED, GL_FLOAT, data.data());
    unbind(0);
}


void
Texture::clear_red(ivec2 size, double color)
{
    error_check ec("Texture::clear_red");
    bind(0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, size.x, size.y, 0,
                 GL_RED, GL_FLOAT, nullptr);
    // TODO: use dynarray instead
    vector<GLfloat> data(size.x, color);
    for (int y = 0; y < size.y; y++)
        // TODO: check performance
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, y, size.x, 1,
                        GL_RED, GL_FLOAT, data.data());
    unbind(0);
}


static void
reset_texture(const Texture& texture, pgamecc::ivec2 size, GLenum format)
{
    error_check ec("Texture::reset");
    texture.bind(0);
    glTexImage2D(GL_TEXTURE_2D, 0, format, size.x, size.y, 0,
                 format, GL_FLOAT, nullptr);
    texture.unbind(0);
}

void
Texture::reset_rgb(ivec2 size)
{
    reset_texture(*this, size, GL_RGB);
}

void
Texture::reset_rgba(ivec2 size)
{
    reset_texture(*this, size, GL_RGBA);
}

void
Texture::reset_depth(ivec2 size)
{
    reset_texture(*this, size, GL_DEPTH_COMPONENT);
}


//
// gl::Sampler
//

Sampler::Sampler()
{
    error_check ec("Sampler constructor");
    glGenSamplers(1, &object);
    glSamplerParameteri(object, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glSamplerParameteri(object, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

namespace pgamecc { namespace gl { namespace detail {
template<> void
Object<Sampler>::destroy()
{
    error_check ec("Sampler destructor");
    glDeleteSamplers(1, &object);
}
}}}


void
Sampler::bind(int unit) const
{
    error_check ec("Sampler::bind");
    glBindSampler(unit, object);
}

void
Sampler::unbind(int unit)
{
    error_check ec("Sampler::unbind");
    glBindSampler(unit, 0);
}


//
// gl::Renderbuffer
//

Renderbuffer::Renderbuffer()
{
    error_check ec("Renderbuffer constructor");
    glGenRenderbuffers(1, &object);

    // "bless" it as a renderbuffer, before this glIsRenderbuffer is false
    glBindRenderbuffer(GL_RENDERBUFFER, object);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

namespace pgamecc { namespace gl { namespace detail {
template<> void
Object<Renderbuffer>::destroy()
{
    error_check ec("Renderbuffer destructor");
    glDeleteRenderbuffers(1, &object);
}
}}}


void
Renderbuffer::bind() const
{
    error_check ec("Renderbuffer::bind");
    glBindRenderbuffer(GL_RENDERBUFFER, object);
}

void
Renderbuffer::unbind()
{
    error_check ec("Renderbuffer::unbind");
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}


void
Renderbuffer::reset_depth(ivec2 size)
{
    error_check ec("Renderbuffer::depth");
    bind();
    glRenderbufferStorage(GL_RENDERBUFFER,
                          GL_DEPTH_COMPONENT16, size.x, size.y);
    unbind();
}



//
// gl::Framebuffer
//

Framebuffer::Framebuffer()
{
    error_check ec("Framebuffer constructor");
    glGenFramebuffers(1, &object);
}

Framebuffer::Framebuffer(const Texture& color, const Renderbuffer& depth) :
    Framebuffer()
{
    attach_color(color);
    attach_depth(depth);
    check();
}

namespace pgamecc { namespace gl { namespace detail {
template<> void
Object<Framebuffer>::destroy()
{
    error_check ec("Framebuffer destructor");
    glDeleteFramebuffers(1, &object);
}
}}}


void
Framebuffer::bind() const
{
    error_check ec("Framebuffer::bind");
    glBindFramebuffer(GL_FRAMEBUFFER, object);
}

void
Framebuffer::unbind()
{
    error_check ec("Framebuffer::unbind");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void
Framebuffer::attach_color(const Texture& texture)
{
    error_check ec("Framebuffer::attach_color");
    bind();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, texture.object, 0);
    unbind();
}

void
Framebuffer::attach_depth(const Texture& texture)
{
    error_check ec("Framebuffer::attach_depth");
    bind();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                           GL_TEXTURE_2D, texture.object, 0);
    unbind();
}

void
Framebuffer::attach_depth(const Renderbuffer& renderbuffer)
{
    error_check ec("Framebuffer::attach_depth");
    bind();
    // must have called renderbuffer.depth() before this
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                              GL_RENDERBUFFER, renderbuffer.object);
    unbind();
}


void
Framebuffer::check() const
{
    bind();
    //glDrawBuffer(GL_NONE); // TODO: maybe need this?
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    unbind();

    if (status != GL_FRAMEBUFFER_COMPLETE)
        throw logic_error("OpenGL error: framebuffer not complete");
}
