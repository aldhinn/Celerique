/*

File: ./include/celerique/math.h
Author: Aldhinn Espinas
Description: This header file contains data structure and function declarations
    for mathematical operations.

License: Mozilla Public License 2.0. (See ./LICENSE).

*/

#if !defined(CELERIQUE_MATH_HEADER_FILE)
#define CELERIQUE_MATH_HEADER_FILE

#include <celerique/logging.h>
#include <celerique/types.h>

// Begin C++ Only Region.
#if defined(__cplusplus)
#include <utility>
#include <stdexcept>
#include <initializer_list>
#include <type_traits>
#include <algorithm>
#include <numeric>

namespace celerique {
    /// @brief A value of this type describes the size of a stack allocated array.
    typedef CeleriqueArraySize ArraySize;

    /// @brief A template class for a static m by n matrix of `TData` data type.
    /// @tparam TData The type of each element in this matrix.
    /// @tparam numRows The number of row vectors.
    /// @tparam numCols The number of column vectors.
    template<ArraySize numRows, ArraySize numCols, typename TData>
    class Matrix;

    /// @brief A template class for a static n-dimensional vector of `TData` data type.
    /// @tparam numElements The number of elements this vector can hold.
    /// @tparam TData The type of each element in this vector.
    template<ArraySize numElements, typename TData>
    class Vec final {
    public:
        /// @brief Initializer list constructor.
        /// @param values The list of values the data is going to be initialized with.
        inline Vec(const ::std::initializer_list<TData>& values = {}) {
            if (values.size() > numElements) {
                ::std::string errorMessage = "Cannot initialize a " + ::std::to_string(numElements) +
                    "-component vector with " + ::std::to_string(values.size()) + " elements.";
                celeriqueLogError(errorMessage);
                throw ::std::out_of_range(errorMessage);
            }

            // Copy values to _data.
            auto endOfDataWithValuesIterator = ::std::copy(values.begin(), values.end(), _data);
            // Store zeros if the values provided are not sufficient to fill.
            ::std::fill(endOfDataWithValuesIterator, ::std::end(_data), static_cast<TData>(0));
        }

        /// @brief The size of the vector.
        /// @return The number of elements this vector can hold..
        inline ArraySize size() const { return numElements; }
        /// @brief Reset the data with a specified value.
        /// @param value (default 0). The specified value to be set for all members.
        inline void reset(TData value = static_cast<TData>(0)) {
            ::std::fill(::std::begin(_data), ::std::end(_data), value);
        }
        /// @brief Get the reference to the element in the specified index.
        /// @param index The index to be accessed.
        /// @return The reference at the specified index.
        inline TData& operator[](ArraySize index) {
            if (index >= numElements) {
                ::std::string errorMessage = "Unable to access index " + ::std::to_string(index) +
                    " of a " + ::std::to_string(numElements) + "-component vector. (First index is 0).";
                celeriqueLogError(errorMessage);
                throw ::std::out_of_range(errorMessage);
            }
            return _data[index];
        }
        /// @brief Get the reference to the element in the specified index.
        /// @param index The index to be accessed.
        /// @return The const reference at the specified index.
        inline const TData& operator[](ArraySize index) const {
            if (index >= numElements) {
                ::std::string errorMessage = "Unable to access index " + ::std::to_string(index) +
                    " of a " + ::std::to_string(numElements) + "-component vector. (First index is 0).";
                celeriqueLogError(errorMessage);
                throw ::std::out_of_range(errorMessage);
            }
            return _data[index];
        }

    // Copy constructors and assignment operators.
    public:
        /// @brief Copy constructor.
        /// @param other The other instance this is copying data from.
        inline Vec(const Vec& other) {
            ::std::copy(::std::begin(other._data), ::std::end(other._data), _data);
        }
        /// @brief Copy assignment operator.
        /// @param other The other instance this is copying data from.
        /// @return The reference to this object.
        inline Vec& operator=(const Vec& other) {
            ::std::copy(::std::begin(other._data), ::std::end(other._data), _data);
            return *this;
        }

    // Private member variables.
    private:
        /// @brief The stack allocated container for the data.
        TData _data[numElements];

