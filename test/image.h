#include <pgamecc/image.h>

#include <iostream>

using std::ostream;

namespace pgamecc {

template<typename Pixel>
inline ostream&
operator<<(ostream& os, Image<Pixel> m) {
    os << "Image(";
    for (int j = 0; j < m.size().y; j++) {
        if (j)
            os << ' ';
        os << '{';
        for (int i = 0; i < m.size().x; i++) {
            if (i)
                os << ' ';
            os << m[ivec2{i, j}];
        }
        os << '}';
    }
    os << ')';
    return os;
}

}
