#include "NewsCalculatorTests.h"

#include "clinical/NewsCalculator.h"

using namespace clinical;

// Compile-time constexpr evaluation verification
static_assert(NewsCalculator::Calculate(PhysiologicalParameters{ 16, 96, false, 37.0, 120, 70, 'A' })->total == 0,
    "NEWS zero-score calculation must be evaluated at compile time");
static_assert(NewsCalculator::Calculate(PhysiologicalParameters{ 16, 96, false, 37.0, 120, 70, 'A' })->risk == RiskLevel::Low,
    "NEWS zero-score risk level must be RiskLevel::Low at compile time");
static_assert(NewsCalculator::Calculate(PhysiologicalParameters{ 25, 91, true, 37.0, 120, 70, 'A' })->total >= 7,
    "NEWS high risk total score must be >= 7 at compile time");
static_assert(NewsCalculator::Calculate(PhysiologicalParameters{ 25, 91, true, 37.0, 120, 70, 'A' })->risk == RiskLevel::High,
    "NEWS high risk level must be RiskLevel::High at compile time");
static_assert(!NewsCalculator::Calculate(PhysiologicalParameters{ 16, 49, false, 37.0, 120, 70, 'A' }).has_value(),
    "Invalid physiological input must yield nullopt at compile time");

namespace
{
    // -------------------------------------------------------------------------
    // Risk level scenarios
    // -------------------------------------------------------------------------

    void ShouldCalculateLowRiskExample()
    {
        PhysiologicalParameters parameters{ 22, 94, false, 37.0, 120, 88, 'A' };
        const auto result = NewsCalculator::Calculate(parameters);

        AssertTrue(result.has_value(), "Low risk example should produce a valid score");
        AssertEqual(3, result->total, "Total score mismatch for low risk example");
        AssertEqual(std::string("Low"), result->riskLevel, "Risk level should be Low");
        AssertTrue(result->risk == RiskLevel::Low, "Risk enum should be Low");
    }

    void ShouldCalculateZeroScore()
    {
        // All parameters in the normal (0-score) range
        PhysiologicalParameters parameters{ 16, 96, false, 37.0, 120, 70, 'A' };
        const auto result = NewsCalculator::Calculate(parameters);

        AssertTrue(result.has_value(), "Zero-score case should be valid");
        AssertEqual(0, result->total, "Zero-score total mismatch");
        AssertEqual(std::string("Low"), result->riskLevel, "Zero-score risk should be Low");
        AssertTrue(result->risk == RiskLevel::Low, "Risk enum should be Low");
    }

    void ShouldReturnLowMediumWhenSingleRedScoreExists()
    {
        // RR=7 scores 3 (red), rest scores 0 -> total 3, has red -> Low-Medium
        PhysiologicalParameters parameters{ 7, 96, false, 37.0, 120, 88, 'A' };
        const auto result = NewsCalculator::Calculate(parameters);

        AssertTrue(result.has_value(), "Single red score case should be valid");
        AssertEqual(3, result->total, "Single red score total mismatch");
        AssertEqual(std::string("Low-Medium"), result->riskLevel, "Single red score risk mismatch");
        AssertTrue(result->risk == RiskLevel::LowMedium, "Risk enum should be LowMedium");
    }

    void ShouldCalculateMediumRisk()
    {
        // RR=22 (+2), SpO2=95 (+1), Temp=39.5 (+2) -> total 5, no red -> Medium
        PhysiologicalParameters parameters{ 22, 95, false, 39.5, 120, 70, 'A' };
        const auto result = NewsCalculator::Calculate(parameters);

        AssertTrue(result.has_value(), "Medium risk case should be valid");
        AssertEqual(5, result->total, "Medium risk total mismatch");
        AssertEqual(std::string("Medium"), result->riskLevel, "Medium risk level mismatch");
        AssertTrue(result->risk == RiskLevel::Medium, "Risk enum should be Medium");
    }

    void ShouldCalculateHighRisk()
    {
        // RR=25 (+3), SpO2=91 (+3), supplemental (+2) -> total 8 -> High
        PhysiologicalParameters parameters{ 25, 91, true, 37.0, 120, 70, 'A' };
        const auto result = NewsCalculator::Calculate(parameters);

        AssertTrue(result.has_value(), "High risk case should be valid");
        AssertTrue(result->total >= 7, "High risk total should be >= 7");
        AssertEqual(std::string("High"), result->riskLevel, "High risk level mismatch");
        AssertTrue(result->risk == RiskLevel::High, "Risk enum should be High");
    }

