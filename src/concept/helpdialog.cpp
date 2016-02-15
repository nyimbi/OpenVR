#include "helpdialog.h"
#include "ui_helpdialog.h"
#include "QFile"
#include "QTextStream"

HelpDialog::HelpDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::HelpDialog)
{
    ui->setupUi(this);

    QString aboutHtml= loadFile("docs/about.html");

    ui->htmlAbout->setHtml(aboutHtml);

    QString helpHtml= loadFile("docs/help.html");

    ui->htmlUsing->setHtml(helpHtml);

    QString licenceHtml= loadFile("docs/licence.html");

    ui->htmlLicence->setHtml(licenceHtml);


}

HelpDialog::~HelpDialog()
{
    delete ui;
}

void HelpDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

QString HelpDialog::loadFile(QString fileName){

    QFile fileReader(fileName);
    if(!fileReader.exists()){
        return "";
    }

    if (!fileReader.open (QFile::ReadOnly))
         return "";
    QTextStream stream ( &fileReader );
    QString fileContents ="";
    QString line;
    do {
         line = stream.readLine();
        fileContents+=line.toUtf8();
    }while(!line.isNull());

    return fileContents.toUtf8();
}
