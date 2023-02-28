#include <QApplication>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include <QCheckBox>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QProcess>

class PackageInstaller : public QDialog {
  Q_OBJECT
public:
  PackageInstaller(QWidget *parent = 0);

private slots:
  void searchPackages();
  void installPackages();
  void upgradePackages();
  void uninstallPackages();
  void showPackageDetails();
  void addRepository();
  void manageSecurityUpdates();
  void updatePackageList(QListWidgetItem* item);

private:
  QLineEdit *searchBox;
  QListWidget *packageList;
  QListWidget *selectedList;
  QPushButton *installButton;
  QPushButton *upgradeButton;
  QPushButton *uninstallButton;
  QPushButton *detailsButton;
  QPushButton *addRepositoryButton;
  QPushButton *securityUpdatesButton;

  QStringList packages;
  QStringList selectedPackages;

  void updatePackageInfo(QString package);
  void updateButtonState();
};

PackageInstaller::PackageInstaller(QWidget *parent)
  : QDialog(parent)
{
  setWindowTitle(tr("Package Installer"));

  QLabel *searchLabel = new QLabel(tr("Search for package:"));
  searchBox = new QLineEdit;
  QPushButton *searchButton = new QPushButton(tr("Search"));
  connect(searchButton, &QPushButton::clicked, this, &PackageInstaller::searchPackages);

  QHBoxLayout *searchLayout = new QHBoxLayout;
  searchLayout->addWidget(searchLabel);
  searchLayout->addWidget(searchBox);
  searchLayout->addWidget(searchButton);

  packageList = new QListWidget;
  connect(packageList, &QListWidget::itemClicked, this, &PackageInstaller::updatePackageList);

  QGroupBox *packageBox = new QGroupBox(tr("Packages"));
  QVBoxLayout *packageLayout = new QVBoxLayout;
  packageLayout->addWidget(packageList);
  packageBox->setLayout(packageLayout);

  selectedList = new QListWidget;

  QGroupBox *selectedBox = new QGroupBox(tr("Selected packages"));
  QVBoxLayout *selectedLayout = new QVBoxLayout;
  selectedLayout->addWidget(selectedList);
  selectedBox->setLayout(selectedLayout);

  QVBoxLayout *listLayout = new QVBoxLayout;
  listLayout->addWidget(packageBox);
  listLayout->addWidget(selectedBox);

  QCheckBox *dependenciesCheckbox = new QCheckBox(tr("Install dependencies"));
  dependenciesCheckbox->setChecked(true);

  installButton = new QPushButton(tr("Install"));
  installButton->setEnabled(false);
  connect(installButton, &QPushButton::clicked, this, &PackageInstaller::installPackages);

  upgradeButton = new QPushButton(tr("Upgrade"));
  upgradeButton->setEnabled(false);
  connect(upgradeButton, &QPushButton::clicked, this, &PackageInstaller::upgradePackages);

  uninstallButton = new QPushButton(tr("Uninstall"));
  uninstallButton->setEnabled(false);
  connect(uninstallButton, &QPushButton::clicked, this, &PackageInstaller::uninstallPackages);

detailsButton = new QPushButton(tr("Details"));
detailsButton->setEnabled(false);
connect(detailsButton, &QPushButton::clicked, this, &PackageInstaller::showPackageDetails);

QGroupBox *actionBox = new QGroupBox(tr("Actions"));
QVBoxLayout *actionLayout = new QVBoxLayout;
actionLayout->addWidget(dependenciesCheckbox);
actionLayout->addWidget(installButton);
actionLayout->addWidget(upgradeButton);
actionLayout->addWidget(uninstallButton);
actionLayout->addWidget(detailsButton);
actionBox->setLayout(actionLayout);

addRepositoryButton = new QPushButton(tr("Add repository"));
connect(addRepositoryButton, &QPushButton::clicked, this, &PackageInstaller::addRepository);

securityUpdatesButton = new QPushButton(tr("Manage security updates"));
connect(securityUpdatesButton, &QPushButton::clicked, this, &PackageInstaller::manageSecurityUpdates);

QVBoxLayout *mainLayout = new QVBoxLayout;
mainLayout->addLayout(searchLayout);
mainLayout->addLayout(listLayout);
mainLayout->addWidget(actionBox);
mainLayout->addWidget(addRepositoryButton);
mainLayout->addWidget(securityUpdatesButton);

setLayout(mainLayout);
}

