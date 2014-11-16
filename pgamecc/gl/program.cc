#include "program.h"

#include <iomanip>
#include <list>
#include <experimental/optional>
#include <stdexcept>
#include <sstream>
#include <utility>
#include <vector>

#include <glm/gtc/type_ptr.hpp>

using std::string;
using std::setw;
using std::list;
using std::experimental::optional;
using std::ostringstream;
using std::logic_error;
using std::runtime_error;
using std::move;
using std::vector;

using namespace pgamecc::gl;


//
// gl::Shader
//

template<typename Get, typename GetInfoLog>
static string
get_info_log(GLuint shader, Get get, GetInfoLog getInfoLog)
{
    GLint alloc = 0;
    get(shader, GL_INFO_LOG_LENGTH, &alloc);

    vector<char> buffer(alloc);
    GLint length = 0;
    getInfoLog(shader, buffer.size(), &length, buffer.data());
    assert(0 <= length && length < buffer.size()); // null character excluded
    return string(buffer.data(), length);
}


// Preprocess shader source to allow #include directives.

namespace {

struct Preprocessor {
    Preprocessor(string source) : source(source) {}

    string source;
    struct {
        string::size_type i;
    } state = {};

    static bool isspace(char c) {
        return c==' ' || c=='\t' || c=='\v' || c=='\f';
    }

    // Normalizes newlines, replaces runs of whitespace with single spaces.
    // TODO: continuation lines, comments
    char get() {
        if (state.i == source.size())
            return '\n';
        if (isspace(source[state.i])) {
            state.i++;
            while (state.i != source.size() && isspace(source[state.i]))
                state.i++;
            return ' ';
        }
        if (source[state.i] == '\r') {
            state.i++;
            if (state.i != source.size() && source[state.i] == '\n')
                state.i++;
            return '\n';
        }
        return source[state.i++];
    };

    bool accept(string token) {
        auto save = state;
        for (auto c: token)
            if (c != get()) {
                state = save;
                return false;
            }
        return true;
    }

    // emits line by line so user can count them
    template<typename Include, typename Version, typename Emit>
    void parse(Include include, Version version, Emit emit) {
        while (state.i != source.size()) {
            bool got_version = false;
            string::size_type line = state.i;

            accept(" ");
            if (accept("#version ")) {
                got_version = true;
                goto line;
            }
            if (accept("#include ")) {
                string delim;
                if (accept("\""))
                    delim = "\"";
                else if (accept("<"))
                    delim = ">";
                else
                    goto line;
                string filename;
                for (;;) {
                    if (accept("\n"))
                        goto line;
                    if (accept(delim))
                        break;
                    filename.push_back(get());
                }
                accept(" ");
                if (!accept("\n"))
                    goto line;
                include(filename);
                continue;
            }

        line:
            while (get() != '\n');
            emit(source.substr(line, state.i-line));
            if (got_version)
                version();
        }
    }
};

}

Shader::Shader(GLenum type, Source source)
{
    error_check ec("Shader constructor");
    object = glCreateShader(type);

    struct Input {
        // Nvidia driver ignores multiple source strings in error message line
        // numbers, so no point trying to meaningfully divide the source into
        // source strings. Instead, build a single string with appropriate #line
        // directives.
        ostringstream src;
        list<string> filenames;
        bool seen_version = false;

        void input(const Source& source) {
            if (!source.files)
                src << source.source;
            else {
                bool line_set = false;
                long line_number = 1;
                auto unit_number = filenames.size();
                filenames.push_back(source.name);
                auto include = [&] (string filename) {
                    if (!source.files->count(filename)) {
                        ostringstream err;
                        err << "included file '" << filename << "' not found";
                        throw logic_error(err.str());
                    }
                    src << "// #include " << filename << '\n';
                    input((*source.files)[filename]);
                    line_number++;
                    line_set = false;
                };
                auto version = [&] () {
                    seen_version = true;
                };
                auto emit = [&] (string line) {
                    if (seen_version && !line_set++)
                        src << "#line " << line_number << ' '
                                        << unit_number << '\n';
                    line_number++;
                    src << line;
                };
                Preprocessor(source.source).parse(include, version, emit);
            }
        }
    };
    Input input;
    input.input(source);
    auto src = input.src.str();
    const GLchar* string = src.data();
    const GLint length = src.size(); // TODO: check size limits
    glShaderSource(object, 1, &string, &length);
    glCompileShader(object);

    GLint status = GL_FALSE;
    glGetShaderiv(object, GL_COMPILE_STATUS, &status);
    if (!status) {
        ostringstream err;
        err << "OpenGL error compiling ";
        switch (type) {
            case GL_VERTEX_SHADER:   err << "vertex shader";   break;
            case GL_FRAGMENT_SHADER: err << "fragment shader"; break;
            default:                 err << "shader";
        }
        auto log = get_info_log(object, glGetShaderiv, glGetShaderInfoLog);
        if (!log.empty())
            err << ":\n" << log;
        if (!input.filenames.empty()) {
            // TODO: instead of listing, rewrite driver error message to replace
            // unit number with filename, optionally explaining which file and
            // line number it was included from, all the way to the main file
            err << "shader files:\n";
            int unit = 0;
            for (auto& filename: input.filenames)
                err << setw(5) << unit++ << '\t' << filename << '\n';
        }
        throw runtime_error(err.str());
    }
}

