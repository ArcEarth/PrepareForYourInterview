#include <vector>
#include <deque>
#include <limits>

namespace stdx
{
	namespace impl
	{
		using index_t = ptrdiff_t;
		using std::vector;

		template <class TCost, class Ty>
		inline void compute_slack(
			const TCost & cost,
			const vector<Ty>& lx,
			const vector<Ty>& ly,
			index_t x,
			vector<Ty>& slack,
			vector<index_t>& slackx
			)
		{
			static_assert(is_same<Ty, decltype(cost(0, 0))>::value, "Ty must be same TCost(0,0)");
			auto n = ly.size();
			for (index_t y = 0; y < n; ++y)
			{
				if (lx[x] + ly[y] - cost(x, y) < slack[y])
				{
					slack[y] = lx[x] + ly[y] - cost(x, y);
					slackx[y] = x;
				}
			}
		}
	}

	template <class TCost, class TIndexRange>
	auto matching_cost(const TCost &cost, const TIndexRange& matching) -> decltype(cost(0, 0))
	{
		using Scalar = remove_cv_t<remove_const_t<decltype(cost(0, 0))>>;
		Scalar sum(0);
		size_t sizem = std::size(matching);
		for (ptrdiff_t i = 0; i < sizem; i++)
		{
			auto j = matching[i];
			if (j != -1)
				sum += cost(i, j);
		}
		return sum;
	}

	// requires cost_(i,j);
	template <class TCost>
	std::vector<ptrdiff_t>
		max_weight_bipartite_matching(const TCost &cost, size_t n_rows, size_t n_cols);

	template <class TCost>
	std::vector<ptrdiff_t>
		max_weight_bipartite_matching(const TCost &cost_) {
		size_t nr = cost_.rows();
		size_t nc = cost_.cols();
		return max_weight_bipartite_matching<TCost>(cost_, nr, nc);
	}

