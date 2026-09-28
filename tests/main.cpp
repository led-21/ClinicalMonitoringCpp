#include "TestFramework.h"
#include "NewsCalculatorTests.h"
#include "IcuBedManagementTests.h"
#include "ObserverNotificationTests.h"

#include <exception>
#include <iostream>
#include <vector>

int main()
{
    std::vector<TestCase> tests;
    RegisterNewsCalculatorTests(tests);
    RegisterIcuBedManagementTests(tests);
    RegisterObserverNotificationTests(tests);

    int failed = 0;
    for (const auto& test : tests)
    {
        try
        {
            test.action();
            std::cout << "[PASS] " << test.name << "\n";
        }
        catch (const std::exception& ex)
        {
            ++failed;
            std::cout << "[FAIL] " << test.name << " - " << ex.what() << "\n";
        }
        catch (...)
        {
            ++failed;
            std::cout << "[FAIL] " << test.name << " - unknown exception\n";
        }
    }

    std::cout << "\n========================================\n"
              << "Test Results: " << tests.size() << " tests executed\n"
              << "Passed: " << (tests.size() - static_cast<std::size_t>(failed)) << "\n"
              << "Failed: " << failed << "\n"
              << "========================================\n";

    return failed == 0 ? 0 : 1;
}
