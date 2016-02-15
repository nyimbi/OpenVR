#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

#include <QDialog>

namespace Ui {
    class ControlPanel;
}

class ControlPanel : public QDialog
{
    Q_OBJECT

public:
    explicit ControlPanel(QWidget *parent = 0);
    ~ControlPanel();

protected:
    void changeEvent(QEvent *e);
    bool saveSettings();

private:
    Ui::ControlPanel *ui;

private slots:
//    void on_brightnessSlider_valueChanged(int value);
    void on_inpPu_currentIndexChanged(QString );
    void on_buttonBox_rejected();
    void on_buttonBox_accepted();
    void on_inpRa_currentIndexChanged(QString );
    void on_inpLga_currentIndexChanged(QString );
    void on_inpState_currentIndexChanged(QString  );
    void loadStates();
    void loadLgas(QString);
    void loadRas(QString);
    void loadPollingUnits(QString);
    void setSelectedRa(QString);
    void restoreSettings();
    void loadFPSensitivity();
    void getMacAddress();
    void getSystemInfo();
    void selectFileDialog();
    bool saveSensitivity();
    void restoreSensitivitySetting();
    void loadAddressFormats();
    bool saveAddressFormat();
    void on_btnUpdateDetails_clicked();
    void on_inpOperator_currentIndexChanged(int index);
    void on_selectOperator_clicked();
    void on_btnRegister_clicked();
    void loadOperators();
};

#endif // CONTROLPANEL_H
