#include "clinical/LedIndicator.h"

namespace clinical
{
    void LedIndicator::updateRiskLevel(const ScoreUpdateData& data) const
    {
        output << "LED Indicator -> Color: " << data.highestRiskColor
            << " (Highest risk: " << data.highestRiskLevel << ")\n";
    }

    void LedIndicator::OnScoreUpdated(const ScoreUpdateData& data)
    {
        updateRiskLevel(data);
    }
}
