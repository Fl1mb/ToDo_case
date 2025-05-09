#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "authorization.h"
#include "api.h"
#include <QMainWindow>
#include <QStandardItemModel>
#include <QListView>
#include <QListWidget>
#include <QTabWidget>
#include <QPushButton>
#include <QSplitter>
#include <QApplication>
#include <QInputDialog>
#include <QTextEdit>
#include <QDateEdit>
#include <QSpinBox>
#include <QTimer>
#include <QHash>
#include <unordered_map>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QLabel>

class MainWindow;
class TaskItemDelegate;

class MainWindow : public QMainWindow
{
public:
    explicit MainWindow(const QString& url,QMainWindow* parent = nullptr);

private slots:
    void handleLogin(const UserModel& user);
    void handleRegistration(const UserModel& user);
    void handleFoldersReceived(const QJsonArray& foldersJson);
    void handleTasksReceived(const QJsonDocument& tasksJson);
    void handleOperationResult(bool success, const QString& msg);
    void createNewFolder();
    void createNewTask();
    void updateTaskStatus(QListWidgetItem* item);
    void onAuthSuccess(const QString& token);
    void onAuthError(const QString& error);

private:
    void setupUI();
    void loadUserData();
    void refreshFolders();
    void refreshTasks(int folder_id);
    void showTaskDetails(QListWidgetItem* item);

    std::unique_ptr<ApiClient> client;
    std::unique_ptr<AuthorizationWidget> authWidget;
    std::unique_ptr<QWidget> todoWidget;

    std::unordered_map<int, FolderModel> folders;

    bool isAuthorized;
    UserModel user;


    //UI elements
    std::unique_ptr<QStandardItemModel> foldersModel;
    std::unique_ptr<QListView> foldersView;
    std::unique_ptr<QListWidget> tasksList;
    std::unique_ptr<QTabWidget> tasksTab;
};

class TaskItemDelegate : public QStyledItemDelegate{
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    void paint(QPainter* painter, const QStyleOptionViewItem &option, const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

#endif // MAINWINDOW_H
