#include "ObserverNotificationTests.h"

#include "clinical/LedIndicator.h"
#include "clinical/ScoreObserver.h"
#include "clinical/ScoreUpdateNotifier.h"

#include <iostream>
#include <sstream>

using namespace clinical;

namespace
{
    class RecordingObserver : public IScoreObserver
    {
    public:
        void OnScoreUpdated(const ScoreUpdateData& data) override
        {
            ++notificationCount;
            lastRiskLevel = data.highestRiskLevel;
            lastColor = data.highestRiskColor;
        }

        int notificationCount = 0;
        std::string lastRiskLevel;
        std::string lastColor;
    };

    class ThrowingObserver : public IScoreObserver
    {
    public:
        void OnScoreUpdated(const ScoreUpdateData&) override
        {
            throw std::runtime_error("Observer hardware communication fault");
        }
    };

    void ShouldNotifySubscribedObserver()
    {
        ScoreUpdateNotifier notifier;
        RecordingObserver observer;
        notifier.Subscribe(&observer);

        ScoreUpdateData data;
        data.highestRiskLevel = "High";
        data.highestRiskColor = "Red";
        notifier.Notify(data);

        AssertEqual(1, observer.notificationCount, "Notifier should call observer once");
        AssertEqual(std::string("High"), observer.lastRiskLevel, "Notifier should forward risk level");
        AssertEqual(std::string("Red"), observer.lastColor, "Notifier should forward color");
    }

    void ShouldNotifyMultipleObservers()
    {
        ScoreUpdateNotifier notifier;
        RecordingObserver obs1, obs2, obs3;
        notifier.Subscribe(&obs1);
        notifier.Subscribe(&obs2);
        notifier.Subscribe(&obs3);

        ScoreUpdateData data;
        data.highestRiskLevel = "Medium";
        data.highestRiskColor = "Yellow";
        notifier.Notify(data);

        AssertEqual(1, obs1.notificationCount, "Observer 1 should be notified");
        AssertEqual(1, obs2.notificationCount, "Observer 2 should be notified");
        AssertEqual(1, obs3.notificationCount, "Observer 3 should be notified");
    }

    void ShouldIgnoreNullObserver()
    {
        ScoreUpdateNotifier notifier;
        notifier.Subscribe(nullptr);

        ScoreUpdateData data;
        data.highestRiskLevel = "Low";
        data.highestRiskColor = "Green";

        bool threw = false;
        try { notifier.Notify(data); }
        catch (...) { threw = true; }

        AssertTrue(!threw, "Notifier should not throw when null observer is subscribed");
    }

    void ShouldNotCrashWithNoObservers()
    {
        ScoreUpdateNotifier notifier;
        ScoreUpdateData data;
        data.highestRiskLevel = "Low";
        data.highestRiskColor = "Green";

        bool threw = false;
        try { notifier.Notify(data); }
        catch (...) { threw = true; }

        AssertTrue(!threw, "Notifier should not throw when no observers are subscribed");
    }

    void ShouldFireLedIndicatorCallback()
    {
        std::ostringstream sink;
        LedIndicator led(sink);

        ScoreUpdateData data;
        data.highestRiskLevel = "High";
        data.highestRiskColor = "Red";

        bool threw = false;
        try { led.OnScoreUpdated(data); }
        catch (...) { threw = true; }

        AssertTrue(!threw, "LedIndicator::OnScoreUpdated should not throw");
        AssertTrue(!sink.str().empty(), "LedIndicator should produce output on update");
    }

    void ShouldLedIndicatorOutputContainColor()
    {
        std::ostringstream sink;
        LedIndicator led(sink);

        ScoreUpdateData data;
        data.highestRiskLevel = "Medium";
        data.highestRiskColor = "Yellow";

        led.OnScoreUpdated(data);

        AssertTrue(sink.str().find("Yellow") != std::string::npos,
            "LedIndicator output should contain the risk color");
    }

    void ShouldNotifyObserversWithSpecificPayload()
    {
        ScoreUpdateNotifier notifier;
        RecordingObserver observer;
        notifier.Subscribe(&observer);

        ScoreUpdateData data;
        data.highestRiskLevel = "High";
        data.highestRiskColor = "Red";
        notifier.Notify(data);

        AssertEqual(1, observer.notificationCount, "Notifier should notify subscribed observers");
        AssertEqual(std::string("Red"), observer.lastColor, "Notifier should forward latest LED color");
    }

    void ShouldNotifyPairOfObservers()
    {
        ScoreUpdateNotifier notifier;
        RecordingObserver obs1, obs2;
        notifier.Subscribe(&obs1);
        notifier.Subscribe(&obs2);

        ScoreUpdateData data;
        data.highestRiskLevel = "Medium";
        data.highestRiskColor = "Yellow";
        notifier.Notify(data);

        AssertEqual(1, obs1.notificationCount, "Observer 1 should be notified");
        AssertEqual(1, obs2.notificationCount, "Observer 2 should be notified");
        AssertEqual(std::string("Medium"), obs1.lastRiskLevel, "Observer 1 should receive correct risk level");
    }

    void ShouldDeliverMultipleNotificationsSequentially()
    {
        ScoreUpdateNotifier notifier;
        RecordingObserver observer;
        notifier.Subscribe(&observer);

        for (int i = 0; i < 5; ++i)
        {
            ScoreUpdateData data;
            data.highestRiskLevel = "Low";
            data.highestRiskColor = "Green";
            notifier.Notify(data);
        }

        AssertEqual(5, observer.notificationCount, "Observer should receive all 5 notifications");
    }

