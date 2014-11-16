#ifndef PGAMECC_LOC_H
#define PGAMECC_LOC_H

#include <pgamecc/types.h>

namespace pgamecc {


// location of some object

struct dloc {
    dvec3 p; // position
    dquat q; // orientation

    dloc& operator+=(dvec3 t) { p += t; return *this; }
    dloc& operator-=(dvec3 t) { p -= t; return *this; }
    dloc operator+(dvec3 t) const { dloc r = *this; return r += t; }
    dloc operator-(dvec3 t) const { dloc r = *this; return r -= t; }
    dvec3 operator*(dvec3 x) const { return q * x + p; }
    dloc operator*(dloc l) const { return { (*this) * l.p, q * l.q }; }
    dloc operator~() const { auto i = glm::inverse(q); return { i * -p, i }; };

    glm::dmat4 mat4_cast() const {
        return glm::translate(p) * glm::mat4_cast(q);
    }

    static dloc from_mat4(glm::dmat4 M) {
        return { dvec3(M[3]), glm::quat_cast(glm::dmat3(M)) };
    }

    friend std::ostream& operator<<(std::ostream& os, dloc l) {
        return os << "dloc{" << l.p << ", " << l.q << '}';
    }
};


// index of an octant (0-7)

class ioct {
    // class used transiently so no point saving space with a char
    int b;

public:
    explicit ioct(int b) : b(b) {}
    ioct(bool x, bool y, bool z) : b(x | y << 1 | z << 2) {}
    ioct(bvec3 v) : ioct(v.x, v.y, v.z) {}

    operator bool() const { return b; }

    bool operator==(ioct _) { return b == _.b; }
    bool operator!=(ioct _) { return b != _.b; }

    bool x() const { return b & 1; }
    bool y() const { return b & 2; }
    bool z() const { return b & 4; }

    int i() const { return b; }

    bvec3 bvec3_cast() const { return bvec3(x(), y(), z()); }

    ivec3 operator*(int h) const {
        return ivec3(x() * h, y() * h, z() * h);
    }

    ioct operator~() { return ioct{b ^ (1<<3)-1}; }

    // iterate over entire domain

    struct all {
        class iterator {
            int b;

            iterator(int b) : b(b) {}
            friend class all;

        public:
            iterator& operator++() { b++; return *this; }
            bool operator==(iterator it) const { return b == it.b; }
            bool operator!=(iterator it) const { return b != it.b; }
            ioct operator*() { return ioct{b}; }
        };

        static iterator begin() { return iterator{0}; }
        static iterator end()   { return iterator{1<<3}; }
    };

    friend std::ostream& operator<<(std::ostream& os, ioct b) {
        return os << "ioct{0b" <<
            b.x()["01"] << b.y()["01"] << b.z()["01"] << "}";
    }
};


// binary state of eight octants

class boct {
    int b;

    class bit {
        int& b;
        int i;

        bit(int& b, int i) : b(b), i(i) {}
        friend class boct;

    public:
        operator bool() { return b & 1 << i; }

        void operator|=(bool value) { b |= value << i; }
        void operator=(bool value) { b = b & ((1<<8)-1 ^ 1 << i) | value << i; }
    };

public:
    explicit boct(int b) : b(b) {}

    static boct empty() { return boct{0}; }

    operator bool() const { return b; }

    bool operator==(boct _) { return b == _.b; }
    bool operator!=(boct _) { return b != _.b; }

    bool operator[](ioct i) const { return b & 1 << i.i(); }
    bit operator[](ioct i) { return { b, i.i() }; }

    int i() const { return b; }

    boct operator~() { return boct{b ^ (1<<8)-1}; }

    // iterate over entire domain

    struct all {
        class iterator {
            int b;

            iterator(int b) : b(b) {}
            friend class all;

        public:
            iterator& operator++() { b++; return *this; }
            bool operator==(iterator it) const { return b == it.b; }
            bool operator!=(iterator it) const { return b != it.b; }
            boct operator*() { return boct{b}; }
        };

        static iterator begin() { return iterator{0}; }
        static iterator end()   { return iterator{1<<8}; }
    };

    friend std::ostream& operator<<(std::ostream& s, const boct b) {
        s << "boct{0b";
        for (auto i: ioct::all())
            s << b[~i];
        return s << "}";
    }
};


// axis-aligned rotations and reflections (octahedral symmetry group)

class irot {
    // Which axis and with what sign in original orientation corresponds to
    // given axis in rotated orientation.
    struct {
        // -3 to 3; 3 bits would actually suffice
        int x : 4;
        int y : 4;
        int z : 4;
    };

    irot(int x, int y, int z) : x(x), y(y), z(z) {}

    static int get(ivec3 v, int i) {
        assert(-3 <= i && i <= 3 && i != 0);
        int x = v[abs(i)-1];
        return i < 0 ? -x : x;
    }

