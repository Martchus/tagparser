#ifndef TAGPARSER_PROGRESS_FEEDBACK_H
#define TAGPARSER_PROGRESS_FEEDBACK_H

#include "./exceptions.h"

#include <atomic>
#include <cstdint>
#include <functional>
#include <string>

namespace TagParser {

template <typename ActualProgressFeedback> class BasicProgressFeedback {
public:
    using Callback = std::function<void(ActualProgressFeedback &feedback)>;

    explicit BasicProgressFeedback(const Callback &callback, const Callback &percentageOnlyCallback = Callback());
    explicit BasicProgressFeedback(Callback &&callback = Callback(), Callback &&percentageOnlyCallback = Callback());

    const std::string &step() const;
    std::uint8_t stepPercentage() const;
    std::uint8_t overallPercentage() const;
    void updateStep(const std::string &step, std::uint8_t stepPercentage = 0);
    void updateStep(std::string &&step, std::uint8_t stepPercentage = 0);
    void updateStepPercentage(std::uint8_t stepPercentage);
    void updateStepPercentageFromFraction(double stepPercentage);
    void updateOverallPercentage(std::uint8_t overallPercentage);

private:
    Callback m_callback;
    Callback m_percentageOnlyCallback;
    std::string m_step;
    std::uint8_t m_stepPercentage;
    std::uint8_t m_overallPercentage;
};

/*!
 * \brief Constructs a new BasicProgressFeedback.
 *
 * It will call \a callback on the next step and \a percentageOnlyCallback when only the percentage changes.
 */
template <typename ActualProgressFeedback>
inline BasicProgressFeedback<ActualProgressFeedback>::BasicProgressFeedback(const Callback &callback, const Callback &percentageOnlyCallback)
    : m_callback(callback)
    , m_percentageOnlyCallback(percentageOnlyCallback)
    , m_stepPercentage(0)
    , m_overallPercentage(0)
{
}

/*!
 * \brief Constructs a new BasicProgressFeedback.
 *
 * It will call \a callback on the next step and \a percentageOnlyCallback when only the percentage changes.
 */
template <typename ActualProgressFeedback>
inline BasicProgressFeedback<ActualProgressFeedback>::BasicProgressFeedback(Callback &&callback, Callback &&percentageOnlyCallback)
    : m_callback(callback)
    , m_percentageOnlyCallback(percentageOnlyCallback)
    , m_stepPercentage(0)
    , m_overallPercentage(0)
{
}

/*!
 * \brief Returns the name of the current step (initially empty).
 */
template <typename ActualProgressFeedback> inline const std::string &BasicProgressFeedback<ActualProgressFeedback>::step() const
{
    return m_step;
}

/*!
 * \brief Returns the percentage of the current step (initially 0, supposed to be a value from 0 to 100).
 * \remarks A percentage of 0 means that the percentage is currently unknown; 100 means finished.
 */
template <typename ActualProgressFeedback> inline std::uint8_t BasicProgressFeedback<ActualProgressFeedback>::stepPercentage() const
{
    return m_stepPercentage;
}

/*!
 * \brief Returns the overall percentage (initially 0, supposed to be a value from 0 to 100).
 * \remarks A percentage of 0 means that the percentage is currently unknown; 100 means finished.
 */
template <typename ActualProgressFeedback> inline std::uint8_t BasicProgressFeedback<ActualProgressFeedback>::overallPercentage() const
{
    return m_overallPercentage;
}

/*!
 * \brief Updates the current step and invokes the first callback specified on construction.
 * \remarks Supposed to be called only by the operation itself.
 */
template <typename ActualProgressFeedback>
inline void BasicProgressFeedback<ActualProgressFeedback>::updateStep(const std::string &step, std::uint8_t stepPercentage)
{
    m_step = step;
    m_stepPercentage = stepPercentage;
    if (m_callback) {
        m_callback(*static_cast<ActualProgressFeedback *>(this));
    }
}

/*!
 * \brief Updates the current step and invokes the first callback specified on construction.
 * \remarks Supposed to be called only by the operation itself.
 */
template <typename ActualProgressFeedback>
inline void BasicProgressFeedback<ActualProgressFeedback>::updateStep(std::string &&step, std::uint8_t stepPercentage)
{
    m_step = step;
    m_stepPercentage = stepPercentage;
    if (m_callback) {
        m_callback(*static_cast<ActualProgressFeedback *>(this));
    }
}

/*!
 * \brief Updates the current step percentage and invokes the second callback specified on construction (or the first if only one has been specified).
 * \remarks Supposed to be called only by the operation itself.
 */
template <typename ActualProgressFeedback>
inline void BasicProgressFeedback<ActualProgressFeedback>::updateStepPercentage(std::uint8_t stepPercentage)
{
    m_stepPercentage = stepPercentage;
    if (m_percentageOnlyCallback) {
        m_percentageOnlyCallback(*static_cast<ActualProgressFeedback *>(this));
    } else if (m_callback) {
        m_callback(*static_cast<ActualProgressFeedback *>(this));
    }
}

/*!
 * \brief Updates the current step percentage and invokes the second callback specified on construction (or the first if only one has been specified).
 * \param stepPercentage Specifies the percentage which is supposed to be a value from 0.0 to 1.0.
 * \remarks Supposed to be called only by the operation itself.
 */
template <typename ActualProgressFeedback>
inline void BasicProgressFeedback<ActualProgressFeedback>::updateStepPercentageFromFraction(double stepPercentage)
{
    updateStepPercentage(static_cast<std::uint8_t>(stepPercentage * 100.0));
}

/*!
 * \brief Updates the overall percentage and invokes the second callback specified on construction (or the first if only one has been specified).
 * \remarks Supposed to be called only by the operation itself.
 */
template <typename ActualProgressFeedback>
inline void BasicProgressFeedback<ActualProgressFeedback>::updateOverallPercentage(std::uint8_t overallPercentage)
{
    m_overallPercentage = overallPercentage;
    if (m_percentageOnlyCallback) {
        m_percentageOnlyCallback(*static_cast<ActualProgressFeedback *>(this));
    } else if (m_callback) {
        m_callback(*static_cast<ActualProgressFeedback *>(this));
    }
}

class ProgressFeedback : public BasicProgressFeedback<ProgressFeedback> {
public:
    explicit ProgressFeedback(const Callback &callback, const Callback &percentageOnlyCallback = Callback());
    explicit ProgressFeedback(Callback &&callback = Callback(), Callback &&percentageOnlyCallback = Callback());
};

/*!
 * \brief Constructs a new ProgressFeedback.
 *
 * It will call \a callback on the next step and \a percentageOnlyCallback when only the percentage changes.
 */
inline ProgressFeedback::ProgressFeedback(const Callback &callback, const Callback &percentageOnlyCallback)
    : BasicProgressFeedback<ProgressFeedback>(callback, percentageOnlyCallback)
{
}

/*!
 * \brief Constructs a new ProgressFeedback.
 *
 * It will call \a callback on the next step and \a percentageOnlyCallback when only the percentage changes.
 */
inline ProgressFeedback::ProgressFeedback(Callback &&callback, Callback &&percentageOnlyCallback)
    : BasicProgressFeedback<ProgressFeedback>(callback, percentageOnlyCallback)
{
}

class AbortableProgressFeedback : public BasicProgressFeedback<AbortableProgressFeedback> {
public:
    explicit AbortableProgressFeedback(const Callback &callback, const Callback &percentageOnlyCallback = Callback());
    explicit AbortableProgressFeedback(Callback &&callback = Callback(), Callback &&percentageOnlyCallback = Callback());
    AbortableProgressFeedback(const AbortableProgressFeedback &);

