//! FontCache - Qt依存の実装
#pragma once
#include <QFont>
#include <QFontMetrics>
#include <QImage>
#include <QPainter>
#include "font_base.hpp"

class FontArray_QtDep;
using FontArray_Dep = FontArray_QtDep;
//! フォント作成クラス: 環境依存
/*! フォントの設定毎に用意する */
class FontArray_QtDep {
	CCoreID			_coreID;
	// QFontとCCoreIDの対応
	QFont			_font;
	QFontMetrics	_met;
	// QImageは対応する最大サイズで確保
	QImage			_image;
	QPainter		_painter;
	bool			_bAA;

	//! 描画に必要な範囲を取得
	Rect _boundingRect(char32_t code) const;

	public:
		FontArray_QtDep(FontArray_QtDep&& dep);
		FontArray_QtDep(const std::string& name, CCoreID cid);
		FontArray_QtDep& operator = (FontArray_QtDep&& dep);

		//! 結果的にCCoreIDが同じになるパラメータの値を統一
		/*! (依存クラスによってはサイズが縦しか指定できなかったりする為) */
		CCoreID adjustParams(CCoreID cid);

		//! 使用テクスチャとUV範囲、カーソル移動距離など取得
		/*! \return first=フォントピクセルデータ
					second=フォント原点に対する描画オフセット */
		std::pair<spn::ByteBuff, Rect> getChara(char32_t code);
		int avgWidth() const;
		int maxWidth() const;
		int height() const;
		int leading() const;
		int width(char32_t c) const;
};
