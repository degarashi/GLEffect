#pragma once

#include <QMainWindow>
#include <QWindow>
#include <memory>

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

class QOpenGLPaintDevice;
class OpenGLWindow : public QWindow {
	Q_OBJECT
	bool	_bUpdatePending,
			_bAnimating;

	using UPContext = std::shared_ptr<QOpenGLContext>;
	using UPDevice = std::shared_ptr<QOpenGLPaintDevice>;
	UPContext	_context;
	UPDevice	_device;

	protected:
		bool event(QEvent* event);
		void exposeEvent(QExposeEvent* event);
		void resizeEvent(QResizeEvent* event);

	public slots:
		void renderLater();
		void renderNow();

	public:
		explicit OpenGLWindow(QWindow* parent=nullptr);
		virtual void render(QPainter* painter);
		virtual void render();
		virtual void initialize();

		void setAnimating(bool bAnim);
};