	// requires cost_(i,j); cost_.rows(); cost_.cols();
	template <class TCost>
	std::vector<ptrdiff_t>
		max_weight_bipartite_matching(const TCost &cost_, size_t nr, size_t nc)
	{
		using scalar_t = remove_cv_t<remove_const_t<decltype(cost_(0, 0))>>;
		using index_t = ptrdiff_t;
		using namespace impl;
		using namespace std;

		// the matching maps X -> Y
		vector<index_t> xy;
		// Kuhn-Munkres Algorithm
		if (nr * nc == 0)
			return xy;

		size_t n = max<size_t>(nr, nc);
		// A matrix proxy object, support cost(i,j) 
		// Returns 0 if out of range of (nr,nc)
		// So that we can handle non-square cost matrix
		auto cost = [&cost_, nr, nc](index_t i, index_t j) ->scalar_t {
			return (i < (index_t)nr && j < (index_t)nc) ? cost_(i, j) : scalar_t(0);
		};

		vector<index_t>		yx; // matching maps Y -> X
		vector<char>		S, T;  // Set stores X /belongs S, T /belongs Y
		vector<scalar_t>	lx(n, 0), ly(n, 0); // Lx, Ly Label in KM
		vector<scalar_t>	slack; // the Slack of the edge 
		vector<index_t>		slackx; // the edge of edge Y -> X
		vector<index_t>		aug_path; // the argument path

		// Initially, nothing is matched. 
		xy.assign(n, -1);
		yx.assign(n, -1);
		/*
		We maintain the following invariant:
		Vertex x is matched to vertex xy[x] and
		vertex y is matched to vertex yx[y].

		A value of -1 means a vertex isn't matched to anything.  Moreover,
		x corresponds to rows of the cost matrix and y corresponds to the
		columns of the cost matrix.  So we are matching X to Y.
		*/

		// Create an initial feasible labeling.  Moreover, in the following
		// code we will always have: 
		//     for all valid x and y:  lx[x] + ly[y] >= cost(x,y)
		// Intialize flexable labels

		for (index_t i = 0; i < nr; i++)
		{
			auto _max = cost(i, 0);
			for (index_t j = 1; j < nc; j++)
			{
				auto s = cost(i, j);
				if (s > _max) _max = s;
			}
			lx[i] = _max;
		}

		// Now grow the match set by picking edges from the equality subgraph until
		// we have a complete matching.
		for (index_t match_size = 0; match_size < n; ++match_size)
		{
			deque<index_t> q;

			// Empty out the S and T sets
			S.assign(n, false);
			T.assign(n, false);

			// clear out old slack values
			slack.assign(n, numeric_limits<scalar_t>::max());
			slackx.resize(n);
			/*
			slack and slackx are maintained such that we always
			have the following (once they get initialized by compute_slack() below):
			- for all y:
			- let x == slackx[y]
			- slack[y] == lx[x] + ly[y] - cost(x,y)
			*/

			aug_path.assign(n, -1);

			for (index_t x = 0; x < n; ++x)
			{
				// If x is not matched to anything
				if (xy[x] == -1)
				{
					q.push_back(x);
					S[x] = true;

					compute_slack(cost, lx, ly, x, slack, slackx);
					break;
				}
			}


			index_t x_start = 0;
			index_t y_start = 0;

			// Find an augmenting path.  
			bool found_augmenting_path = false;
			while (!found_augmenting_path)
			{
				while (q.size() > 0 && !found_augmenting_path)
				{
					auto x = q.front();
					q.pop_front();
					for (index_t y = 0; y < n; ++y)
					{
						if (cost(x, y) == lx[x] + ly[y] && !T[y])
						{
							// if vertex y isn't matched with anything
							if (yx[y] == -1)
							{
								y_start = y;
								x_start = x;
								found_augmenting_path = true;
								break;
							}

							T[y] = true;
							q.push_back(yx[y]);

							aug_path[yx[y]] = x;
							S[yx[y]] = true;
							compute_slack(cost, lx, ly, yx[y], slack, slackx);
						}
					}
				}

				if (found_augmenting_path)
					break;

				// Since we didn't find an augmenting path we need to improve the 
				// feasible labeling stored in lx and ly.  We also need to keep the
				// slack updated accordingly.
				scalar_t delta = numeric_limits<scalar_t>::max();
				for (index_t i = 0; i < T.size(); ++i)
				{
					if (!T[i])
						delta = std::min<scalar_t>(delta, slack[i]);
				}
				for (index_t i = 0; i < T.size(); ++i)
				{
					if (S[i])
						lx[i] -= delta;

					if (T[i])
						ly[i] += delta;
					else
						slack[i] -= delta;
				}

				q.clear();
				for (index_t y = 0; y < n; ++y)
				{
					if (!T[y] && slack[y] == 0)
					{
						// if vertex y isn't matched with anything
						if (yx[y] == -1)
						{
							x_start = slackx[y];
							y_start = y;
							found_augmenting_path = true;
							break;
						}
						else
						{
							T[y] = true;
							if (!S[yx[y]])
							{
								q.push_back(yx[y]);

								aug_path[yx[y]] = slackx[y];
								S[yx[y]] = true;
								compute_slack(cost, lx, ly, yx[y], slack, slackx);
							}
						}
					}
				}
			} // end while (!found_augmenting_path)

			  // Flip the edges aDenseIndex the augmenting path.  This means we will add one more
			  // item to our matching.
			for (index_t cx = x_start, cy = y_start, ty;
				cx != -1;
				cx = aug_path[cx], cy = ty)
			{
				ty = xy[cx];
				yx[cy] = cx;
				xy[cx] = cy;
			}
		}

		if (nr < n)
			xy.resize(nr);
		else if (nc < n)
			for (auto& xyi : xy)
				if (xyi >= nc)
					xyi = -1;

		return xy;
	}
}