    public:
        // `TData` restrictions.
        static_assert(
            ::std::is_integral<TData>::value | std::is_floating_point<TData>::value |
            ::std::is_enum<TData>::value | !::std::is_same<TData, bool>::value,
            "TData should be a non-struct, non-class and non-boolean type."
        );
        // `numElements` restrictions.
        static_assert(numElements >= 2, "Vectors with less than 2 elements are scalars.");

    // Friend methods.
    public:
        /// @brief The dot product operation.
        /// @tparam T The type of each element in this vector.
        /// @tparam N The number of elements this vector can hold.
        /// @param leftVec The left hand side vector.
        /// @param rightVec The right hand side vector.
        /// @return The dot product result.
        template<ArraySize N, typename T>
        friend T operator*(
            const ::celerique::Vec<N, T>& leftVec,
            const ::celerique::Vec<N, T>& rightVec
        );
        /// @brief The dot product operation.
        /// @tparam T The type of each element in this vector.
        /// @tparam N The number of elements this vector can hold.
        /// @param leftVec The left hand side vector.
        /// @param rightVec The right hand side vector.
        /// @return The dot product result.
        template<ArraySize N, typename T>
        friend bool operator==(
            const ::celerique::Vec<N, T>& leftVec,
            const ::celerique::Vec<N, T>& rightVec
        );
        /// @brief The dot product operation between a matrix and a vector.
        /// @tparam T The type of each element in this matrix.
        /// @tparam R The number of row vectors.
        /// @tparam C The number of column vectors.
        /// @param leftMat The left-hand side matrix.
        /// @param rightVec The right-hand side vector.
        /// @return The resulting dot product vector.
        template<ArraySize R, ArraySize C, typename T>
        friend ::celerique::Vec<R, T> operator*(
            const ::celerique::Matrix<R, C, T>& leftMat,
            const ::celerique::Vec<C, T>& rightVec
        );
    };

    /// @brief A 2-dimensional float vector.
    typedef Vec<2, float> Vec2;
    /// @brief A 3-dimensional float vector.
    typedef Vec<3, float> Vec3;
    /// @brief A 4-dimensional float vector.
    typedef Vec<4, float> Vec4;

    /// @brief The dot product operation.
    /// @tparam TData The type of each element in this vector.
    /// @tparam numElements The number of elements this vector can hold.
    /// @param leftVec The left hand side vector.
    /// @param rightVec The right hand side vector.
    /// @return The dot product result.
    template<ArraySize numElements, typename TData>
    inline TData operator*(
        const ::celerique::Vec<numElements, TData>& leftVec,
        const ::celerique::Vec<numElements, TData>& rightVec
    ) {
        // Summing all the products of the left vector's n^th element to the right vector's n^th element.
        return ::std::inner_product(
            ::std::begin(leftVec._data), ::std::end(leftVec._data),
            ::std::begin(rightVec._data), static_cast<TData>(0)
        );
    }

    /// @brief The equality operation.
    /// @tparam TData The type of each element in this vector.
    /// @tparam numElements The number of elements this vector can hold.
    /// @param leftVec The left hand side vector.
    /// @param rightVec The right hand side vector.
    /// @return The equality value.
    template<ArraySize numElements, typename TData>
    inline bool operator==(
        const ::celerique::Vec<numElements, TData>& leftVec,
        const ::celerique::Vec<numElements, TData>& rightVec
    ) {
        // Iterate over each others's component values and compare.
        return ::std::equal(::std::begin(leftVec._data), ::std::end(leftVec._data), ::std::begin(rightVec._data));
    }

    /// @brief The inequality operation.
    /// @tparam TData The type of each element in this vector.
    /// @tparam numElements The number of elements this vector can hold.
    /// @param leftVec The left hand side vector.
    /// @param rightVec The right hand side vector.
    /// @return The inequality value.
    template<ArraySize numElements, typename TData>
    inline bool operator!=(
        const ::celerique::Vec<numElements, TData>& leftVec,
        const ::celerique::Vec<numElements, TData>& rightVec
    ) {
        return !(leftVec == rightVec);
    }

