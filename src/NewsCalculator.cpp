#include "clinical/NewsCalculator.h"

#include <cctype>

namespace clinical
{
    std::optional<NewsScore> NewsCalculator::Calculate(const PhysiologicalParameters& p)
    {
        if (!IsValidInput(p))
        {
            return std::nullopt;
        }

        NewsScore result;
        result.componentScores.reserve(7);

        result.componentScores.push_back(ScoreRespirationRate(p.respirationRate));
        result.componentScores.push_back(ScoreOxygenSaturation(p.oxygenSaturation));
        result.componentScores.push_back(ScoreSupplementalOxygen(p.supplementalOxygen));
        result.componentScores.push_back(ScoreTemperature(p.temperature));
        result.componentScores.push_back(ScoreSystolicBP(p.systolicBP));
        result.componentScores.push_back(ScoreHeartRate(p.heartRate));
        result.componentScores.push_back(ScoreConsciousness(p.levelOfConsciousness));

        bool hasRedScore = false;
        for (int value : result.componentScores)
        {
            result.total += value;
            if (value == 3)
            {
                hasRedScore = true;
            }
        }

        result.riskLevel = CalculateRiskLevel(result.total, hasRedScore);
        return result;
    }

    bool NewsCalculator::IsValidInput(const PhysiologicalParameters& p)
    {
        if (p.respirationRate < 0 || p.respirationRate > 150) return false;
        if (p.oxygenSaturation < 50 || p.oxygenSaturation > 100) return false;
        if (p.temperature < 20.0 || p.temperature > 50.0) return false;
        if (p.systolicBP < 0 || p.systolicBP > 300) return false;
        if (p.heartRate < 15 || p.heartRate > 250) return false;

        const char level = static_cast<char>(std::toupper(static_cast<unsigned char>(p.levelOfConsciousness)));
        return level == 'A' || level == 'V' || level == 'P' || level == 'U';
    }

    int NewsCalculator::ScoreRespirationRate(int value)
    {
        if (value <= 8) return 3;
        if (value <= 11) return 1;
        if (value <= 20) return 0;
        if (value <= 24) return 2;
        return 3;
    }

    int NewsCalculator::ScoreOxygenSaturation(int value)
    {
        if (value <= 91) return 3;
        if (value <= 93) return 2;
        if (value <= 95) return 1;
        return 0;
    }

    int NewsCalculator::ScoreSupplementalOxygen(bool supplemental)
    {
        return supplemental ? 2 : 0;
    }

    int NewsCalculator::ScoreTemperature(double value)
    {
        if (value <= 35.0) return 3;
        if (value <= 36.0) return 1;
        if (value <= 38.0) return 0;
        if (value <= 39.0) return 1;
        return 2;
    }

    int NewsCalculator::ScoreSystolicBP(int value)
    {
        if (value <= 90) return 3;
        if (value <= 100) return 2;
        if (value <= 110) return 1;
        if (value <= 219) return 0;
        return 3;
    }

    int NewsCalculator::ScoreHeartRate(int value)
    {
        if (value <= 40) return 3;
        if (value <= 50) return 1;
        if (value <= 90) return 0;
        if (value <= 110) return 1;
        if (value <= 130) return 2;
        return 3;
    }

    int NewsCalculator::ScoreConsciousness(char value)
    {
        const char level = static_cast<char>(std::toupper(static_cast<unsigned char>(value)));
        return level == 'A' ? 0 : 3;
    }

    std::string NewsCalculator::CalculateRiskLevel(int total, bool hasRedScore)
    {
        if (total >= 7) return "High";
        if (total >= 5) return "Medium";
        if (hasRedScore) return "Low-Medium";
        return "Low";
    }
}
