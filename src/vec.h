#ifndef VEC_H_
#define VEC_H_

#include <array>
#include <cassert>
#include <cmath>
#include <numeric>
#include <ostream>
#include <type_traits>
#include "matrix.h"

template <size_t SIZE, typename T>
struct Vec {
	static_assert(SIZE > 0);

	Vec(void) : data{ 0 }
	{
	}

	template<size_t MAT_SIZE>
	Vec(const Matrix<MAT_SIZE, 1, T>& m)
	{
		for (size_t i = 0; i < std::min(SIZE, MAT_SIZE); i++)
			data[i] = m(i, 0);
		for (size_t i = MAT_SIZE; i < SIZE; i++)
			data[i] = 0;
	}

	template<size_t MAT_SIZE>
	Vec(const Matrix<1, MAT_SIZE, T>& m)
	{
		for (size_t i = 0; i < std::min(SIZE, MAT_SIZE); i++)
			data[i] = m(0, i);
		for (size_t i = MAT_SIZE; i < SIZE; i++)
			data[i] = 0;
	}

	template<size_t OTHER_V_SIZE>
	Vec(const Vec<OTHER_V_SIZE, T>& v)
	{
		for (size_t i = 0; i < std::min(SIZE, OTHER_V_SIZE); i++)
			data[i] = v[i];
		for (size_t i = OTHER_V_SIZE; i < SIZE; i++)
			data[i] = 0;
	}

	template <typename... RestArgs>
	Vec(typename std::enable_if<sizeof...(RestArgs) + 1 == SIZE, T>::type first, RestArgs... rest)
		: data{ first, T(rest)... }
	{
	}

	T getx(void) const { return data[0]; }
	void putx(T val) { data[0] = val; }
	__declspec(property(get = getx, put = putx)) T x;
	__declspec(property(get = getx, put = putx)) T u;

	template <size_t SZ = SIZE, typename std::enable_if_t<SZ >= 2, int> = 0>
	T gety(void) const { return data[1]; }
	template <size_t SZ = SIZE, typename std::enable_if_t<SZ >= 2, int> = 0>
	void puty(T val) { data[1] = val; }
	__declspec(property(get = gety, put = puty)) T y;
	__declspec(property(get = gety, put = puty)) T v;

	template <size_t SZ = SIZE, typename std::enable_if_t<SZ >= 3, int> = 0>
	T getz(void) const { return data[2]; }
	template <size_t SZ = SIZE, typename std::enable_if_t<SZ >= 3, int> = 0>
	void putz(T val) { data[2] = val; }
	__declspec(property(get = getz, put = putz)) T z;

	template <size_t SZ = SIZE, typename std::enable_if_t<SZ >= 4, int> = 0>
	T getw(void) const { return data[3]; }
	template <size_t SZ = SIZE, typename std::enable_if_t<SZ >= 4, int> = 0>
	void putw(T val) { data[3] = val; }
	__declspec(property(get = getw, put = putw)) T w;

	/* Return a vector length (magnitude) */
	T length(void) const
	{
		T len = 0;
		for (size_t i = 0; i < SIZE; i++)
			len += data[i] * data[i];
		return std::sqrt(len);
	}

	Vec<SIZE, T>& normalize(void)
	{
		T len = length();
		if (len > 0) {
			T inv_len = 1 / len;
			for (auto& i : data)
				i *= inv_len;
		}
		return *this;
	}

	T dot(const Vec<SIZE, T>& rhs) const
	{
		T sum = 0;
		for (size_t i = 0; i < SIZE; i++)
			sum += data[i] * rhs.data[i];
		return sum;
	}

	T& operator[](size_t index)
	{
		assert(index < SIZE);
		return data[index];
	}

	T const& operator[](size_t index) const
	{
		assert(index < SIZE);
		return data[index];
	}

	Vec<SIZE, T>& operator+=(const Vec<SIZE, T>& rhs)
	{
		for (size_t i = 0; i < SIZE; i++)
			data[i] += rhs.data[i];
		return *this;
	}

	Vec<SIZE, T>& operator-=(const Vec<SIZE, T>& rhs)
	{
		for (size_t i = 0; i < SIZE; i++)
			data[i] -= rhs.data[i];
		return *this;
	}

	Vec<SIZE, T>& operator*=(T scalar)
	{
		for (auto& i : data)
			i *= scalar;
		return *this;
	}

	Vec<SIZE, T>& operator/=(T scalar)
	{
		for (auto& i : data)
			i /= scalar;
		return *this;
	}

	/* cross product is defined only for 3d space */
	template <size_t SZ = SIZE, typename std::enable_if_t<SZ == 3, int> = 0>
	Vec<SIZE, T>& operator^=(const Vec<SIZE, T>& rhs)
	{
		T x = this->y * rhs.z - this->z * rhs.y;
		T y = this->z * rhs.x - this->x * rhs.z;
		T z = this->x * rhs.y - this->y * rhs.x;
		this->data = { x, y, z };
		return *this;
	}

	Vec<SIZE, T> operator+(const Vec<SIZE, T>& rhs) const
	{
		auto copy = *this;
		copy += rhs;
		return copy;
	}

	Vec<SIZE, T> operator-(const Vec<SIZE, T>& rhs) const
	{
		auto copy = *this;
		copy -= rhs;
		return copy;
	}

	Vec<SIZE, T> operator*(T scalar) const
	{
		auto v = *this;
		for (auto& i : v.data)
			i *= scalar;
		return v;
	}

	T operator*(const Vec<SIZE, T>& rhs) const
	{
		T sum = 0;
		for (size_t i = 0; i < SIZE; i++)
			sum += data[i] * rhs.data[i];
		return sum;
	}

	/* cross product is defined only for 3d space */
	template <size_t SZ = SIZE, typename std::enable_if_t<SZ == 3, int> = 0>
	Vec<SIZE, T> operator^(const Vec<SIZE, T>& rhs) const
	{
		auto copy = *this;
		copy ^= rhs;
		return copy;
	}

	Vec<SIZE, T> operator/(T scalar) const
	{
		auto copy = *this;
		copy /= scalar;
		return copy;
	}

protected:
	std::array<T, SIZE> data;
};

template <size_t SIZE, typename T>
Vec<SIZE, T> operator*(T scalar, const Vec<SIZE, T>& v)
{
	return v * scalar;
}

template <size_t SIZE, typename T>
std::ostream& operator<<(std::ostream& ostream, const Vec<SIZE, T>& v)
{
	ostream << "{" << v[0];
	for (size_t i = 1; i < SIZE; i++)
		ostream << ", " << v[i];
	ostream << "}";

	return ostream;
}

using Vec2f = Vec<2, float>;
using Vec2i = Vec<2, int>;
using Vec3f = Vec<3, float>;
using Vec3i = Vec<3, int>;
using Vec4f = Vec<4, float>;

#endif /* VEC_H_ */
