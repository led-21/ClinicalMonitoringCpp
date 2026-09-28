#include "ConsoleUI.h"

#include "clinical/Dashboard.h"
#include "clinical/ICU.h"
#include "clinical/LedIndicator.h"
#include "clinical/NewsCalculator.h"
#include "clinical/ScoreUpdateNotifier.h"

#include <cctype>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

namespace clinical
{
    namespace
    {
        int RiskRank(const std::string& riskLevel)
        {
            if (riskLevel == "Low") return 0;
            if (riskLevel == "Low-Medium") return 1;
            if (riskLevel == "Medium") return 2;
            if (riskLevel == "High") return 3;
            return -1;
        }

        std::string RiskColor(const std::string& riskLevel)
        {
            if (riskLevel == "Low") return "Green";
            if (riskLevel == "Low-Medium") return "Yellow";
            if (riskLevel == "Medium") return "Orange";
            if (riskLevel == "High") return "Red";
            return "Unknown";
        }

        bool ReadInt(const std::string& prompt, int& value)
        {
            std::cout << prompt;
            std::cin >> value;
            if (std::cin.fail())
            {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                return false;
            }
            return true;
        }

        bool ReadDouble(const std::string& prompt, double& value)
        {
            std::cout << prompt;
            std::cin >> value;
            if (std::cin.fail())
            {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                return false;
            }
            return true;
        }

        bool ReadSupplementalOxygen(bool& supplementalOxygen)
        {
            std::string input;
            std::cout << "Any Supplemental Oxygen (Y/N): ";
            std::cin >> input;

            if (input.empty())
            {
                return false;
            }

            const char c = static_cast<char>(std::toupper(static_cast<unsigned char>(input[0])));
            if (c == 'Y')
            {
                supplementalOxygen = true;
                return true;
            }
            if (c == 'N')
            {
                supplementalOxygen = false;
                return true;
            }

            return false;
        }

        bool ReadConsciousness(char& levelOfConsciousness)
        {
            std::cout << "Level of Consciousness (A/V/P/U): ";
            std::cin >> levelOfConsciousness;

            levelOfConsciousness = static_cast<char>(std::toupper(static_cast<unsigned char>(levelOfConsciousness)));
            return levelOfConsciousness == 'A' || levelOfConsciousness == 'V'
                || levelOfConsciousness == 'P' || levelOfConsciousness == 'U';
        }

        ScoreUpdateData BuildScoreUpdateData(const ICU& icu)
        {
            ScoreUpdateData data;
            int bestRank = -1;

            for (const auto& [bedNumber, patient] : icu.GetOccupiedBeds())
            {
                const auto& history = patient->GetHistory();
                if (history.empty())
                {
                    continue;
                }

                const auto& latest = history.back();
                DashboardEntry entry;
                entry.bedNumber = bedNumber;
                entry.patientName = patient->GetFullName();
                entry.riskLevel = latest.score.riskLevel;
                entry.updatedAt = latest.recordedAt;
                data.entries.push_back(entry);

                const int rank = RiskRank(latest.score.riskLevel);
                if (rank > bestRank)
                {
                    bestRank = rank;
                    data.highestRiskLevel = latest.score.riskLevel;
                    data.highestRiskColor = RiskColor(latest.score.riskLevel);
                }
            }

            if (bestRank < 0)
            {
                data.highestRiskLevel = "Low";
                data.highestRiskColor = "Green";
            }

            return data;
        }

        void PrintMenu()
        {
            std::cout << "\n=== Clinical Monitoring System (ICU) ===\n"
                << "n - calculate new NEWS score\n"
                << "h - print patient history\n"
                << "a - print all patients latest score\n"
                << "d - discharge patient\n"
                << "x - exit\n"
                << "Option: ";
        }

