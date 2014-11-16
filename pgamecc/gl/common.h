#ifndef PGAMECC_GL_COMMON_H
#define PGAMECC_GL_COMMON_H

// only need gl.h, but glew.h must precede it, so this is more convenient
#include <GL/glew.h>

#include <cassert>
#include <string>
#include <type_traits>

#include <glm/glm.hpp>


namespace pgamecc {
namespace gl {

class error_check {
    std::string function;

public:
    error_check(const std::string function);
    ~error_check() noexcept(false);
};


namespace detail {

// for checking that OpenGL context doesn't change (destroying all objects)
// during the lifetime of our proxies
extern int current_context_iteration;

// common base class for all OpenGL proxies
template<typename Derived>
class Object {
#ifdef PGAMECC_DEBUG
    int context_iteration;
#endif

protected:
    GLuint object; // OpenGL object (e.g. texture) represented by this proxy

public:
    Object() :
        object(0) // in case derived constructor throws
    {
#ifdef PGAMECC_DEBUG
        context_iteration = current_context_iteration;
#endif
    }
    // noncopyable
    Object(Object&& r) {
        object = r.object;
        r.object = 0;
    }
    Object& operator=(Object&& r) {
        destroy();
        if (this != &r) {
            object = r.object;
            r.object = 0;
        }
        return *this;
    }
    ~Object() {
#ifdef PGAMECC_DEBUG
        assert(context_iteration == current_context_iteration);
#endif
        destroy(); // GL functions silently ignore 0 names, so we can call this
                   // even for moved-from objects
    }
    void destroy(); // defined for each deriving class
};


template<typename> struct is_vec : std::false_type {};
template<> struct is_vec<glm::vec2> : std::true_type {};
template<> struct is_vec<glm::vec3> : std::true_type {};
template<> struct is_vec<glm::vec4> : std::true_type {};
template<> struct is_vec<glm::ivec2> : std::true_type {};
template<> struct is_vec<glm::ivec3> : std::true_type {};
template<> struct is_vec<glm::ivec4> : std::true_type {};
// no dvec because double isn't useful on the GPU

template<typename> struct is_mat : std::false_type {};
template<> struct is_mat<glm::mat2> : std::true_type {};
template<> struct is_mat<glm::mat3> : std::true_type {};
template<> struct is_mat<glm::mat4> : std::true_type {};

} // detail


} // gl
} // pgamecc

#endif
