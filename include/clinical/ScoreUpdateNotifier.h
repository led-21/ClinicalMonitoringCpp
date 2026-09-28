#pragma once

#include "ScoreObserver.h"

#include <vector>

namespace clinical
{
    class ScoreUpdateNotifier
    {
    public:
        void Subscribe(IScoreObserver* observer);
        void Unsubscribe(IScoreObserver* observer);
        void Notify(const ScoreUpdateData& data) const;
        [[nodiscard]] std::size_t ObserverCount() const noexcept;

    private:
        std::vector<IScoreObserver*> observers;
    };
}
