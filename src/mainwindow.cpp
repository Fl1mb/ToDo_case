#include "mainwindow.h"

MainWindow::MainWindow(const QString& url, QMainWindow *parent): QMainWindow(parent)
{
    client = std::make_unique<ApiClient>(url);

    authWidget = std::make_unique<AuthorizationWidget>();
    this->setCentralWidget(authWidget.get());
    this->setWindowTitle("ToDo App - Authorization");
    resize({600, 400});

    QObject::connect(authWidget.get(), &AuthorizationWidget::LoginReq, this, &MainWindow::handleLogin);
    QObject::connect(authWidget.get(), &AuthorizationWidget::RegReq, this, &MainWindow::handleRegistration);
    QObject::connect(client.get(), &ApiClient::AuthSuccess, this, &MainWindow::onAuthSuccess);
    QObject::connect(client.get(), &ApiClient::AuthError, this, &MainWindow::onAuthError);

}

void MainWindow::handleLogin(const UserModel &user)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    this->client->loginUser(QString::fromStdString(user.username), QString::fromStdString(user.password));
}

void MainWindow::handleRegistration(const UserModel &user)
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
    this->client->registerUser(QString::fromStdString(user.username),
                               QString::fromStdString(user.password),
                               QString::fromStdString(user.email),
                               QString::fromStdString(user.full_name));
}

void MainWindow::handleFoldersReceived(const QJsonArray &foldersJson)
{
    //Clear old folders
    folders.clear();

    //Parsing
    for(const auto& folderVal : foldersJson){
        QJsonObject folderObj = folderVal.toObject();

        FolderModel folder;
        folder.folder_id = folderObj["folder_id"].toInt();
        folder.name = folderObj["name"].toString().toStdString();


        //Converting timestamp
        QJsonObject createdAt = folderObj["created_at"].toObject();
        qint64 seconds = createdAt["seconds"].toVariant().toLongLong();
        folder.created_at = QDateTime::fromSecsSinceEpoch(seconds);

        folders[folder.folder_id] = folder;
    }
    refreshFolders();
}

void MainWindow::handleTasksReceived(const QJsonDocument &tasksJson)
{
    // Получаем объект из JSON
    QJsonObject jsonObject = tasksJson.object();

    // Проверяем наличие поля "tasks" и что это массив
    if (jsonObject.contains("tasks") && jsonObject["tasks"].isArray()) {
        QJsonArray tasksArray = jsonObject["tasks"].toArray();

        // Очищаем старые задачи во всех папках
        for (auto& folderPair : folders) {
            folderPair.second.tasks.clear();
        }

        // Парсим новые задачи
        for (const auto& taskVal : tasksArray) {
            QJsonObject taskObj = taskVal.toObject();

            TaskModel task;
            task.task_id = taskObj["task_id"].toInt();
            task.folder_id = taskObj["folder_id"].toInt();
            task.user_id = taskObj["user_id"].toInt();
            task.title = taskObj["title"].toString().toStdString();
            task.description = taskObj["description"].toString().toStdString();
            task.priority = taskObj["priority"].toInt();
            task.is_completed = taskObj["is_completed"].toBool();

            // Конвертируем timestamp
            QJsonObject dueTime = taskObj["due_time"].toObject();
            qint64 seconds = dueTime["seconds"].toVariant().toLongLong();
            task.Due_time = QDateTime::fromSecsSinceEpoch(seconds);

            // Добавляем задачу в соответствующую папку
            if (folders.count(task.folder_id)) {
                folders[task.folder_id].tasks.push_back(task);
            }
        }
        if (foldersView->currentIndex().isValid()) {
            int currentFolderId = foldersView->currentIndex().data(Qt::UserRole + 1).toInt();
            refreshTasks(currentFolderId);
        }
    } else {
        qDebug() << "Invalid tasks format in JSON response";
    }


}

void MainWindow::handleOperationResult(bool success, const QString &msg)
{
    if (!success) {
        QMessageBox::warning(this, tr("Operation Failed"), msg);
    }
}

