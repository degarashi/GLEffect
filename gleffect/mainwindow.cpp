#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), _spUI(std::make_shared<Ui::MainWindow>()) {
	_spUI->setupUi(this);
}