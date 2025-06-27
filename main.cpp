#include <QApplication>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QWebEngineView>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QMessageBox>
#include <QIcon>
#include <QCloseEvent>
#include <QWebEnginePage>
#include <QWebEngineFullScreenRequest>
#include <QStandardPaths>

class YouTubeWidget : public QWebEngineView {
    Q_OBJECT
public:
    YouTubeWidget(QWidget *parent = nullptr)
    : QWebEngineView(parent) {
        setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint);
        setAttribute(Qt::WA_DeleteOnClose, false);

        // Create dedicated profile with persistent storage
        QWebEngineProfile* profile = new QWebEngineProfile("YouTubePersistentProfile", this);
        QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        
        // Configure persistent session storage (PROVEN WORKING METHOD)
        profile->setPersistentCookiesPolicy(QWebEngineProfile::ForcePersistentCookies);
        profile->setCachePath(dataPath + "/youtube_cache");
        profile->setPersistentStoragePath(dataPath + "/youtube_storage");
        profile->setHttpCacheType(QWebEngineProfile::DiskHttpCache);

        // Create page with our configured profile
        QWebEnginePage* page = new QWebEnginePage(profile, this);
        setPage(page);
        
        // Configure QWebEngineSettings
        QWebEngineSettings *settings = page->settings();
        settings->setAttribute(QWebEngineSettings::LocalStorageEnabled, true);
        settings->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
        settings->setAttribute(QWebEngineSettings::ScrollAnimatorEnabled, false);
        settings->setAttribute(QWebEngineSettings::FullScreenSupportEnabled, true);

        // Fullscreen handling (working version from before)
        connect(page, &QWebEnginePage::fullScreenRequested, [this](QWebEngineFullScreenRequest request) {
            if (request.toggleOn()) {
                this->showFullScreen();
            } else {
                this->showNormal();
            }
            request.accept();
        });

        // Load YouTube
        load(QUrl("https://www.youtube.com/"));
        resize(1280, 720);
    }

    void toggleVisibility() {
        if (isVisible()) {
            hide();
        } else {
            showNormal();
            raise();
            activateWindow();
        }
    }

protected:
    void closeEvent(QCloseEvent *event) override {
        hide();
        event->ignore();
    }
};

class TrayIcon : public QSystemTrayIcon {
    Q_OBJECT
public:
    TrayIcon(QObject *parent = nullptr)
    : QSystemTrayIcon(parent) {
        setIcon(QIcon(":/images/youtube-icon.svg"));

        QMenu *menu = new QMenu();
        QAction *toggleAction = menu->addAction("Show/Hide");
        QAction *quitAction = menu->addAction("Quit");

        connect(toggleAction, &QAction::triggered, this, &TrayIcon::toggleWindow);
        connect(quitAction, &QAction::triggered, this, &TrayIcon::quitApplication);
        setContextMenu(menu);

        youTubeWidget = new YouTubeWidget();

        connect(this, &QSystemTrayIcon::activated, this, &TrayIcon::onTrayIconActivated);
    }

    ~TrayIcon() {
        delete youTubeWidget;
    }

private slots:
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::Trigger) {
            toggleWindow();
        }
    }

    void toggleWindow() {
        youTubeWidget->toggleVisibility();
    }

    void quitApplication() {
        youTubeWidget->close();
        QApplication::quit();
    }

private:
    YouTubeWidget *youTubeWidget;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("claudemods YouTube Widget v1.0");
    app.setOrganizationName("KDE Plasma");

    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        QMessageBox::critical(nullptr, "Error", "System tray not available.");
        return 1;
    }

    TrayIcon trayIcon;
    trayIcon.show();

    return app.exec();
}

#include "main.moc"