void MainWindow::createNewFolder()
{
    bool ok;
    QInputDialog dialog(this);
    dialog.setWindowTitle("Create New Folder");
    dialog.setLabelText("Folder Name:");
    dialog.setFixedSize(400, 200); // Фиксированный размер для аккуратного вида

    // Переопределяем layout для лучшего контроля
    QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(dialog.layout());
    if (layout) {
        layout->setContentsMargins(20, 20, 20, 20);
        layout->setSpacing(15);
    }

    QString folderName = dialog.getText(this, tr("Create New Folder"),
                                        tr("Folder Name:"),
                                        QLineEdit::Normal, "", &ok);

    if (ok && !folderName.isEmpty()) {
        QApplication::setOverrideCursor(Qt::WaitCursor);
        client->createFolder(folderName);

        static QMetaObject::Connection conn;
        conn = QObject::connect(client.get(), &ApiClient::operationCompleted, [&](bool success, const QString& msg) {
            QApplication::restoreOverrideCursor();
            if (success) {
                client->getFolders();
            } else {
                QMessageBox::warning(this, tr("Error"), msg);
            }
            disconnect(conn);
        });
    }
}

void MainWindow::createNewTask()
{

    QModelIndex index = foldersView->currentIndex();
    if (!index.isValid()) {
        QMessageBox::warning(this, tr("No Folder Selected"),
                             tr("Please select a folder first"));
        return;
    }
    int folderId = index.data(Qt::UserRole + 1).toInt();

    QDialog dialog(this);
    dialog.setStyleSheet(R"(
        QDialog {
            background-color: #f5f5f5;
        }
        QLabel {
            color: #333333;
        }
        QLineEdit, QTextEdit, QDateTimeEdit, QSpinBox {
            background-color: white;
            color: #333333;
            border: 1px solid #ddd;
            border-radius: 3px;
            padding: 5px;
        }
        QPushButton {
            background-color: #4a90e2;
            color: white;
            border: none;
            padding: 5px 15px;
            border-radius: 4px;
        }
        QPushButton:hover {
            background-color: #3a7bc8;
        }
    )");

    QFormLayout form(&dialog);

    QLineEdit titleEdit;
    QTextEdit descEdit;
    QDateTimeEdit dueDateEdit;
    dueDateEdit.setDateTime(QDateTime::currentDateTime().addDays(1));
    dueDateEdit.setDisplayFormat("dd.MM.yyyy HH:mm");
    QSpinBox prioritySpin;
    prioritySpin.setRange(1, 3);
    prioritySpin.setValue(2);

    form.addRow(tr("Title:"), &titleEdit);
    form.addRow(tr("Description:"), &descEdit);
    form.addRow(tr("Due Date:"), &dueDateEdit);
    form.addRow(tr("Priority (1-3):"), &prioritySpin);

    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                               Qt::Horizontal, &dialog);
    form.addRow(&buttonBox);

    connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        QApplication::setOverrideCursor(Qt::WaitCursor);
        static QMetaObject::Connection conn;
        conn = connect(client.get(), &ApiClient::operationCompleted, [this, folderId](bool success, const QString& msg) {
            QApplication::restoreOverrideCursor();
            if (success) {
                client->getAllTasks();
            } else {
                QMessageBox::warning(this, tr("Error"), msg);
            }
            disconnect(conn);
        });

        client->createTask(titleEdit.text(), descEdit.toPlainText(),
                           dueDateEdit.dateTime(),
                           prioritySpin.value(), folderId);
    }
}

void MainWindow::updateTaskStatus(QListWidgetItem *item)
{
    int taskId = item->data(Qt::UserRole).toInt();    
    client->toggleTaskCompletion(taskId);

    // Опционально: можно добавить обработку ошибок
    connect(client.get(), &ApiClient::operationCompleted, [this, item](bool success, const QString& msg) {
        if (!success) {
            QMessageBox::warning(this, tr("Error"), msg);
            // Возвращаем предыдущее состояние
            item->setCheckState(item->checkState() == Qt::Checked ? Qt::Unchecked : Qt::Checked);
        }
    });
}


void MainWindow::onAuthSuccess(const QString &token)
{
    this->isAuthorized = true;
    QApplication::restoreOverrideCursor();
    this->setupUI();
    this->loadUserData();

}

void MainWindow::onAuthError(const QString &error)
{
    QApplication::restoreOverrideCursor();

    QMessageBox msgBox(this);
    msgBox.setStyleSheet(
        "QMessageBox {"
        "   background-color: #f8f8f8;"
        "   font-family: 'Segoe UI', Arial, sans-serif;"
        "}"
        "QLabel {"
        "   color: #d32f2f;"
        "   font-size: 14px;"
        "}"
        "QPushButton {"
        "   padding: 5px 15px;"
        "   border-radius: 4px;"
        "   background-color: #d32f2f;"
        "   color: white;"
        "}"
        "QPushButton:hover {"
        "   background-color: #b71c1c;"
        "}"
        );

    msgBox.setWindowTitle(tr("Ошибка авторизации"));
    msgBox.setText(tr("<b>Произошла ошибка при авторизации</b>"));
    msgBox.setInformativeText(error);
    msgBox.setIcon(QMessageBox::Critical);

    QPushButton *retryButton = msgBox.addButton(tr("Повторить"), QMessageBox::ActionRole);
    QPushButton *cancelButton = msgBox.addButton(QMessageBox::Cancel);

    msgBox.exec();
}

