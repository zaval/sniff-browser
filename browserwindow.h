#ifndef BROWSERWINDOW_H
#define BROWSERWINDOW_H

#include <QMainWindow>
#include <QWebPage>
#include <QWebFrame>
#include <QWebHistory>
#include <QCompleter>
#include <QStringList>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFileDialog>
#include "interceptormanager.h"
#include "networkreplyproxy.h"

struct RequestDetails {
    uint ts;
    QString url;
    struct Request {
        QByteArray headers;
        QByteArray body;
        QString method;
    } request;
    struct Response {
        QByteArray headers;
        QByteArray body;
    } response;
};

namespace Ui {
class BrowserWindow;
}

class BrowserWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit BrowserWindow(QWidget *parent = 0);
    ~BrowserWindow();

private slots:
    void on_toolButton_go_clicked();

    void on_lineEdit_url_returnPressed();

    void on_webView_main_loadFinished(bool arg1);

    void on_toolButton_undo_clicked();

    void on_toolButton_redo_clicked();

    void process_reply(NetworkReplyProxy *proxy);
    void handle_reply(const QUrl &url, const QByteArray &data, const QByteArray &headers);
    void handle_request(const QNetworkRequest &request, const QByteArray &data);

    void on_actionSave_triggered();

private:
    Ui::BrowserWindow *ui;
    InterceptorAccessManager *nam;
    QCompleter *urlCompleter;
    QStringList urlHistory;
    QList<RequestDetails> requestHistory;
    QMap<QString, RequestDetails> waitingRequestHistory;

};

#endif // BROWSERWINDOW_H
