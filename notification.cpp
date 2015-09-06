#include "./notification.h"

using namespace std;
using namespace ChronoUtilities;

namespace Media {

/*!
 * \class Media::Notification
 * \brief The Notification class holds a notification message of a certain notification type.
 *
 * These notifications are generated when parsing or making data.
 */

/*!
 * \brief Constructs a new Notification with the specified \a type, \a message and \a context.
 */
Notification::Notification(NotificationType type, const string &message, const string &context) :
    m_type(type),
    m_msg(message),
    m_context(context),
    m_creationTime(DateTime::now())
{}

/*!
 * \brief Returns the notification type as C-style string.
 */
const char *Notification::typeName() const
{
    switch(m_type) {
    case NotificationType::Information:
        return "information";
    case NotificationType::Warning:
        return "warning";
    case NotificationType::Critical:
        return "critical";
    case NotificationType::None:
    default:
        return "";
    }
}

/*!
 * \brief Sorts the specified \a notifications by time (ascending).
 */
void Notification::sortByTime(NotificationList &notifications)
{
    notifications.sort([] (const Notification &first, const Notification &second) {
        return first.creationTime() < second.creationTime();
    });
}

}
