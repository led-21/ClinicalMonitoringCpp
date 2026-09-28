#pragma once

#include "Patient.h"

#include <array>
#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace clinical
{
    class ICU
    {
    public:
        static constexpr std::size_t Capacity = 6;

        [[nodiscard]] Patient* GetPatientAtBed(int bedNumber);
        [[nodiscard]] const Patient* GetPatientAtBed(int bedNumber) const;

        Patient* CreatePatientAtBed(int bedNumber, const std::string& id, const std::string& firstName, const std::string& lastName);

        [[nodiscard]] Patient* FindPatientById(const std::string& id, int* bedNumber = nullptr);
        [[nodiscard]] const Patient* FindPatientById(const std::string& id, int* bedNumber = nullptr) const;

        bool DischargePatientById(const std::string& id, std::string* patientFullName = nullptr);

        [[nodiscard]] std::vector<std::pair<int, const Patient*>> GetOccupiedBeds() const;

    private:
        [[nodiscard]] static bool IsValidBedNumber(int bedNumber) noexcept
        {
            return bedNumber >= 1 && static_cast<std::size_t>(bedNumber) <= Capacity;
        }

        std::array<std::unique_ptr<Patient>, Capacity> beds{};
    };
}
