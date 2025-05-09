#include "api.h"
#include <QDebug>

ApiClient::ApiClient(const QString &url, QObject *parent) : QObject(parent),
    baseUrl(url)
{
    manager = std::make_unique<QNetworkAccessManager>(this);
}

void ApiClient::sendRequest(const QString &method, const QString &endpoint, const QJsonObject &json, bool requiresAuth)
{
    QNetworkRequest request(QUrl(baseUrl + endpoint));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    if(requiresAuth && !authToken.isEmpty()){
        request.setRawHeader("Authorization", authToken.toUtf8());
    }

    QNetworkReply* reply = nullptr;
    QByteArray jsonData = QJsonDocument(json).toJson();

    if(method == "GET"){
        reply = manager->get(request);
    }else if(method == "POST"){
        reply = manager->post(request, jsonData);
    }else if(method == "PUT"){
        reply = manager->put(request, jsonData);
    }else if(method == "DELETE"){
        reply = manager->deleteResource(request);
    }else if(method == "PATCH"){
        request.setRawHeader("HTTP", "PATCH");
        reply = manager->post(request, jsonData);
    }
    qDebug() << method << authToken << baseUrl + endpoint;
    if(!reply)return;

    QObject::connect(reply, &QNetworkReply::finished, [=](){
        if(reply->error() == QNetworkReply::NoError){
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            if(endpoint == "/login"){
                authToken = doc.object()["access_token"].toString();
                emit this->AuthSuccess(authToken);
            }else if(endpoint == "/register"){
                QJsonObject data;
                data["username"] = json["username"];
                data["password"] = json["password"];
                this->sendRequest("POST", "/login", data, false);
            }else if(endpoint.startsWith("/folders") && method == "GET"){
                emit this->foldersReceived(doc.array());
            }else if(endpoint.startsWith("/tasks") && method == "GET"){
                emit this->taskReceived(doc);
                qDebug() << doc;
            }else{
                emit this->operationCompleted(true, "Success");
            }
        }else{
            QString error = reply->errorString();
            if(reply->error() == QNetworkReply::AuthenticationRequiredError){
                error = "Authentication failed";
            }
            emit this->operationCompleted(false, error);
            if (endpoint == "/login" || endpoint == "/register") {
                emit this->AuthError(error);
            }
        }
        reply->deleteLater();
    });

}

void ApiClient::registerUser(const QString &username, const QString &password, const QString &email, const QString &fullname)
{
    QJsonObject data;
    data["username"] = username;
    data["email"] = email;
    data["password"] = password;
    data["full_name"] = fullname;
    sendRequest("POST", "/register", data, false);
}

void ApiClient::loginUser(const QString &username, const QString &password)
{
    QJsonObject data;
    data["username"] = username;
    data["password"] = password;
    sendRequest("POST", "/login", data, false);
}

void ApiClient::getFolders()
{
    sendRequest("GET", "/folders");
}

void ApiClient::createFolder(const QString &name)
{
    QJsonObject data;
    data["name"] = name;
    sendRequest("POST", "/folders", data);
}

void ApiClient::getFolder(int folder_id)
{
    sendRequest("GET", QString("/folders/%1").arg(folder_id));
}

void ApiClient::updateFolder(int folder_id, const QString &new_name)
{
    QJsonObject data;
    data["name"] = new_name;
    sendRequest("PUT", QString("/folders/%1").arg(folder_id), data);
}

void ApiClient::deleteFolder(int folder_id)
{
    sendRequest("DELETE", QString("/folders/%1").arg(folder_id));
}

void ApiClient::getFolderTasks(int folder_id)
{
    sendRequest("GET", QString("/folders/%1/tasks").arg(folder_id));
}

void ApiClient::getAllTasks()
{
    sendRequest("GET", "/tasks");
}

void ApiClient::createTask(const QString &title, const QString &desc, const QDateTime &due_time, int priority, int folder_id)
{
    QJsonObject data;
    data["title"] = title;
    data["description"] = desc;
    data["due_time"] = due_time.toString("yyyy-MM-ddTHH:mm:ssZ");
    data["priority"] = priority;
    data["folder_id"] = folder_id;
    sendRequest("POST", "/tasks", data);
}

void ApiClient::getTask(int task_id)
{
    sendRequest("GET", QString("/tasks/%1").arg(task_id));
}

void ApiClient::updateTask(int task_id, const QJsonObject &updates)
{
    sendRequest("PUT", QString("/tasks/%1").arg(task_id), updates);
}

void ApiClient::deleteTask(int task_id)
{
    sendRequest("DELETE", QString("/tasks/%1").arg(task_id));
}

void ApiClient::toggleTaskCompletion(int task_id)
{
    sendRequest("PUT", QString("/tasks/%1/toggle").arg(task_id));
}

void ApiClient::moveTaskToFolder(int task_id, int new_folder)
{
    QJsonObject data;
    data["new_folder_id"] = new_folder;
    sendRequest("PUT", QString("/tasks/%1/move").arg(task_id), data);
}





