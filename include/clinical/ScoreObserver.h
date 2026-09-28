#pragma once

#include "Models.h"

namespace clinical
{
    class IScoreObserver
    {
    public:
        virtual ~IScoreObserver() = default;
        virtual void OnScoreUpdated(const ScoreUpdateData& data) = 0;
    };
}