    /// @brief A template class for a static m by n matrix of `TData` data type.
    /// @tparam TData The type of each element in this matrix.
    /// @tparam numRows The number of row vectors.
    /// @tparam numCols The number of column vectors.
    template<ArraySize numRows, ArraySize numCols, typename TData>
    class Matrix final {
    public:
        /// @brief Initializer list constructor.
        /// @param rowVectors The list of row vectors the data is going to be initialized with.
        inline Matrix(const ::std::initializer_list<::std::initializer_list<TData>>& rowVectors = {}) {
            if (rowVectors.size() > numCols) {
                ::std::string errorMessage = "Cannot initialize a " + ::std::to_string(numRows) +
                    "x" + ::std::to_string(numCols) + " matrix with more than " +
                    ::std::to_string(numRows) + " row vectors.";
                celeriqueLogError(errorMessage);
                throw ::std::out_of_range(errorMessage);
            }

            // Row index tracker.
            ArraySize rowIndex = 0;
            for (const ::std::initializer_list<TData>& rowVec : rowVectors) {
                if (rowVec.size() > numCols) {
                    ::std::string errorMessage = "Cannot initialize a " + ::std::to_string(numRows) + "x"
                        + ::std::to_string(numCols) + " matrix with a row vector that has more than " +
                        ::std::to_string(numCols) + " elements.";
                    celeriqueLogError(errorMessage);
                    throw ::std::out_of_range(errorMessage);
                }
                // Copy rowVec to _data.
                auto endOfDataWithValuesIterator = ::std::copy(rowVec.begin(), rowVec.end(), _data[rowIndex]);
                // Store zeros if the values provided are not sufficient to fill.
                ::std::fill(endOfDataWithValuesIterator, ::std::end(_data[rowIndex]), static_cast<TData>(0));
                // Track row index being assigned.
                rowIndex++;
            }
            // Store zeros if there aren't enough row vectors provided to fill the matrix.
            ::std::fill(::std::begin(_data[rowIndex]), ::std::end(_data[numRows - 1]), static_cast<TData>(0));
        }

        /// @brief The number of row vectors.
        /// @return `numRows` value.
        inline ArraySize rows() const { return numRows; }
        /// @brief The number of column vectors.
        /// @return `numCols` value.
        inline ArraySize columns() const { return numCols; }
        /// @brief Reset the data with a specified value.
        /// @param value (default 0). The specified value to be set for all members.
        inline void reset(TData value = static_cast<TData>(0)) {
            ::std::fill_n(_data[0], numRows * numCols, value);
        }
        /// @brief Get the reference to the element in the specified indices.
        /// @param rowIndex The row index to be accessed.
        /// @param colIndex The column index to be accessed.
        /// @return The reference to the specified index.
        inline TData& operator()(ArraySize rowIndex, ArraySize colIndex) {
            if (rowIndex >= numRows || colIndex >= numCols) {
                ::std::string errorMessage = "Unable to access indices " + ::std::to_string(rowIndex) +
                    "," + ::std::to_string(colIndex) + " of a " + ::std::to_string(numRows) + "x" +
                    ::std::to_string(numCols) + " matrix. (First index is 0).";
                celeriqueLogError(errorMessage);
                throw ::std::out_of_range(errorMessage);
            }
            return _data[rowIndex][colIndex];
        }
        /// @brief Get the reference to the element in the specified indices.
        /// @param rowIndex The row index to be accessed.
        /// @param colIndex The column index to be accessed.
        /// @return The const reference to the specified index.
        inline const TData& operator()(ArraySize rowIndex, ArraySize colIndex) const {
            if (rowIndex >= numRows || colIndex >= numCols) {
                ::std::string errorMessage = "Unable to access indices " + ::std::to_string(rowIndex) +
                    "," + ::std::to_string(colIndex) + " of a " + ::std::to_string(numRows) + "x" +
                    ::std::to_string(numCols) + " matrix. (First index is 0).";
                celeriqueLogError(errorMessage);
                throw ::std::out_of_range(errorMessage);
            }
            return _data[rowIndex][colIndex];
        }

    // Copy constructors and assignment operators.
    public:
        /// @brief Copy constructor.
        /// @param other The other instance this is copying data from.
        inline Matrix(const Matrix& other) {
            ::std::copy_n(other._data[0], numRows * numCols, _data[0]);
        }
        /// @brief Copy assignment operation.
        /// @param other The other instance this is copying data from.
        /// @return The reference to this instance.
        inline Matrix& operator=(const Matrix& other) {
            ::std::copy_n(other._data[0], numRows * numCols, _data[0]);
            return *this;
        }

