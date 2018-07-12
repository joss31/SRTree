 #ifndef RSTARVISITOR_H
 #define RSTARVISITOR_H
 
 #include "RStarBoundingBox.h"
 
template <typename Node, typename Leaf>
struct RStarAcceptOverlapping
{
	const typename Node::BoundingBox &m_bound;
	explicit RStarAcceptOverlapping(const typename Node::BoundingBox &bound) : m_bound(bound) {}
	
	bool operator()(const Node * const node) const 
	{ 
		return m_bound.overlaps(node->bound);
	}
	
	bool operator()(const Leaf * const leaf) const 
	{ 
		return m_bound.overlaps(leaf->bound); 
	}
	
	private: RStarAcceptOverlapping(){}
};


template <typename Node, typename Leaf>
struct RStarAcceptEnclosing
{
	const typename Node::BoundingBox &m_bound;
	explicit RStarAcceptEnclosing(const typename Node::BoundingBox &bound) : m_bound(bound) {}
	
	bool operator()(const Node * const node) const 
	{ 
		return m_bound.overlaps(node->bound);
	}
	
	bool operator()(const Leaf * const leaf) const 
	{ 
		return m_bound.encloses(leaf->bound); 
	}
	
	private: RStarAcceptEnclosing(){}
};


template <typename Node, typename Leaf>
struct RStarAcceptAny
{
	bool operator()(const Node * const node) const { return true; }
	bool operator()(const Leaf * const leaf) const { return true; }
};

template <typename Leaf>
struct RStarRemoveLeaf{

	const bool ContinueVisiting;
	RStarRemoveLeaf() : ContinueVisiting(true) {}

	bool operator()(const Leaf * const leaf) const
	{
		return true; 
	}
};

template <typename Leaf>
struct RStarRemoveSpecificLeaf
{
	mutable bool ContinueVisiting;
	bool m_remove_duplicates;
	const typename Leaf::leaf_type &m_leaf;
	
	explicit RStarRemoveSpecificLeaf(const typename Leaf::leaf_type &leaf, bool remove_duplicates = false) : 
		ContinueVisiting(true), m_remove_duplicates(remove_duplicates), m_leaf(leaf) {}
		
	bool operator()(const Leaf * const leaf) const
	{
		if (ContinueVisiting && m_leaf == leaf->leaf)
		{
			if (!m_remove_duplicates)
				ContinueVisiting = false;
			return true;
		}
		return false;
	}
	
	private: RStarRemoveSpecificLeaf(){}
};


#endif