void PackageInstaller::searchPackages()
{
QString query = searchBox->text();
QProcess process;
process.start("apt-cache search " + query);
process.waitForFinished();
QString output = process.readAllStandardOutput();
packages = output.split("\n");
packageList->clear();
foreach (QString package, packages) {
packageList->addItem(package.split(" ").first());
}
}

void PackageInstaller::installPackages()
{
QString command = "sudo apt-get install -y ";
if (dependenciesCheckbox->isChecked()) {
command += "--install-recommends ";
}
command += selectedPackages.join(" ");
QProcess process;
process.start(command);
process.waitForFinished();
if (process.exitCode() == 0) {
QMessageBox::information(this, tr("Installation complete"), tr("Packages installed successfully."));
} else {
QMessageBox::critical(this, tr("Installation error"), process.readAllStandardError());
}
selectedPackages.clear();
selectedList->clear();
updateButtonState();
}

void PackageInstaller::upgradePackages()
{
QProcess process;
process.start("sudo apt-get upgrade -y");
process.waitForFinished();
if (process.exitCode() == 0) {
QMessageBox::information(this, tr("Upgrade complete"), tr("Packages upgraded successfully."));
} else {
QMessageBox::critical(this, tr("Upgrade error"), process.readAllStandardError());
}
updateButtonState();
}

void PackageInstaller::uninstallPackages()
{
QString command = "sudo apt-get remove -y " + selectedPackages.join(" ");
QProcess process;
process.start(command);
process.waitForFinished();
if (process.exitCode() == 0) {
QMessageBox::information(this, tr("Uninstallation complete"), tr("Packages uninstalled successfully."));
} else {
QMessageBox::critical(this, tr("Uninstallation error"), process.readAllStandardError());
}
selectedPackages.clear();
selectedList->clear();
updateButtonState();
}

void PackageInstaller::showPackageDetails()
{
QString package = selectedPackages.first();
QProcess process;
process.start("apt-cache show " + package);
process.waitForFinished();
QString output = process.readAllStandardOutput();
QMessageBox::information(this, package, output);
}

void PackageInstaller::addRepository()
{
// TODO: implement addRepository function
}

void PackageInstaller::manageSecurityUpdates()
{
// TODO: implement manageSecurityUpdates function
}

void PackageInstaller::updatePackageList(QListWidgetItem* item)
{
QString package = item->text();
updatePackageInfo(package);
if (!selectedPackages.contains(package)) {
selectedPackages.append(package);
selectedList->addItem(package);
updateButtonState();
}
}

void PackageInstaller::updatePackageInfo(QString package)
{
QProcess process;
process.start("apt-cache show " + package);
process.waitForFinished();
QString output = process.readAllStandardOutput();
QStringList info = output.split("\n");
QString version, size, description;
foreach (QString line, info) {
if (line.startsWith("Version: ")) {
version = line.mid(9);
} else if (line.startsWith("Installed-Size: ")) {
size = line.mid(17);
} else if (line.startsWith("Description: ")) {
description = line.mid(13);
}
}
QString packageInfo = package + " " + version + " (" + size + ")\n" + description;
detailsButton->setText(packageInfo);
detailsButton->setEnabled(true);
}

void PackageInstaller::updateButtonState()
{
if (selectedPackages.isEmpty()) {
installButton->setEnabled(false);
upgradeButton->setEnabled(false);
uninstallButton->setEnabled(false);
} else {
installButton->setEnabled(true);
upgradeButton->setEnabled(true);
uninstallButton->setEnabled(true);
}
}

int main(int argc, char *argv[])
{
QApplication app(argc, argv);
PackageInstaller installer;
installer.show();
return app.exec();
}


