#pragma once
#include <vector>

namespace stdx
{
	class seg_tree
	{
		using index_t = int;
		using value_t = int;

		struct node_t
		{
			// affordance
			value_t value;

			// control block
			bool	is_unifrom; // flag bit indicates weather the value in this node is valiad
		};

		index_t		   _root;
		index_t		   _begin;
		index_t		   _length;
		index_t		   _capicity;
		std::vector<node_t> _nodes;

		static unsigned long pow2ceil(unsigned long val)
		{
			if (val <= 2) return val;
			unsigned long msb_idx;
			_BitScanReverse(&msb_idx, val - 1);
			return 1 << (msb_idx + 1);
		}

	public:
		seg_tree(index_t length, value_t init_val = value_t(0))
		{
			_begin = 0;
			_root = 1;
			_length = length;
			int cap = pow2ceil(length + 1);
			_capicity = cap;
			_nodes.resize(_capicity * 2);

			for (int i = 0; i < cap * 2; i++)
			{
				auto & node = _nodes[i];
				node.is_unifrom = true;
				node.value = init_val;
			}
		}

	private:

		bool try_merge_segment(index_t nid)
		{
			// merge children if they are both uniform
			bool uni = nid < _capicity && _nodes[nid << 1].is_unifrom && _nodes[nid << 1 | 1].is_unifrom && _nodes[nid << 1].value == _nodes[nid << 1 | 1].value;

			if (uni)
				_nodes[nid].value = _nodes[nid << 1].value;

			_nodes[nid].is_unifrom = uni;

			return uni;
		}

	public:
		void set_node(index_t id, value_t val)
		{
			_nodes[id].is_unifrom = true;
			_nodes[id].value = val;
		}

		void insert(index_t begin, index_t end, value_t value)
		{
			begin -= this->_begin;
			if (begin >= end || begin >= this->_length)
				return;

			int i = begin + _capicity;
			int j = end + _capicity + 1;

			for (; (i ^ j) >> 1; i >>= 1, j >>= 1)
			{
				if (~i & 1) // if i is left child
					set_node(i | 1, value);

				if (j & 1) // if j is right child
					set_node(j ^ 1, value);

				try_merge_segment(i >> 1);
				try_merge_segment(j >> 1);
			}

			while (i > 1 && try_merge_segment(i >> 1))
				i >>= 1;
		}

		void extend();
		size_t length() const;
		index_t begin() const;
		index_t end() const;

		value_t get(index_t position) const
		{
			index_t p = position - this->_begin + 1;
			//cout << "[query] position = " << position << endl;
			index_t nid = this->_root;
			int s = 0, len = this->_capicity;
			while (!_nodes[nid].is_unifrom)
			{
				const auto & node = _nodes[nid];

				//cout << "[query] current node == " << nid << " [" << s << ", " << s + len << ") = ";
				//if (node.is_unifrom) cout << node.value; else cout << "*";
				//cout << endl;

				auto mid = s + len / 2;

				nid = nid << 1 | (p >= mid); // advance to child
				len >>= 1;
				s |= ((p < mid) - 1) & len;
			}

			const auto & node = _nodes[nid];
			//cout << "[query] current node == " << nid << " [" << s << ", " << s + len << ") = ";
			return _nodes[nid].value;
		}

		bool	is_uniform(index_t begin, index_t end) const;

		value_t get_combined(index_t begin, index_t end) const;
	};
}