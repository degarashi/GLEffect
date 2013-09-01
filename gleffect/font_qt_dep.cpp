#include "font_qt_dep.hpp"

namespace {
	const int Weight[1 << CCoreID::BFAt<CCoreID::Weight>::length] = {
		QFont::Light, QFont::Normal, QFont::DemiBold, QFont::Bold, QFont::Black
	};
}

FontArray_QtDep::FontArray_QtDep(FontArray_QtDep&& dep):
	_font(std::move(dep._font)), _met(std::move(dep._met)), _image(std::move(dep._image)),
	_bAA(dep._bAA)
{}
FontArray_QtDep::FontArray_QtDep(const std::string& name, CCoreID cid):
	_coreID(adjustParams(cid)),
	_font(name.c_str(), _coreID.at<CCoreID::Height>(), Weight[_coreID.at<CCoreID::Weight>()], _coreID.at<CCoreID::Italic>()),
	_met(_font),
	_image(_met.maxWidth(), _met.height(), QImage::Format_ARGB32),
	_bAA(_coreID.at<CCoreID::Flag>() == CCoreID::Flag_AA)
{}
CCoreID FontArray_QtDep::adjustParams(CCoreID cid) {
	// widthは無効。heightのみでサイズ指定
	cid.at<CCoreID::Width>() = 0;
	return cid;
}
Rect FontArray_QtDep::_boundingRect(char32_t code) const {
	QRect r = _met.boundingRect(QString::fromUcs4(reinterpret_cast<const uint*>(&code), 1));
	return Rect(r.left(), r.right(), r.top(), r.bottom());
}

std::pair<spn::ByteBuff, Rect> FontArray_QtDep::getChara(char32_t code) {
	Rect rct = _boundingRect(code);
	if(rct.width() > _image.width() ||
	   rct.height() > _image.height())
		throw std::runtime_error("フォントサイズが不正(用意されたQImageよりフォントのほうが大きい)");

	int ofs[2] = {-rct.x0, -rct.y0-1};
	// 一旦QImageで取得して戻り値にコピー
	_painter.setFont(_font);
	bool bAA = _coreID.at<CharID::Flag>() == CharID::Flag_AA;
	_painter.setRenderHint(QPainter::Antialiasing, bAA);
	_painter.begin(&_image);
	_painter.setFont(_font);
	_painter.fillRect(QRect(0,0,_image.width(), _image.height()), QColor(0,0,0,255));
	_painter.setPen(QColor(255,255,255,255));
	_painter.drawText(ofs[0], ofs[1], QString::fromUcs4(reinterpret_cast<const uint*>(&code), 1));
	_painter.end();

	// white spaceなど描画する物がない場合は負数のwidthが返る
	if(rct.width() <= 0) {
		// 描画範囲を0にしてUVは適当な値=0を返す
		return std::make_pair(spn::ByteBuff(0), rct*=0);
	}
//	auto b = _image.save(QString("/tmp/fontCh_") + QChar(code) + ".bmp");
	int lineOfs = rct.width()*sizeof(uint32_t);
	spn::ByteBuff buff(lineOfs * rct.height());
	auto bpl = static_cast<size_t>(_image.bytesPerLine());
	int sw = rct.width(),
		sh = rct.height();
	auto src = reinterpret_cast<const uint8_t*>(_image.constBits());
	auto dst = reinterpret_cast<uint8_t*>(&buff[0]);
	for(int i=0 ; i<sh ; i++) {
		auto* pDst = reinterpret_cast<uint32_t*>(dst);
		auto* pSrc = reinterpret_cast<const uint32_t*>(src);
		for(int j=0 ; j<sw ; j++)
			*pDst++ = spn::SetAlphaR(spn::ARGBtoRGBA(*pSrc++));	// 1pixel = 32bit
		src += bpl;
		dst += lineOfs;
	}
	return std::make_pair(std::move(buff), rct);
}

int FontArray_QtDep::avgWidth() const {
	return _met.averageCharWidth();
}
int FontArray_QtDep::maxWidth() const {
	return _met.maxWidth();
}
int FontArray_QtDep::height() const {
	return _met.height();
}
int FontArray_QtDep::leading() const {
	return _met.leading();
}
int FontArray_QtDep::width(char32_t c) const {
	if(c > 0x10000)
		return _met.width(QString::fromUcs4(reinterpret_cast<const uint*>(&c), 1));
	return _met.width(static_cast<QChar>(c));
}
