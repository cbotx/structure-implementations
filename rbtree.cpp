#include <iostream>
#include <vector>
#include <queue>
#include <array>
#include <algorithm>
#include <utility>
#include <list>
#include <assert.h>
#include <stack>


struct RBTNode {
	RBTNode(int x) : val(x) {};
	bool black = false;
	int val;
	RBTNode* lc = nullptr;
	RBTNode* rc = nullptr;
	RBTNode* par = nullptr;
};


class RBTree {

public:
	RBTree() = default;

	virtual ~RBTree() {
		clear();
	}

	void insert(int x) {
		RBTNode* node = new RBTNode(x);
		if (_empty) {
			node->black = true;
			root = node;
			_empty = false;
		}
		else {
			RBTNode* cur = root;
			RBTNode* f = nullptr;
			while (cur) {
				f = cur;
				if (cur->val >= x) {
					cur = cur->lc;
					if (!cur) {
						f->lc = node;
						break;
					}
				}
				else {
					cur = cur->rc;
					if (!cur) {
						f->rc = node;
						break;
					}
				}
			}
			node->par = f;
			_update(node);
		}
	}

	RBTNode* find(int x) {
		RBTNode* node = root;
		while (node) {
			if (node->val == x) return node;
			if (node->val > x) {
				node = node->lc;
			}
			else {
				node = node->rc;
			}
		}
		return nullptr;
	}

	bool remove(int x) {
		RBTNode* node = find(x);
		if (!node) return false;
		if (node == root && !(node->lc) && !(node->rc)) {
			_empty = true;
			_delete(node);
			return true;
		}
		else if (node->lc && node->rc) {
			RBTNode* cur = node->lc;
			while (cur->rc) {
				cur = cur->rc;
			}
			_lswap(node, cur);
			node = cur;
		}
		if (!node->black) {
			_delete(node);
			return true;
		}
		else {
			RBTNode* ch = (node->lc) ? node->lc : node->rc;
			if (ch) {
				if (node == root) {
					_delete(node);
					root = ch;
					ch->par = nullptr;
				}
				else {
					RBTNode* p = node->par;
					RBTNode** pch = (p->lc == node) ? &(p->lc) : &(p->rc);
					_delete(node);
					*pch = ch;
					ch->par = p;
					ch->black = true;
				}
				return true;
			}
			else {
				bool first = true;
				RBTNode* p, *s, *c, *d;
				p = node->par;
				while (p) {
					if (p->lc == node) {
						s = p->rc;
						c = s->lc;
						d = s->rc;
					}
					else {
						s = p->lc;
						c = s->rc;
						d = s->lc;
					}
					if (first) {
						first = false;
						_delete(node);
					}
					// case 3
					if (!s->black) {
						_rotate(p, s);
						s->black = true;
						p->black = false;
					}
					// case 6
					else if (d && !d->black) {
						_rotate(p, s);
						s->black = p->black;
						p->black = true;
						d->black = true;
						return true;
					}
					// case 5
					else if (c && !c->black) {
						_rotate(s, c);
						c->black = true;
						s->black = false;
					}
					// case 1
					else if (p->black) {
						s->black = false;
						node = p;
						p = node->par;
					}
					// case 4
					else {
						p->black = true;
						s->black = false;
						return true;
					}
				}
				return true;
			}
		}
		assert(0);
	}

	void clear() {
		_recursive_destroy(root);
	}

	void print() {
		std::queue<RBTNode*> q;
		q.push(root);
		int cnt = 1;
		int c2 = 0;
		while (!q.empty()) {
			RBTNode* node = q.front();
			q.pop();
			++cnt;
			if (node) ++c2;
			if (node) {
				std::cout << (node->black ? 'b' : 'r') << node->val << ' ';
				q.push(node->lc);
				q.push(node->rc);
			}
			else {
				std::cout << " . ";
				q.push(nullptr);
				q.push(nullptr);
			}
			if (cnt && (!(cnt & (cnt - 1)))) {
				std::cout << std::endl;
				if (c2 == 0) return;
				c2 = 0;
			}
		}
	}

private:

	void _recursive_destroy(RBTNode* node) {
		if (node) {
			_recursive_destroy(node->lc);
			_recursive_destroy(node->rc);
			delete node;
		}
	}

	void _delete(RBTNode* &node) {
		if (node == root) {
			delete root;
			root = nullptr;
		}
		else {
			assert(node->par);
			if (node->par->lc == node) {
				node->par->lc = nullptr;
			}
			else {
				node->par->rc = nullptr;
			}
			delete node;
			node = nullptr;
		}
	}

	void _lswap(RBTNode* a, RBTNode* b) {
		if (a == b) return;
		std::swap(a->val, b->val);
	}

	void _update(RBTNode* n) {
		if (!n || n->black) return;
		RBTNode* f = n->par;
		if (!f || f->black) return;
		RBTNode* ff = f->par;
		if (!ff) {
			f->black = false;
		}
		else if (ff->lc && ff->rc && ff->lc->black == ff->rc->black) {
			ff->lc->black = true;
			ff->rc->black = true;
			ff->black = false;
			_update(ff);
		}
		else {
			if (ff->lc == f && f->rc == n || ff->rc == f && f->lc == n) {
				_rotate(f, n);
				std::swap(f, n);
			}
			_rotate(ff, f);
			ff->black = false;
			f->black = true;
		}
	}

	void _rotate(RBTNode* f, RBTNode* n) {
		RBTNode* ff = f->par;
		if (ff) {
			if (ff->lc == f) {
				ff->lc = n;
			}
			else {
				ff->rc = n;
			}
		}
		if (root == f) root = n;
		n->par = ff;
		f->par = n;
		if (f->lc == n) {
			f->lc = n->rc;
			n->rc = f;
			if (f->lc) f->lc->par = f;
		}
		else {
			f->rc = n->lc;
			n->lc = f;
			if (f->rc) f->rc->par = f;
		}
	}

	bool _empty = true;
	RBTNode* root = nullptr;
};

int main() {
	// Testing code
	RBTree tr;
	for (int i = 0; i < 10; ++i) {
		tr.insert(i);
	}
	for (int i = 10; i > 0; --i) {
		tr.insert(i);
	}
	tr.print();
	for (int i = 0; i < 10; ++i) {
		tr.remove(i);
		tr.print();
	}
	for (int i = 0; i <= 10; ++i) {
		tr.remove(i);
		tr.print();
	}
	for (int i = 0; i < 10; ++i) {
		tr.insert(i);
		tr.print();
	}
}