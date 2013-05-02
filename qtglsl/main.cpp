#include "mainwindow.h"
#include <QApplication>
#include <QMessageBox>
#include "glresource.hpp"
#include "glx.hpp"

void TestGLX() {
	GLEffect gle;
	gle.readGLX("test.glx");
// 	gle.setTechnique("TestTech");
// 	gle.setPass("TestPass");
// 	gle.applySetting();
// 	gle.setPass("AnotherPass");
// 	gle.applySetting();
// 	gle.setVDecl({0, 0, 4, VSem.POSITION});
// 	gle.setUniform(vec4(1,2,3,4), "the_param");
// 	gle.setMacro("the_entry", 128);

// 	glDrawElements(...);
// 	gle.saveParams();
// 	gle.restoreParams();
}

int main(int argc, char *argv[]) {
	// OpenGL関数群を読み込む
	LoadXGLFunc();
	TestGLX();

	QApplication a(argc, argv);
	MainWindow w;
	w.show();

	try {
		return a.exec();
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
