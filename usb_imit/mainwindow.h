#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QTableWidgetItem>
#include <QPoint>

#include "usbdevice.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QVector<USBDevice> devices;
    QString latestFirmware;
    QMenu *contextMenu = nullptr;
    QAction *action_update = nullptr;


    void initializeDevices();
    void setupDeviceTableProperties();
    QString getLatestFirmwareFromServer();
    bool isVersionOutdated(const QString& current, const QString& latest);
    void updateLatestVersion();
    void addToDevicesTable();
    void updateDevicesTable();;
    void showUpdateDialog(int index);

private slots:
    void action_update_triggered();
    void custom_contextMenu_requested(const QPoint &position);
    void on_pushButton_updateSelected_clicked();
    void on_pushButton_updateAll_clicked();
    void on_pushButton_checkLatest_clicked();
};
#endif // MAINWINDOW_H
