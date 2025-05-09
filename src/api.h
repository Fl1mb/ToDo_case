#ifndef API_H
#define API_H

#include <QtNetwork/QNetworkAccessManager>
#include <QObject>
#include <QJsonObject>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QJsonDocument>
#include<QByteArray>
#include <QJsonArray>
#include <memory>

class ApiClient : public QObject{
    Q_OBJECT
public:
    explicit ApiClient(const QString& url, QObject* parent = nullptr);

    ///Реализация методов api
    void registerUser(const QString& username, const QString& password,
                      const QString& email, const QString& fullname);
    void loginUser(const QString& username, const QString& password);

    void getFolders();
    void createFolder(const QString& name);
    void getFolder(int folder_id);
    void updateFolder(int folder_id, const QString& new_name);
    void deleteFolder(int folder_id);
    void getFolderTasks(int folder_id);
    void getAllTasks();
    void createTask(const QString& title, const QString& desc,
                    const QDateTime& due_time, int priority,
                    int folder_id);
    void getTask(int task_id);
    void updateTask(int task_id, const QJsonObject& updates);
    void deleteTask(int task_id);
    void toggleTaskCompletion(int task_id);
    void moveTaskToFolder(int task_id, int new_folder);

signals:
    void AuthSuccess(const QString& token);
    void AuthError(const QString& error);
    void foldersReceived(const QJsonArray& folders);
    void taskReceived(const QJsonDocument& tasks);
    void operationCompleted(bool success, const QString& message);

private:

    void sendRequest(const QString& method, const QString& endpoint,
                     const QJsonObject& json = QJsonObject(),
                     bool requiresAuth = true);
    QString authToken;
    std::unique_ptr<QNetworkAccessManager> manager;
    QString baseUrl;
};


#endif // API_H
