#include "ClassEditDialog.h"

ClassEditDialog::ClassEditDialog(QWidget *parent)
	: QDialog(parent)
	, ui(new Ui::ClassEditDialogClass())
{
	ui->setupUi(this);
}

ClassEditDialog::~ClassEditDialog()
{
	delete ui;
}
