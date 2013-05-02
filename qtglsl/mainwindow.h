#pragma once

#include <memory>
#include <QMainWindow>
#include <QGLWidget>

namespace Ui {
	class MainWindow;
}

//! メインウィンドウ
class MainWindow : public QMainWindow {
	Q_OBJECT
	std::shared_ptr<Ui::MainWindow> _spUI;

	public:
		explicit MainWindow(QWidget* parent=nullptr);
};

class TestGL : public QGLWidget {
	Q_OBJECT
//	std::shared_ptr<MGLProgram>		_spg;

	public:
		explicit TestGL(QWidget* parent=nullptr);
	protected:
		void initializeGL();
		void resizeGL(int, int);
		void paintGL();
};
