#ifndef STATUSPROVIDER_H
#define STATUSPROVIDER_H

#include "./notification.h"

#include <functional>
#include <vector>

namespace Media {

class MediaFileInfo;

class TAG_PARSER_EXPORT StatusProvider
{
    // FIXME: make transferNotifications() public in next minor release and get rid of the friend class again
    friend class MediaFileInfo;

public:
    typedef std::function<void (StatusProvider &sender)> CallbackFunction;
    typedef std::vector<CallbackFunction> CallbackVector;
    typedef std::pair<int, CallbackFunction> CallbackPair;

    const NotificationList &notifications() const;
    bool hasNotifications() const;
    bool hasCriticalNotifications() const;
    NotificationType worstNotificationType() const;
    const std::string &currentStatus() const;
    double currentPercentage() const;
    size_t registerCallback(CallbackFunction callback);
    void unregisterCallback(size_t id);
    void unregisterAllCallbacks();
    void forwardStatus(StatusProvider *other = nullptr);
    StatusProvider *usedProvider();
    void tryToAbort();
    bool isAborted() const;
    void invalidateStatus();
    void invalidateNotifications();
    void updateStatus(const std::string &status);
    void updateStatus(const std::string &status, double percentage);
    void updatePercentage(double percentage);
    void addNotification(const Notification &notification);
    void addNotification(NotificationType type, const std::string &message, const std::string &context);
    //void addNotifications(const StatusProvider &from);
    //void addNotifications(const std::string &higherContext, const StatusProvider &from);
    //void addNotifications(const NotificationList &notifications);

protected:
    StatusProvider();

private:
    void invokeCallbacks();
    void updateWorstNotificationType(NotificationType notificationType);
    //void transferNotifications(StatusProvider &from);