void MainWindow::setupUI()
{
    //Основной виджет для ToDo интерфейса
    todoWidget = std::make_unique<QWidget>();
    QHBoxLayout* mainLayout = new QHBoxLayout(todoWidget.get());

    //Левая панель - папки
    QWidget* leftPanel = new QWidget();
    QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);

    foldersModel = std::make_unique<QStandardItemModel>();
    foldersView = std::make_unique<QListView>();
    foldersView->setModel(foldersModel.get());
    foldersView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    QObject::connect(foldersView.get(), &QListView::clicked, this, [this](const QModelIndex& index){
        int folder_id = index.data(Qt::UserRole + 1).toInt();
        refreshTasks(folder_id);
    });

    QPushButton* newFolderBtn = new QPushButton("New Folder");
    QObject::connect(newFolderBtn, &QPushButton::clicked, this, &MainWindow::createNewFolder);

    leftLayout->addWidget(foldersView.get());
    leftLayout->addWidget(newFolderBtn);

    // Add Crash button
    QPushButton* crashBtn = new QPushButton("Crash");
    crashBtn->setStyleSheet("background-color: #ff4444; color: white;");
    QObject::connect(crashBtn, &QPushButton::clicked, this, [&]() {
        this->client->sendRequest("GET", "/crash");
    });
    //Правая панель - задачи
    QWidget* rightPanel = new QWidget();
    QVBoxLayout* rightLayout = new QVBoxLayout(rightPanel);


    tasksList = std::make_unique<QListWidget>(todoWidget.get());
    tasksList->setItemDelegate(new TaskItemDelegate(tasksList.get()));
    tasksList->setStyleSheet(R"(
    QListWidget {
        background-color: #f8f8f8;
        border: 1px solid #ddd;
        border-radius: 5px;
    }
    QListWidget::item {
        border-bottom: 1px solid #e0e0e0;
        padding: 8px;
    }
    QListWidget::item:hover {
        background-color: #eef7ff;
    }
    )");
    QObject::connect(tasksList.get(), &QListWidget::itemChanged, this, &MainWindow::updateTaskStatus);
    QObject::connect(tasksList.get(), &QListWidget::itemPressed, this, [this](QListWidgetItem* item){
        this->showTaskDetails(item);
    });
    QPushButton* newTaskBtn = new QPushButton("Create Task");
    QObject::connect(newTaskBtn, &QPushButton::clicked, this, &MainWindow::createNewTask);


    rightLayout->addWidget(tasksList.get());
    rightLayout->addWidget(newTaskBtn);
    rightLayout->addWidget(crashBtn);

    //Разделитель
    QSplitter* splitter = new QSplitter(Qt::Horizontal);
    splitter->addWidget(leftPanel);
    splitter->addWidget(rightPanel);
    splitter->setSizes({200, 400});

    mainLayout->addWidget(splitter);

    // Обновленный стиль для всего приложения
    this->setStyleSheet(R"(
    QWidget {
        font-family: 'Segoe UI', Arial, sans-serif;
        font-size: 12px;
    }
    QPushButton {
        background-color: #4a90e2;
        color: white;
        border: none;
        padding: 8px 16px;
        border-radius: 4px;
    }
    QPushButton:hover {
        background-color: #3a7bc8;
    }
    QListView, QListWidget {
        background-color: #ffffff;
        border: 1px solid #dddddd;
        border-radius: 5px;
        padding: 5px;
    }
    QSplitter::handle {
        background: #e0e0e0;
        width: 5px;
    }
)");

    // Обновленный стиль для foldersView
    foldersView->setStyleSheet(R"(
    QListView {
        background-color: #ffffff;
        border: 1px solid #dddddd;
        border-radius: 5px;
    }
    QListView::item {
        padding: 8px;
        border-bottom: 1px solid #eeeeee;
        color: #333333;
    }
    QListView::item:hover {
        background-color: #f0f7ff;
        color: #000000;
    }
    QListView::item:selected {
        background-color: #d0e3ff;
        color: #000000;
    }
)");

    // Обновленный стиль для tasksList
    tasksList->setStyleSheet(R"(
    QListWidget {
        background-color: #ffffff;
        border: 1px solid #dddddd;
        border-radius: 5px;
    }
    QListWidget::item {
        border-bottom: 1px solid #eeeeee;
        padding: 8px;
        color: #333333;
    }
    QListWidget::item:hover {
        background-color: #f0f7ff;
    }
)");

    //Заменяем виджет авторизации на ToDo interface
    this->setCentralWidget(todoWidget.get());
    this->setWindowTitle("ToDo App - ");

}

