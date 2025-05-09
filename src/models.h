#ifndef MODELS_H
#define MODELS_H
#include <string>
#include <QDateTime>
#include <vector>

struct UserModel{
    int id;
    std::string username;
    std::string email;
    std::string password;
    std::string full_name;
    bool is_active;
    QDateTime created_at;
};

struct TaskModel{
    int user_id;
    int task_id;
    int folder_id;
    std::string title;
    std::string description;
    QDateTime Due_time;
    int priority;
    bool is_completed;
    QDateTime created_at;
    QDateTime updated_at;
};

struct FolderModel{
    int folder_id;
    int user_id;
    std::string name;
    QDateTime created_at;
    std::vector<TaskModel> tasks;
};

#endif // MODELS_H
