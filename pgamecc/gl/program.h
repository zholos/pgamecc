#ifndef PGAMECC_GL_PROGRAM_H
#define PGAMECC_GL_PROGRAM_H

#include "common.h"
#include "buffer.h"

#include <pgamecc/files.h>

#include <string>
#include <type_traits>

#include <glm/glm.hpp>


namespace pgamecc {
namespace gl {

class Shader : public detail::Object<Shader> {
public:
    Shader(GLenum type, Source);

    // named constructors
    static Shader vertex(Source);
    static Shader fragment(Source);

    friend class Program;
};


class Program : public detail::Object<Program> {
    class Uniform {
        const GLuint program;
        const GLint location;

    public:
        Uniform(GLuint p, GLint l) : program(p), location(l) {}
        void set(int value) const;
        void set(float value) const;
        void set(const glm::vec2& value) const;
        void set(const glm::vec3& value) const;
        void set(const glm::vec4& value) const;
        void set(const glm::mat2& value) const;
        void set(const glm::mat3& value) const;
        void set(const glm::mat4& value) const;
        int get_int() const;
    };

    class UniformBlock {
        const GLuint program;
        const GLuint index;

        class Uniform {
            const GLuint program;
            const GLuint index;
            const GLint offset;

            void set_void(const void*, size_t) const;

        public:
            Uniform(GLuint p, GLuint i, GLint o) :
                program(p), index(i), offset(o) {}
            void set(int value) const;
            void set(double value) const;

            template<typename T>
            std::enable_if_t<detail::is_vec<T>::value ||
                             detail::is_mat<T>::value>
            set(const T& value) const { set_void(&value, sizeof value); }
        };

    public:
        UniformBlock(GLuint p, GLuint i) : program(p), index(i) {}
        void bind(GLuint binding) const;
        size_t size_bytes() const;
        Uniform uniform(const std::string& name);
    };

    class Attrib {
        const GLuint program;
        const GLuint index;

        template<typename T>
        void array_buffer(const detail::TargetBuffer<GL_ARRAY_BUFFER>&,
                          int attrib_size,
                          size_t offset = 0, size_t stride = 0) const;

        void instanced_(GLuint divisor) const;

    public:
        Attrib(GLuint p, GLuint i) : program(p), index(i) {}

        const Attrib& instanced(GLuint divisor = 1) const {
            instanced_(divisor);
            return *this;
        }
        const Attrib& uninstanced() const { return instanced(0); }

        template<typename T, GLenum usage>
        std::enable_if_t<std::is_same<T, GLfloat>::value ||
                         std::is_same<T, GLint>::value>
        array(const Array<T, usage>& array, int attrib_size = 1,
              size_t offset = 0, size_t stride = 0) const {
            array_buffer<T>(array, attrib_size, offset, stride);
        }
        template<typename T, GLenum usage>
        std::enable_if_t<detail::is_vec<T>::value>
        array(const Array<T, usage>& array) const {
            array_buffer<typename T::value_type>(array, T{}.length());
        }

        // TODO: corresponding format() functions
        void array(GLuint binding) const;

        void unarray() const;
        void set(float value) const;
        void set(const glm::vec2& value) const;
        void set(const glm::vec3& value) const;
        void set(const glm::vec4& value) const;
    };

public:
    Program();
    Program(Source vertex_source, Source fragment_source);

    void attach(const Shader&);
    void link();
    void validate() const;

    Uniform uniform(const std::string& name);
    UniformBlock uniform_block(const std::string& name);
    Attrib attrib(const std::string& name);
    Attrib attrib(GLuint index);

    void use() const;
    static void unuse();
};


} // gl
} // pgamecc

#endif
