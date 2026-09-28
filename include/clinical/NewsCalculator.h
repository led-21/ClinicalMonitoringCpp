#pragma once

#include "Models.h"

#include <optional>
#include <string>

namespace clinical
{
    class NewsCalculator
    {
    public:
        [[nodiscard]] static constexpr std::optional<NewsScore> Calculate(const PhysiologicalParameters& p) noexcept
        {
            if (!IsValidInput(p))
            {
                return std::nullopt;
            }

            NewsScore result{};
            result.componentScores[0] = ScoreRespirationRate(p.respirationRate);
            result.componentScores[1] = ScoreOxygenSaturation(p.oxygenSaturation);
            result.componentScores[2] = ScoreSupplementalOxygen(p.supplementalOxygen);
            result.componentScores[3] = ScoreTemperature(p.temperature);
            result.componentScores[4] = ScoreSystolicBP(p.systolicBP);
            result.componentScores[5] = ScoreHeartRate(p.heartRate);
            result.componentScores[6] = ScoreConsciousness(p.levelOfConsciousness);

            bool hasRedScore = false;
            for (int value : result.componentScores)
            {
                result.total += value;
                if (value == 3)
                {
                    hasRedScore = true;
                }
            }

            result.risk = CalculateRisk(result.total, hasRedScore);
            result.riskLevel = ToString(result.risk);
            return result;
        }

        [[nodiscard]] static constexpr bool IsValidInput(const PhysiologicalParameters& p) noexcept
        {
            if (p.respirationRate < 0 || p.respirationRate > 150) return false;
            if (p.oxygenSaturation < 50 || p.oxygenSaturation > 100) return false;
            if (p.temperature < 20.0 || p.temperature > 50.0) return false;
            if (p.systolicBP < 0 || p.systolicBP > 300) return false;
            if (p.heartRate < 15 || p.heartRate > 250) return false;

            return ParseConsciousnessLevel(p.levelOfConsciousness).has_value();
        }

        [[nodiscard]] static constexpr int ScoreRespirationRate(int value) noexcept
        {
            if (value <= 8) return 3;
            if (value <= 11) return 1;
            if (value <= 20) return 0;
            if (value <= 24) return 2;
            return 3;
        }

        [[nodiscard]] static constexpr int ScoreOxygenSaturation(int value) noexcept
        {
            if (value <= 91) return 3;
            if (value <= 93) return 2;
            if (value <= 95) return 1;
            return 0;
        }

        [[nodiscard]] static constexpr int ScoreSupplementalOxygen(bool supplemental) noexcept
        {
            return supplemental ? 2 : 0;
        }

        [[nodiscard]] static constexpr int ScoreTemperature(double value) noexcept
        {
            if (value <= 35.0) return 3;
            if (value <= 36.0) return 1;
            if (value <= 38.0) return 0;
            if (value <= 39.0) return 1;
            return 2;
        }

        [[nodiscard]] static constexpr int ScoreSystolicBP(int value) noexcept
        {
            if (value <= 90) return 3;
            if (value <= 100) return 2;
            if (value <= 110) return 1;
            if (value <= 219) return 0;
            return 3;
        }

        [[nodiscard]] static constexpr int ScoreHeartRate(int value) noexcept
        {
            if (value <= 40) return 3;
            if (value <= 50) return 1;
            if (value <= 90) return 0;
            if (value <= 110) return 1;
            if (value <= 130) return 2;
            return 3;
        }

        [[nodiscard]] static constexpr int ScoreConsciousness(char value) noexcept
        {
            const auto level = ParseConsciousnessLevel(value);
            if (!level.has_value()) return 3;
            return ScoreConsciousness(level.value());
        }

        [[nodiscard]] static constexpr int ScoreConsciousness(ConsciousnessLevel level) noexcept
        {
            return level == ConsciousnessLevel::Alert ? 0 : 3;
        }

        [[nodiscard]] static constexpr RiskLevel CalculateRisk(int total, bool hasRedScore) noexcept
        {
            if (total >= 7) return RiskLevel::High;
            if (total >= 5) return RiskLevel::Medium;
            if (hasRedScore) return RiskLevel::LowMedium;
            return RiskLevel::Low;
        }

        [[nodiscard]] static std::string CalculateRiskLevel(int total, bool hasRedScore)
        {
            return std::string(ToString(CalculateRisk(total, hasRedScore)));
        }
    };
}
