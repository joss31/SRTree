#ifndef RSTARTREE_H
#define RSTARTREE_H

#include <list>
#include <vector>
#include <limits>
#include <algorithm>
#include <cassert>
#include <functional>

#include <iostream>
#include <sstream>
#include <fstream>

#include "RStarBoundingBox.h"

// R* tree parametros
#define RTREE_REINSERT_P 0.30
#define RTREE_CHOOSE_SUBTREE_P 32

#define RSTAR_TEMPLATE 

template <typename BoundedItem, typename LeafType>
struct RStarLeaf : BoundedItem {
	
	typedef LeafType leaf_type;
	LeafType leaf;
};

template <typename BoundedItem>
struct RStarNode : BoundedItem {
	std::vector< BoundedItem* > items;
	bool hasLeaves;
};

#include "RStarVisitor.h"

template <
	typename LeafType, 
	std::size_t dimensions, std::size_t min_child_items, std::size_t max_child_items
>
class RStarTree {
public:

	typedef RStarBoundedItem<dimensions>		BoundedItem;
	typedef typename BoundedItem::BoundingBox	BoundingBox;
	
	typedef RStarNode<BoundedItem> 				Node;
	typedef RStarLeaf<BoundedItem, LeafType> 	Leaf;
	
	typedef RStarAcceptOverlapping<Node, Leaf>	AcceptOverlapping;
	typedef RStarAcceptEnclosing<Node, Leaf>	AcceptEnclosing;
	typedef RStarAcceptAny<Node, Leaf>			AcceptAny;

	typedef RStarRemoveLeaf<Leaf>				RemoveLeaf;
	typedef RStarRemoveSpecificLeaf<Leaf>		RemoveSpecificLeaf;
	
	RStarTree() : m_root(NULL), m_size(0)
	{
		assert(1 <= min_child_items && min_child_items <= max_child_items/2);
	}
	
    ~RStarTree() {
		Remove(
			AcceptAny(), 
			RemoveLeaf()
		);
	}
	
	void Insert(LeafType leaf, const BoundingBox &bound)
	{

		Leaf * newLeaf = new Leaf();
		newLeaf->bound = bound;
		newLeaf->leaf  = leaf;

		if (!m_root)
		{
			m_root = new Node();
			m_root->hasLeaves = true;
			
			m_root->items.reserve(min_child_items);
			m_root->items.push_back(newLeaf);
			m_root->bound = bound;
		}
		else
			InsertInternal(newLeaf, m_root);
			
		m_size += 1;
	}

	template <typename Acceptor, typename Visitor>
	Visitor Query(const Acceptor &accept, Visitor visitor)
	{
		if (m_root)
		{	
			QueryFunctor<Acceptor, Visitor> query(accept, visitor);
			query(m_root);
		}
		
		return visitor;
	}

	template <typename Acceptor, typename LeafRemover>
	void Remove( const Acceptor &accept, LeafRemover leafRemover)
	{
		std::list<Leaf*> itemsToReinsert;

		if (!m_root)
			return;
		
		RemoveFunctor<Acceptor, LeafRemover> remove(accept, leafRemover, &itemsToReinsert, &m_size);
		remove(m_root, true);
		
		if (!itemsToReinsert.empty())
		{
			typename std::list< Leaf* >::iterator it = itemsToReinsert.begin();
			typename std::list< Leaf* >::iterator end = itemsToReinsert.end();
		
			for(;it != end; it++)
				InsertInternal(*it, m_root);
		}
	}
	
	void RemoveBoundedArea( const BoundingBox &bound )
	{
		Remove(AcceptEnclosing(bound), RemoveLeaf());
	}
	
	void RemoveItem( const LeafType &item, bool removeDuplicates = true )
	{
		Remove( AcceptAny(), RemoveSpecificLeaf(item, removeDuplicates));
	}
	
	
	std::size_t GetSize() const { return m_size; }
	std::size_t GetDimensions() const { return dimensions; }
	
	
protected:
	
	Node * ChooseSubtree(Node * node, const BoundingBox * bound)
	{
		if (static_cast<Node*>(node->items[0])->hasLeaves)
		{
			if (max_child_items > (RTREE_CHOOSE_SUBTREE_P*2)/3  && node->items.size() > RTREE_CHOOSE_SUBTREE_P)
			{
				std::partial_sort( node->items.begin(), node->items.begin() + RTREE_CHOOSE_SUBTREE_P, node->items.end(),
					SortBoundedItemsByAreaEnlargement<BoundedItem>(bound));
				
				return static_cast<Node*>(* std::min_element(node->items.begin(), node->items.begin() + RTREE_CHOOSE_SUBTREE_P,
					SortBoundedItemsByOverlapEnlargement<BoundedItem>(bound)));
			}
			
			return static_cast<Node*>(* std::min_element(node->items.begin(), node->items.end(),
				SortBoundedItemsByOverlapEnlargement<BoundedItem>(bound)));	
		}

		return static_cast<Node*>(*	std::min_element( node->items.begin(), node->items.end(),
				SortBoundedItemsByAreaEnlargement<BoundedItem>(bound)));
	}
	
