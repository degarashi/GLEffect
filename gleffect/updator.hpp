#pragma once
#include "spinner/matrix.hpp"
#include "spinner/matstack.hpp"
#include <type_traits>
#include <memory>

class GLEffect;
// -------------- 間に合わせ実装。後でちゃんとした物にする --------------
using Priority = uint32_t;
using MStack = spn::MatStack<spn::Mat44, spn::MatStackTag::PushLeft>;
// 表示とUpdateで共通の機構を使う -> Drawerを外部から削除出来るようにする
class IUpdate {
	bool _bDead = false;
	public:
		void destroy() {
			_bDead = true;
		}
		void setAsAlive() {
			_bDead = false;
		}
		bool update0(float dt) {
			if(_bDead)
				return true;
			update(dt);
			return false;
		}
		virtual void update(float dt) = 0;
};

template <class T>
class Group {
	struct Item {
		T			value;
		Priority	priority;
		Item *pPrev, *pNext;

		template <class T2>
		Item(Priority prio, T2&& t2): value(std::forward<T2>(t2)), priority(prio), pPrev(nullptr), pNext(nullptr) {}

		Item* deleteThis() {
			auto* pn = pNext;
			if(pNext)
				pNext->pPrev = pPrev;
			if(pPrev)
				pPrev->pNext = pNext;
			delete this;
			return pn;
		}
		void insertPrev(Item* item) {
			if(pPrev)
				pPrev->pNext = item;
			item->pNext = this;
			item->pPrev = pPrev;
			pPrev = item;
		}
	};
	Item*	_pBegin;
	size_t	_size;
	std::vector<Item*>	_toRem;

	public:
		Group(): _pBegin(nullptr), _size(0) {}
		~Group() {
			clear();
		}
		void clear() {
			auto* p = _pBegin;
			while(p) {
				auto* p2 = p->pNext;
				delete p;
				p = p2;
			}
			_toRem.clear();
			_size = 0;
			_pBegin = nullptr;
		}

		template <class T2>
		void add(Priority prio, T2&& p) {
			Item* ni = new Item(prio, std::forward<T2>(p));
			++_size;
			if(!_pBegin)
				_pBegin = ni;
			else {
				auto* ptr = _pBegin;
				while(ptr->pNext) {
					if(prio <= ptr->pNext->priority) {
						ptr->pNext->insertPrev(ni);
						return;
					}
					ptr = ptr->pNext;
				}
				ptr->pNext = ni;
				ptr->pNext->pPrev= ptr;
			}
		}
		// 戻り値あり
		template <class CB>
		void _iterate(CB&& cb, std::false_type) {
			auto* p = _pBegin;
			while(p) {
				if(cb(p->value))
					_toRem.push_back(p);
				p = p->pNext;
			}
		}
		// 戻り値なし
		template <class CB>
		void _iterate(CB&& cb, std::true_type) {
			auto* p = _pBegin;
			while(p) {
				cb(p->value);
				p = p->pNext;
			}
		}
		template <class CB>
		void iterate(CB&& cb) {
			_iterate(std::forward<CB>(cb), typename std::is_same<void, decltype(std::declval<CB>()(nullptr))>::type());
			for(auto* p : _toRem) {
				auto* pn = p->deleteThis();
				if(p == _pBegin)
					_pBegin = pn;
			}
			_size -= _toRem.size();
			_toRem.clear();
		}
		size_t size() const {
			return _size;
		}
};

using SPUpdate = std::shared_ptr<IUpdate>;
using UpdGroup = Group<SPUpdate>;