    // Private member variables.
    private:
        /// @brief The stack allocated container for the data.
        TData _data[numRows][numCols];

    public:
        // `TData` restrictions.
        static_assert(
            ::std::is_integral<TData>::value | std::is_floating_point<TData>::value |
            ::std::is_enum<TData>::value | !::std::is_same<TData, bool>::value,
            "TData should be a non-struct, non-class and non-boolean type."
        );
        // `numRows` and `numCols` restrictions.
        static_assert(
            (numRows >= 2 || numRows >= 2) && numRows > 0 && numCols > 0,
            "1x1 or 0x0 matrices are non-sense."
        );

    // Friend methods.
    public:
        /// @brief The dot product operation between a matrix and a vector.
        /// @tparam T The type of each element in this matrix.
        /// @tparam R The number of row vectors.
        /// @tparam C The number of column vectors.
        /// @param leftMat The left-hand side matrix.
        /// @param rightVec The right-hand side vector.
        /// @return The resulting dot product vector.
        template<ArraySize R, ArraySize C, typename T>
        friend ::celerique::Vec<R, T> operator*(
            const ::celerique::Matrix<R, C, T>& leftMat,
            const ::celerique::Vec<C, T>& rightVec
        );
        /// @brief The equality operation.
        /// @tparam T The type of each element in this matrix.
        /// @tparam R The number of row vectors.
        /// @tparam C The number of column vectors.
        /// @param leftMat The left-hand side matrix.
        /// @param rightMat The right-hand side matrix.
        /// @return The equality value.
        template<ArraySize R, ArraySize C, typename T>
        friend bool operator==(
            const ::celerique::Matrix<R, C, T>& leftMat,
            const ::celerique::Matrix<R, C, T>& rightMat
        );
        /// @brief The dot product operation between a matrix and another matrix.
        /// @tparam TData The type of each element in this matrix.
        /// @tparam numRowsLeft The number of row vectors on the left-hand side matrix.
        /// @tparam numColsLeft The number of column vectors on the left-hand side matrix.
        /// @tparam numRowsRight The number of row vectors on the right-hand side matrix.
        /// @tparam numColsRight The number of column vectors on the right-hand side matrix.
        /// @param leftMat The left-hand side matrix.
        /// @param rightMat The right-hand side matrix.
        /// @return The resulting dot product matrix.
        template<ArraySize numRowsLeft, ArraySize numColsLeft,
        ArraySize numRowsRight, ArraySize numColsRight, typename T>
        friend ::celerique::Matrix<numRowsLeft, numColsRight, T> operator*(
            const ::celerique::Matrix<numRowsLeft, numColsLeft, T>& leftMat,
            const ::celerique::Matrix<numRowsRight, numColsRight, T>& rightMat
        );
    };

    /// @brief A 2x2 float matrix.
    typedef Matrix<2, 2, float> Mat2x2;
    /// @brief A 3x3 float matrix.
    typedef Matrix<3, 3, float> Mat3x3;
    /// @brief A 4x4 float matrix.
    typedef Matrix<4, 4, float> Mat4x4;

    /// @brief The dot product operation between a matrix and a vector.
    /// @tparam TData The type of each element in this matrix.
    /// @tparam numRows The number of row vectors.
    /// @tparam numCols The number of column vectors.
    /// @param leftMat The left-hand side matrix.
    /// @param rightVec The right-hand side vector.
    /// @return The resulting dot product vector.
    template<ArraySize numRows, ArraySize numCols, typename TData>
    inline ::celerique::Vec<numRows, TData> operator*(
        const ::celerique::Matrix<numRows, numCols, TData>& leftMat,
        const ::celerique::Vec<numCols, TData>& rightVec
    ) {
        // The container for the resulting dot product.
        ::celerique::Vec<numRows, TData> product;
        // Iterate over all the row vectors and get the dot product with the vector.
        for (ArraySize rowIndex = 0; rowIndex < numRows; rowIndex++) {
            // Iterate over the columns and dot product with the vector and sum their component products.
            product[rowIndex] = ::std::inner_product(
                ::std::begin(leftMat._data[rowIndex]), ::std::end(leftMat._data[rowIndex]),
                ::std::begin(rightVec._data), static_cast<TData>(0)
            );
        }

        return product;
    }

