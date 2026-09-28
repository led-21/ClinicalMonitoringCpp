#pragma once

#include "Models.h"
#include "ScoreObserver.h"

#include <iostream>

namespace clinical
{
    class Dashboard : public IScoreObserver
    {
    public:
        explicit Dashboard(std::ostream& out = std::cout) : output(out) {}

        void updateDashboardInfo(const ScoreUpdateData& data) const;
        void OnScoreUpdated(const ScoreUpdateData& data) override;

    private:
        std::ostream& output;
    };
}
