#pragma once

#include <QMainWindow>
#include <QWindow>
#include <memory>
#include "spinner/vector.hpp"

namespace Ui {
	class MainWindow;
}

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
		bool event(QEvent* event) override;
		void exposeEvent(QExposeEvent* event) override;
		void resizeEvent(QResizeEvent* event) override;

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

struct SimInitial {
	int	nbox, nIter;
	float mass, inertia;
};
struct SimEnv {
	spn::Vec2	gravity,
				air;
};
namespace boom {
	struct RCoeff;
}
class TestGL;
//! メイン兼コントロールパネルウィンドウ
class MainWindow : public QMainWindow {
	Q_OBJECT
	using SP_UI = std::shared_ptr<Ui::MainWindow>;
	using SP_GL = std::shared_ptr<TestGL>;
	SP_UI		_spUI;
	SP_GL		_view;

	private slots:
		void changeCoeff();
		void changeEnv();
		void changeInitial();
		void viewMousePressEv(QMouseEvent* e);
	signals:
		void sigCoeff(const boom::RCoeff& c);
		void sigEnv(const SimEnv& e);
		void sigInitial(const SimInitial& in);
	public:
		explicit MainWindow(QWidget* parent=nullptr);
};