    // -------------------------------------------------------------------------
    // Respiration rate boundaries (<=8->3, 9-11->1, 12-20->0, 21-24->2, >=25->3)
    // -------------------------------------------------------------------------

    void ShouldScoreRespirationRateBoundaries()
    {
        auto calc = [](int rr) {
            PhysiologicalParameters p{ rr, 96, false, 37.0, 120, 70, 'A' };
            return NewsCalculator::Calculate(p);
        };

        AssertEqual(3, calc(8)->componentScores[0],  "RR=8 should score 3");
        AssertEqual(1, calc(9)->componentScores[0],  "RR=9 should score 1");
        AssertEqual(1, calc(11)->componentScores[0], "RR=11 should score 1");
        AssertEqual(0, calc(12)->componentScores[0], "RR=12 should score 0");
        AssertEqual(0, calc(20)->componentScores[0], "RR=20 should score 0");
        AssertEqual(2, calc(21)->componentScores[0], "RR=21 should score 2");
        AssertEqual(2, calc(24)->componentScores[0], "RR=24 should score 2");
        AssertEqual(3, calc(25)->componentScores[0], "RR=25 should score 3");
    }

    // -------------------------------------------------------------------------
    // SpO2 boundaries (<=91->3, 92-93->2, 94-95->1, >=96->0)
    // -------------------------------------------------------------------------

    void ShouldScoreOxygenSaturationBoundaries()
    {
        auto calc = [](int spo2) {
            PhysiologicalParameters p{ 16, spo2, false, 37.0, 120, 70, 'A' };
            return NewsCalculator::Calculate(p);
        };

        AssertEqual(3, calc(91)->componentScores[1], "SpO2=91 should score 3");
        AssertEqual(2, calc(92)->componentScores[1], "SpO2=92 should score 2");
        AssertEqual(2, calc(93)->componentScores[1], "SpO2=93 should score 2");
        AssertEqual(1, calc(94)->componentScores[1], "SpO2=94 should score 1");
        AssertEqual(1, calc(95)->componentScores[1], "SpO2=95 should score 1");
        AssertEqual(0, calc(96)->componentScores[1], "SpO2=96 should score 0");
    }

    // -------------------------------------------------------------------------
    // Supplemental oxygen
    // -------------------------------------------------------------------------

    void ShouldScoreSupplementalOxygen()
    {
        auto calc = [](bool supp) {
            PhysiologicalParameters p{ 16, 96, supp, 37.0, 120, 70, 'A' };
            return NewsCalculator::Calculate(p);
        };

        AssertEqual(0, calc(false)->componentScores[2], "No supplemental O2 should score 0");
        AssertEqual(2, calc(true)->componentScores[2],  "Supplemental O2 should score 2");
    }

    // -------------------------------------------------------------------------
    // Temperature boundaries (<=35->3, 35.1-36->1, 36.1-38->0, 38.1-39->1, >39->2)
    // -------------------------------------------------------------------------

    void ShouldScoreTemperatureBoundaries()
    {
        auto calc = [](double temp) {
            PhysiologicalParameters p{ 16, 96, false, temp, 120, 70, 'A' };
            return NewsCalculator::Calculate(p);
        };

        AssertEqual(3, calc(35.0)->componentScores[3], "Temp=35.0 should score 3");
        AssertEqual(1, calc(35.5)->componentScores[3], "Temp=35.5 should score 1");
        AssertEqual(1, calc(36.0)->componentScores[3], "Temp=36.0 should score 1");
        AssertEqual(0, calc(36.1)->componentScores[3], "Temp=36.1 should score 0");
        AssertEqual(0, calc(38.0)->componentScores[3], "Temp=38.0 should score 0");
        AssertEqual(1, calc(38.5)->componentScores[3], "Temp=38.5 should score 1");
        AssertEqual(1, calc(39.0)->componentScores[3], "Temp=39.0 should score 1");
        AssertEqual(2, calc(39.1)->componentScores[3], "Temp=39.1 should score 2");
    }

    // -------------------------------------------------------------------------
    // Systolic BP boundaries (<=90->3, 91-100->2, 101-110->1, 111-219->0, >=220->3)
    // -------------------------------------------------------------------------

