#include "clinical/ICU.h"

namespace clinical
{
    Patient* ICU::GetPatientAtBed(int bedNumber)
    {
        if (!IsValidBedNumber(bedNumber))
        {
            return nullptr;
        }

        return beds[static_cast<std::size_t>(bedNumber - 1)].get();
    }

    const Patient* ICU::GetPatientAtBed(int bedNumber) const
    {
        if (!IsValidBedNumber(bedNumber))
        {
            return nullptr;
        }

        return beds[static_cast<std::size_t>(bedNumber - 1)].get();
    }

    Patient* ICU::CreatePatientAtBed(int bedNumber, std::string_view id, std::string_view firstName, std::string_view lastName)
    {
        if (!IsValidBedNumber(bedNumber))
        {
            return nullptr;
        }

        const std::size_t index = static_cast<std::size_t>(bedNumber - 1);
        if (!beds[index])
        {
            beds[index] = std::make_unique<Patient>(std::string(id), std::string(firstName), std::string(lastName));
        }

        return beds[index].get();
    }

    std::optional<BedLookupResult> ICU::FindPatient(std::string_view id)
    {
        for (std::size_t i = 0; i < beds.size(); ++i)
        {
            if (beds[i] && beds[i]->GetId() == id)
            {
                return BedLookupResult{ beds[i].get(), static_cast<int>(i + 1) };
            }
        }
        return std::nullopt;
    }

    std::optional<ConstBedLookupResult> ICU::FindPatient(std::string_view id) const
    {
        for (std::size_t i = 0; i < beds.size(); ++i)
        {
            if (beds[i] && beds[i]->GetId() == id)
            {
                return ConstBedLookupResult{ beds[i].get(), static_cast<int>(i + 1) };
            }
        }
        return std::nullopt;
    }

    std::optional<std::string> ICU::DischargePatient(std::string_view id)
    {
        for (std::size_t i = 0; i < beds.size(); ++i)
        {
            if (beds[i] && beds[i]->GetId() == id)
            {
                std::string fullName = beds[i]->GetFullName();
                beds[i].reset();
                return fullName;
            }
        }
        return std::nullopt;
    }

    Patient* ICU::FindPatientById(const std::string& id, int* bedNumber)
    {
        const auto result = FindPatient(id);
        if (result.has_value())
        {
            if (bedNumber)
            {
                *bedNumber = result->bedNumber;
            }
            return result->patient;
        }
        return nullptr;
    }

    const Patient* ICU::FindPatientById(const std::string& id, int* bedNumber) const
    {
        const auto result = FindPatient(id);
        if (result.has_value())
        {
            if (bedNumber)
            {
                *bedNumber = result->bedNumber;
            }
            return result->patient;
        }
        return nullptr;
    }

    bool ICU::DischargePatientById(const std::string& id, std::string* patientFullName)
    {
        const auto dischargedName = DischargePatient(id);
        if (dischargedName.has_value())
        {
            if (patientFullName)
            {
                *patientFullName = dischargedName.value();
            }
            return true;
        }
        return false;
    }

    std::vector<std::pair<int, const Patient*>> ICU::GetOccupiedBeds() const
    {
        std::vector<std::pair<int, const Patient*>> occupiedBeds;
        for (std::size_t i = 0; i < beds.size(); ++i)
        {
            if (beds[i])
            {
                occupiedBeds.emplace_back(static_cast<int>(i + 1), beds[i].get());
            }
        }

        return occupiedBeds;
    }
}
