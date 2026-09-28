#pragma once

#include "Models.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace clinical
{
    class Patient
    {
    public:
        Patient(std::string patientId, std::string firstName, std::string lastName)
            : id(std::move(patientId)), first(std::move(firstName)), last(std::move(lastName))
        {
        }

        [[nodiscard]] const std::string& GetId() const noexcept { return id; }
        [[nodiscard]] const std::string& GetFirstName() const noexcept { return first; }
        [[nodiscard]] const std::string& GetLastName() const noexcept { return last; }
        [[nodiscard]] std::string GetFullName() const { return first + " " + last; }

        void AddScore(const NewsScore& score)
        {
            history.push_back({ score, GetCurrentTimestamp() });
        }

        void AddScoreWithTimestamp(const NewsScore& score, std::string timestamp)
        {
            history.push_back({ score, std::move(timestamp) });
        }

        [[nodiscard]] const std::vector<ScoreRecord>& GetHistory() const noexcept { return history; }

    private:
        static std::string GetCurrentTimestamp()
        {
            const auto now = std::chrono::system_clock::now();
            const std::time_t rawTime = std::chrono::system_clock::to_time_t(now);

            std::tm localTime{};
#if defined(_WIN32)
            localtime_s(&localTime, &rawTime);
#else
            localtime_r(&rawTime, &localTime);
#endif

            std::ostringstream output;
            output << std::put_time(&localTime, "%d %b %Y %H:%M");
            return output.str();
        }

        std::string id;
        std::string first;
        std::string last;
        std::vector<ScoreRecord> history;
    };
}
