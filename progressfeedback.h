#ifndef TAGPARSER_PROGRESS_FEEDBACK_H
#define TAGPARSER_PROGRESS_FEEDBACK_H

#include "./exceptions.h"

#include <c++utilities/conversion/types.h>

#include <atomic>
#include <functional>
#include <string>

namespace TagParser {

template <typename ActualProgressFeedback> class BasicProgressFeedback {
public:
    using Callback = std::function<void(ActualProgressFeedback &feedback)>;

    BasicProgressFeedback(const Callback &callback, const Callback &percentageOnlyCallback = Callback());
    BasicProgressFeedback(Callback &&callback, Callback &&percentageOnlyCallback = Callback());

    const std::string &step() const;
    byte stepPercentage() const;
    byte overallPercentage() const;
    void updateStep(const std::string &step, byte stepPercentage = 0);
    void updateStep(std::string &&step, byte stepPercentage = 0);
    void updateStepPercentage(byte stepPercentage);
    void updateStepPercentageFromFraction(double stepPercentage);
    void updateOverallPercentage(byte overallPercentage);

private:
    Callback m_callback;
    Callback m_percentageOnlyCallback;
    std::string m_step;
    byte m_stepPercentage;
    byte m_overallPercentage;
};

template <typename ActualProgressFeedback>
inline BasicProgressFeedback<ActualProgressFeedback>::BasicProgressFeedback(const Callback &callback, const Callback &percentageOnlyCallback)
    : m_callback(callback)
    , m_percentageOnlyCallback(percentageOnlyCallback)
    , m_stepPercentage(0)
    , m_overallPercentage(0)
{
}

template <typename ActualProgressFeedback>
inline BasicProgressFeedback<ActualProgressFeedback>::BasicProgressFeedback(Callback &&callback, Callback &&percentageOnlyCallback)
    : m_callback(callback)
    , m_percentageOnlyCallback(percentageOnlyCallback)
    , m_stepPercentage(0)
    , m_overallPercentage(0)
{
}

template <typename ActualProgressFeedback> inline const std::string &BasicProgressFeedback<ActualProgressFeedback>::step() const
{
    return m_step;
}

template <typename ActualProgressFeedback> inline byte BasicProgressFeedback<ActualProgressFeedback>::stepPercentage() const
{
    return m_stepPercentage;
}

template <typename ActualProgressFeedback> inline byte BasicProgressFeedback<ActualProgressFeedback>::overallPercentage() const
{
    return m_overallPercentage;
}

template <typename ActualProgressFeedback>
inline void BasicProgressFeedback<ActualProgressFeedback>::updateStep(const std::string &step, byte stepPercentage)
{
    m_step = step;
    m_stepPercentage = stepPercentage;
    if (m_callback) {
        m_callback(*static_cast<ActualProgressFeedback *>(this));
    }
}

template <typename ActualProgressFeedback>
inline void BasicProgressFeedback<ActualProgressFeedback>::updateStep(std::string &&step, byte stepPercentage)
{
    m_step = step;
    m_stepPercentage = stepPercentage;
    if (m_callback) {
        m_callback(*static_cast<ActualProgressFeedback *>(this));
    }
}

template <typename ActualProgressFeedback> inline void BasicProgressFeedback<ActualProgressFeedback>::updateStepPercentage(byte stepPercentage)
{
    m_stepPercentage = stepPercentage;
    if (m_percentageOnlyCallback) {
        m_percentageOnlyCallback(*static_cast<ActualProgressFeedback *>(this));
    } else if (m_callback) {
        m_callback(*static_cast<ActualProgressFeedback *>(this));
    }
}

template <typename ActualProgressFeedback>
inline void BasicProgressFeedback<ActualProgressFeedback>::updateStepPercentageFromFraction(double stepPercentage)
{
    updateStepPercentage(static_cast<byte>(stepPercentage * 100.0));
}

template <typename ActualProgressFeedback> inline void BasicProgressFeedback<ActualProgressFeedback>::updateOverallPercentage(byte overallPercentage)
{
    m_overallPercentage = overallPercentage;
    if (m_percentageOnlyCallback) {
        m_percentageOnlyCallback(*static_cast<ActualProgressFeedback *>(this));
    } else if (m_callback) {
        m_callback(*static_cast<ActualProgressFeedback *>(this));
    }
}

class ProgressFeedback : public BasicProgressFeedback<ProgressFeedback> {
    ProgressFeedback(const Callback &callback, const Callback &percentageOnlyCallback = Callback());
    ProgressFeedback(Callback &&callback, Callback &&percentageOnlyCallback = Callback());
};

inline ProgressFeedback::ProgressFeedback(const Callback &callback, const Callback &percentageOnlyCallback)
    : BasicProgressFeedback<ProgressFeedback>(callback, percentageOnlyCallback)
{
}

inline ProgressFeedback::ProgressFeedback(Callback &&callback, Callback &&percentageOnlyCallback)
    : BasicProgressFeedback<ProgressFeedback>(callback, percentageOnlyCallback)
{
}

class AbortableProgressFeedback : public BasicProgressFeedback<AbortableProgressFeedback> {
public:
    AbortableProgressFeedback(const Callback &callback, const Callback &percentageOnlyCallback = Callback());
    AbortableProgressFeedback(Callback &&callback, Callback &&percentageOnlyCallback = Callback());

    bool isAborted() const;
    void tryToAbort();
    void stopIfAborted() const;
    void nextStepOrStop(const std::string &step, byte stepPercentage = 0);
    void nextStepOrStop(std::string &&step, byte stepPercentage = 0);

private:
    std::atomic_bool m_aborted;
};

inline AbortableProgressFeedback::AbortableProgressFeedback(const Callback &callback, const Callback &percentageOnlyCallback)
    : BasicProgressFeedback<AbortableProgressFeedback>(callback, percentageOnlyCallback)
    , m_aborted(false)
{
}

inline AbortableProgressFeedback::AbortableProgressFeedback(Callback &&callback, Callback &&percentageOnlyCallback)
    : BasicProgressFeedback<AbortableProgressFeedback>(callback, percentageOnlyCallback)
    , m_aborted(false)
{
}

inline bool AbortableProgressFeedback::isAborted() const
{
    return m_aborted.load();
}

inline void AbortableProgressFeedback::tryToAbort()
{
    return m_aborted.store(true);
}

inline void AbortableProgressFeedback::stopIfAborted() const
{
    if (isAborted()) {
        throw OperationAbortedException();
    }
}

inline void AbortableProgressFeedback::nextStepOrStop(const std::string &status, byte percentage)
{
    if (isAborted()) {
        throw OperationAbortedException();
    }
    updateStep(status, percentage);
}

inline void AbortableProgressFeedback::nextStepOrStop(std::string &&status, byte percentage)
{
    if (isAborted()) {
        throw OperationAbortedException();
    }
    updateStep(status, percentage);
}

} // namespace TagParser

#endif // TAGPARSER_PROGRESS_FEEDBACK_H