    Node * InsertInternal(Leaf * leaf, Node * node, bool firstInsert = true)
	{
		node->bound.stretch(leaf->bound);
	
        if (node->hasLeaves)
		{
			node->items.push_back(leaf);
		}else{
            Node * tmp_node = InsertInternal( leaf, ChooseSubtree(node, &leaf->bound), firstInsert );
			
			if (!tmp_node)
				return NULL;
				
			node->items.push_back(tmp_node);
		}

        if (node->items.size() > max_child_items )
		{
			
			return OverflowTreatment(node, firstInsert);
		}
			
		return NULL;
	}
	
	Node * OverflowTreatment(Node * level, bool firstInsert)
	{

		if (level != m_root && firstInsert)
		{
			Reinsert(level);
			return NULL;
		}
		
		Node * splitItem = Split(level);
		
		if (level == m_root)
		{
			Node * newRoot = new Node();
			newRoot->hasLeaves = false;
			
			newRoot->items.reserve(min_child_items);
			newRoot->items.push_back(m_root);
			newRoot->items.push_back(splitItem);
			
			newRoot->bound.reset();
			for_each(newRoot->items.begin(), newRoot->items.end(), StretchBoundingBox<BoundedItem>(&newRoot->bound));
			
			m_root = newRoot;
			return NULL;
		}
		return splitItem;
	}

	Node * Split(Node * node)
	{
		Node * newNode = new Node();
		newNode->hasLeaves = node->hasLeaves;

		const std::size_t n_items = node->items.size();
		const std::size_t distribution_count = n_items - 2*min_child_items + 1;
		
		std::size_t split_axis = dimensions+1, split_edge = 0, split_index = 0;
		int split_margin = 0;
		
		BoundingBox R1, R2;

		assert(n_items == max_child_items + 1);
		assert(distribution_count > 0);
		assert(min_child_items + distribution_count-1 <= n_items);
		
		for (std::size_t axis = 0; axis < dimensions; axis++)
		{
			int margin = 0;
			double overlap = 0, dist_area, dist_overlap;
			std::size_t dist_edge = 0, dist_index = 0;
		
			dist_area = dist_overlap = std::numeric_limits<double>::max();
			
			for (std::size_t edge = 0; edge < 2; edge++)
			{
				if (edge == 0)
					std::sort(node->items.begin(), node->items.end(), SortBoundedItemsByFirstEdge<BoundedItem>(axis));
				else
					std::sort(node->items.begin(), node->items.end(), SortBoundedItemsBySecondEdge<BoundedItem>(axis));

				for (std::size_t k = 0; k < distribution_count; k++)
		        {
					double area = 0;
				
					R1.reset();
					for_each(node->items.begin(), node->items.begin()+(min_child_items+k), StretchBoundingBox<BoundedItem>(&R1));
							
					R2.reset();
					for_each(node->items.begin()+(min_child_items+k+1), node->items.end(), StretchBoundingBox<BoundedItem>(&R2));

                    margin 	+= R1.edgeDeltas() + R2.edgeDeltas();
					area 	+= R1.area() + R2.area();
					overlap =  R1.overlap(R2);
					

					if (overlap < dist_overlap || (overlap == dist_overlap && area < dist_area))
					{
						dist_edge = 	edge;
						dist_index = 	min_child_items+k;
						dist_overlap = 	overlap;
						dist_area = 	area;
					}		
				}
			}
			
			if (split_axis == dimensions+1 || split_margin > margin )
			{
				split_axis 		= axis;
				split_margin 	= margin;
				split_edge 		= dist_edge;
				split_index 	= dist_index;
			}
		}
	
		if (split_edge == 0)
			std::sort(node->items.begin(), node->items.end(), SortBoundedItemsByFirstEdge<BoundedItem>(split_axis));

		else if (split_axis != dimensions-1)
			std::sort(node->items.begin(), node->items.end(), SortBoundedItemsBySecondEdge<BoundedItem>(split_axis));	
		
		newNode->items.assign(node->items.begin() + split_index, node->items.end());
		node->items.erase(node->items.begin() + split_index, node->items.end());
		
		node->bound.reset();
		std::for_each(node->items.begin(), node->items.end(), StretchBoundingBox<BoundedItem>(&node->bound));
		
		newNode->bound.reset();
		std::for_each(newNode->items.begin(), newNode->items.end(), StretchBoundingBox<BoundedItem>(&newNode->bound));
		
		return newNode;
	}

