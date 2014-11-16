#include "color.h"

using namespace pgamecc::color;


static double
gamma_compress(double v) {
    return v > .0031308 ? 1.055 * std::pow(v, 1/2.4) - .055
                        : 12.92 * v;
}

static double
gamma_expand(double v) {
    return v > .04045 ? std::pow((v + .055) / 1.055, 2.4)
                        : v / 12.92;
}

static double
hue_as_red(double h) {
    h = (h < 0 ? 6 : 0) + std::fmod(h, 6); // normalize to [0, 6]
    return std::max(0., std::min(1., -1 + std::abs(h - 3)));
};


sRGB
RGB::srgb() const {
    return { gamma_compress(r),
             gamma_compress(g),
             gamma_compress(b) };
}


RGB
sRGB::rgb() const {
    return { gamma_expand(r),
             gamma_expand(g),
             gamma_expand(b) };
}


HSL
sRGB::hue() const {
    return HSL::hue(hsl().h);
}


HSL
sRGB::hsl() const {
    double M = std::max({ r, g, b }),
           m = std::min({ r, g, b }),
           c = M - m,
           l = .5 * (M + m);
    double h = c == 0 ? 0 :
               M == g ? 2 + (b - r) / c :
               M == b ? 4 + (r - g) / c :
                        (g < b ? 6 : 0) + (g - b) / c;
    double s = c == 0 ? 0 : c / (1 - std::fabs(2 * l - 1));
    return { h, s, l };
}


YCH
sRGB::luma() const {
    return YCH::luma(ych().y);
}


YCH
sRGB::ych() const {
    double M = std::max({ r, g, b }),
           m = std::min({ r, g, b }),
           c = M - m;
    double y = .2126 * r + .7152 * g + .0722 * b; // Rec. 709 formula
    return { y, c, hue().h };
}


sRGB
HSL::srgb() const {
    double c = s * (1 - std::fabs(2 * l - 1));
    double m = l - c / 2;
    double r = hue_as_red(h),
           g = hue_as_red(h - 2),
           b = hue_as_red(h - 4);
    return { r * c + m,
             g * c + m,
             b * c + m };
}


sRGB
YCH::srgb() const {
    auto v1 = HSL::hue(h).srgb();
    double y1 = v1.luma().y;
    return { (v1.r - y1) * c + y,
             (v1.g - y1) * c + y,
             (v1.b - y1) * c + y };
}
