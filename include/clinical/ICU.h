#pragma once

#include "Models.h"
#include "Patient.h"

#include <array>
#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
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

        Patient* CreatePatientAtBed(int bedNumber, std::string_view id, std::string_view firstName, std::string_view lastName);

        // Modern C++ API with std::optional and structured return values
        [[nodiscard]] std::optional<BedLookupResult> FindPatient(std::string_view id);
        [[nodiscard]] std::optional<ConstBedLookupResult> FindPatient(std::string_view id) const;
        std::optional<std::string> DischargePatient(std::string_view id);

        // Backward-compatible API
        [[nodiscard]] Patient* FindPatientById(const std::string& id, int* bedNumber = nullptr);
        [[nodiscard]] const Patient* FindPatientById(const std::string& id, int* bedNumber = nullptr) const;
        bool DischargePatientById(const std::string& id, std::string* patientFullName = nullptr);

        [[nodiscard]] std::vector<std::pair<int, const Patient*>> GetOccupiedBeds() const;

    private:
        [[nodiscard]] static constexpr bool IsValidBedNumber(int bedNumber) noexcept
        {
            return bedNumber >= 1 && static_cast<std::size_t>(bedNumber) <= Capacity;
        }

        std::array<std::unique_ptr<Patient>, Capacity> beds{};
    };
}
