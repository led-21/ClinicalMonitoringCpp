#pragma once

#include <array>
#include <cctype>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace clinical
{
    enum class ConsciousnessLevel : char
    {
        Alert = 'A',
        Voice = 'V',
        Pain = 'P',
        Unresponsive = 'U'
    };

    [[nodiscard]] constexpr std::optional<ConsciousnessLevel> ParseConsciousnessLevel(char c) noexcept
    {
        switch (c)
        {
            case 'A': case 'a': return ConsciousnessLevel::Alert;
            case 'V': case 'v': return ConsciousnessLevel::Voice;
            case 'P': case 'p': return ConsciousnessLevel::Pain;
            case 'U': case 'u': return ConsciousnessLevel::Unresponsive;
            default: return std::nullopt;
        }
    }

    [[nodiscard]] constexpr char ToChar(ConsciousnessLevel level) noexcept
    {
        return static_cast<char>(level);
    }

    [[nodiscard]] constexpr std::string_view ToString(ConsciousnessLevel level) noexcept
    {
        switch (level)
        {
            case ConsciousnessLevel::Alert:        return "Alert";
            case ConsciousnessLevel::Voice:        return "Voice";
            case ConsciousnessLevel::Pain:         return "Pain";
            case ConsciousnessLevel::Unresponsive: return "Unresponsive";
        }
        return "Unknown";
    }

    enum class RiskLevel
    {
        Low,
        LowMedium,
        Medium,
        High
    };

    [[nodiscard]] constexpr std::string_view ToString(RiskLevel level) noexcept
    {
        switch (level)
        {
            case RiskLevel::Low:       return "Low";
            case RiskLevel::LowMedium: return "Low-Medium";
            case RiskLevel::Medium:    return "Medium";
            case RiskLevel::High:      return "High";
        }
        return "Unknown";
    }

    [[nodiscard]] constexpr int RiskRank(RiskLevel level) noexcept
    {
        switch (level)
        {
            case RiskLevel::Low:       return 0;
            case RiskLevel::LowMedium: return 1;
            case RiskLevel::Medium:    return 2;
            case RiskLevel::High:      return 3;
        }
        return -1;
    }

    enum class AlertColor
    {
        Green,
        Yellow,
        Orange,
        Red
    };

    [[nodiscard]] constexpr std::string_view ToString(AlertColor color) noexcept
    {
        switch (color)
        {
            case AlertColor::Green:  return "Green";
            case AlertColor::Yellow: return "Yellow";
            case AlertColor::Orange: return "Orange";
            case AlertColor::Red:    return "Red";
        }
        return "Unknown";
    }

    [[nodiscard]] constexpr AlertColor ColorForRisk(RiskLevel level) noexcept
    {
        switch (level)
        {
            case RiskLevel::Low:       return AlertColor::Green;
            case RiskLevel::LowMedium: return AlertColor::Yellow;
            case RiskLevel::Medium:    return AlertColor::Orange;
            case RiskLevel::High:      return AlertColor::Red;
        }
        return AlertColor::Green;
    }

    struct PhysiologicalParameters
    {
        int respirationRate = 0;
        int oxygenSaturation = 0;
        bool supplementalOxygen = false;
        double temperature = 0.0;
        int systolicBP = 0;
        int heartRate = 0;
        char levelOfConsciousness = 'A';

        [[nodiscard]] constexpr std::optional<ConsciousnessLevel> GetTypedConsciousness() const noexcept
        {
            return ParseConsciousnessLevel(levelOfConsciousness);
        }
    };

    struct NewsScore
    {
        int total = 0;
        RiskLevel risk = RiskLevel::Low;
        std::string riskLevel = "Low";
        std::array<int, 7> componentScores{};
    };

    struct ScoreRecord
    {
        NewsScore score;
        std::string recordedAt;
    };

    struct DashboardEntry
    {
        int bedNumber = 0;
        std::string patientName;
        std::string riskLevel;
        std::string updatedAt;
        RiskLevel risk = RiskLevel::Low;
    };

    struct ScoreUpdateData
    {
        std::vector<DashboardEntry> entries;
        std::string highestRiskLevel;
        std::string highestRiskColor;
        RiskLevel highestRisk = RiskLevel::Low;
        AlertColor alertColor = AlertColor::Green;
    };

    class Patient;

    struct BedLookupResult
    {
        Patient* patient = nullptr;
        int bedNumber = 0;
    };

    struct ConstBedLookupResult
    {
        const Patient* patient = nullptr;
        int bedNumber = 0;
    };
}
