#ifndef Magnum_Math_BoolVector_h
#define Magnum_Math_BoolVector_h
/*
    Copyright © 2010, 2011, 2012 Vladimír Vondruš <mosra@centrum.cz>

    This file is part of Magnum.

    Magnum is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License version 3
    only, as published by the Free Software Foundation.

    Magnum is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License version 3 for more details.
*/

/** @file
 * @brief Class Magnum::Math::BoolVector
 */

#include <cstdint>
#include <Utility/Debug.h>

namespace Magnum { namespace Math {

#ifndef DOXYGEN_GENERATING_OUTPUT
namespace Implementation {
    template<std::size_t ...> struct Sequence {};

    /* E.g. GenerateSequence<3>::Type is Sequence<0, 1, 2> */
    template<std::size_t N, std::size_t ...sequence> struct GenerateSequence:
        GenerateSequence<N-1, N-1, sequence...> {};

    template<std::size_t ...sequence> struct GenerateSequence<0, sequence...> {
        typedef Sequence<sequence...> Type;
    };

    template<class T> inline constexpr T repeat(T value, std::size_t) { return value; }
}
#endif

/**
@brief %Vector storing boolean values
@tparam size    Bit count

Result of component-wise comparison from Vector. The boolean values are stored
as bits in array of unsigned bytes, unused bits have undefined value which
doesn't affect comparison or all() / none() / any() functions. See also
@ref matrix-vector for brief introduction.
*/
template<std::size_t size> class BoolVector {
    static_assert(size != 0, "BoolVector cannot have zero elements");

    public:
        static const std::size_t Size = size;               /**< @brief %Vector size */
        static const std::size_t DataSize = (size-1)/8+1;   /**< @brief %Vector storage size */

        /** @brief Construct zero-filled boolean vector */
        inline constexpr BoolVector(): _data() {}

        /**
         * @brief Construct boolean vector from segment values
         * @param first Value for first 8bit segment
         * @param next  Values for next Bbit segments
         */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        template<class ...T> inline constexpr /*implicit*/ BoolVector(std::uint8_t first, T... next);
        #else
        template<class ...T, class U = typename std::enable_if<sizeof...(T)+1 == DataSize, bool>::type> inline constexpr /*implicit*/ BoolVector(std::uint8_t first, T... next): _data{first, std::uint8_t(next)...} {}
        #endif

        /** @brief Construct boolean vector with one value for all fields */
        #ifdef DOXYGEN_GENERATING_OUTPUT
        inline explicit BoolVector(T value);
        #else
        template<class T, class U = typename std::enable_if<std::is_same<bool, T>::value && size != 1, bool>::type> inline constexpr explicit BoolVector(T value): BoolVector(typename Implementation::GenerateSequence<DataSize>::Type(), value ? FullSegmentMask : 0) {}
        #endif

        /** @brief Copy constructor */
        inline constexpr BoolVector(const BoolVector<size>&) = default;

        /** @brief Copy assignment */
        inline BoolVector<size>& operator=(const BoolVector<size>&) = default;

        /**
         * @brief Raw data
         * @return %Array of DataSize length
         *
         * @see operator[](), set()
         */
        inline std::uint8_t* data() { return _data; }
        inline constexpr const std::uint8_t* data() const { return _data; } /**< @overload */

        /** @brief Bit at given position */
        inline constexpr bool operator[](std::size_t i) const {
            return (_data[i/8] >> i%8) & 0x01;
        }

        /** @brief Set bit at given position */
        inline BoolVector<size>& set(std::size_t i, bool value) {
            _data[i/8] |= ((value & 0x01) << i%8);
            return *this;
        }

        /** @brief Equality comparison */
        inline bool operator==(const BoolVector<size>& other) const {
            for(std::size_t i = 0; i != size/8; ++i)
                if(_data[i] != other._data[i]) return false;

            /* Check last segment */
            if(size%8 && (_data[DataSize-1] & LastSegmentMask) != (other._data[DataSize-1] & LastSegmentMask))
                return false;

            return true;
        }

        /** @brief Non-equality comparison */
        inline bool operator!=(const BoolVector<size>& other) const {
            return !operator==(other);
        }

        /** @brief Whether all bits are set */
        bool all() const {
            /* Check all full segments */
            for(std::size_t i = 0; i != size/8; ++i)
                if(_data[i] != FullSegmentMask) return false;

            /* Check last segment */
            if(size%8 && (_data[DataSize-1] & LastSegmentMask) != LastSegmentMask)
                return false;

            return true;
        }

        /** @brief Whether no bits are set */
        bool none() const {
            /* Check all full segments */
            for(std::size_t i = 0; i != size/8; ++i)
                if(_data[i]) return false;

            /* Check last segment */
            if(size%8 && (_data[DataSize-1] & LastSegmentMask))
                return false;

            return true;
        }

        /** @brief Whether any bit is set */
        inline bool any() const {
            return !none();
        }

        /** @brief Bitwise inversion */
        inline BoolVector<size> operator~() const {
            BoolVector<size> out;

            for(std::size_t i = 0; i != DataSize; ++i)
                out._data[i] = ~_data[i];

            return out;
        }

        /**
         * @brief Bitwise AND and assign
         *
         * The computation is done in-place.
         */
        inline BoolVector<size>& operator&=(const BoolVector<size>& other) {
            for(std::size_t i = 0; i != DataSize; ++i)
                _data[i] &= other._data[i];

            return *this;
        }

        /**
         * @brief Bitwise AND
         *
         * @see operator&=()
         */
        inline BoolVector<size> operator&(const BoolVector<size>& other) const {
            return BoolVector<size>(*this) &= other;
        }

        /**
         * @brief Bitwise OR and assign
         *
         * The computation is done in-place.
         */
        inline BoolVector<size>& operator|=(const BoolVector<size>& other) {
            for(std::size_t i = 0; i != DataSize; ++i)
                _data[i] |= other._data[i];

            return *this;
        }

        /**
         * @brief Bitwise OR
         *
         * @see operator|=()
         */
        inline BoolVector<size> operator|(const BoolVector<size>& other) const {
            return BoolVector<size>(*this) |= other;
        }

        /**
         * @brief Bitwise XOR and assign
         *
         * The computation is done in-place.
         */
        inline BoolVector<size>& operator^=(const BoolVector<size>& other) {
            for(std::size_t i = 0; i != DataSize; ++i)
                _data[i] ^= other._data[i];

            return *this;
        }

        /**
         * @brief Bitwise XOR
         *
         * @see operator^=()
         */
        inline BoolVector<size> operator^(const BoolVector<size>& other) const {
            return BoolVector<size>(*this) ^= other;
        }

    private:
        enum: std::uint8_t {
            FullSegmentMask = 0xFF,
            LastSegmentMask = (1 << size%8) - 1
        };

        /* Implementation for Vector<size, T>::Vector(U) */
        template<std::size_t ...sequence> inline constexpr explicit BoolVector(Implementation::Sequence<sequence...>, std::uint8_t value): _data{Implementation::repeat(value, sequence)...} {}

        std::uint8_t _data[(size-1)/8+1];
};

/** @debugoperator{Magnum::Math::BoolVector} */
template<std::size_t size> Corrade::Utility::Debug operator<<(Corrade::Utility::Debug debug, const BoolVector<size>& value) {
    debug << "BoolVector(";
    debug.setFlag(Corrade::Utility::Debug::SpaceAfterEachValue, false);
    for(std::size_t i = 0; i != size; ++i) {
        if(i && !(i%8)) debug << " ";
        debug << (value[i] ? "1" : "0");
    }
    debug << ")";
    debug.setFlag(Corrade::Utility::Debug::SpaceAfterEachValue, true);
    return debug;
}

}}

#endif
