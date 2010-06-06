/**
 * \file ConfigDialog.cpp
 * \author Denis Martinez
 */

#include "ConfigDialog.h"

#include <QFontDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QDialogButtonBox>
#include <QDebug>

#include "env/Settings.h"
#include "env/Toolkit.h"
#include "gui/EditorFactory.h"
#include "gui/LexerArduino.h"
#include "IDEApplication.h"

ConfigWidget::ConfigWidget(QWidget *parent)
    : QxtConfigWidget(parent)
{
    setupUi();
}

void ConfigWidget::initializePage(int index)
{
    Settings *settings = ideApp->settings();

    switch (index)
    {
    case EditorIndex:
        setupFontChooser();
        static const QString sampleText =
            "/* Example code */\n"
            "#include <EEPROM/EEPROM.h>\n\n"
            "int a, b = 3;\n"
            "void loop()\n"
            "{\n"
            "    Serial.println(\"Hello, World!\");\n"
            "}\n";
        mEditor->setText(sampleText);
        break;
    case PathsIndex:
        uiPaths.arduinoPathEdit->setText(settings->arduinoPath());
        uiPaths.sketchbookPathEdit->setText(settings->sketchPath());
        break;
    case BuildIndex:
        uiBuild.verboseBox->setChecked(settings->verboseUpload());
        break;
    }
}

void ConfigWidget::setupUi()
{
    QWidget *page = new QWidget;
    uiEditor.setupUi(page);
    mEditor = EditorFactory::createEditor(QString());
    uiEditor.editorFrame->layout()->addWidget(mEditor);
    addPage(page, QIcon(":/images/32x32/accessories-text-editor.png"), tr("Editor"));

    page = new QWidget;
    uiPaths.setupUi(page);
    addPage(page, QIcon(":/images/32x32/folder.png"), tr("Paths"));

    page = new QWidget;
    uiBuild.setupUi(page);
    addPage(page, QIcon(":/images/32x32/applications-development.png"), tr("Build"));

    connect(uiPaths.arduinoPathEdit, SIGNAL(textChanged(const QString &)), this, SLOT(fieldChange()));
    connect(uiPaths.sketchbookPathEdit, SIGNAL(textChanged(const QString &)), this, SLOT(fieldChange()));
    connect(uiBuild.verboseBox, SIGNAL(stateChanged(int)), this, SLOT(fieldChange()));

    connect(uiEditor.fontChooseButton, SIGNAL(clicked()), this, SLOT(chooseFont()));
    connect(uiPaths.arduinoPathButton, SIGNAL(clicked()), this, SLOT(chooseArduinoPath()));
    connect(uiPaths.sketchbookPathButton, SIGNAL(clicked()), this, SLOT(chooseSketchbookPath()));
}

void ConfigWidget::updateFontLabel(const QFont &f)
{
    static QString format("%0 %1");
    QString text = format.arg(f.family()).arg(f.pointSize());
    uiEditor.fontLabel->setProperty("selectedFont", f);
    uiEditor.fontLabel->setFont(f);
    uiEditor.fontLabel->setText(text);
}

void ConfigWidget::setupFontChooser()
{
    QFont f = mEditor->lexer()->font(LexerArduino::Default);
    updateFontLabel(f);
}

void ConfigWidget::chooseFont()
{
    bool ok;
    QFont initialFont = mEditor->lexer()->font(LexerArduino::Default);
    QFont f = QFontDialog::getFont(&ok, initialFont, this);
    if (ok)
    {
        QsciLexer *lexer = mEditor->lexer();
        lexer->setDefaultFont(f);
        lexer->setFont(f);
        updateFontLabel(f);
        mChangedFields << uiEditor.fontLabel;
    }
}

void ConfigWidget::chooseArduinoPath()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Choose Arduino path"), uiPaths.arduinoPathEdit->text());
    if (! path.isEmpty())
            uiPaths.arduinoPathEdit->setText(path);
}

void ConfigWidget::chooseSketchbookPath()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Choose Sketchbook path"), uiPaths.sketchbookPathEdit->text());
    if (! path.isEmpty())
        uiPaths.sketchbookPathEdit->setText(path);
}

void ConfigWidget::fieldChange()
{
    QWidget *w = qobject_cast<QWidget *>(sender());
    Q_ASSERT(w != NULL);
    mChangedFields << w;
}

bool ConfigWidget::saveConfig()
{
    Settings *settings = ideApp->settings();
    foreach (QWidget *field, mChangedFields)
    {
        if (field == uiPaths.arduinoPathEdit)
        {
            QString path = uiPaths.arduinoPathEdit->text();
            if (! Toolkit::isValidArduinoPath(path))
            {
                QMessageBox::warning(this, tr("Invalid arduino path"), tr("This path does not contain a valid Arduino installation, please choose another."));
                return false;
            }
            settings->setArduinoPath(path);
        }
        else if (field == uiPaths.sketchbookPathEdit)
        {
            settings->setSketchPath(uiPaths.sketchbookPathEdit->text());
            #pragma message("TODO: update the sketchbook browser")
        }
        else if (field == uiBuild.verboseBox)
            settings->setVerboseUpload(uiBuild.verboseBox->isChecked());
    }
    mChangedFields.clear();

    // save all the editor/lexer settings
    settings->saveEditorSettings(mEditor);
    LexerArduino *lexer = dynamic_cast<LexerArduino *>(mEditor->lexer());
    Q_ASSERT(lexer != NULL);
    settings->saveLexerProperties(lexer);
    // update any existing editor
    ideApp->mainWindow()->configureEditors();
    return true;
}

ConfigDialog::ConfigDialog(QWidget *parent)
    : QxtConfigDialog(parent)
{
    setWindowTitle(tr("Configuration"));
    setupUi();
}

void ConfigDialog::setupUi()
{
    setMinimumWidth(500);
    setMinimumHeight(400);

    mConfigWidget = new ConfigWidget;
    QxtConfigWidget *oldConfigWidget = configWidget();
    setConfigWidget(mConfigWidget);
    delete oldConfigWidget;

    dialogButtonBox()->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Apply | QDialogButtonBox::Cancel);
    connect(dialogButtonBox()->button(QDialogButtonBox::Apply), SIGNAL(clicked()), this, SLOT(apply()));
}

bool ConfigDialog::apply()
{
    return mConfigWidget->saveConfig();
}

void ConfigDialog::accept()
{
    if (apply())
    {
        hide();
        setResult(QDialog::Accepted);
    }
}
