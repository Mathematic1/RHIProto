#pragma once

namespace RHI {
template<typename T, typename U> [[nodiscard]] bool arraysAreDifferent(const T& a, const U& b)
{
    if (a.size() != b.size())
        return true;

    for (uint32_t i = 0; i < uint32_t(a.size()); i++)
    {
        if (a[i] != b[i])
            return true;
    }

    return false;
}
}
