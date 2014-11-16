#!/usr/bin/env python2

import sys, os.path, argparse, itertools

def encode_data(data):
    if all(32 <= ord(c) < 127 or c in "\t\r\n" for c in data) and \
            "??" not in data:
        for repeat in itertools.count():
            for delim in itertools.product("\"'", repeat=repeat):
                delim = "".join(delim)
                if delim not in data:
                    print 'R"' + delim + '(' + data + ')' + delim + '"'
                    return
    else:
        s = ""
        for c in data:
            if c in '\\"':
                c = '\\' + c
            elif not 32 <= ord(c) < 127 or c == '?': # avoid trigraphs
                c = "\\{:03o}".format(ord(c))
            if len(s) + len(c) > 78:
                print '"' + s + '"'
                s = c
            else:
                s += c
        if len(s) > 77:
            print '"' + s + '"'
            s = ""
        print '"' + s + '"s'

def main():
    parser = argparse.ArgumentParser(
        description="Convert data files into a C++ source file. "
                    "For use with pgamecc.")
    parser.add_argument("-o", metavar="output")
    parser.add_argument("-b", action="store_true")
    parser.add_argument("-v", metavar="variable", required=True)
    parser.add_argument("file", nargs="*")
    args = parser.parse_args()

    if args.o:
        sys.stdout = open(args.o, "w")

    namespace, _, variable = args.v.rpartition("::")

    print "#include <pgamecc/files.h>"
    print "using namespace std::string_literals;"
    print
    if namespace:
        print "namespace " + namespace + " {"
    print "pgamecc::Files " + variable + "{{{"
    for filename in args.file:
        with open(filename, "rb") as file:
            if args.b:
                filename = os.path.basename(filename)
            print '    { "' + filename + '",'
            encode_data(file.read())
            print '},'

    print "}}};"
    if namespace:
        print "}"

if __name__ == "__main__":
    main()