    /// @brief The dot product operation between a matrix and another matrix.
    /// @tparam TData The type of each element in this matrix.
    /// @tparam numRowsLeft The number of row vectors on the left-hand side matrix.
    /// @tparam numColsLeft The number of column vectors on the left-hand side matrix.
    /// @tparam numRowsRight The number of row vectors on the right-hand side matrix.
    /// @tparam numColsRight The number of column vectors on the right-hand side matrix.
    /// @param leftMat The left-hand side matrix.
    /// @param rightMat The right-hand side matrix.
    /// @return The resulting dot product matrix.
    template<ArraySize numRowsLeft, ArraySize numColsLeft,
    ArraySize numRowsRight, ArraySize numColsRight, typename TData>
    inline ::celerique::Matrix<numRowsLeft, numColsRight, TData> operator*(
        const ::celerique::Matrix<numRowsLeft, numColsLeft, TData>& leftMat,
        const ::celerique::Matrix<numRowsRight, numColsRight, TData>& rightMat
    ) {
        static_assert(
            numColsLeft == numRowsRight,
            "The number of column vectors on the left-hand side matrix must match "
            "the number of row vectors on the right-hand side matrix."
        );

        // The container for the resulting dot product.
        ::celerique::Matrix<numRowsLeft, numColsRight, TData> matrixProduct;
        // Iterate over the product indices.
        for (ArraySize rowIndex = 0; rowIndex < numRowsLeft; rowIndex++) {
            for (ArraySize colIndex = 0; colIndex < numColsRight; colIndex++) {
                // The container for the vector dot product.
                TData result = static_cast<TData>(0);
                // The value of the element in the product matrix is the dot product
                // of the left-hand side matrix's row vector and the right-hand side
                // matrix's column vector.
                for (ArraySize i = 0; i < numRowsRight; i++) {
                    result += leftMat(rowIndex, i) * rightMat(i, colIndex);
                }
                matrixProduct(rowIndex, colIndex) = result;
            }
        }

        return matrixProduct;
    }

    /// @brief The equality operation.
    /// @tparam TData The type of each element in this matrix.
    /// @tparam numRows The number of row vectors.
    /// @tparam numCols The number of column vectors.
    /// @param leftMat The left-hand side matrix.
    /// @param rightMat The right-hand side matrix.
    /// @return The equality value.
    template<ArraySize numRows, ArraySize numCols, typename TData>
    inline bool operator==(
        const ::celerique::Matrix<numRows, numCols, TData>& leftMat,
        const ::celerique::Matrix<numRows, numCols, TData>& rightMat
    ) {
        // Iterate over each others's component values and compare.
        return ::std::equal(
            ::std::begin(leftMat._data[0]), ::std::end(leftMat._data[numRows-1]),
            ::std::begin(rightMat._data[0])
        );
    }

    /// @brief The inequality operation.
    /// @tparam TData The type of each element in this matrix.
    /// @tparam numRows The number of row vectors.
    /// @tparam numCols The number of column vectors.
    /// @param leftMat The left-hand side matrix.
    /// @param rightMat The right-hand side matrix.
    /// @return The inequality value.
    template<ArraySize numRows, ArraySize numCols, typename TData>
    inline bool operator!=(
        const ::celerique::Matrix<numRows, numCols, TData>& leftMat,
        const ::celerique::Matrix<numRows, numCols, TData>& rightMat
    ) {
        return !(leftMat == rightMat);
    }
}

/// @brief The dot product operation.
/// @tparam TData The type of each element in this vector.
/// @tparam numElements The number of elements this vector can hold..
/// @param leftVec The left hand side vector.
/// @param rightVec The right hand side vector.
/// @return The dot product result.
template<CeleriqueArraySize numElements, typename TData>
inline TData operator*(
    const ::celerique::Vec<numElements, TData>& leftVec,
    const ::celerique::Vec<numElements, TData>& rightVec
) {
    return ::celerique::operator*(leftVec, rightVec);
}

