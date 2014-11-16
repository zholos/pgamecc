#ifndef PGAMECC_FILES_H
#define PGAMECC_FILES_H

#include <map>
#include <string>


namespace pgamecc {

class Files;

struct Source {
    const Files* files;
    std::string name;
    std::string source;

    Source(std::string source) : files(nullptr), source(source) {}
    Source(const char* source) : Source(std::string(source)) {}

private:
    Source(const Files& files, std::string name, std::string source) :
        files(&files), name(name), source(source) {}
    friend class Files;
};

class Files {
    typedef std::map<std::string, std::string> data_type;
    const data_type data;

public:
    Files(data_type data) : data(std::move(data)) {}
    bool count(const std::string& name) const { return data.count(name); }
    Source operator[](const std::string& name) const {
        return { *this, name, data.at(name) };
    }
};


} // pgamecc

#endif
