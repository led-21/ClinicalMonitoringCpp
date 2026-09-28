#include "clinical/ScoreUpdateNotifier.h"

#include <algorithm>

namespace clinical
{
    ScopedSubscription::ScopedSubscription(ScoreUpdateNotifier* n, IScoreObserver* obs) noexcept
        : notifier(n), observer(obs)
    {
    }

    ScopedSubscription::~ScopedSubscription()
    {
        Disconnect();
    }

    ScopedSubscription::ScopedSubscription(ScopedSubscription&& other) noexcept
        : notifier(other.notifier), observer(other.observer)
    {
        other.notifier = nullptr;
        other.observer = nullptr;
    }

    ScopedSubscription& ScopedSubscription::operator=(ScopedSubscription&& other) noexcept
    {
        if (this != &other)
        {
            Disconnect();
            notifier = other.notifier;
            observer = other.observer;
            other.notifier = nullptr;
            other.observer = nullptr;
        }
        return *this;
    }

    void ScopedSubscription::Disconnect() noexcept
    {
        if (notifier && observer)
        {
            notifier->Unsubscribe(observer);
            notifier = nullptr;
            observer = nullptr;
        }
    }

    bool ScopedSubscription::IsConnected() const noexcept
    {
        return notifier != nullptr && observer != nullptr;
    }

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

    ScopedSubscription ScoreUpdateNotifier::SubscribeScoped(IScoreObserver* observer)
    {
        Subscribe(observer);
        return ScopedSubscription(this, observer);
    }

    void ScoreUpdateNotifier::Notify(const ScoreUpdateData& data) const
    {
        // Safe snapshot iteration: protects against modifications during notification
        const auto currentObservers = observers;
        for (IScoreObserver* observer : currentObservers)
        {
            if (observer)
            {
                try
                {
                    observer->OnScoreUpdated(data);
                }
                catch (const std::exception& ex)
                {
                    if (errorHandler)
                    {
                        errorHandler(observer, &ex);
                    }
                }
                catch (...)
                {
                    if (errorHandler)
                    {
                        errorHandler(observer, nullptr);
                    }
                }
            }
        }
    }

    std::size_t ScoreUpdateNotifier::ObserverCount() const noexcept
    {
        return observers.size();
    }

    void ScoreUpdateNotifier::SetErrorHandler(ErrorHandler handler)
    {
        errorHandler = std::move(handler);
    }
}
