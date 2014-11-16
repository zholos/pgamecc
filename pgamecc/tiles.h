#ifndef PGAMECC_TILES_H
#define PGAMECC_TILES_H

#include <pgamecc/types.h>
#include <pgamecc/util.h>

#include <algorithm>
#include <cmath>
#include <map>

namespace pgamecc {

// A grid is a planar arrangement of tiles. Each tile has a distinct pair of
// integer coordinates.

// A tiling determines how tile coordinates map to cartesian coordinates and
// which coordinates specify neighboring tiles.

// In this hex tiling the axes are set at a 60-degree angle, and neighbors are
// numbered counterclockwise starting with [0] in the x direction. Illustration:
//
//                                               ,. y
//                                               /'
//                                              /
//                             ,'.   ,'.   ,'.
//                       [2] .'   `.'   `.'   `. [1]
//                           |-2, 2|-1, 2| 0, 2|
//                          ,'.   ,'.   ,'.   ,'.
//                        .'   `.'   `.'   `.'   `.
//                        |-2, 1|-1, 1| 0, 1| 1, 1|
//                       ,'.   ,'.   ,'.   ,'.   ,'.
//                     .'   `.'   `.'   `.'   `.'   `.
//                     |-2, 0|-1, 0| 0, 0| 1, 0| 2, 0|   ---> x
//                 [3] '.   ,'.   ,'.   ,'.   ,'.   ,' [0]
//                       `.'   `.'   `.'   `.'   `.'
//                        |-1,-1| 0,-1| 1,-1| 2,-1|
//                        '.   ,'.   ,'.   ,'.   ,'
//                          `.'   `.'   `.'   `.'
//                           | 0,-2| 1,-2| 2,-2|
//                       [4] '.   ,'.   ,'.   ,' [5]
//                             `.'   `.'   `.'
//

class HexTiling {
    // produces the repeating sequence 1, 0, -1, -1, 0, 1, ...
    static int wave(int a) {
        return (std::abs(5-2*mod_down(a, 6))-3)/2;
    }

public:
    int dist(ivec2 from, ivec2 to) {
        ivec2 d = from - to;
        return d.x < 0 ? d.y < 0 ? -d.x - d.y : std::max(-d.x, d.y) :
                         d.y < 0 ? std::max(d.x, -d.y) : d.x + d.y;
    };

    static ivec2 next(ivec2 center, int a, int step = 1) {
        return center + ivec2(wave(a), wave(a+4)) * step;
    }


    // iterators for shaped groups of tiles

    class DiskTiles {
        const ivec2 center;
        const int inner, outer; // equal for circle, 0 to radius for disk

    public:
        class iterator {
            const ivec2 center;
            int r; // current circle radius
            int a; // angle of first tile of edge
            int i; // tile number in edge

        public:
            iterator(ivec2 center, int r, int a, int i) :
                center(center), r(r), a(a), i(i) {}

            iterator& operator++() {
                if (++i >= r) {
                    i = 0;
                    if (++a == 6) {
                        a = 0;
                        r++;
                    }
                }
                return *this;
            }

            bool operator==(iterator it) const {
                assert(center == it.center);
                return r == it.r && a == it.a && i == it.i;
            }
            bool operator!=(iterator it) const { return !(*this == it); }

            ivec2 operator*() {
                return next(next(center, a, r), a+2, i);
            }
        };

        DiskTiles(ivec2 center, int inner, int outer) :
            center(center), inner(inner), outer(outer)
        {
        }

        iterator begin() const {
            return { center, inner, inner ? 0 : 5, 0 };
        }

        iterator end() const {
            return { center, outer+1, 0, 0 };
        }
    };

    static DiskTiles circle(ivec2 center, int radius) {
        return { center, radius, radius };
    }

    static DiskTiles disk(ivec2 center, int radius) {
        return { center, 0, radius };
    }


    // cartesian x axis coincides with the grid x axis and tile side is 1

    // coordinates of center of tile
    dvec2 to_cartesian(ivec2 center) {
        return dvec2(std::sqrt(3) * (center.x + center.y*.5), center.y*1.5);
    }

    ivec2 from_cartesian(dvec2 coords) {
        dvec2 u = dvec2(coords.x/std::sqrt(3) - coords.y/3, coords.y/1.5);

        int a = std::floor(u.y - u.x);
        int b = std::floor(2*u.x + u.y);
        int c = std::floor(2*u.y + u.x);

        return ivec2(div_down(b - a + 1, 3), div_down(c + a + 2, 3));
    }
};


template<typename Value>
using Grid = std::map<ivec2, Value, ivec2_compare>;


}

#endif
