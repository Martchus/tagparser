#include "./statusprovider.h"

#include <c++utilities/conversion/stringbuilder.h>

using namespace std;
using namespace ConversionUtilities;

namespace Media {

/*!
 * \class Media::StatusProvider
 * \brief The StatusProvider class acts as a base class for objects providing status information.
 */

/*!
 * \brief Constructs a new StatusProvider.
 *
 * Since the class is meant to be used as base class the constructor is protected.
 */
StatusProvider::StatusProvider() :
    m_worstNotificationType(NotificationType::None),
    m_percentage(0.0),
    m_abort(false),
    m_forward(nullptr)
{}

/*!
 * \brief Registers a callback function. This function will be called when the status/progress changes.
 * \param callback Specifies the function to be called.
 * \returns Returns an ID for registration. This ID might be used to unregister the callback function using
 *          unregisterCallback().
 *
 * \sa CallbackFunction
 * \sa unregisterCallback()
 */
size_t StatusProvider::registerCallback(CallbackFunction callback)
{
    size_t id = 0;
    for(auto &registred : m_callbacks) {
        if(!registred) {
            registred = callback;
            return id;
        }
        ++id;
    }
    m_callbacks.push_back(callback);
    return id;
}

/*!
 * \brief This method is meant to be called by the derived class to add a \a notification.
 */
void StatusProvider::addNotification(const Notification &notification)
{
    m_notifications.push_back(notification);
    m_worstNotificationType |= notification.type();
    invokeCallbacks();
}

/*!
 * \brief This method is meant to be called by the derived class to add a notification of the specified
 *        \a type, \a message and \a context.
 */
void StatusProvider::addNotification(NotificationType type, const string &message, const string &context)
{
    m_notifications.emplace_back(type, message, context);
    m_worstNotificationType |= type;
    invokeCallbacks();
}

/*!
 * \brief This method is meant to be called by the derived class to add all notifications \a from another
 *        StatusProvider instance.
 */
void StatusProvider::addNotifications(const StatusProvider &from)
{
    if(&from == this) {
        return;
    }
    m_notifications.insert(m_notifications.end(), from.m_notifications.cbegin(), from.m_notifications.cend());
    m_worstNotificationType |= from.worstNotificationType();
    invokeCallbacks();
}

/*!
 * \brief This method is meant to be called by the derived class to add all notifications \a from another
 *        StatusProvider instance.
 *
 * The specified \a higherContext is concatenated with the original context string.
 */
void StatusProvider::addNotifications(const string &higherContext, const StatusProvider &from)
{
    if(&from == this) {
        return;
    }
    for(const auto &notification : from.m_notifications) {
        addNotification(notification.type(), notification.message(), higherContext % ',' % ' ' + notification.context());
    }
}

/*!
 * \brief This method is meant to be called by the derived class to add the specified \a notifications.
 */
void StatusProvider::addNotifications(const NotificationList &notifications)
{
    m_notifications.insert(m_notifications.end(), notifications.cbegin(), notifications.cend());
    if(m_worstNotificationType != Notification::worstNotificationType()) {
        for(const Notification &notification : notifications) {
            if((m_worstNotificationType |= notification.type()) == Notification::worstNotificationType()) {
                break;
            }
        }
    }
    invokeCallbacks();
}

}
