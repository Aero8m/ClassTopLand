#pragma once

#include <QDialog>
#include "ui_ClassEditDialog.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ClassEditDialogClass; };
QT_END_NAMESPACE

class ClassEditDialog : public QDialog
{
	Q_OBJECT

public:
	ClassEditDialog(QWidget *parent = nullptr);
	~ClassEditDialog();

private:
	Ui::ClassEditDialogClass *ui;
};
