#pragma once

#include "ScoreObserver.h"

#include <functional>
#include <vector>

namespace clinical
{
    class ScoreUpdateNotifier;

    class ScopedSubscription
    {
    public:
        ScopedSubscription() noexcept = default;
        ScopedSubscription(ScoreUpdateNotifier* notifier, IScoreObserver* observer) noexcept;
        ~ScopedSubscription();

        ScopedSubscription(const ScopedSubscription&) = delete;
        ScopedSubscription& operator=(const ScopedSubscription&) = delete;

        ScopedSubscription(ScopedSubscription&& other) noexcept;
        ScopedSubscription& operator=(ScopedSubscription&& other) noexcept;

        void Disconnect() noexcept;
        [[nodiscard]] bool IsConnected() const noexcept;

    private:
        ScoreUpdateNotifier* notifier = nullptr;
        IScoreObserver* observer = nullptr;
    };

    class ScoreUpdateNotifier
    {
    public:
        using ErrorHandler = std::function<void(IScoreObserver*, const std::exception*)>;

        void Subscribe(IScoreObserver* observer);
        void Unsubscribe(IScoreObserver* observer);

        [[nodiscard]] ScopedSubscription SubscribeScoped(IScoreObserver* observer);

        void Notify(const ScoreUpdateData& data) const;
        [[nodiscard]] std::size_t ObserverCount() const noexcept;

        void SetErrorHandler(ErrorHandler handler);

    private:
        std::vector<IScoreObserver*> observers;
        ErrorHandler errorHandler;
    };
}
