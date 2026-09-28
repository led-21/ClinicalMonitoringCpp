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
}
