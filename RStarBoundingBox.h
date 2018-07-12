#ifndef RStarBoundingBox_H
#define RStarBoundingBox_H

#include <limits>
#include <utility>
#include <cstddef>
#include <string>
#include <sstream>


template <std::size_t dimensions>
struct RStarBoundingBox {

	std::pair<int, int> edges[dimensions];
	
	void reset()
	{
		for (std::size_t axis = 0; axis < dimensions; axis++)
		{
			edges[axis].first = std::numeric_limits<int>::max();
			edges[axis].second = std::numeric_limits<int>::min();
		}
	}
	
	static RStarBoundingBox MaximumBounds()
	{
		RStarBoundingBox<dimensions> bound;
		bound.reset();
		return bound;
	}
	
	bool stretch(const RStarBoundingBox<dimensions> &bb)
	{
		bool ret = false;
		
		for (std::size_t axis = 0; axis < dimensions; axis++)
		{
			
			if (edges[axis].first > bb.edges[axis].first)
			{
				edges[axis].first = bb.edges[axis].first;
				ret = true;
			}
		
			if (edges[axis].second < bb.edges[axis].second)
			{
				edges[axis].second = bb.edges[axis].second;
				ret = true;
			}
		}
			
		return ret;
	}
	
	inline int edgeDeltas() const
	{
		int distance = 0;
		for (std::size_t axis = 0; axis < dimensions; axis++)
			distance += edges[axis].second - edges[axis].first;
			
		return distance;
	}
	
	inline double area() const
	{
		double area = 1;
		for (std::size_t axis = 0; axis < dimensions; axis++)
			area *= (double)(edges[axis].second - edges[axis].first);
		
		return area;
	}
	
	inline bool encloses(const RStarBoundingBox<dimensions>& bb) const
	{
		for (std::size_t axis = 0; axis < dimensions; axis++)
			if (bb.edges[axis].first < edges[axis].first || edges[axis].second < bb.edges[axis].second)
				return false;
		
		return true;
	}
	
	inline bool overlaps(const RStarBoundingBox<dimensions>& bb) const
	{
		// if (!(x1 < y2) && !(x2 > y1))
		for (std::size_t axis = 0; axis < dimensions; axis++)
		{		
			if (!(edges[axis].first < bb.edges[axis].second) || !(bb.edges[axis].first < edges[axis].second))
				return false;
		}

		return true;
	}
	
	double overlap(const RStarBoundingBox<dimensions>& bb) const
	{
		double area = 1.0;
		for (std::size_t axis = 0; area && axis < dimensions; axis++)
		{
			const int x1 = edges[axis].first;
			const int x2 = edges[axis].second;
			const int y1 = bb.edges[axis].first;
			const int y2 = bb.edges[axis].second;
		
			// borde izquierdo fuera borde izquierdo
			if (x1 < y1)
			{
				// el borde derecho dentro del borde izquierdo
				if (y1 < x2)
				{
					// borde derecho fuera del borde derecho
					if (y2 < x2)
						area *= (double)( y2 - y1 );
					else
						area *= (double)( x2 - y1 );
						
					continue;
				}
			}
			// borde derecho dentro del borde izquierdo
			else if (x1 < y2)
			{
				// borde derecho fuera del borde derecho
				if (x2 < y2)
					area *= (double)( x2 - x1 );
				else
					area *= (double)( y2 - x1 );
					
				continue;
			}
			
			return 0.0;
		}

		return area;
	}
	
	double distanceFromCenter(const RStarBoundingBox<dimensions>& bb) const
	{
		double distance = 0, t;
		for (std::size_t axis = 0; axis < dimensions; axis++)
		{
			t = ((double)edges[axis].first + (double)edges[axis].second + 
			     (double)bb.edges[axis].first + (double)bb.edges[axis].second)
				 /2.0;
			distance += t*t;
		}
			
		return distance;
	}
	
