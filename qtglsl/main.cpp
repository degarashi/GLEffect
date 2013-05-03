#include "mainwindow.h"
#include <QApplication>
#include <QMessageBox>

int main(int argc, char *argv[]) {
	QApplication a(argc, argv);
	MainWindow w;
	TestGL* testGL = new TestGL();
	w.setCentralWidget(testGL);
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
