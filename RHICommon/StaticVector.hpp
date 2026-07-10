#pragma once

#include <array>
#include <cassert>
#include <initializer_list>
#include <utility>
#include <vector>

namespace RHI {

template <typename T, std::size_t Capacity>
class StaticVector {
  public:
    using value_type = T;
    using size_type = std::size_t;
    using iterator = typename std::array<T, Capacity>::iterator;
    using const_iterator = typename std::array<T, Capacity>::const_iterator;

    StaticVector() = default;

    StaticVector(std::initializer_list<T> values) {
        assign(values.begin(), values.end());
    }

    template <typename Allocator>
    StaticVector(const std::vector<T, Allocator> &values) {
        assign(values.begin(), values.end());
    }

    StaticVector &operator=(std::initializer_list<T> values) {
        assign(values.begin(), values.end());
        return *this;
    }

    template <typename Allocator>
    StaticVector &operator=(const std::vector<T, Allocator> &values) {
        assign(values.begin(), values.end());
        return *this;
    }

    template <typename Iterator>
    void assign(Iterator first, Iterator last) {
        clear();
        for (; first != last; ++first) {
            push_back(*first);
        }
    }

    void resize(const size_type size) {
        assert(size <= Capacity);
        if (size < m_Size) {
            for (size_type index = size; index < m_Size; ++index) {
                m_Data[index] = T{};
            }
        } else {
            for (size_type index = m_Size; index < size; ++index) {
                m_Data[index] = T{};
            }
        }
        m_Size = size;
    }

    void clear() {
        resize(0);
    }

    void push_back(const T &value) {
        assert(m_Size < Capacity);
        m_Data[m_Size++] = value;
    }

    void push_back(T &&value) {
        assert(m_Size < Capacity);
        m_Data[m_Size++] = std::move(value);
    }

    [[nodiscard]]
    bool empty() const {
        return m_Size == 0;
    }

    [[nodiscard]]
    size_type size() const {
        return m_Size;
    }

    [[nodiscard]]
    static constexpr size_type capacity() {
        return Capacity;
    }

    T *data() {
        return m_Data.data();
    }

    const T *data() const {
        return m_Data.data();
    }

    iterator begin() {
        return m_Data.begin();
    }

    const_iterator begin() const {
        return m_Data.begin();
    }

    const_iterator cbegin() const {
        return m_Data.cbegin();
    }

    iterator end() {
        return m_Data.begin() + static_cast<std::ptrdiff_t>(m_Size);
    }

    const_iterator end() const {
        return m_Data.begin() + static_cast<std::ptrdiff_t>(m_Size);
    }

    const_iterator cend() const {
        return m_Data.cbegin() + static_cast<std::ptrdiff_t>(m_Size);
    }

    T &operator[](const size_type index) {
        assert(index < m_Size);
        return m_Data[index];
    }

    const T &operator[](const size_type index) const {
        assert(index < m_Size);
        return m_Data[index];
    }

    T &back() {
        assert(m_Size > 0);
        return m_Data[m_Size - 1];
    }

    const T &back() const {
        assert(m_Size > 0);
        return m_Data[m_Size - 1];
    }

  private:
    std::array<T, Capacity> m_Data{};
    size_type m_Size = 0;
};

} // namespace RHI