void MainWindow::loadUserData()
{
    // Подключаем обработчики данных
    connect(client.get(), &ApiClient::foldersReceived,
            this, &MainWindow::handleFoldersReceived);
    connect(client.get(), &ApiClient::taskReceived,
            this, &MainWindow::handleTasksReceived);
    connect(client.get(), &ApiClient::operationCompleted,
            this, &MainWindow::handleOperationResult);

    // Загружаем начальные данные
    client->getFolders();
    client->getAllTasks();
}

void MainWindow::refreshFolders()
{
    foldersModel->clear();

    // Добавляем папки в модель
    for (const auto& folderPair : folders) {
        const FolderModel& folder = folderPair.second;

        QStandardItem* item = new QStandardItem(QString::fromStdString(folder.name));
        item->setData(folder.folder_id, Qt::UserRole + 1); // Сохраняем ID папки
        foldersModel->appendRow(item);
    }
}

void MainWindow::refreshTasks(int folder_id)
{
    tasksList->blockSignals(true);
    tasksList->clear();

    if (folders.count(folder_id) == 0) {
        tasksList->blockSignals(false); // Разблокируем перед выходом
        return;
    }

    if (folders.count(folder_id) == 0) {
        return;
    }

    const FolderModel& folder = folders[folder_id];
    // Блокируем сигналы перед обновлением


    qDebug() << folder.tasks.size();
    // Добавляем задачи в список
    for (const TaskModel& task : folder.tasks) {
        QListWidgetItem* taskItem = new QListWidgetItem(tasksList.get());
        taskItem->setText(QString::fromStdString(task.title));
        taskItem->setData(Qt::UserRole, task.task_id);
        taskItem->setData(Qt::UserRole + 1, task.priority);
        taskItem->setData(Qt::CheckStateRole, task.is_completed);
        taskItem->setFlags(taskItem->flags() | Qt::ItemIsUserCheckable);
        taskItem->setCheckState(task.is_completed ? Qt::Checked : Qt::Unchecked);

        // Устанавливаем цвет в зависимости от приоритета
        if (task.priority == 1) {
            taskItem->setBackground(QColor(255, 230, 230));
        } else if (task.priority == 2) {
            taskItem->setBackground(QColor(255, 255, 230));
        }

        // Добавляем подсказку
        QString tooltip = QString("Description: %1\nDue: %2")
                              .arg(QString::fromStdString(task.description))
                              .arg(task.Due_time.toString("dd.MM.yyyy HH:mm"));
        taskItem->setToolTip(tooltip);
    }

    tasksList->blockSignals(false);
}