namespace pgamecc { namespace gl { namespace detail {
template<> void
Object<Shader>::destroy()
{
    error_check ec("Shader destructor");
    glDeleteShader(object);
}
}}}

Shader
Shader::vertex(Source source)
{
    return { GL_VERTEX_SHADER, move(source) };
}

Shader
Shader::fragment(Source source)
{
    return { GL_FRAGMENT_SHADER, move(source) };
}



//
// gl::Program
//

Program::Program()
{
    error_check ec("Program constructor");
    object = glCreateProgram();
}

Program::Program(Source vertex_source, Source fragment_source) :
    Program()
{
    attach(Shader::vertex(move(vertex_source)));
    attach(Shader::fragment(move(fragment_source)));
    link();
}

namespace pgamecc { namespace gl { namespace detail {
template<> void
Object<Program>::destroy()
{
    error_check ec("Program destructor");
    glDeleteProgram(object);
}
}}}

void
Program::attach(const Shader& shader)
{
    error_check ec("Program::attach()");
    glAttachShader(object, shader.object);
}


template<typename Link>
static void
link_program(GLuint program, Link link, GLenum status_name,
             const char* error_message)
{
    error_check ec("Program::link");
    link(program);
    GLint status = GL_FALSE;
    glGetProgramiv(program, status_name, &status);
    if (!status) {
        ostringstream err;
        err << error_message;
        auto log = get_info_log(program, glGetProgramiv, glGetProgramInfoLog);
        if (!log.empty())
            err << ":\n" << log;
        throw runtime_error(err.str());
    }
}

void
Program::link()
{
    link_program(object, glLinkProgram, GL_LINK_STATUS,
                 "OpenGL error linking program");
}

void
Program::validate() const
{
    link_program(object, glValidateProgram, GL_VALIDATE_STATUS,
                 "OpenGL error validating program");
}


Program::Uniform
Program::uniform(const string& name)
{
    error_check ec("Program::uniform()");

    GLint location = glGetUniformLocation(object, name.c_str());
    if (location == -1) {
        ostringstream err;
        err << "no uniform named '" << name << "' in program";
        throw logic_error(err.str());
    }

    return { object, location };
}

static GLuint
current_program() {
    GLint program;
    glGetIntegerv(GL_CURRENT_PROGRAM, &program);
    return program; // implementation-defined conversion, but should be fine
}

namespace {
struct program_and_error_check : error_check {
    program_and_error_check(GLuint program, const string function) :
        error_check(function)
    {
#ifndef NDEBUG
        if (program != current_program()) {
            ostringstream err;
            err << "called " << function << " while different program in use";
            throw logic_error(err.str());
        }
#endif
    }
};
}

void
Program::Uniform::set(int value) const
{
    program_and_error_check ec(program, "Program::Uniform::set(int)");
    glUniform1i(location, value);
}

void
Program::Uniform::set(float value) const
{
    program_and_error_check ec(program, "Program::Uniform::set(double)");
    glUniform1d(location, value);
}

void
Program::Uniform::set(const glm::vec2& value) const
{
    program_and_error_check ec(program, "Program::Uniform::set(glm::vec2)");
    glUniform2fv(location, 1, glm::value_ptr(value));
}

void
Program::Uniform::set(const glm::vec3& value) const
{
    program_and_error_check ec(program, "Program::Uniform::set(glm::vec3)");
    glUniform3fv(location, 1, glm::value_ptr(value));
}

void
Program::Uniform::set(const glm::vec4& value) const
{
    program_and_error_check ec(program, "Program::Uniform::set(glm::vec4)");
    glUniform4fv(location, 1, glm::value_ptr(value));
}

void
Program::Uniform::set(const glm::mat2& value) const
{
    program_and_error_check ec(program, "Program::Uniform::set(glm::mat2)");
    glUniformMatrix2fv(location, 1, GL_FALSE, glm::value_ptr(value));
}

void
Program::Uniform::set(const glm::mat3& value) const
{
    program_and_error_check ec(program, "Program::Uniform::set(glm::mat3)");
    glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(value));
}

void
Program::Uniform::set(const glm::mat4& value) const
{
    program_and_error_check ec(program, "Program::Uniform::set(glm::mat4)");
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
}


int
Program::Uniform::get_int() const
{
    // TODO: verify uniform type for debugging
    error_check ec("Program::Uniform::get_int()");
    GLint result;
    glGetUniformiv(program, location, &result);
    return result;
}