    void ShouldScoreSystolicBPBoundaries()
    {
        auto calc = [](int bp) {
            PhysiologicalParameters p{ 16, 96, false, 37.0, bp, 70, 'A' };
            return NewsCalculator::Calculate(p);
        };

        AssertEqual(3, calc(90)->componentScores[4],  "SBP=90 should score 3");
        AssertEqual(2, calc(91)->componentScores[4],  "SBP=91 should score 2");
        AssertEqual(2, calc(100)->componentScores[4], "SBP=100 should score 2");
        AssertEqual(1, calc(101)->componentScores[4], "SBP=101 should score 1");
        AssertEqual(1, calc(110)->componentScores[4], "SBP=110 should score 1");
        AssertEqual(0, calc(111)->componentScores[4], "SBP=111 should score 0");
        AssertEqual(0, calc(219)->componentScores[4], "SBP=219 should score 0");
        AssertEqual(3, calc(220)->componentScores[4], "SBP=220 should score 3");
    }

    // -------------------------------------------------------------------------
    // Heart rate boundaries (<=40->3, 41-50->1, 51-90->0, 91-110->1, 111-130->2, >=131->3)
    // -------------------------------------------------------------------------

    void ShouldScoreHeartRateBoundaries()
    {
        auto calc = [](int hr) {
            PhysiologicalParameters p{ 16, 96, false, 37.0, 120, hr, 'A' };
            return NewsCalculator::Calculate(p);
        };

        AssertEqual(3, calc(40)->componentScores[5],  "HR=40 should score 3");
        AssertEqual(1, calc(41)->componentScores[5],  "HR=41 should score 1");
        AssertEqual(1, calc(50)->componentScores[5],  "HR=50 should score 1");
        AssertEqual(0, calc(51)->componentScores[5],  "HR=51 should score 0");
        AssertEqual(0, calc(90)->componentScores[5],  "HR=90 should score 0");
        AssertEqual(1, calc(91)->componentScores[5],  "HR=91 should score 1");
        AssertEqual(1, calc(110)->componentScores[5], "HR=110 should score 1");
        AssertEqual(2, calc(111)->componentScores[5], "HR=111 should score 2");
        AssertEqual(2, calc(130)->componentScores[5], "HR=130 should score 2");
        AssertEqual(3, calc(131)->componentScores[5], "HR=131 should score 3");
    }

    // -------------------------------------------------------------------------
    // Level of consciousness (A->0, V/P/U->3; lowercase accepted)
    // -------------------------------------------------------------------------

    void ShouldScoreConsciousnessLevels()
    {
        auto calc = [](char loc) {
            PhysiologicalParameters p{ 16, 96, false, 37.0, 120, 70, loc };
            return NewsCalculator::Calculate(p);
        };

        AssertEqual(0, calc('A')->componentScores[6], "LOC=A should score 0");
        AssertEqual(3, calc('V')->componentScores[6], "LOC=V should score 3");
        AssertEqual(3, calc('P')->componentScores[6], "LOC=P should score 3");
        AssertEqual(3, calc('U')->componentScores[6], "LOC=U should score 3");
        AssertEqual(0, calc('a')->componentScores[6], "LOC=a (lower) should score 0");
        AssertEqual(3, calc('v')->componentScores[6], "LOC=v (lower) should score 3");
    }

    void ShouldHaveSevenComponentScores()
    {
        PhysiologicalParameters p{ 16, 96, false, 37.0, 120, 70, 'A' };
        const auto result = NewsCalculator::Calculate(p);

        AssertTrue(result.has_value(), "Should produce a valid result");
        AssertEqual(7, static_cast<int>(result->componentScores.size()),
            "Should always produce exactly 7 component scores");
    }

    // -------------------------------------------------------------------------
    // Invalid input rejection
    // -------------------------------------------------------------------------

    void ShouldRejectInvalidData()
    {
        PhysiologicalParameters parameters{ 22, 49, false, 37.0, 120, 88, 'A' };
        const auto result = NewsCalculator::Calculate(parameters);

        AssertTrue(!result.has_value(), "SpO2=49 should be rejected as invalid");
    }

    void ShouldRejectInvalidRespirationRate()
    {
        PhysiologicalParameters p{ 151, 96, false, 37.0, 120, 70, 'A' };
        AssertTrue(!NewsCalculator::Calculate(p).has_value(), "RR=151 should be rejected");
    }

