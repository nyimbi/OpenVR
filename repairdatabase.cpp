#include "repairdatabase.h"
#include "ui_repairdatabase.h"
#include "QSqlQueryModel"
#include "QSortFilterProxyModel"

RepairDatabase::RepairDatabase(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RepairDatabase)
{

    ui->setupUi(this);

   // Backup Database
    QSqlQueryModel *model = new QSqlQueryModel;
    model->setQuery("REPAIR TABLE `administrators`, `administrator_codes`, `audit_logs`, `auto_scans`, `duplicate_maps`, `experiment_results`, `fingerprint_sensitivity`, `fp_fingerprints`, `local_governments`, `operator_codes`, `logs`, `polling_units`, `registered_operators`, `registrations`, `registration_areas`, `reports`, `settings`, `states`, `tmp_search`, `tmp_afis_scans`  ");

    QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(model);
    proxyModel->setFilterRegExp(QRegExp("Error", Qt::CaseInsensitive, QRegExp::FixedString));
    proxyModel->setFilterKeyColumn(2);

    ui->repairView->setModel(proxyModel);
}

RepairDatabase::~RepairDatabase()
{
    delete ui;
}