    void ShouldSupportUnsubscribe()
    {
        ScoreUpdateNotifier notifier;
        RecordingObserver observer;
        notifier.Subscribe(&observer);
        notifier.Unsubscribe(&observer);

        ScoreUpdateData data;
        data.highestRiskLevel = "High";
        data.highestRiskColor = "Red";
        notifier.Notify(data);

        AssertEqual(0, observer.notificationCount, "Unsubscribed observer should not receive updates");
    }

    void ShouldAutoUnsubscribeOnScopedSubscriptionDestruction()
    {
        ScoreUpdateNotifier notifier;
        RecordingObserver observer;

        {
            auto scopedSub = notifier.SubscribeScoped(&observer);
            AssertTrue(scopedSub.IsConnected(), "Scoped subscription should be connected initially");
            AssertEqual(1, static_cast<int>(notifier.ObserverCount()), "Observer count should be 1");

            ScoreUpdateData data;
            data.highestRiskLevel = "Low";
            data.highestRiskColor = "Green";
            notifier.Notify(data);
            AssertEqual(1, observer.notificationCount, "Observer should receive notification while scoped");
        }

        // scopedSub went out of scope here
        AssertEqual(0, static_cast<int>(notifier.ObserverCount()), "Observer count should be 0 after scope exit");

        ScoreUpdateData data2;
        data2.highestRiskLevel = "High";
        data2.highestRiskColor = "Red";
        notifier.Notify(data2);
        AssertEqual(1, observer.notificationCount, "Observer should NOT receive notification after scope exit");
    }

    void ShouldSupportMoveSemanticsOnScopedSubscription()
    {
        ScoreUpdateNotifier notifier;
        RecordingObserver observer;

        auto sub1 = notifier.SubscribeScoped(&observer);
        AssertTrue(sub1.IsConnected(), "sub1 should be connected");

        // Move construct
        ScopedSubscription sub2(std::move(sub1));
        AssertTrue(!sub1.IsConnected(), "sub1 should be disconnected after move");
        AssertTrue(sub2.IsConnected(), "sub2 should be connected after move");

        // Move assign
        ScopedSubscription sub3;
        sub3 = std::move(sub2);
        AssertTrue(!sub2.IsConnected(), "sub2 should be disconnected after move assign");
        AssertTrue(sub3.IsConnected(), "sub3 should be connected after move assign");

        sub3.Disconnect();
        AssertTrue(!sub3.IsConnected(), "sub3 should be disconnected after explicit Disconnect");
        AssertEqual(0, static_cast<int>(notifier.ObserverCount()), "Observer count should be 0");
    }

    void ShouldIsolateObserverExceptionAndContinueNotifying()
    {
        ScoreUpdateNotifier notifier;
        ThrowingObserver faultyObserver;
        RecordingObserver healthyObserver;

        bool errorCallbackTriggered = false;
        notifier.SetErrorHandler([&](IScoreObserver* obs, const std::exception* ex) {
            if (obs == &faultyObserver && ex != nullptr)
            {
                errorCallbackTriggered = true;
            }
        });

        // Register faulty observer first, followed by healthy observer
        notifier.Subscribe(&faultyObserver);
        notifier.Subscribe(&healthyObserver);

        ScoreUpdateData data;
        data.highestRiskLevel = "High";
        data.highestRiskColor = "Red";

        // Must not throw despite faultyObserver throwing std::runtime_error
        bool threw = false;
        try
        {
            notifier.Notify(data);
        }
        catch (...)
        {
            threw = true;
        }

        AssertTrue(!threw, "Notify() must guarantee exception safety and not leak observer exceptions");
        AssertTrue(errorCallbackTriggered, "Error handler should be invoked when observer throws");
        AssertEqual(1, healthyObserver.notificationCount, "Healthy observer must still receive notification");
        AssertEqual(std::string("High"), healthyObserver.lastRiskLevel, "Healthy observer should receive data");
    }
}

void RegisterObserverNotificationTests(std::vector<TestCase>& tests)
{
    tests.push_back({ "Notifier - Notify single observer",                       ShouldNotifySubscribedObserver });
    tests.push_back({ "Notifier - Notify multiple observers",                    ShouldNotifyMultipleObservers });
    tests.push_back({ "Notifier - Ignore null observer subscription",            ShouldIgnoreNullObserver });
    tests.push_back({ "Notifier - Safe notify with empty observer list",         ShouldNotCrashWithNoObservers });
    tests.push_back({ "LedIndicator - Callback triggers without throwing",       ShouldFireLedIndicatorCallback });
    tests.push_back({ "LedIndicator - Output contains corresponding risk color",  ShouldLedIndicatorOutputContainColor });
    tests.push_back({ "Notifier - Deliver specific risk color in payload",       ShouldNotifyObserversWithSpecificPayload });
    tests.push_back({ "Notifier - Broadcast to observer pair",                   ShouldNotifyPairOfObservers });
    tests.push_back({ "Notifier - Deliver multiple sequential notifications",    ShouldDeliverMultipleNotificationsSequentially });
    tests.push_back({ "Notifier - Support dynamic unsubscription (RAII-friendly)", ShouldSupportUnsubscribe });
    tests.push_back({ "Notifier - RAII ScopedSubscription auto-disconnects on destruction", ShouldAutoUnsubscribeOnScopedSubscriptionDestruction });
    tests.push_back({ "Notifier - ScopedSubscription supports move semantics",   ShouldSupportMoveSemanticsOnScopedSubscription });
    tests.push_back({ "Notifier - Exception isolation ensures broadcast to all healthy observers", ShouldIsolateObserverExceptionAndContinueNotifying });
}
