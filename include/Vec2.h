#pragma once
#include <cmath>
struct Vec2 {
    float x{}, y{};
    Vec2() = default;
    Vec2(float _x, float _y) : x(_x), y(_y) {}
    Vec2 operator+(const Vec2& o) const { return {x + o.x, y + o.y}; }
    Vec2 operator-(const Vec2& o) const { return {x - o.x, y - o.y}; }
    Vec2 operator*(float s)      const { return {x * s,   y * s  }; }
    Vec2& operator+=(const Vec2& o){ x += o.x; y += o.y; return *this; }
    Vec2& operator*=(float s)      { x *= s;   y *= s;   return *this; }
    float  dot(const Vec2& o) const { return x*o.x + y*o.y; }
    float  len()             const { return std::sqrt(dot(*this)); }
};