/// @brief The equality operation.
/// @tparam TData The type of each element in this vector.
/// @tparam numElements The number of elements this vector can hold.
/// @param leftVec The left hand side vector.
/// @param rightVec The right hand side vector.
/// @return The equality value.
template<CeleriqueArraySize numElements, typename TData>
inline bool operator==(
    const ::celerique::Vec<numElements, TData>& leftVec,
    const ::celerique::Vec<numElements, TData>& rightVec
) {
    return ::celerique::operator==(leftVec, rightVec);
}

/// @brief The inequality operation.
/// @tparam TData The type of each element in this vector.
/// @tparam numElements The number of elements this vector can hold.
/// @param leftVec The left hand side vector.
/// @param rightVec The right hand side vector.
/// @return The inequality value.
template<CeleriqueArraySize numElements, typename TData>
inline bool operator!=(
    const ::celerique::Vec<numElements, TData>& leftVec,
    const ::celerique::Vec<numElements, TData>& rightVec
) {
    return ::celerique::operator!=(leftVec, rightVec);
}

/// @brief The dot product operation between a matrix and a vector.
/// @tparam TData The type of each element in this matrix.
/// @tparam numRows The number of row vectors.
/// @tparam numCols The number of column vectors.
/// @param leftMat The left-hand side matrix.
/// @param rightVec The right-hand side vector.
/// @return The resulting dot product vector.
template<CeleriqueArraySize numRows, CeleriqueArraySize numCols, typename TData>
inline ::celerique::Vec<numRows, TData> operator*(
    const ::celerique::Matrix<numRows, numCols, TData>& leftMat,
    const ::celerique::Vec<numCols, TData>& rightVec
) {
    return ::celerique::operator*(leftMat, rightVec);
}

/// @brief The dot product operation between a matrix and another matrix.
/// @tparam TData The type of each element in this matrix.
/// @tparam numRowsLeft The number of row vectors on the left-hand side matrix.
/// @tparam numColsLeft The number of column vectors on the left-hand side matrix.
/// @tparam numRowsRight The number of row vectors on the right-hand side matrix.
/// @tparam numColsRight The number of column vectors on the right-hand side matrix.
/// @param leftMat The left-hand side matrix.
/// @param rightMat The right-hand side matrix.
/// @return The resulting dot product matrix.
template<CeleriqueArraySize numRowsLeft, CeleriqueArraySize numColsLeft,
CeleriqueArraySize numRowsRight, CeleriqueArraySize numColsRight, typename TData>
inline ::celerique::Matrix<numRowsLeft, numColsRight, TData> operator*(
    const ::celerique::Matrix<numRowsLeft, numColsLeft, TData>& leftMat,
    const ::celerique::Matrix<numRowsRight, numColsRight, TData>& rightMat
) {
    return ::celerique::operator*(leftMat, rightMat);
}

/// @brief The equality operation.
/// @tparam TData The type of each element in this matrix.
/// @tparam numRows The number of row vectors.
/// @tparam numCols The number of column vectors.
/// @param leftMat The left-hand side matrix.
/// @param rightMat The right-hand side matrix.
/// @return The equality value.
template<CeleriqueArraySize numRows, CeleriqueArraySize numCols, typename TData>
inline bool operator==(
    const ::celerique::Matrix<numRows, numCols, TData>& leftMat,
    const ::celerique::Matrix<numRows, numCols, TData>& rightMat
) {
    return ::celerique::operator==(leftMat, rightMat);
}

/// @brief The inequality operation.
/// @tparam TData The type of each element in this matrix.
/// @tparam numRows The number of row vectors.
/// @tparam numCols The number of column vectors.
/// @param leftMat The left-hand side matrix.
/// @param rightMat The right-hand side matrix.
/// @return The inequality value.
template<CeleriqueArraySize numRows, CeleriqueArraySize numCols, typename TData>
inline bool operator!=(
    const ::celerique::Matrix<numRows, numCols, TData>& leftMat,
    const ::celerique::Matrix<numRows, numCols, TData>& rightMat
) {
    return ::celerique::operator!=(leftMat, rightMat);
}

#endif
// End C++ Only Region.

#endif
// End of file.
// DO NOT WRITE BEYOND HERE.