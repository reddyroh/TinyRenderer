#pragma once

#include <cmath>
#include <iostream>

template <class t> struct Vec2 {
	// share the same memory space: provide multiple ways of accessing the vector's data
	union {
		struct { t x, y; };
		struct { t u, v; };
		t raw[2];
	};

	Vec2() : x(0), y(0) {}
	Vec2(t _x, t _y) : x(_x), y(_y) {}

	// Addition
	inline Vec2<t> operator +(const Vec2<t>& V) const { 
		return Vec2<t>(x + V.x, y + V.y); 
	}

	// Subtraction
	inline Vec2<t> operator -(const Vec2<t>& V) const { 
		return Vec2<t>(x - V.x, y - V.y); 
	}

	// Scalar multiplication
	inline Vec2<t> operator *(float f) const { 
		return Vec2<t>(x * f, y * f); 
	}

	// Output formatting
	template <class> friend std::ostream& operator<<(std::ostream& s, Vec2<t>& v);
};

template <class t> struct Vec3 {
	union {
		struct { t x, y, z; };
		struct { t r, g, b; };
		t raw[3];
	};

	Vec3() : x(0), y(0), z(0) {}
	Vec3(t _x, t _y, t _z) : x(_x), y(_y), z(_z) {}

	// Addition
	inline Vec3<t> operator +(const Vec3<t>& V) const {
		return Vec3<t>(x + V.x, y + V.y, z + V.z); 
	};

	// Subtraction
	inline Vec3<t> operator -(const Vec3<t>& V) const {
		return Vec3<t>(x - V.x, y - V.y, z - V.z);
	};

	// Scalar multiplication
	inline Vec3<t> operator *(float f) const { 
		return Vec3<t>(x * f, y * f, z * f); 
	};

	// Cross product
	inline Vec3<t> operator ^(const Vec3<t>& V) const { 
		return Vec3<t>(y * V.z - z * V.y, z * V.x - x * V.z, x * V.y - y * V.x);
	};

	// Dot product
	inline t operator *(const Vec3<t>& V) const { 
		return x * V.x + y * V.y + z * V.z; 
	}

	// Norm
	float norm() const { return std::sqrt(x*x + y*y + z*z); }

	// Normalize
	Vec3<t>& normalize(t l=1) {
		*this = (*this) * (l / norm());
		return *this;
	}

	// Output formatting
	template <class> friend std::ostream& operator<<(std::ostream& s, Vec3<t>& v);
};

typedef Vec2<float> Vec2f;
typedef Vec2<int> Vec2i;
typedef Vec3<float> Vec3f;
typedef Vec3<int> Vec3i;

template <class t> std::ostream& operator<<(std::ostream& s, Vec2<t>& v) {
	s << "(" << v.x << ", " << v.y << ")\n";
	return s;
}

template <class t> std::ostream& operator<<(std::ostream& s, Vec3<t>& v) {
	s << "(" << v.x << ", " << v.y << ", " << v.z << ")\n";
	return s;
}