void MainWindow::showTaskDetails(QListWidgetItem *item)
{
    if (!item) return;

    int taskId = item->data(Qt::UserRole).toInt();
    int folderId = foldersView->currentIndex().data(Qt::UserRole + 1).toInt();
    bool isComleted = item->data(Qt::CheckStateRole).toBool();

    if (folders.count(folderId)) {
        const FolderModel& folder = folders[folderId];
        auto it = std::find_if(folder.tasks.begin(), folder.tasks.end(),
                               [taskId](const TaskModel& t) { return t.task_id == taskId; });

        if (it != folder.tasks.end()) {
            const TaskModel& task = *it;

            QDialog dialog(this);
            dialog.setWindowTitle("Task Details");
            dialog.resize(500, 350);
            dialog.setStyleSheet(R"(
                QDialog {
                    background-color: #f5f5f5;
                }
                QLabel {
                    color: #333333;
                }
                QTextEdit {
                    background-color: white;
                    border: 1px solid #dddddd;
                    border-radius: 3px;
                    color: #333333;
                }
                QPushButton {
                    background-color: #4a90e2;
                    color: white;
                    min-width: 80px;
                    padding: 5px;
                }
            )");

            QVBoxLayout *layout = new QVBoxLayout(&dialog);

            // Заголовок
            QLabel *titleLabel = new QLabel(QString("<h2 style='color:#2a5885'>%1</h2>")
                                                .arg(QString::fromStdString(task.title)));
            layout->addWidget(titleLabel);

            // Описание
            QLabel *descLabel = new QLabel("Description:");
            descLabel->setStyleSheet("font-weight: bold;");
            layout->addWidget(descLabel);

            QTextEdit *descEdit = new QTextEdit(QString::fromStdString(task.description));
            descEdit->setReadOnly(true);
            layout->addWidget(descEdit);

            // Детали
            QFrame *detailsFrame = new QFrame();
            detailsFrame->setFrameShape(QFrame::StyledPanel);
            detailsFrame->setStyleSheet("background-color: white; border-radius: 5px;");

            QFormLayout *detailsLayout = new QFormLayout(detailsFrame);
            detailsLayout->setContentsMargins(15, 15, 15, 15);
            detailsLayout->setSpacing(10);

            QLabel *dueTitle = new QLabel("Due Date:");
            dueTitle->setStyleSheet("font-weight: bold;");
            QLabel *dueLabel = new QLabel(task.Due_time.toString("dd.MM.yyyy HH:mm"));

            QLabel *priorityTitle = new QLabel("Priority:");
            priorityTitle->setStyleSheet("font-weight: bold;");
            QLabel *priorityLabel = new QLabel();
            priorityLabel->setText(QString("<span style='color:%1'>●</span> %2")
                                       .arg(task.priority == 1 ? "#ff6464" :
                                                (task.priority == 2 ? "#ffb464" : "#64c864"))
                                       .arg(task.priority));

            QLabel *statusTitle = new QLabel("Status:");
            statusTitle->setStyleSheet("font-weight: bold;");
            QLabel *statusLabel = new QLabel(task.is_completed ?
                                                 "<span style='color:#64c864'>Completed</span>" :
                                                 "<span style='color:#ffb464'>Pending</span>");

            detailsLayout->addRow(dueTitle, dueLabel);
            detailsLayout->addRow(priorityTitle, priorityLabel);
            detailsLayout->addRow(statusTitle, statusLabel);

            layout->addWidget(detailsFrame);

            // Кнопки
            QPushButton* ok_but = new QPushButton("ok");
            QPushButton* done_but = new QPushButton(isComleted? "return" : "done");
            QObject::connect(ok_but, &QPushButton::clicked, &dialog, &QDialog::accept);
            QObject::connect(done_but, &QPushButton::clicked, [this, taskId, &dialog](){
                client->toggleTaskCompletion(taskId);
                client->getAllTasks();
                dialog.accept();
            });
            QHBoxLayout* buttns = new QHBoxLayout;
            buttns->addWidget(done_but);
            buttns->addWidget(ok_but);
            layout->addLayout(buttns);

            dialog.exec();
        }
    }
}

void TaskItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    painter->save();

    // Настройка фона
    QColor bgColor = (opt.state & QStyle::State_Selected) ? QColor(200, 230, 255) : QColor(255, 255, 255);
    painter->fillRect(opt.rect, bgColor);

    // Настройка текста
    QFont font = opt.font;
    font.setPointSize(10);
    painter->setFont(font);

    // Проверяем статус выполнения задачи
    bool isCompleted = index.data(Qt::CheckStateRole).toBool();
    // Устанавливаем цвет текста - серый для выполненных задач
    painter->setPen(isCompleted ? QColor(150, 150, 150) : QColor(51, 51, 51));

    QRect textRect = opt.rect.adjusted(10, 5, -40, -5);
    painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, opt.text);

    // Если задача выполнена, рисуем зачеркивающую линию
    if (isCompleted) {
        painter->setPen(QPen(QColor(150, 150, 150), 1));
        int y = textRect.center().y();
        painter->drawLine(textRect.left(), y, textRect.right(), y);
    }

    // Рисуем приоритет
    int priority = index.data(Qt::UserRole + 1).toInt();
    QColor priorityColor;
    if (priority == 1) {
        priorityColor = QColor(255, 100, 100); // Красный для высокого приоритета
    } else if (priority == 2) {
        priorityColor = QColor(255, 200, 100); // Оранжевый для среднего
    } else {
        priorityColor = QColor(100, 200, 100); // Зеленый для низкого
    }

    painter->setBrush(priorityColor);
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(opt.rect.right() - 30, opt.rect.center().y() - 5, 10, 10);

    painter->restore();
}

QSize TaskItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QSize(100, 40);
}