	void Reinsert(Node * node)
	{
		std::vector< BoundedItem* > removed_items;

		const std::size_t n_items = node->items.size();
		const std::size_t p = (std::size_t)((double)n_items * RTREE_REINSERT_P) > 0 ? (std::size_t)((double)n_items * RTREE_REINSERT_P) : 1;
		
		assert(n_items == max_child_items + 1);
		
		std::partial_sort(node->items.begin(), node->items.end() - p, node->items.end(), 
			SortBoundedItemsByDistanceFromCenter<BoundedItem>(&node->bound));
			
		removed_items.assign(node->items.end() - p, node->items.end());
		node->items.erase(node->items.end() - p, node->items.end());
		
		node->bound.reset();
		for_each(node->items.begin(), node->items.end(), StretchBoundingBox<BoundedItem>(&node->bound));
		
		for (typename std::vector< BoundedItem* >::iterator it = removed_items.begin(); it != removed_items.end(); it++)
			InsertInternal( static_cast<Leaf*>(*it), m_root, false);
	}
	
	template <typename Acceptor, typename Visitor>
	struct VisitFunctor : std::unary_function< const BoundingBox *, void > {
	
		const Acceptor &accept;
		Visitor &visit;
		
		explicit VisitFunctor(const Acceptor &a, Visitor &v) : accept(a), visit(v) {}
	
		void operator()( BoundedItem * item ) 
		{
			Leaf * leaf = static_cast<Leaf*>(item);
		
			if (accept(leaf))
				visit(leaf);
		}
	};
	
    template <typename Acceptor, typename Visitor>
	struct QueryFunctor : std::unary_function< const BoundedItem, void > {
		const Acceptor &accept;
		Visitor &visitor;
		
		explicit QueryFunctor(const Acceptor &a, Visitor &v) : accept(a), visitor(v) {}
	
		void operator()(BoundedItem * item)
		{
			Node * node = static_cast<Node*>(item);
		
			if (visitor.ContinueVisiting && accept(node))
			{
				if (node->hasLeaves)
					for_each(node->items.begin(), node->items.end(), VisitFunctor<Acceptor, Visitor>(accept, visitor));
				else
					for_each(node->items.begin(), node->items.end(), *this);
			}
		}
	};
	
	template <typename Acceptor, typename LeafRemover>
	struct RemoveLeafFunctor : 
		std::unary_function< const BoundingBox *, bool > 
	{
		const Acceptor &accept;
		LeafRemover &remove;
		std::size_t * size;
		
		explicit RemoveLeafFunctor(const Acceptor &a, LeafRemover &r, std::size_t * s) :
			accept(a), remove(r), size(s) {}
	
		bool operator()(BoundedItem * item ) const {
			Leaf * leaf = static_cast<Leaf *>(item);
			
			if (accept(leaf) && remove(leaf))
			{
				--(*size);
				delete leaf;
				return true;
			}
			
			return false;
		}
	};
	
	
	template <typename Acceptor, typename LeafRemover>
	struct RemoveFunctor :
		std::unary_function< const BoundedItem *, bool > 
	{
		const Acceptor &accept;
		LeafRemover &remove;
		
		std::list<Leaf*> * itemsToReinsert;
		std::size_t * m_size;
	
		explicit RemoveFunctor(const Acceptor &na, LeafRemover &lr, std::list<Leaf*>* ir, std::size_t * size)
			: accept(na), remove(lr), itemsToReinsert(ir), m_size(size) {}
	
		bool operator()(BoundedItem * item, bool isRoot = false)
		{
			Node * node = static_cast<Node*>(item);
		
			if (accept(node))
			{	
				if (node->hasLeaves)
					node->items.erase(std::remove_if(node->items.begin(), node->items.end(), RemoveLeafFunctor<Acceptor, LeafRemover>(accept, remove, m_size)), node->items.end());
				else
					node->items.erase(std::remove_if(node->items.begin(), node->items.end(), *this), node->items.end() );

				if (!isRoot)
				{
					if (node->items.empty())
					{
						delete node;
						return true;
					}
					else if (node->items.size() < min_child_items)
					{
						QueueItemsToReinsert(node);
						return true;
					}
				}
				else if (node->items.empty())
				{
					node->hasLeaves = true;
					node->bound.reset();
				}
			}			

			return false;
			
		}

		void QueueItemsToReinsert(Node * node)
		{
			typename std::vector< BoundedItem* >::iterator it = node->items.begin();
			typename std::vector< BoundedItem* >::iterator end = node->items.end();
		
			if (node->hasLeaves)
			{
				for(; it != end; it++)
					itemsToReinsert->push_back(static_cast<Leaf*>(*it));
			}
			else
				for (; it != end; it++)
					QueueItemsToReinsert(static_cast<Node*>(*it));
					
			delete node;
		}
	};
	

private:
	Node * m_root;
	
	std::size_t m_size;
};

#undef RSTAR_TEMPLATE

#undef RTREE_SPLIT_M
#undef RTREE_REINSERT_P
#undef RTREE_CHOOSE_SUBTREE_P




#endif

