#ifndef USERMANAGER_H
#define USERMANAGER_H

#include <QMap>
#include <QString>
#include <QStringList>

// 一条用户记录：密码 + 角色
struct UserInfo
{
    QString password;
    QString role; // "admin" / "engineer" / "analyst"
};

class UserManager
{
public:
    explicit UserManager(const QString &filePath);

    // 验证登录，成功返回 true
    bool verify(const QString &username, const QString &password) const;

    // 注册新用户（角色固定 analyst），返回 false 表示用户名已存在
    bool registerUser(const QString &username, const QString &password);

    // 查询角色
    QString roleOf(const QString &username) const;

    // 管理员功能：获取全部用户列表（用于显示）
    QStringList allUsernames() const;
    QString roleOfUser(const QString &username) const;

    // 管理员功能：删除用户，不能删除自己
    bool deleteUser(const QString &username);

    // 管理员功能：更改角色
    void changeRole(const QString &username, const QString &newRole);

private:
    void load();
    void save();

    QString m_filePath;
    QMap<QString, UserInfo> m_users; // key=用户名, value=UserInfo
};

#endif // USERMANAGER_H