Program::UniformBlock
Program::uniform_block(const string& name)
{
    error_check ec("Program::uniform_block()");

    GLuint index = glGetUniformBlockIndex(object, name.c_str());
    if (index == GL_INVALID_INDEX) {
        ostringstream err;
        err << "no uniform block named '" << name << "' in program";
        throw logic_error(err.str());
    }

    return { object, index };
}

void
Program::UniformBlock::bind(GLuint binding) const
{
    error_check ec("Program::UniformBlock::bind()");
    glUniformBlockBinding(program, index, binding);
}

size_t
Program::UniformBlock::size_bytes() const
{
    error_check ec("Program::UniformBlock::size_bytes()");
    GLint param;
    glGetActiveUniformBlockiv(program, index,
                              GL_UNIFORM_BLOCK_DATA_SIZE, &param);
    return param;
}

Program::UniformBlock::Uniform
Program::UniformBlock::uniform(const std::string& name)
{
    error_check ec("Program::UniformBlock::uniform()");
    const char* names[1] = { name.c_str() };
    GLuint indices[1];
    GLint params[1];
    glGetUniformIndices(program, 1, names, indices);
    glGetActiveUniformsiv(program, 1, indices, GL_UNIFORM_OFFSET, params);
    return { program, index, params[0] };
}

void
Program::UniformBlock::Uniform::set(int value) const
{
    GLint copy = value;
    error_check ec("Program::UniformBlock::Uniform::set()");
    glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof copy, &copy);
}

void
Program::UniformBlock::Uniform::set(double value) const
{
    GLfloat copy = value;
    error_check ec("Program::UniformBlock::Uniform::set()");
    glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof copy, &copy);
}

void
Program::UniformBlock::Uniform::set_void(const void* data, size_t size) const
{
    // TODO: program check? buffer bound check?
    error_check ec("Program::UniformBlock::Uniform::set()");
    glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
}


Program::Attrib
Program::attrib(const string& name)
{
    error_check ec("Program::attrib()");

    GLint index = glGetAttribLocation(object, name.c_str());
    if (index == -1) {
        ostringstream err;
        err << "no attrib named '" << name << "' in program";
        throw logic_error(err.str());
    }
    return attrib((GLuint)index);
}

Program::Attrib
Program::attrib(GLuint index)
{
    return { object, (GLuint)index };
}

void
Program::Attrib::instanced_(GLuint divisor) const
{
    program_and_error_check ec(program, "Program::Attrib::instanced()");
    glVertexAttribDivisor(index, divisor);
}

namespace pgamecc {
namespace gl {
template<> void
Program::Attrib::array_buffer<GLfloat>(
    const detail::TargetBuffer<GL_ARRAY_BUFFER>& array,
    int attrib_size, size_t offset, size_t stride) const
{
    program_and_error_check ec(program, "Program::Attrib::array()");
    glEnableVertexAttribArray(index);
    array.bind();
    glVertexAttribPointer(index, attrib_size, GL_FLOAT, GL_FALSE,
                          stride, (GLvoid*)(offset * sizeof(GLfloat)));
    array.unbind();
}

template<> void
Program::Attrib::array_buffer<GLint>(
    const detail::TargetBuffer<GL_ARRAY_BUFFER>& array,
    int attrib_size, size_t offset, size_t stride) const
{
    program_and_error_check ec(program, "Program::Attrib::array()");
    glEnableVertexAttribArray(index);
    array.bind();
    glVertexAttribIPointer(index, attrib_size, GL_INT,
                           stride, (GLvoid*)(offset * sizeof(GLint)));
    array.unbind();
}
}
}

void
Program::Attrib::array(GLuint binding) const
{
    program_and_error_check ec(program, "Program::Attrib::array()");
    glVertexAttribBinding(index, binding);
}

void
Program::Attrib::unarray() const
{
    program_and_error_check ec(program, "Program::Attrib::unarray()");
    glDisableVertexAttribArray(index);
}


void
Program::Attrib::set(float value) const
{
    program_and_error_check ec(program, "Program::Attrib::set()");
    unarray();
    glVertexAttrib1f(index, value);
}

void
Program::Attrib::set(const glm::vec2& value) const
{
    program_and_error_check ec(program, "Program::Attrib::set()");
    unarray();
    glVertexAttrib2fv(index, glm::value_ptr(value));
}

void
Program::Attrib::set(const glm::vec3& value) const
{
    program_and_error_check ec(program, "Program::Attrib::set()");
    unarray();
    glVertexAttrib3fv(index, glm::value_ptr(value));
}

void
Program::Attrib::set(const glm::vec4& value) const
{
    program_and_error_check ec(program, "Program::Attrib::set()");
    unarray();
    glVertexAttrib4fv(index, glm::value_ptr(value));
}


void
Program::use() const
{
    error_check ec("Program::use()");
    glUseProgram(object);
}

void
Program::unuse()
{
    error_check ec("Program::unuse()");
    glUseProgram(0);
}