    void ShouldRejectInvalidTemperature()
    {
        PhysiologicalParameters p1{ 16, 96, false, 19.9, 120, 70, 'A' };
        PhysiologicalParameters p2{ 16, 96, false, 50.1, 120, 70, 'A' };
        AssertTrue(!NewsCalculator::Calculate(p1).has_value(), "Temp=19.9 should be rejected");
        AssertTrue(!NewsCalculator::Calculate(p2).has_value(), "Temp=50.1 should be rejected");
    }

    void ShouldRejectInvalidHeartRate()
    {
        PhysiologicalParameters p{ 16, 96, false, 37.0, 120, 14, 'A' };
        AssertTrue(!NewsCalculator::Calculate(p).has_value(), "HR=14 should be rejected");
    }

    void ShouldRejectInvalidConsciousness()
    {
        PhysiologicalParameters p{ 16, 96, false, 37.0, 120, 70, 'X' };
        AssertTrue(!NewsCalculator::Calculate(p).has_value(), "LOC=X should be rejected");
    }

    void ShouldSupportTypedEnumsAndHelpers()
    {
        PhysiologicalParameters p{ 16, 96, false, 37.0, 120, 70, 'A' };
        const auto result = NewsCalculator::Calculate(p);

        AssertTrue(result.has_value(), "Calculation with valid parameters should succeed");
        AssertTrue(result->risk == RiskLevel::Low, "RiskLevel should be Low enum");
        AssertEqual(0, RiskRank(result->risk), "Risk rank for Low should be 0");
        AssertTrue(ColorForRisk(result->risk) == AlertColor::Green, "Alert color for Low should be Green");
        AssertEqual(std::string_view("Green"), ToString(AlertColor::Green), "ToString for Green should match");
        AssertEqual(std::string_view("Alert"), ToString(ConsciousnessLevel::Alert), "ToString for Alert should match");
    }
}

void RegisterNewsCalculatorTests(std::vector<TestCase>& tests)
{
    // Risk levels
    tests.push_back({ "NEWS - Calculate zero score for normal parameters",  ShouldCalculateZeroScore });
    tests.push_back({ "NEWS - Calculate low risk example",                  ShouldCalculateLowRiskExample });
    tests.push_back({ "NEWS - Identify low-medium with red score",          ShouldReturnLowMediumWhenSingleRedScoreExists });
    tests.push_back({ "NEWS - Calculate medium risk",                       ShouldCalculateMediumRisk });
    tests.push_back({ "NEWS - Calculate high risk",                         ShouldCalculateHighRisk });

    // Component boundaries
    tests.push_back({ "NEWS - Respiration rate boundaries",                 ShouldScoreRespirationRateBoundaries });
    tests.push_back({ "NEWS - Oxygen saturation boundaries",                ShouldScoreOxygenSaturationBoundaries });
    tests.push_back({ "NEWS - Supplemental oxygen scoring",                 ShouldScoreSupplementalOxygen });
    tests.push_back({ "NEWS - Temperature boundaries",                      ShouldScoreTemperatureBoundaries });
    tests.push_back({ "NEWS - Systolic BP boundaries",                      ShouldScoreSystolicBPBoundaries });
    tests.push_back({ "NEWS - Heart rate boundaries",                       ShouldScoreHeartRateBoundaries });
    tests.push_back({ "NEWS - Consciousness levels",                        ShouldScoreConsciousnessLevels });
    tests.push_back({ "NEWS - Exactly 7 component scores produced",         ShouldHaveSevenComponentScores });

    // Invalid inputs
    tests.push_back({ "NEWS - Reject invalid SpO2",                         ShouldRejectInvalidData });
    tests.push_back({ "NEWS - Reject invalid respiration rate",             ShouldRejectInvalidRespirationRate });
    tests.push_back({ "NEWS - Reject invalid temperature",                  ShouldRejectInvalidTemperature });
    tests.push_back({ "NEWS - Reject invalid heart rate",                   ShouldRejectInvalidHeartRate });
    tests.push_back({ "NEWS - Reject invalid consciousness value",          ShouldRejectInvalidConsciousness });

    // Strong typing
    tests.push_back({ "NEWS - Strongly typed enums and conversion helpers", ShouldSupportTypedEnumsAndHelpers });
}
