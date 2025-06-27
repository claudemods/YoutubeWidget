#include <QApplication>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QWebEngineView>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QMessageBox>
#include <QIcon>
#include <QCloseEvent>

class YouTubeWidget : public QWebEngineView {
    Q_OBJECT
public:
    YouTubeWidget(QWidget *parent = nullptr)
    : QWebEngineView(parent) {
        setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint);
        setAttribute(Qt::WA_DeleteOnClose, false); // Changed to false to prevent deletion on close

        // Configure QWebEngineProfile
        QWebEngineProfile *profile = this->page()->profile();
        profile->setPersistentCookiesPolicy(QWebEngineProfile::ForcePersistentCookies);
        profile->setHttpCacheType(QWebEngineProfile::MemoryHttpCache);
        profile->setHttpCacheMaximumSize(50 * 1024 * 1024);

        // Configure QWebEngineSettings
        QWebEngineSettings *settings = this->settings();
        settings->setAttribute(QWebEngineSettings::LocalStorageEnabled, true);
        settings->setAttribute(QWebEngineSettings::JavascriptEnabled, true);
        settings->setAttribute(QWebEngineSettings::ScrollAnimatorEnabled, false);

        // Load YouTube
        load(QUrl("https://www.youtube.com/"));
        resize(1280, 720); // Set a reasonable size for the window
    }

    void toggleVisibility() {
        if (isVisible()) {
            hide(); // Hide the widget if it's currently visible
        } else {
            showNormal(); // Show the widget in its normal state
            raise();      // Bring the widget to the front
            activateWindow(); // Activate the window
        }
    }

protected:
    void closeEvent(QCloseEvent *event) override {
        // Override close event to hide instead of closing
        hide();
        event->ignore();
    }
};

class TrayIcon : public QSystemTrayIcon {
    Q_OBJECT
public:
    TrayIcon(QObject *parent = nullptr)
    : QSystemTrayIcon(parent) {
        // Set the tray icon (replace ":/images/youtube-icon.svg" with your actual icon path)
        setIcon(QIcon(":/images/youtube-icon.svg"));

        // Create a context menu for the tray icon
        QMenu *menu = new QMenu();
        QAction *toggleAction = menu->addAction("Show/Hide");
        QAction *quitAction = menu->addAction("Quit");

        connect(toggleAction, &QAction::triggered, this, &TrayIcon::toggleWindow);
        connect(quitAction, &QAction::triggered, this, &TrayIcon::quitApplication);
        setContextMenu(menu);

        // Initialize the YouTube widget
        youTubeWidget = new YouTubeWidget();

        // Connect the tray icon's activation signal to toggle visibility
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
        // Properly clean up before quitting
        youTubeWidget->close(); // This will now actually close the widget
        QApplication::quit();
    }

private:
    YouTubeWidget *youTubeWidget;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("YouTube System Tray");
    app.setOrganizationName("KDE Plasma");

    // Check if the system tray is available
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        QMessageBox::critical(nullptr, "Error", "System tray not available.");
        return 1;
    }

    // Create and show the tray icon
    TrayIcon trayIcon;
    trayIcon.show();

    return app.exec();
}

#include "main.moc"
