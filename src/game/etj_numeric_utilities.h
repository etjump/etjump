#pragma once

namespace Numeric
{
    template <typename T>
    T clamp(T value, double min, double max)
    {
        if (value < min)
        {
            value = min;
        }
        else if (value > max)
        {
            value = max;
        }
        return value;
    }
}