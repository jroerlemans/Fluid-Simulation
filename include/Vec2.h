#pragma once
#include <cmath>

struct Vec2 {
    float x{}, y{};
    Vec2() = default;
    Vec2(float _x, float _y) : x(_x), y(_y) {}

    // --- Original Methods ---
    Vec2 operator+(const Vec2& o) const { return {x + o.x, y + o.y}; }
    Vec2 operator-(const Vec2& o) const { return {x - o.x, y - o.y}; }
    Vec2 operator*(float s)      const { return {x * s,   y * s  }; }
    Vec2 operator/(float s)      const { return {x / s,   y / s  }; } // <-- ADDED THIS
    Vec2& operator+=(const Vec2& o){ x += o.x; y += o.y; return *this; }
    Vec2& operator*=(float s)      { x *= s;   y *= s;   return *this; }
    float  dot(const Vec2& o) const { return x*o.x + y*o.y; }
    float  len()             const { return std::sqrt(dot(*this)); }

    // --- Added Methods ---
    Vec2 operator-() const { return {-x, -y}; }
    Vec2& operator-=(const Vec2& o){ x -= o.x; y -= o.y; return *this; }
    float lenSq() const { return x*x + y*y; }

    Vec2 perpendicular() const { return {-y, x}; }

    Vec2 normalized() const {
        float l = len();
        if (l > 0) {
            return {x / l, y / l};
        }
        return {0, 0};
    }
};