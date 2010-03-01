/**
 * \file IDEApplication.h
 * \author Denis Martinez
 * \date 2010-02-28
 */

#ifndef IDEAPPLICATION_H
#define IDEAPPLICATION_H

#include <QApplication>

#include "gui/MainWindow.h"
#include "gui/FirstTimeWizard.h"
#include "env/Settings.h"

class IDEApplication : public QApplication
{
public:
    IDEApplication(int argc, char **argv);
    void initializeTemplates();
    void initializeGui();
    void initializeSettings();
    const QString &dataPath() { return mDataPath; }

private:
    QString mDataPath;
    MainWindow *mainWindow;
};

#define ideApp (static_cast<IDEApplication *>(qApp))

#endif // IDEAPPLICATION_H