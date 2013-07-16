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
	boom::RCoeff rc = {120.f,
						10.f,
						-1.0f,0,0};
	std::unique_ptr<RigidMgrGL> glRM(new RigidMgrGL(IItg::sptr(new itg::ImpEular), rc));

	QApplication app(argc, argv);
	QSurfaceFormat fmt;
	fmt.setSamples(0);

	TestGL testGL;
	testGL.setFormat(fmt);
	testGL.resize(1024, 768);
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
