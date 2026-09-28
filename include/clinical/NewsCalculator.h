#pragma once

#include "Models.h"

#include <optional>
#include <string>

namespace clinical
{
    class NewsCalculator
    {
    public:
        [[nodiscard]] static std::optional<NewsScore> Calculate(const PhysiologicalParameters& p);

    private:
        [[nodiscard]] static bool IsValidInput(const PhysiologicalParameters& p);
        [[nodiscard]] static int ScoreRespirationRate(int value);
        [[nodiscard]] static int ScoreOxygenSaturation(int value);
        [[nodiscard]] static int ScoreSupplementalOxygen(bool supplemental);
        [[nodiscard]] static int ScoreTemperature(double value);
        [[nodiscard]] static int ScoreSystolicBP(int value);
        [[nodiscard]] static int ScoreHeartRate(int value);
        [[nodiscard]] static int ScoreConsciousness(char value);
        [[nodiscard]] static std::string CalculateRiskLevel(int total, bool hasRedScore);
    };
}