    static int get(ioct v, int i) {
        assert(-3 <= i && i <= 3 && i != 0);
        bool x = v.i() >> abs(i)-1 & 1;
        return i < 0 ? !x : x;
    }

    int get(int i) const {
        return get(ivec3(x, y, z), i);
    }

    irot rotate(int angle, const int a, const int b, const int c) const {
        angle = ((angle % 4) + 4) % 4;
        irot r = *this;
        for (int i = 0; i < angle; i++)
            r = irot(r.get(a), r.get(b), r.get(c));
        return r;
    }

    irot repeat(int count, int modulo) {
        count = ((count % modulo) + modulo) % modulo;
        irot r;
        for (int i = 0; i < count; i++)
            r = (*this) * r;
        return r;
    }

public:
    irot() : x(1), y(2), z(3) {}

    static irot rotate_x(int a = 1) { return irot{1, -3, 2}.repeat(a, 4); }
    static irot rotate_y(int a = 1) { return irot{3, 2, -1}.repeat(a, 4); }
    static irot rotate_z(int a = 1) { return irot{-2, 1, 3}.repeat(a, 4); }

    static irot rotate_xyz(int a = 1) { return irot{3, 1, 2}.repeat(a, 3); }

    static irot flip_x() { return {-1, 2, 3}; }
    static irot flip_y() { return {1, -2, 3}; }
    static irot flip_z() { return {1, 2, -3}; }

    static irot face(ioct i) {
        bool flip = i.x() ^ i.y() ^ i.z();
        return {(1-i.x()*2)*(flip?2:1), (1-i.y()*2)*(flip?1:2), (1-i.z()*2)*3};
    }

    irot operator*(irot r) const {
        return { r.get(x), r.get(y), r.get(z) };
    }

    irot operator~() const {
        int m[3];
        if (x < 0) m[-x-1] = -1; else m[x-1] = 1;
        if (y < 0) m[-y-1] = -2; else m[y-1] = 2;
        if (z < 0) m[-z-1] = -3; else m[z-1] = 3;
        return { m[0], m[1], m[2] };
    }

    ivec3 operator*(ivec3 v) const {
        return ivec3(get(v, x), get(v, y), get(v, z));
    }

    ioct operator*(ioct i) const {
        return { get(i, x), get(i, y), get(i, z) };
    }

    // rotate octant values
    boct operator*(const boct b) const {
        // TOOD: optimize
        boct r{0};
        for (auto i: ioct::all())
            r[*this * i] |= b[i];
        return r;
    }

    bool operator==(irot r) const { return x == r.x && y == r.y && z == r.z; }
    bool operator!=(irot r) const { return !(*this == r); }

    dquat quat_cast() const {
        // Determined by experimentation; this is essentially a lookup table.
        // Unspecified for reflections.
        constexpr auto d = 1 / std::sqrt(2.);
        if (abs(x)==1 && abs(y)==2) {
            dquat q{0, 0, 0, 0};
            q[y>0|(z>0)<<1] = 1;
            return q;
        } else if (abs(x)==2 && abs(y)==3)
            return dquat(.5, y>0?-.5:.5, z>0?-.5:.5, x>0?-.5:.5);
        else if (abs(x)==3 && abs(y)==1)
            return dquat(.5, z<0?-.5:.5, x<0?-.5:.5, y<0?-.5:.5);
        else if (abs(x)==1)
            return x>0 ? dquat(d, z>0?d:-d, 0, 0) : dquat(0, 0, d, z>0?d:-d);
        else if (abs(x)==2)
            return z>0 ? dquat(d, 0, 0, y>0?d:-d) : dquat(0, y>0?d:-d, d, 0);
        else
            return y>0 ? dquat(d, 0, x>0?d:-d, 0) : dquat(0, d, 0, x>0?d:-d);
    }

    friend std::ostream& operator<<(std::ostream& s, irot r) {
        return s << "irot{" << r.x << ", " << r.y << ", " << r.z << "}";
    }
};


// location on an integer grid

struct iloc {
    ivec3 p; // position
    irot r; // orientation

    iloc& operator+=(ivec3 t) { p += t; return *this; }
    iloc& operator-=(ivec3 t) { p -= t; return *this; }
    iloc operator+(ivec3 t) const { iloc r = *this; return r += t; }
    iloc operator-(ivec3 t) const { iloc r = *this; return r -= t; }
    ivec3 operator*(ivec3 x) const { return r * x + p; }
    iloc operator*(iloc l) const { return { (*this) * l.p, r * l.r }; }
    iloc operator~() const { auto i = ~r; return { i * -p, i }; };

    bool operator==(iloc _) const { return p == _.p && r == _.r; }
    bool operator!=(iloc _) const { return !(*this == _); }

    dloc dloc_cast() const { return { dvec3(p), r.quat_cast() }; }

    friend std::ostream& operator<<(std::ostream& os, iloc l) {
        return os << "iloc{" << l.p << ", " << l.r << '}';
    }
};


}

#endif
