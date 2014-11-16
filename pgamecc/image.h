#ifndef PGAMECC_IMAGE_H
#define PGAMECC_IMAGE_H

#include <pgamecc/types.h>

#include <iterator>
#include <map>
#include <stdexcept>
#include <type_traits>
#include <vector>

namespace pgamecc {

template<typename Color>
class Gradient {
    std::map<double, Color> points;

public:
    Color operator()(double v) const {
        // x in range [it0, it1)
        auto it1 = points.upper_bound(v);
        if (it1 == points.begin())
            return it1 == points.end() ? Color() : it1->second;
        auto it0 = std::prev(it1);
        if (it1 == points.end())
            return it0->second;

        // linear interpolation
        double a = (v - it0->first) / (it1->first - it0->first);
        return it0->second * (1-a) + it1->second * a;
    }

    Color& operator[](double v) {
        return points.insert(std::make_pair(v, (*this)(v))).first->second;
    }
};



template<typename Color>
class Image;

namespace detail {

// helper to call constructor for correct type
template<typename Func>
auto
make_image_sized(ivec2 size, const Func& f) ->
    Image<std::remove_reference_t<decltype(f(size))>>
{
    return { size, f };
}

}

template<typename Color>
class Image {
    ivec2 _size;
    std::vector<Color> _pixels;

public:
    Image(ivec2 size) :
        _size(size), _pixels((size_t)size.x * size.y) {}

    template<typename Func>
    Image(ivec2 size, const Func& f) :
        _size(size)
    {
        _pixels.reserve((size_t)size.x * size.y);
        for (int y = 0; y < size.y; y++)
            for (int x = 0; x < size.x; x++)
                _pixels.push_back(f(ivec2(x, y)));
    }

    ivec2 size() const { return _size; }
    const std::vector<Color>& pixels() const { return _pixels; }

private:
    size_t offset(ivec2 i) const {
        if (i.x < 0 || i.x >= _size.x || i.y < 0 || i.y >= _size.y)
            throw std::out_of_range("coordinates outside Image bounds");
        return i.x + i.y * _size.x;
    }

public:
    Color  operator[](ivec2 i) const { return _pixels[offset(i)]; }
    Color& operator[](ivec2 i)       { return _pixels[offset(i)]; }

private:
    struct SamplerBase {
        const Image& image;

        Color sample(ivec2 i) const {
            return image[glm::clamp(i, ivec2(0), image._size-1)];
        }
    };

    class LinearSampler : SamplerBase {
        using SamplerBase::sample;

    public:
        LinearSampler(const Image& image) : SamplerBase{image} {}

        Color operator()(dvec2 p) const {
            auto f = p * dvec2(LinearSampler::image._size) - .5;
            auto i = ivec2(glm::floor(f));
            f -= i;

            // fast shortcut for common case
            if (f == dvec2(0, 0))
                return sample(i);

            auto v0 = (1-f.x) * sample(i) + f.x * sample(i+ivec2(1, 0));
            auto v1 = (1-f.x) * sample(i+ivec2(0, 1)) + f.x * sample(i+1);
            return (1-f.y) * v0 + f.y * v1;
        }
    };

public:
    LinearSampler linear() const { return LinearSampler{*this}; }

public:
    template<typename Func>
    auto apply(const Func& f) {
        return detail::make_image_sized(
            _size,
            [&](ivec2 i) { return f((*this)[i]); });
    }
};

template<typename Func>
auto
make_image(ivec2 size, const Func& f) {
    return detail::make_image_sized(
        size,
        [&](ivec2 i) { return f((dvec2(i)+.5)/dvec2(size)); });
}

}

#endif
