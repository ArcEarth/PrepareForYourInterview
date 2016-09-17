#pragma once


namespace stdx
{
	template <class T>
	inline const T& as_const(T& t)
	{
		return t;
	}

	template <class T>
	inline const T* as_const(T* t)
	{
		return t;
	}


	class bst_map {
		using key_t = int;
		using value_t = int;

		//The structure of the node is as follows:
		struct node {
			node * parent;
			node * left, *right;
			key_t key;
			value_t val;

			node(node* p, key_t k, value_t v)
				: parent(p), key(k), val(v), left(nullptr), right(nullptr) {}
		};

		// an sentinel class which can be compared with key types
		// emulates infinity or negetive infinity
		struct sentinel {
			bool less;
			bool operator<(key_t rhs) { return this->less; }
			bool operator==(key_t rhs) { return false; }
		};

		static constexpr sentinel leftmost() { return sentinel{ true }; }
		static constexpr sentinel rightmost() { return sentinel{ false }; }

		static inline node*& get_child(node*p, int is_right)
		{
			return (&p->left)[is_right];
		}
		static inline const node* get_child(const node*p, int is_right)
		{
			return (&p->left)[is_right];
		}

		// parent's child ptr (ref) to this node
		node*& parents_child_ptr(node* p)
		{
			auto par = p->parent;
			return !par ? root : get_child(par, par->right == p);
		}


		// if key presented in subtree p, then it returns that node
		// if key not presented, it returns where the search end
		// i.e. where it should be inserted
		// Templated this function to allows sentinals to involve
		template <class _Key>
		static const node* search(const node* p, _Key key) noexcept
		{
			auto q = p; // p's parent
			while (p)
			{
				if (key == p->key) return p;
				q = p;
				p = get_child(p, !(key < p->key));
			}
			return q;
		}

		// mutable version
		template <class _Key>
		static node* search(node* p, _Key key)
		{
			return const_cast<node*>(search<_Key>(as_const(p), key));
		}

	public:

		bool contains(key_t key) const noexcept
		{
			auto p = search(root, key);
			return p && p->key == key;
		}

		// get the key's mapped value
		// throws out_of_range when key is not presented.
		value_t get(key_t key) const
		{
			auto p = search(root, key);
			if (p && p->key == key) return p->val;
			else //throw std::out_of_range("Key does not exist.");
				throw "Key does not exist.";
		}

		value_t try_get(key_t key, value_t default_val) const noexcept
		{
			auto p = search(root, key);
			if (p && p->key == key) return p->val;
			else return default_val;
		}

		// the evil mutable index operator, WIP
		value_t& operator[](key_t key)
		{
			auto p = search(root, key);
			if (p && p->key == key) return p->val;
			return p->val;
		}

		bool try_insert(key_t key, value_t val) noexcept
		{
			auto p = search(root, key);
			if (p && p->key == key) return false; // already exist
			auto& c = p ? get_child(p, !(key < p->key)) : root;
			c = new node(p, key, val);
			return c != nullptr; // check if the memery allocation failed
		}

		void erase(node* p) noexcept
		{
			int ece = !p->left | !p->right << 1;

			if (!ece) // both child, swap it with left-subtree's rightmost
			{
				auto c = search(p->left, rightmost()); // find the second largest elem
				p->key = c->key; p->val = c->val; // swap its content with p
				p = c; ece = 2; // than erase this node (must be signle-child case)
			}

			// single child case
			auto c = get_child(p, ece & 1); // get the non-empty child
			if (c) c->parent = p->parent;
			parents_child_ptr(p) = c; // attach the child to p's parent

									  // free the memery
			p->left = p->right = nullptr;
			delete p;
		}

		void erase(key_t key) noexcept
		{
			auto p = search(root, key);
			if (!p || !(p->key == key)) return;
			erase(p);
		}

		bst_map()
		{
			root = nullptr;
		}

		~bst_map()
		{
			if (root) delete root;
		}

	private:
		node*  root;
	};

}