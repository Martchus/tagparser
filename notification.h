#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include <c++utilities/chrono/datetime.h>

#include <string>
#include <list>

namespace Media {

/*!
 * \brief Specifies the notification type.
 *
 * The notification types are ordered by their troublesomeness.
 */
enum class NotificationType
{
    None = 0, /**< indicates that no notifications are present; should not be used when construction a notfication */
    Debug = 1, /**< indicates a debbuging notification */
    Information = 2, /**< indicates an informal notification */
    Warning = 3, /**< indicates a warning */
    Critical = 4 /**< indicates a critical notification */
};


/*!
 * \brief Sets \a lhs to \a rhs if \a rhs is worse then \a lhs and returns \a lhs.
 */
inline NotificationType& operator |=(NotificationType &lhs, const NotificationType &rhs)
{
    if(lhs < rhs) {
        lhs = rhs;
    }
    return lhs;
}

class Notification;

typedef std::list<Notification> NotificationList;

class LIB_EXPORT Notification
{
public:
    Notification(NotificationType type, const std::string &message, const std::string &context);

    NotificationType type() const;
    const char *typeName() const;
    const std::string &message() const;
    const std::string &context() const;
    const ChronoUtilities::DateTime &creationTime() const;
    static constexpr inline NotificationType worstNotificationType();
    static void sortByTime(NotificationList &notifications);

private:
    NotificationType m_type;
    std::string m_msg;
    std::string m_context;
    ChronoUtilities::DateTime m_creationTime;
};

/*!
 * \brief Returns the notification type.
 */
inline NotificationType Notification::type() const
{
    return m_type;
}

/*!
 * \brief Returns the message.
 */
inline const std::string &Notification::message() const
{
    return m_msg;
}

/*!
 * \brief Returns the context, eg. "parsing element xyz".
 */
inline const std::string &Notification::context() const
{
    return m_context;
}

/*!
 * \brief Returns the time when the notification originally was created.
 */
inline const ChronoUtilities::DateTime &Notification::creationTime() const
{
    return m_creationTime;
}

/*!
 * \brief Returns the worst notification type.
 */
constexpr NotificationType Notification::worstNotificationType()
{
    return NotificationType::Critical;
}

}

#endif // NOTIFICATION_H
