#pragma once

#include <string>
#include <vector>

namespace clinical
{
    struct PhysiologicalParameters
    {
        int respirationRate = 0;
        int oxygenSaturation = 0;
        bool supplementalOxygen = false;
        double temperature = 0.0;
        int systolicBP = 0;
        int heartRate = 0;
        char levelOfConsciousness = 'A';
    };

    struct NewsScore
    {
        int total = 0;
        std::string riskLevel;
        std::vector<int> componentScores;
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
    };

    struct ScoreUpdateData
    {
        std::vector<DashboardEntry> entries;
        std::string highestRiskLevel;
        std::string highestRiskColor;
    };
}
