#pragma once
#include <vector>

template <typename T>
class Grid
{
private:
    size_t m_width;
    size_t m_height;
    std::vector<T> m_data;

public:
    Grid(size_t width, size_t height)
        : m_width(width)
        , m_height(height)
    {
        m_data.resize(width * height);
    }

    size_t Width()
    {
        return m_width;
    }

    size_t Height()
    {
        return m_height;
    }

    void Clear()
    {
        m_data.clear();
        m_data.resize(m_width * m_height);
    }

    T& operator[](std::pair<size_t, size_t> pair)
    {
        return m_data[pair.second * m_width + pair.first];
    }

    T& operator[](size_t index)
    {
        return m_data[index];
    }

#pragma region row-major iterators
    typename std::vector<T>::iterator begin()
    {
        return m_data.begin();
    }

    typename std::vector<T>::iterator end()
    {
        return m_data.end();
    }
#pragma endregion
};