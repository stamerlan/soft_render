#ifndef MATRIX_H_
#define MATRIX_H_

#include <ostream>
#include <type_traits>

template<size_t ROWS, size_t COLUMNS, typename T>
class matrix_t {
	static_assert(ROWS > 0, "matrix must have at least one row");
	static_assert(COLUMNS > 0, "matrix must have at least on column");
public:
	matrix_t(void) : data_{ 0 }
	{
	}

	/* TODO: Add nested initializer list constructor */
	template <typename... RestArgs>
	matrix_t(T first, RestArgs... rest)
		: data_{ first, T(rest)... }
	{
	}

	/** Load identity matrix*/
	template <size_t R = ROWS, size_t C = COLUMNS, typename std::enable_if_t<R == C, int> = 0>
	void identity(void)
	{
		for (size_t r = 0; r < ROWS; r++)
			for (size_t c = 0; c < COLUMNS; c++)
				data_[r][c] = r == c ? static_cast<T>(1) : static_cast<T>(0);
	}

	/** Multiply matrices */
	template<size_t RHS_COLUMNS>
	matrix_t<ROWS, RHS_COLUMNS, T> operator*(const matrix_t<COLUMNS, RHS_COLUMNS, T>& rhs) const
	{
		matrix_t<ROWS, RHS_COLUMNS, T> ret;
		for (size_t r = 0; r < ROWS; r++) {
			for (size_t c = 0; c < RHS_COLUMNS; c++) {
				for (size_t i = 0; i < COLUMNS; i++)
					ret(r, c) += data_[r][i] * rhs(i, c);
			}
		}
		return ret;
	}

	matrix_t<ROWS, COLUMNS, T> operator/=(T rhs)
	{
		for (size_t i = 0; i < ROWS; i++)
			for (size_t j = 0; j < COLUMNS; j++)
				data_[i][j] /= rhs;
		return *this;
	}

	matrix_t<ROWS, COLUMNS, T> operator/(T rhs)
	{
		auto copy = *this;
		copy /= rhs;
		return copy;
	}

	T& operator()(size_t row, size_t column)
	{
		return data_[row][column];
	}

	T operator()(size_t row, size_t column) const
	{
		return data_[row][column];
	}

protected:
	T data_[ROWS][COLUMNS];
};

template<size_t ROWS, size_t COLUMNS, typename T>
std::ostream& operator<<(std::ostream& ostream, const matrix_t<ROWS, COLUMNS, T>& m)
{
	for (size_t i = 0; i < ROWS; i++) {
		ostream << "[";
		for (size_t j = 0; j < COLUMNS; j++)
			ostream << " " << m(i, j);
		ostream << "]\n";
	}

	return ostream;
}

using mat4x4f_t = matrix_t<4, 4, float>;
using mat4x1f_t = matrix_t<4, 1, float>;

#endif /* MARIX_H_ */
