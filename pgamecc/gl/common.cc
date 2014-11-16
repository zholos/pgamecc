#include "common.h"

#include <exception>
#include <sstream>
#include <stdexcept>

using std::string;
using std::uncaught_exception;
using std::ostringstream;
using std::logic_error;

using namespace pgamecc::gl;


//
// common utilities
//

static string
error_string(GLenum error)
{
    switch (error) {
    case GL_NO_ERROR:
        return "no error";
    case GL_INVALID_ENUM:
    case GL_INVALID_VALUE:
        return "invalid argument";
    case GL_INVALID_OPERATION:
        return "invalid operation";
    case GL_STACK_OVERFLOW:
    case GL_STACK_UNDERFLOW:
    case GL_OUT_OF_MEMORY:
    case GL_TABLE_TOO_LARGE:
        return "memory error";
    default:
        return "unknown error";
    }
};

static void
throw_error(string prefix, string function, GLenum error)
{
    ostringstream err;
    err << prefix << " in " << function << ": " << error_string(error);
    throw logic_error(err.str()); // this is for errors that shouldn't happen
};

error_check::error_check(const string function) :
    function(function)
{
    if (GLenum error = glGetError())
        throw_error("existing OpenGL error", function, error);
}

error_check::~error_check() noexcept(false) {
    if (std::uncaught_exception())
        ; // probably stack-unwinding, so don't cause crash
    else
        if (GLenum error = glGetError())
            throw_error("unchecked OpenGL error", function, error);
}


int detail::current_context_iteration;
