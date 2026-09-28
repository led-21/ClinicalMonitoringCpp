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

    Patient* ICU::CreatePatientAtBed(int bedNumber, const std::string& id, const std::string& firstName, const std::string& lastName)
    {
        if (!IsValidBedNumber(bedNumber))
        {
            return nullptr;
        }

        const std::size_t index = static_cast<std::size_t>(bedNumber - 1);
        if (!beds[index])
        {
            beds[index] = std::make_unique<Patient>(id, firstName, lastName);
        }

        return beds[index].get();
    }

    Patient* ICU::FindPatientById(const std::string& id, int* bedNumber)
    {
        for (std::size_t i = 0; i < beds.size(); ++i)
        {
            const auto& patient = beds[i];
            if (patient && patient->GetId() == id)
            {
                if (bedNumber)
                {
                    *bedNumber = static_cast<int>(i + 1);
                }
                return patient.get();
            }
        }

        return nullptr;
    }

    const Patient* ICU::FindPatientById(const std::string& id, int* bedNumber) const
    {
        for (std::size_t i = 0; i < beds.size(); ++i)
        {
            const auto& patient = beds[i];
            if (patient && patient->GetId() == id)
            {
                if (bedNumber)
                {
                    *bedNumber = static_cast<int>(i + 1);
                }
                return patient.get();
            }
        }

        return nullptr;
    }

    bool ICU::DischargePatientById(const std::string& id, std::string* patientFullName)
    {
        for (std::size_t i = 0; i < beds.size(); ++i)
        {
            if (beds[i] && beds[i]->GetId() == id)
            {
                if (patientFullName)
                {
                    *patientFullName = beds[i]->GetFirstName() + " " + beds[i]->GetLastName();
                }

                beds[i].reset();
                return true;
            }
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
