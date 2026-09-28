#include "clinical/Dashboard.h"

namespace clinical
{
    void Dashboard::updateDashboardInfo(const ScoreUpdateData& data) const
    {
        output << "\n=== Dashboard Update ===\n";

        if (data.entries.empty())
        {
            output << "No admitted patients with calculated NEWS score.\n";
        }
        else
        {
            for (const auto& entry : data.entries)
            {
                output << "Bed " << entry.bedNumber
                    << " | Patient: " << entry.patientName
                    << " | Risk: " << entry.riskLevel
                    << " | Updated: " << entry.updatedAt
                    << "\n";
            }
        }

        output << "========================\n";
    }

    void Dashboard::OnScoreUpdated(const ScoreUpdateData& data)
    {
        updateDashboardInfo(data);
    }
}
