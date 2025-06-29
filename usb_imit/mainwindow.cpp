#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle("USB проверка прошивки");

    //--- настраиваем юи таблички
    setupDeviceTableProperties();

    //--- пописываем устройства
    initializeDevices();

    //--- берем текущую версию
    updateLatestVersion();

    //--- заполняем + декорируем
    addToDevicesTable();
}

MainWindow::~MainWindow()
{
    delete contextMenu;
    delete ui;
}

void MainWindow::initializeDevices()
{
    devices = {
            USBDevice("Устройство 1", "12345", "1.0.2"),
            USBDevice("Устройство 2", "23456", "1.5.3"),
            USBDevice("Устройство 3", "34567", "2.1.0"),
            USBDevice("Устройство 4", "45678", "1.2.0"),
            USBDevice("Устройство 5", "56789", "1.7.4")
    };
}

void MainWindow::setupDeviceTableProperties()
{
    ui->devicesTable->setColumnCount(4);
    ui->devicesTable->setHorizontalHeaderLabels({"#", "Устройство", "Серийный номер", "Версия прошивки"});
    ui->devicesTable->horizontalHeader()->setStretchLastSection(true);
    ui->devicesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    ui->devicesTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->devicesTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    //--- настраиваем ширину столбцов
    for (int i = 0; i < ui->devicesTable->columnCount(); ++i) {
            ui->devicesTable->resizeColumnToContents(i);
            int contentWidth = ui->devicesTable->columnWidth(i);
            ui->devicesTable->setColumnWidth(i, contentWidth + 20);
    }

    //--- Включаем контекстное меню для таблицы
    ui->devicesTable->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->devicesTable, &QTableWidget::customContextMenuRequested,this, &MainWindow::custom_contextMenu_requested);

    //--- Создаем контекстное меню
    contextMenu = new QMenu(this);
    action_update = new QAction("Обновить", this);
    contextMenu->addAction(action_update);
    connect(action_update, &QAction::triggered, this, &MainWindow::action_update_triggered);
}

QString MainWindow::getLatestFirmwareFromServer()
{
    QFile file("firmware_version.txt");
    if (!file.exists()) {
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << "2.1.0\n";
            file.close();
        }
    }

    if (file.open(QIODevice::ReadWrite | QIODevice::Text)) {
        QTextStream in(&file);
        QString version = in.readLine();
        file.close();
        //--- проверяем на корректность записанного в файле
        //--- нельзя буквы, пробелы и другие спец символы
        QRegularExpression regex("^\\d+(?:\\.\\d+)*$");
        if (regex.match(version).hasMatch())
        {
            return version;
        }
    }
    return "2.1.0";
}

bool MainWindow::isVersionOutdated(const QString &current, const QString &latest)
{
    if (current == latest) return false;

        QStringList currentParts = current.split('.');
        QStringList latestParts = latest.split('.');

        for (int i = 0; i < qMin(currentParts.size(), latestParts.size()); ++i) {
            unsigned short currentPart = currentParts[i].toShort();
            unsigned short latestPart = latestParts[i].toShort();

            if (latestPart > currentPart) return true;
            if (latestPart < currentPart) return false;
        }

        return latestParts.size() > currentParts.size();//--- случай если все совпало,но мб еще подверсии и тд
}

void MainWindow::updateLatestVersion()
{
    latestFirmware = getLatestFirmwareFromServer();
    ui->label_latestFirmware->setText(latestFirmware);
}

void MainWindow::addToDevicesTable()
{
    ui->devicesTable->setRowCount(devices.size());
    for (int i = 0; i < devices.size(); ++i) {
        const USBDevice& device = devices[i];

        ui->devicesTable->setItem(i, 0, new QTableWidgetItem(QString::number(i + 1)));
        ui->devicesTable->setItem(i, 1, new QTableWidgetItem(device.getName()));
        ui->devicesTable->setItem(i, 2, new QTableWidgetItem(device.getSerialNumber()));

        QFont font;
        font.setBold(true);
        QTableWidgetItem* firmwareItem = new QTableWidgetItem(device.getFirmware());
        if (isVersionOutdated(device.getFirmware(), latestFirmware))
        {
            firmwareItem->setForeground(Qt::red);
            firmwareItem->setFont(font);
            firmwareItem->setToolTip("Доступно обновление");
        }
        else
        {
            firmwareItem->setForeground(Qt::darkGreen);
            firmwareItem->setFont(font);
            firmwareItem->setToolTip("Актуальная версия");
        }
        ui->devicesTable->setItem(i, 3, firmwareItem);
    }
}

void MainWindow::updateDevicesTable()
{
    ui->devicesTable->clearContents();
    addToDevicesTable();
}

void MainWindow::showUpdateDialog(int index)
{
    USBDevice &device = devices[index];

    //--- версия актуальна
    if (!isVersionOutdated(device.getFirmware(), latestFirmware)) {
        QMessageBox::information(this,
                                 "Актувльная версия",
                                 QString("На %1 установлена актуальная версия прошивки")
                                    .arg(device.getName()));
        return;
    }

    //--- окно уточнение
    QMessageBox msgBox (QMessageBox::Question,
                        "Доступно обновление прошивки",
                        QString("%1\nТекущая версия: %2\nДоступное обновление: %3\n\nОбновить?")
                            .arg(device.getName())
                            .arg(device.getFirmware())
                            .arg(latestFirmware),
                        QMessageBox::Yes | QMessageBox::No,
                        this);
    //--- корректировка текста кнопок
    msgBox.setButtonText(QMessageBox::Yes, "Да");
    msgBox.setButtonText(QMessageBox::No, "Нет");
    msgBox.setDefaultButton(QMessageBox::Yes);

    //--- обработка выбора пользователя
    if (msgBox.exec() == QMessageBox::Yes)
    {
        device.setFirmware(latestFirmware);
        QMessageBox::information(this,
                                 "Обновление завершено",
                                 QString("%1 было обновлено до версии прошивки %2.")
                                    .arg(device.getName())
                                    .arg(latestFirmware));
    }
}

void MainWindow::action_update_triggered()
{
    int selectedRow = ui->devicesTable->currentRow();
    if (selectedRow >= 0 && selectedRow < devices.size())
    {
        showUpdateDialog(selectedRow);
        updateDevicesTable();
    }
    else
    {
        QMessageBox::warning(this, "Ничего не выбрано", "Необходимо выбрать устройство.");
    }
}

void MainWindow::custom_contextMenu_requested(const QPoint &position)
{
    QTableWidgetItem *item = ui->devicesTable->itemAt(position);
    if (item)
    {   //--- показываем выпадающее меню
        contextMenu->exec(ui->devicesTable->viewport()->mapToGlobal(position));
    }
}

void MainWindow::on_pushButton_updateSelected_clicked()
{
    action_update_triggered();
}

void MainWindow::on_pushButton_updateAll_clicked()
{
    for (int i = 0; i < devices.size(); ++i)
    {
        USBDevice& device = devices[i];
        if (isVersionOutdated(device.getFirmware(),latestFirmware))//--- обновляем только те, у которых более старая версия (равная или новее - пропускаем)
        {
            device.setFirmware(latestFirmware);
        }
    }
    QMessageBox::information(this,
                             "Обновление завершено",
                             QString("Устройства были обновлены до версии прошивки %1.")
                                .arg(latestFirmware));
    updateDevicesTable();
}

void MainWindow::on_pushButton_checkLatest_clicked()
{
    updateLatestVersion();
    updateDevicesTable();
}