	bool operator==(const RStarBoundingBox<dimensions>& bb)
	{
		for (std::size_t axis = 0; axis < dimensions; axis++)
			if (edges[axis].first != bb.edges[axis].first || edges[axis].second != bb.edges[axis].second)
				return false;
			
		return true;
	}
	
	
	std::string ToString() const
	{
		std::stringstream name("");
		name << "[";
		for (std::size_t axis = 0; axis < dimensions; axis++)
		{
			name << "(" << edges[axis].first << "," << edges[axis].second << ")";
			if (axis != dimensions -1)
				name << ",";
		}
		name << "]";
		
		return name.str();
	}
};



template <std::size_t dimensions>
struct RStarBoundedItem {
	typedef RStarBoundingBox<dimensions> BoundingBox;

	BoundingBox bound;
};


// for_each(items.begin(), items.end(), StretchBoundedItem::BoundingBox(bound));
template <typename BoundedItem>
struct StretchBoundingBox : 
	public std::unary_function< const BoundedItem * const, void >
{
	typename BoundedItem::BoundingBox * m_bound;
	explicit StretchBoundingBox(typename BoundedItem::BoundingBox * bound) : m_bound(bound) {}

	void operator() (const BoundedItem * const item)
	{
		m_bound->stretch(item->bound);
	}
};


template <typename BoundedItem>
struct SortBoundedItemsByFirstEdge : 
	public std::binary_function< const BoundedItem * const, const BoundedItem * const, bool >
{
	const std::size_t m_axis;
	explicit SortBoundedItemsByFirstEdge (const std::size_t axis) : m_axis(axis) {}
	
	bool operator() (const BoundedItem * const bi1, const BoundedItem * const bi2) const 
	{
		return bi1->bound.edges[m_axis].first < bi2->bound.edges[m_axis].first;
	}
};

template <typename BoundedItem>
struct SortBoundedItemsBySecondEdge : 
	public std::binary_function< const BoundedItem * const, const BoundedItem * const, bool >
{
	const std::size_t m_axis;
	explicit SortBoundedItemsBySecondEdge (const std::size_t axis) : m_axis(axis) {}

	bool operator() (const BoundedItem * const bi1, const BoundedItem * const bi2) const 
	{
		return bi1->bound.edges[m_axis].second < bi2->bound.edges[m_axis].second;
	}
};


template <typename BoundedItem>
struct SortBoundedItemsByDistanceFromCenter : 
	public std::binary_function< const BoundedItem * const, const BoundedItem * const, bool >
{
	const typename BoundedItem::BoundingBox * const m_center;
	explicit SortBoundedItemsByDistanceFromCenter(const typename BoundedItem::BoundingBox * const center) : m_center(center) {}

	bool operator() (const BoundedItem * const bi1, const BoundedItem * const bi2) const 
	{
		return bi1->bound.distanceFromCenter(*m_center) < bi2->bound.distanceFromCenter(*m_center);
	}
};

template <typename BoundedItem>
struct SortBoundedItemsByAreaEnlargement : 
	public std::binary_function< const BoundedItem * const, const BoundedItem * const, bool >
{
	const double area;
	explicit SortBoundedItemsByAreaEnlargement(const typename BoundedItem::BoundingBox * center) : area(center->area()) {}

	bool operator() (const BoundedItem * const bi1, const BoundedItem * const bi2) const 
	{
		return area - bi1->bound.area() < area - bi2->bound.area();
	}
};

template <typename BoundedItem>
struct SortBoundedItemsByOverlapEnlargement : 
	public std::binary_function< const BoundedItem * const, const BoundedItem * const, bool >
{
	const typename BoundedItem::BoundingBox * const m_center;
	explicit SortBoundedItemsByOverlapEnlargement(const typename BoundedItem::BoundingBox * const center) : m_center(center) {}

	bool operator() (const BoundedItem * const bi1, const BoundedItem * const bi2) const 
	{
		return bi1->bound.overlap(*m_center) < bi2->bound.overlap(*m_center);
	}
};


#endif
