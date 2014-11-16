#ifndef PGAMECC_GL_BUFFER_H
#define PGAMECC_GL_BUFFER_H

#include "common.h"

#include <memory>
#include <vector>
#include <utility>


namespace pgamecc {
namespace gl {

namespace detail {

class GenericBuffer : public detail::Object<GenericBuffer> {
protected:
    GenericBuffer();

protected:
    class map_deleter {
        const GenericBuffer& buffer;
        const GLenum target;
    public:
        map_deleter(const GenericBuffer& b, GLenum t) : buffer(b), target(t) {}
        void operator()(void*);
    };

    void bind_target(GLenum target) const;
    static void unbind_target(GLenum target);

    void load_void(GLenum target, GLenum usage, const void*, size_t);

    std::unique_ptr<void, map_deleter>
    map_void(GLenum target, GLenum usage, size_t, GLenum access);

    void clear_bytes(GLenum target, GLenum usage, size_t);

    size_t size_bytes(GLenum target) const;

    void bind_vertex(GLuint binding, size_t offset, size_t stride);
    void bind_uniform(GLenum target,
                      GLuint binding, size_t size, size_t offset);
};

template<GLenum target>
struct TargetBuffer : GenericBuffer {
    void bind() const { bind_target(target); }
    static void unbind() { unbind_target(target); }

};

} // detail

// T is GLfloat or glm::vec*
template<typename T = GLfloat,
         GLenum target = GL_ARRAY_BUFFER, GLenum usage = GL_STATIC_DRAW>
struct Buffer : detail::TargetBuffer<target> {
    Buffer() = default;

    void load(const T* data, size_t size) {
        this->load_void(target, usage, data, size * sizeof *data);
    }

    template<size_t size>
    void load(const T (&data)[size]) { load(data, size); }

    void load(const std::vector<T>& data) { load(data.data(), data.size()); }

    // load a Buffer<glm::vec> from GLfloat data
    // separate from other loading templates so a mixed compatible initializer
    // can get converted to GLfloat
    template<typename Dummy = void>
    std::enable_if_t<detail::is_vec<std::conditional_t<0, Dummy, T>>::value>
    load(const std::vector<
            typename std::conditional_t<0, Dummy, T>::value_type>& data) {
        this->load_void(target, usage,
                        data.data(), data.size() * sizeof data[0]);
    }

    // load a Buffer<GLfloat> from glm::vec data
    template<typename U>
    std::enable_if_t<detail::is_vec<U>::value &&
                     std::is_same<typename U::value_type, T>::value>
    load(const std::vector<U>& data) {
        this->load_void(target, usage,
                        data.data(), data.size() * sizeof data[0]);
    }

    template<typename U>
    explicit Buffer(const std::vector<U>& data) { load(data); }

    std::unique_ptr<T[], detail::GenericBuffer::map_deleter>
    map_write(size_t size) {
        auto m = this->map_void(target, usage, size * sizeof(T), GL_WRITE_ONLY);
        return { static_cast<T*>(m.release()), std::move(m.get_deleter()) };
    }

    void reset() { load(nullptr, 0); }
    void clear(size_t size) {
        this->clear_bytes(target, usage, size * sizeof(T));
    }

    size_t size() const { return this->size_bytes(target) / sizeof(T); }
};

template<typename T = GLfloat, GLenum usage = GL_STATIC_DRAW>
struct Array : Buffer<T, GL_ARRAY_BUFFER, usage> {
    using Buffer<T, GL_ARRAY_BUFFER, usage>::Buffer;

    void bind(GLuint binding, size_t offset = 0, size_t stride = sizeof(T)) {
        this->bind_vertex(binding, offset, stride);
    }
};

template<typename T = GLfloat>
using StreamArray = Array<T, GL_STREAM_DRAW>;

template<typename T>
struct UniformBuffer : Buffer<T, GL_UNIFORM_BUFFER, GL_STREAM_DRAW> {
protected:
    typedef Buffer<T, GL_UNIFORM_BUFFER, GL_STREAM_DRAW> Buffer_type;

public:
    using Buffer_type::Buffer;

    // need this (and unbind()) to use uniform block item set()
    using Buffer_type::bind;

    void bind(GLuint binding) {
        this->bind_uniform(GL_UNIFORM_BUFFER, binding,
                           this->size() * sizeof(T), 0);
    }
    void bind(GLuint binding, size_t size, size_t offset = 0) {
        this->bind_uniform(GL_UNIFORM_BUFFER, binding,
                           size * sizeof(T), offset * sizeof(T));
    }
};


extern const glm::vec4 quad_strip[4];
extern const glm::vec4 cube_strip[14];


} // gl
} // pgamecc

#endif
