#include "mainwindow.h"
#include "previewdialog.h"
#include "ui_previewdialog.h"
#include "QDebug"

PreviewDialog::PreviewDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PreviewDialog)
{
    ui->setupUi(this);

    connect(ui->savePrintVCButton, SIGNAL(clicked()), this, SLOT( acceptRegistration() ) );

    MainWindow *m_ui;
    m_ui = qobject_cast<MainWindow *>(parent);

//    qDebug() << m_ui->prevLastName;

    ui->lblLastName->setText(m_ui->prevLastName);
    ui->lblFirstName->setText(m_ui->prevFirstName);
    ui->lblMiddleName->setText(m_ui->prevMiddleName);
    ui->lblDobDay->setText(m_ui->prevDobDay);
    ui->lblDobMonth->setText(m_ui->prevDobMonth);
    ui->lblDobYear->setText(m_ui->prevDobYear);
    ui->lblGender->setText(m_ui->prevGender);
    ui->lblOccupation->setText(m_ui->prevOccupation);
    ui->lblAddress->setText(m_ui->prevAddress);
//    ui->lblState->setText(m_ui->prevState);
//    ui->lblLga->setText(m_ui->prevLga);
//    ui->lblWard->setText(m_ui->prevWard);
    ui->lblMobile->setText(m_ui->prevMobile);

//    QString photo = "photos/selected_photox.jpg";
//    QImage photoFront(photo);
    //ui->lblPhotoFront->setPixmap(photoFront);

}

PreviewDialog::~PreviewDialog()
{
    delete ui;
}

void PreviewDialog::acceptRegistration() {

    emit saveAndPrint();
    reject();
}


void PreviewDialog::keyPressEvent(QKeyEvent *e)
{
    switch (e->key())
    {
    case Qt::Key_F1: // Help
//        ui->searchNameEdit->setFocus();
        break;
    case Qt::Key_F2:
        ui->backButton->click();
        break;
    case Qt::Key_F3:
        ui->savePrintVCButton->click();
        break;
        // Default calls the original method to handle standard keys
    default: QWidget::keyPressEvent(e);
    }
}
