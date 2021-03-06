#include "mainwindow.h"
#include "testgl.hpp"
#include "dgassert.hpp"
#include <QApplication>
#include <QMessageBox>
#include <QAbstractButton>
#include "glresource.hpp"

int main(int argc, char *argv[]) {
	std::unique_ptr<GLRes> glRes(new GLRes());
	std::unique_ptr<RigidMgrGL> glRM(new RigidMgrGL(IItg::sptr(new itg::ImpEular)));

	QApplication app(argc, argv);
	QSurfaceFormat fmt;
	fmt.setSamples(0);

	TestGL testGL;
	testGL.setFormat(fmt);
	testGL.resize(640, 480);
	testGL.setPosition(256,256);
	testGL.create();
	testGL.show();
	testGL.setAnimating(true);

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
