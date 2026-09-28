#include "clinical/ScoreUpdateNotifier.h"

#include <algorithm>

namespace clinical
{
    void ScoreUpdateNotifier::Subscribe(IScoreObserver* observer)
    {
        if (observer)
        {
            if (std::find(observers.begin(), observers.end(), observer) == observers.end())
            {
                observers.push_back(observer);
            }
        }
    }

    void ScoreUpdateNotifier::Unsubscribe(IScoreObserver* observer)
    {
        if (observer)
        {
            observers.erase(std::remove(observers.begin(), observers.end(), observer), observers.end());
        }
    }

    void ScoreUpdateNotifier::Notify(const ScoreUpdateData& data) const
    {
        for (IScoreObserver* observer : observers)
        {
            if (observer)
            {
                observer->OnScoreUpdated(data);
            }
        }
    }

    std::size_t ScoreUpdateNotifier::ObserverCount() const noexcept
    {
        return observers.size();
    }
}