    StatusProvider *m_forward;
    std::string m_status;
    double m_percentage;
    CallbackVector m_callbacks;
    NotificationList m_notifications;
    NotificationType m_worstNotificationType;
    bool m_abort;
};

/*!
 * \brief This method is meant to be called by the derived class to report updated status information.
 */
inline void StatusProvider::updateStatus(const std::string &status)
{
    if(m_forward) {
        m_forward->updateStatus(status);
        return;
    }
    m_status = status;
    invokeCallbacks();
}

/*!
 * \brief This method is meant to be called by the derived class to report updated status information.
 *
 * The specified progress \a percentage should be a value between 0 and 1.
 */
inline void StatusProvider::updateStatus(const std::string &status, double percentage)
{
    if(m_forward) {
        m_forward->updateStatus(status, percentage);
        return;
    }
    m_status = status;
    m_percentage = percentage;
    invokeCallbacks();
}

/*!
 * \brief This method is meant to be called by the derived class to report updated progress percentage only.
 *
 * The specified \a percentage should be a value between 0 and 1.
 */
inline void StatusProvider::updatePercentage(double percentage)
{
    if(m_forward) {
        m_forward->updatePercentage(percentage);
        return;
    }
    m_percentage = percentage;
    invokeCallbacks();
}

/*!
 * \brief Returns the provider which callback functions will be called when the status or the percentage is updated.
 *
 * The default is the current instance. This can be changed using the forwardStatus() method.
 */
inline StatusProvider *StatusProvider::usedProvider()
{
    return m_forward ? m_forward->usedProvider() : this;
}

/*!
 * \brief Returns notifications for the current object.
 */
inline const NotificationList &StatusProvider::notifications() const
{
    return m_notifications;
}

/*!
 * \brief Returns an indication whether there are notifications for the current object.
 */
inline bool StatusProvider::hasNotifications() const
{
    return !m_notifications.empty();
}

/*!
 * \brief Returns an indication whether there are critical notifications for the current object.
 */
inline bool StatusProvider::hasCriticalNotifications() const
{
    return m_worstNotificationType == NotificationType::Critical;
}

/*!
 * \brief Returns the worst notification type.
 */
inline NotificationType StatusProvider::worstNotificationType() const
{
    return m_worstNotificationType;
}

/*!
 * \brief Returns a status information for the current object.
 */
inline const std::string &StatusProvider::currentStatus() const
{
    if(m_status.empty() && m_forward) {
        return m_forward->currentStatus();
    }
    return m_status;
}

/*!
 * \brief Returns the progress percentage of the current object.
 */
inline double StatusProvider::currentPercentage() const
{
    if(m_percentage == 0.0 && m_forward) {
        return m_forward->currentPercentage();
    }
    return m_percentage;
}

/*!
 * \brief Returns an indication whether the current operation should be aborted.
 *
 * This flag can be tested when implementing an operation that should be able to be
 * aborted.
 */
inline bool StatusProvider::isAborted() const
{
    return m_abort || (m_forward && m_forward->isAborted());
}

/*!
 * \brief Unregisters a previously registered callback function whith the specified \a id.
 * \param id Specifies the ID of the callback to be unregistered.
 *
 * \sa registerCallback()
 */
inline void StatusProvider::unregisterCallback(size_t id)
{
    if(id < m_callbacks.size()) {
        m_callbacks[id] = nullptr;
    }
}

/*!
 * \brief Unregisters all callback functions.
 *
 * \sa registerCallback()
 */
inline void StatusProvider::unregisterAllCallbacks()
{
    m_callbacks.clear();
}

/*!
 * \brief Forwards all status updates calls and notifications to the specified \a statusProvider.
 *
 * This basically forwards status information updates and notifications to the specified instance.
 *
 * \remarks
 * - Any notifications will *not* be added to the current instance anymore. Instead the
 *   notifications will be added to the specified instance.
 * - The callback methods assigned to the current instance will *not* be called
 *   to inform about status updates anymore. Instead the callback methods associated to
 *   the specified instance will be called.
 * - The current instance is still passed as the sender when invoking callback methods.
 * - The current instance is also considered as aborted if the specified provider is
 *   aborted - even if tryToAbort() has not been called on the current instance.
 * - The current instance will return the status and percentage of the specified
 *   provider if it provides no own status or percentage.
 * - Provide nullptr to revert to the default behaviour.
 * - The methods invalidateStatus() and invalidateNotifications() are *not* forwarded.
 * - The methods updateStatus() and updatePercentage() are *not* forwarded.
 * - The method transferNotifications() is *not* forwarded.
 * - Leads to endless recursion if \a statusProvider forwards (indirectly
 *   or directly) to the current instance. (So better prevent it.)
 * - Set \a other to nullptr to restore the usual behaviour.
 */
inline void StatusProvider::forwardStatus(StatusProvider *other)
{
    m_forward = other;
}

/*!
 * \brief Commands the object to abort the current operation.
 *
 * If the object is currently not operating calling this method has no effect.
 *
 * The current operation might not be stopped immediately.
 */
inline void StatusProvider::tryToAbort()
{
    m_abort = true;
}

/*!
 * \brief Invalidates the current status.
 *
 * The status, the progress percentage and the "aborted"-flag will be wiped. It is
 * recommend to call this method before performing an operation.
 *
 * This method is meant to be called by the derived class before performing operations
 * to wipe possibly still present previous/obsolet status information.
 */
inline void StatusProvider::invalidateStatus()
{
    m_status.clear();
    m_percentage = 0.0;
    m_abort = false;
}

/*!
 * \brief Invalidates the object's notifications.
 *
 * All notifications will be wiped.
 */
inline void StatusProvider::invalidateNotifications()
{
    m_notifications.clear();
    m_worstNotificationType = NotificationType::None;
}

/*!
 * \brief This method is internally called to invoke the registrated callbacks.
 */
inline void StatusProvider::invokeCallbacks()
{
    for(std::function<void (StatusProvider &sender)> &callback : usedProvider()->m_callbacks) {
        if(callback) {
            callback(*this);
        }
    }
}

/*!
 * \brief This method is internally used to update the worst notification type.
 */
inline void StatusProvider::updateWorstNotificationType(NotificationType notificationType)
{
    if(m_forward) {
        m_forward->updateWorstNotificationType(notificationType);
        return;
    }
    if(m_worstNotificationType < notificationType) {
        m_worstNotificationType = notificationType;
    }
}

/*!
 * \brief Transfers all notifications from the specified status provider to the current instance.
 * \remarks In constrast to the similar addNotifications() overload, this method does not copy the notifications. Instead
 *          the notifications are transfered. It also doesn't check whether \a from is the current instance and doesn't
 *          invoke callbacks.
 * \deprecated This method should likely be removed after the status provider rework.
 */
/*inline void StatusProvider::transferNotifications(StatusProvider &from)
{
    m_notifications.splice(m_notifications.end(), from.m_notifications);
    m_worstNotificationType |= from.worstNotificationType();
}*/


}


#endif // STATUSPROVIDER_H
