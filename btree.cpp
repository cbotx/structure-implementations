#include <iostream>
#include <vector>
#include <queue>
#include <array>
#include <algorithm>
#include <utility>
#include <list>
#include <assert.h>
#include <stack>

struct BTNode {
	std::list<std::pair<int, BTNode*>> val;
	BTNode* end = nullptr;
	BTNode* par = nullptr;
	bool leaf = false;
};


class BTree {
	typedef std::list<std::pair<int, BTNode*>>::iterator ITERATOR;
public:
	BTree(int deg) : deg(deg) {
		root = new BTNode;
		root->leaf = true;
	}
	virtual ~BTree() {
		deconstruct(root);
	}
	void deconstruct(BTNode* node) {
		if (!node) return;
		for (auto i : node->val) {
			deconstruct(i.second);
		}
		deconstruct(node->end);
		std::cout << node->val.begin()->first << ' ';
		delete node;
	}
	void print() {
		std::cout << "===============" << std::endl;
		int last_depth = 0;
		std::queue<std::pair<BTNode*, int>> q;
		q.push(std::make_pair(root, 0));
		while (!q.empty()) {
			std::pair<BTNode*, int> cur = q.front();
			q.pop();
			int depth = cur.second;
			if (depth > last_depth) {
				std::cout << std::endl;
				last_depth = depth;
			}
			for (auto i : cur.first->val) {
				std::cout << i.first << " ";
				if (i.second != nullptr) {
					q.push(std::make_pair(i.second, depth + 1));
				}
			}
			if (cur.first->end != nullptr) {
				q.push(std::make_pair(cur.first->end, depth + 1));
			}
			BTNode* par = cur.first->val.begin()->second;
			if (par) {
				std::cout << "*" << par->val.begin()->first;
			}
			std::cout << "|";
		}
		std::cout << std::endl << "===============" << std::endl;
	}
	void insert(int x) {
		BTNode* cur = root;
		std::stack<ITERATOR> st;
		
		while (!cur->leaf) {
			ITERATOR it = lower_bound(cur, x);
			st.push(it);
			cur = (it != cur->val.end()) ? it->second : cur->end;
		}

		assert(cur != nullptr);
		ITERATOR it = lower_bound(cur, x);
		cur->val.insert(it, std::make_pair(x, nullptr));

		while (cur->val.size() >= deg) {
			ITERATOR parit;
			if (!st.empty()) {
				parit = st.top();
				st.pop();
			}
			cur = split(cur, parit);
		}
	}
	bool remove(int x) {
		ITERATOR e;
		BTNode* cur = find(x, e);
		if (cur == nullptr) return false;
		if (!cur->leaf) {
			while (!cur->leaf) {
				ITERATOR it = lower_bound(cur, x);
				cur = (it != cur->val.end()) ? it->second : cur->end;
			}
			e->first = cur->val.back().first;
			cur->val.pop_back();
		}
		else {
			cur->val.erase(e);
		}
		if (cur->val.size() == 0) adjust(cur);
		return true;
	}
	bool find(int x) {
		ITERATOR it;
		return find(x, it) != nullptr;
	}
	BTNode* find(int x, ITERATOR &it) {
		BTNode* cur = root;
		while (cur != nullptr) {
			it = lower_bound(cur, x);
			if (it == cur->val.end()) {
				cur = cur->end;
			}
			else if (it->first == x) {
				break;
			}
			else {
				cur = it->second;
			}
		}
		return cur;
	}
private:
	ITERATOR lower_bound(BTNode* node, int x) {
		ITERATOR it;
		for (it = node->val.begin(); it != node->val.end(); ++it) {
			if (it->first >= x) {
				break;
			}
		}
		return it;
	}
	BTNode* split(BTNode* node, ITERATOR it) {
		if (node->par == nullptr) {
			root = new BTNode;
			node->par = root;
			root->leaf = false;
			root->end = node;
			it = root->val.begin();
		}
		ITERATOR mid = node->val.begin();
		std::advance(mid, (deg - 1) / 2);
		BTNode* lc = new BTNode;
		lc->par = node->par;
		lc->leaf = node->leaf;
		ITERATOR i; 
		for (i = node->val.begin(); i != mid;) {
			lc->val.push_back(*i);
			if (i->second) i->second->par = lc;
			i = node->val.erase(i);	
		}
		if (i->second) i->second->par = lc;
		lc->end = mid->second;
		node->par->val.insert(it, std::make_pair(mid->first, lc));
		node->val.erase(mid);
		return node->par;
	}
	void adjust(BTNode* n) {
		BTNode* lsib = nullptr;
		BTNode* rsib = nullptr;
		BTNode* par = n->par;
		assert(par);
		ITERATOR it, lit;
		for (it = par->val.begin(); it != par->val.end(); ++it) {
			if (it->second == n) {
				ITERATOR i = it;
				++i;
				if (i != par->val.end()) {
					rsib = i->second;
				}
				else {
					rsib = par->end;
				}
				break;
			}
			lit = it;
			lsib = it->second;
		}
		if (lsib && lsib->val.size() > 1) {
			n->val.push_back(*lit);
			n->val.begin()->second = lsib->end;
			lsib->end = lsib->val.back().second;
			*lit = std::make_pair(lsib->val.back().first, lsib);
			lsib->val.pop_back();
		}
		else if (rsib && rsib->val.size() > 1) {
			n->val.push_back(*it);
			n->val.begin()->second = n->end;
			n->end = rsib->val.begin()->second;
			*it = std::make_pair(rsib->val.front().first, n);
			rsib->val.pop_front();
		}
		else {
			BTNode* node;
			if (lsib) {
				lsib->val.push_back(std::make_pair(lit->first, lsib->end));
				lsib->end = n->end;
				par->val.erase(lit);
				if (it != par->val.end()) {
					it->second = lsib;
				}
				else {
					par->end = lsib;
				}
				node = lsib;
			}
			else {
				assert(rsib);
				rsib->val.push_front(std::make_pair(it->first, n->end));
				par->val.erase(it);
				node = rsib;
			}
			delete n;
			if (par->val.size() == 0) {
				if (par == root) {
					root = node;
					node->par = nullptr;
					delete par;
				}
				else {
					adjust(par);
				}
			}
		}
	}
	BTNode* root;
	int deg;
};

