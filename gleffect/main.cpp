#include "mainwindow.h"
#include "testgl.hpp"
#include "dgassert.hpp"
#include <QApplication>
#include <QMessageBox>
#include <QAbstractButton>
#include "glresource.hpp"
#include <QAction>

using namespace boom::geo2d;
int main(int argc, char *argv[]) {
	std::unique_ptr<GLRes> glRes(new GLRes());
	std::unique_ptr<RigidMgrGL> glRM(new RigidMgrGL(IItg::sptr(new itg::ImpEular)));
	GLFormat::InitMap();

	QApplication app(argc, argv);
	MainWindow mwin;
	mwin.show();

	try {
		return app.exec();
	} catch(const std::exception& e) {
		QMessageBox box;
		box.setWindowTitle("error");
		box.setText(QString("there is some error\n%1").arg(e.what()));
		box.setStandardButtons(QMessageBox::Save);
		box.setDefaultButton(QMessageBox::Save);
		box.exec();
	}
	return 1;
}
