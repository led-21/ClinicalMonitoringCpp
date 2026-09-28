#pragma once

#include "Models.h"
#include "ScoreObserver.h"

#include <iostream>

namespace clinical
{
    class LedIndicator : public IScoreObserver
    {
    public:
        explicit LedIndicator(std::ostream& out = std::cout) : output(out) {}

        void updateRiskLevel(const ScoreUpdateData& data) const;
        void OnScoreUpdated(const ScoreUpdateData& data) override;

    private:
        std::ostream& output;
    };
}
