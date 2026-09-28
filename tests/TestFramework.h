#pragma once

#include <functional>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

struct TestCase
{
    std::string name;
    std::function<void()> action;
};

inline void AssertTrue(bool condition, const std::string& message)
{
    if (!condition)
    {
        throw std::runtime_error(message);
    }
}

template <typename TExpected, typename TActual>
inline void AssertEqual(const TExpected& expected, const TActual& actual, const std::string& message)
{
    if (!(expected == actual))
    {
        std::ostringstream output;
        output << message << " | expected: " << expected << " actual: " << actual;
        throw std::runtime_error(output.str());
    }
}