    bool isAborted() const;
    void tryToAbort();
    void stopIfAborted() const;
    void nextStepOrStop(const std::string &step, std::uint8_t stepPercentage = 0);
    void nextStepOrStop(std::string &&step, std::uint8_t stepPercentage = 0);

private:
    std::atomic_bool m_aborted;
};

/*!
 * \brief Constructs a new AbortableProgressFeedback.
 *
 * It will call \a callback on the next step and \a percentageOnlyCallback when only the percentage changes.
 */
inline AbortableProgressFeedback::AbortableProgressFeedback(const Callback &callback, const Callback &percentageOnlyCallback)
    : BasicProgressFeedback<AbortableProgressFeedback>(callback, percentageOnlyCallback)
    , m_aborted(false)
{
}

/*!
 * \brief Constructs a new AbortableProgressFeedback.
 *
 * It will call \a callback on the next step and \a percentageOnlyCallback when only the percentage changes.
 */
inline AbortableProgressFeedback::AbortableProgressFeedback(Callback &&callback, Callback &&percentageOnlyCallback)
    : BasicProgressFeedback<AbortableProgressFeedback>(callback, percentageOnlyCallback)
    , m_aborted(false)
{
}

/*!
 * \brief Constructs a new AbortableProgressFeedback based on \a other.
 */
inline AbortableProgressFeedback::AbortableProgressFeedback(const AbortableProgressFeedback &other)
    : BasicProgressFeedback<AbortableProgressFeedback>(other)
    , m_aborted(other.isAborted())
{
}

/*!
 * \brief Returns whether the operation has been aborted via tryToAbort().
 */
inline bool AbortableProgressFeedback::isAborted() const
{
    return m_aborted.load();
}

/*!
 * \brief Aborts the operation.
 * \remarks The operation will not be killed forcefully. It will be aborted at the next point where it makes sense or even
 *          finish if it makes no sense to abort.
 */
inline void AbortableProgressFeedback::tryToAbort()
{
    return m_aborted.store(true);
}

/*!
 * \brief Throws an OperationAbortedException if aborted.
 * \remarks Supposed to be called only by the operation itself.
 */
inline void AbortableProgressFeedback::stopIfAborted() const
{
    if (isAborted()) {
        throw OperationAbortedException();
    }
}

/*!
 * \brief Throws an OperationAbortedException if aborted; otherwise the data for the next step is set.
 * \remarks Supposed to be called only by the operation itself.
 */
inline void AbortableProgressFeedback::nextStepOrStop(const std::string &status, std::uint8_t percentage)
{
    if (isAborted()) {
        throw OperationAbortedException();
    }
    updateStep(status, percentage);
}

/*!
 * \brief Throws an OperationAbortedException if aborted; otherwise the data for the next step is set.
 * \remarks Supposed to be called only by the operation itself.
 */
inline void AbortableProgressFeedback::nextStepOrStop(std::string &&status, std::uint8_t percentage)
{
    if (isAborted()) {
        throw OperationAbortedException();
    }
    updateStep(status, percentage);
}

} // namespace TagParser

#endif // TAGPARSER_PROGRESS_FEEDBACK_H
