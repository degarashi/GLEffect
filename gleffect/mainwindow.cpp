#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "testgl.hpp"
#include "boomstick/collision.hpp"

using namespace spn;
using namespace boom::geo2d;
MainWindow::MainWindow(QWidget *parent): QMainWindow(parent),
	_spUI(std::make_shared<Ui::MainWindow>()),
	_view(std::make_shared<TestGL>())
{
	_spUI->setupUi(this);
	connect(_spUI->actionQuit_q, SIGNAL(triggered()), qApp, SLOT(quit()));

	QSurfaceFormat fmt;
	fmt.setSamples(0);
	fmt.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
	_view->setFormat(fmt);
	_view->resize(800, 600);
	_view->setPosition(256,256);
	_view->create();
	_view->show();
	_view->setAnimating(true);

	connect(_spUI->resetButton, SIGNAL(clicked()), _view.get(), SLOT(resetScene()));
	connect(_view.get(), SIGNAL(visibleChanged(bool)), this, SLOT(close()));
	connect(this, SIGNAL(sigCoeff(boom::RCoeff)), _view.get(), SLOT(changeCoeff(boom::RCoeff)));
	connect(this, SIGNAL(sigEnv(SimEnv)), _view.get(), SLOT(changeEnv(SimEnv)));
	connect(this, SIGNAL(sigInitial(SimInitial)), _view.get(), SLOT(changeInitial(SimInitial)));
	connect(this, SIGNAL(sigView(SimView)), _view.get(), SLOT(changeView(SimView)));

	// 初期値を設定
	changeCoeff();
	changeEnv();
	changeInitial();
	changeView();
}

void MainWindow::changeCoeff() {
	boom::RCoeff c;
	c.spring = _spUI->spinSpring->value();
	c.dumper = _spUI->spinDumper->value();
	c.fricD = _spUI->spinFricD->value();
	c.fricS = _spUI->spinFricS->value();
	c.fricMS = 0;
	emit sigCoeff(c);
}
void MainWindow::changeEnv() {
	SimEnv e;
	e.gravity = spn::Vec2(_spUI->spinGravX->value(), _spUI->spinGravY->value());
	e.air = spn::Vec2(_spUI->spinAirX->value(), _spUI->spinAirY->value());
	emit sigEnv(e);
}
void MainWindow::changeInitial() {
	SimInitial in;
	in.nbox = _spUI->spinNBox->value();
	in.nIter = _spUI->spinNStep->value();
	in.mass = _spUI->spinMassLinear->value();
	in.inertia = _spUI->spinMassRot->value();
	emit sigInitial(in);
}
void MainWindow::changeView() {
	SimView v;
	v.bVSync = _spUI->chkVSync->isChecked();
	emit sigView(v);
}