        void HandleNewScore(ICU& icu, const ScoreUpdateNotifier& notifier)
        {
            int bed = 0;
            if (!ReadInt("Bed number (1-6): ", bed) || bed < 1 || bed > 6)
            {
                std::cout << "Invalid bed number.\n";
                return;
            }

            Patient* patient = icu.GetPatientAtBed(bed);
            if (!patient)
            {
                std::string id;
                std::string first;
                std::string last;

                std::cout << "Patient ID: ";
                std::cin >> id;
                std::cout << "First name: ";
                std::cin >> first;
                std::cout << "Last name: ";
                std::cin >> last;
                patient = icu.CreatePatientAtBed(bed, id, first, last);
            }

            std::cout << "Bed " << bed << " - "
                << patient->GetId() << " - "
                << patient->GetFullName() << "\n";

            PhysiologicalParameters p{};
            bool valid = true;

            valid = ReadInt("Respiration Rate: ", p.respirationRate) && valid;
            valid = ReadInt("Oxygen Saturation: ", p.oxygenSaturation) && valid;
            valid = ReadSupplementalOxygen(p.supplementalOxygen) && valid;
            valid = ReadDouble("Temperature: ", p.temperature) && valid;
            valid = ReadInt("Systolic BP: ", p.systolicBP) && valid;
            valid = ReadInt("Heart Rate: ", p.heartRate) && valid;
            valid = ReadConsciousness(p.levelOfConsciousness) && valid;

            if (!valid)
            {
                std::cout << "Invalid data. No NEWS score calculated\n";
                return;
            }

            const auto result = NewsCalculator::Calculate(p);
            if (!result.has_value())
            {
                std::cout << "Invalid data. No NEWS score calculated\n";
                return;
            }

            patient->AddScore(result.value());

            std::cout << "NEWS Score: " << result->total
                << " | Risk Level: " << result->riskLevel << "\n";

            notifier.Notify(BuildScoreUpdateData(icu));
        }

        void HandleHistory(ICU& icu)
        {
            std::string id;
            std::cout << "Patient ID: ";
            std::cin >> id;

            int bedNumber = 0;
            Patient* patient = icu.FindPatientById(id, &bedNumber);
            if (!patient)
            {
                std::cout << "No Patient with ID: " << id << "\n";
                return;
            }

            std::cout << "Bed " << bedNumber << ": " << patient->GetId() << " - "
                << patient->GetFullName() << "\n";

            const auto& history = patient->GetHistory();
            if (history.empty())
            {
                std::cout << "No NEWS scores available.\n";
                return;
            }

            for (const auto& record : history)
            {
                std::cout << record.recordedAt
                    << ", NEWS Score: " << record.score.total
                    << ", Risk Level: " << record.score.riskLevel << "\n";
            }
        }

        void HandlePrintAll(const ICU& icu)
        {
            const ScoreUpdateData data = BuildScoreUpdateData(icu);
            if (data.entries.empty())
            {
                std::cout << "No patients with NEWS score.\n";
                return;
            }

            for (const auto& entry : data.entries)
            {
                std::cout << "Bed " << entry.bedNumber
                    << " - " << entry.patientName
                    << " - Risk Level: " << entry.riskLevel
                    << " - " << entry.updatedAt << "\n";
            }
        }

        void HandleDischarge(ICU& icu, const ScoreUpdateNotifier& notifier)
        {
            std::string id;
            std::cout << "Patient ID: ";
            std::cin >> id;

            Patient* patient = icu.FindPatientById(id);
            if (!patient)
            {
                std::cout << "Patient not found. Please try again\n";
                return;
            }

            const std::string fullName = patient->GetFullName();

            std::string confirmation;
            std::cout << "Discharge patient " << fullName << " ? (Y/N)";
            std::cin >> confirmation;

            if (!confirmation.empty() &&
                std::toupper(static_cast<unsigned char>(confirmation[0])) == 'Y')
            {
                icu.DischargePatientById(id);
                notifier.Notify(BuildScoreUpdateData(icu));
                std::cout << "Patient discharged.\n";
                return;
            }

            std::cout << "Discharge cancelled.\n";
        }
    }

    void RunClinicalMonitoringApp()
    {
        ICU icu;
        Dashboard dashboard;
        LedIndicator led;
        ScoreUpdateNotifier notifier;

        notifier.Subscribe(&dashboard);
        notifier.Subscribe(&led);

        while (true)
        {
            PrintMenu();
            char option = '\0';
            std::cin >> option;
            option = static_cast<char>(std::tolower(static_cast<unsigned char>(option)));

            if (option == 'x')
            {
                break;
            }

            if (option == 'n')
            {
                HandleNewScore(icu, notifier);
            }
            else if (option == 'h')
            {
                HandleHistory(icu);
            }
            else if (option == 'a')
            {
                HandlePrintAll(icu);
            }
            else if (option == 'd')
            {
                HandleDischarge(icu, notifier);
            }
            else
            {
                std::cout << "Invalid option.\n";
            }
        }
    }
}
