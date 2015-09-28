#include "owncloudservice.h"
#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include <QSettings>
#include <QFileDialog>
#include <QDebug>

SettingsDialog::SettingsDialog(SimpleCrypt *crypto, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    ui->connectionTestLabel->hide();

    // increase font size for OSX
#ifdef Q_OS_MAC
    QString styleSheet = ui->installInfotextBrowser->styleSheet();
    styleSheet = styleSheet.replace( "font-size: 9pt;", "font-size: 13pt;" );
    ui->installInfotextBrowser->setStyleSheet( styleSheet );

    QString html = ui->installInfotextBrowser->toHtml();
    html = html.replace( "font-size:9pt", "font-size:13pt;" );
    ui->installInfotextBrowser->setHtml(  html );
#endif

    // add information text for Windows users to install OpenSSL
#ifdef Q_OS_WIN32
    QString html = ui->installInfotextBrowser->toHtml();
    html += "<p>If your ownCloud server uses SSL (https) you also need to install <a href='https://www.openssl.org'>OpenSSL</a> into your system directory.</p>";
    html += "<p>You can find binary packages of OpenSSL <a href='http://slproweb.com/products/Win32OpenSSL.html'>here</a>.</p>";
    ui->installInfotextBrowser->setHtml( html );
#endif

    this->crypto = crypto;
    readSettings();
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

/**
 * Starts a connection test
 *
 * @brief SettingsDialog::on_connectButton_clicked
 */
void SettingsDialog::on_connectButton_clicked()
{
    storeSettings();
    ui->connectionTestLabel->hide();
    OwnCloudService *ownCloud = new OwnCloudService( crypto, this );
    ownCloud->settingsConnectionTest( this );
}

void SettingsDialog::storeSettings()
{
    QSettings settings;
    QString url = QString( ui->serverUrlEdit->text() );

    // remove trailing "/" of the server url
    if ( url.endsWith( "/" ) )
    {
        url.chop( 1 );
        ui->serverUrlEdit->setText( url );
    }

    settings.setValue( "ownCloud/serverUrl", url );
    settings.setValue( "ownCloud/userName", ui->userNameEdit->text() );
    settings.setValue( "ownCloud/password", crypto->encryptToString( ui->passwordEdit->text() ) );
    settings.setValue( "ownCloud/localOwnCloudPath", ui->localOwnCloudPathEdit->text() );
}

void SettingsDialog::readSettings()
{
    QSettings settings;
    ui->serverUrlEdit->setText( settings.value( "ownCloud/serverUrl" ).toString() );
    ui->userNameEdit->setText( settings.value( "ownCloud/userName" ).toString() );
    ui->passwordEdit->setText( crypto->decryptToString( settings.value( "ownCloud/password" ).toString() ) );
    ui->localOwnCloudPathEdit->setText( settings.value( "ownCloud/localOwnCloudPath" ).toString() );
}

void SettingsDialog::on_buttonBox_clicked(QAbstractButton *button)
{
    if( button == ui->buttonBox->button( QDialogButtonBox::Ok ) )
    {
        storeSettings();
    }
}

/**
 * Callback function from OwnCloudService to output a success or error message
 *
 * @brief SettingsDialog::connectTestCallback
 * @param appIsValid
 * @param appVersion
 * @param serverVersion
 */
void SettingsDialog::connectTestCallback( bool appIsValid, QString appVersion, QString serverVersion, QString connectionErrorMessage )
{
    if ( appIsValid )
    {
        ui->connectionTestLabel->setStyleSheet( "color: green;" );
        ui->connectionTestLabel->setText( "The connection was made successfully!\nServer version: " + serverVersion + "\nQOwnNotesAPI version: " + appVersion );
    }
    else
    {
        // hide password
        connectionErrorMessage.replace( ui->passwordEdit->text(), "***" );

        ui->connectionTestLabel->setStyleSheet( "color: red;" );
        ui->connectionTestLabel->setText( "There was an error connecting to the ownCloud Server!\nYou also need to have the QOwnNotesAPI app installed and enabled!\n\nConnection error message: " + connectionErrorMessage );
    }

    ui->connectionTestLabel->adjustSize();
    ui->connectionTestLabel->show();
}

/**
 * select the local ownCloud directory
 */
void SettingsDialog::on_localOwnCloudDirectoryButton_clicked()
{
    QString path = ui->localOwnCloudPathEdit->text();

    if ( path == "" )
    {
        path = QDir::homePath() + QDir::separator() + "ownCloud";
    }

    QString dir = QFileDialog::getExistingDirectory( this, tr( "Select ownCloud base directory" ),
                                                 path,
                                                 QFileDialog::ShowDirsOnly );

    if ( dir != "" )
    {
        ui->localOwnCloudPathEdit->setText( dir );
    }
}
