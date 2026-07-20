#include "UserManager.h"
#include <QFile>
#include <QTextStream>

UserManager::UserManager(const QString &filePath)
    : m_filePath(filePath)
{
    load();
}


void UserManager::load()
{
    m_users.clear();

    QFile file(m_filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty())
            continue;

        QStringList parts = line.split(',');
        if (parts.size() >= 3)
            m_users.insert(parts[0], {parts[1], parts[2]});
    }
    file.close();
}

void UserManager::save()
{
    QFile file(m_filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);
    for (auto it = m_users.begin(); it != m_users.end(); ++it)
        out << it.key() << ',' << it.value().password << ',' << it.value().role << '\n';

    file.close();
}

bool UserManager::verify(const QString &username, const QString &password) const
{
    return m_users.contains(username) && m_users.value(username).password == password;
}

bool UserManager::registerUser(const QString &username, const QString &password)
{
    if (m_users.contains(username))
        return false;

    m_users.insert(username, {password, "analyst"});
    save();
    return true;
}

QString UserManager::roleOf(const QString &username) const
{
    return m_users.value(username).role;
}

QStringList UserManager::allUsernames() const
{
    return m_users.keys();
}

QString UserManager::roleOfUser(const QString &username) const
{
    return m_users.value(username).role;
}

bool UserManager::deleteUser(const QString &username)
{
    if (!m_users.contains(username))
        return false;

    m_users.remove(username);
    save();
    return true;
}

void UserManager::changeRole(const QString &username, const QString &newRole)
{
    if (m_users.contains(username)) {
        m_users[username].role = newRole;
        save();
    }
}
