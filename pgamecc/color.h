#ifndef PGAMECC_COLOR_H
#define PGAMECC_COLOR_H

#include <algorithm>
#include <cmath>
#include <iostream>


namespace pgamecc {
namespace color {

struct sRGB;
struct  HSL;
struct  YCH;

// Linear light
struct RGB {
    double r, g, b;

    static RGB gray(double v) { return { v, v, v }; }

    RGB operator+(RGB _) const { return { r + _.r, g + _.g, b + _.b }; }
    RGB operator-(RGB _) const { return { r - _.r, g - _.g, b - _.b }; }
    RGB operator+(double v) const { return (*this) + gray(v); }
    RGB operator-(double v) const { return (*this) - gray(v); }
    RGB operator*(double v) const { return { r * v, g * v, b * v }; }
    RGB operator/(double v) const { return { r / v, g / v, b / v }; }

    friend RGB operator+(double v, RGB _) { return RGB::gray(v) + _; }
    friend RGB operator-(double v, RGB _) { return RGB::gray(v) - _; }
    friend RGB operator*(double v, RGB _) { return { v*_.r, v*_.g, v*_.b }; }
    friend RGB operator/(double v, RGB _) { return { v/_.r, v/_.g, v/_.b }; }

    sRGB srgb() const;

    friend std::ostream& operator<<(std::ostream& os, RGB _) {
        return os << "color::RGB(" << _.r << " " << _.g << " " << _.b << ")";
    }
};


struct sRGB {
    double r, g, b;

    RGB rgb()  const;
    HSL hue()  const; // max saturation
    HSL hsl()  const;
    YCH luma() const; // min saturation
    YCH ych()  const;

    friend std::ostream& operator<<(std::ostream& os, sRGB _) {
        return os << "color::sRGB(" << _.r << " " << _.g << " " << _.b << ")";
    }
};

struct sRGBA {
    double r, g, b, a;

    sRGBA(sRGB _, double alpha = 1) : r(_.r), g(_.g), b(_.b), a(alpha) {}
};

struct HSL {
    double h, s, l;

    static HSL hue(double h) { return { h, 1, .5 }; }

    HSL operator+(double _h) const { return { h + _h, s, l }; }
    HSL operator-(double _h) const { return { h - _h, s, l }; }

    sRGB srgb() const;
};

struct YCH {
    double y, c, h;

    static YCH luma(double y) { return { y, 0, 0 }; }

    YCH operator+(YCH _) const { return { y + _.y, c + _.c, h + _.h }; }
    YCH operator-(YCH _) const { return { y - _.y, c - _.c, h - _.h }; }
    YCH operator+(double _y) const { return { y + _y, c, h }; }
    YCH operator-(double _y) const { return { y - _y, c, h }; }

    sRGB srgb() const;
};


} // color
} // pgamecc

#endif
