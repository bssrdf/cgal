// Copyright (c) 2007-08  INRIA Sophia-Antipolis (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org); you may redistribute it under
// the terms of the Q Public License version 1.0.
// See the file LICENSE.QPL distributed with CGAL.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $URL: 
// $Id: 
//
// Author(s) : Pierre Alliez and Laurent Saboret

#ifndef CGAL_AVERAGE_SPACING_3_H
#define CGAL_AVERAGE_SPACING_3_H

#include <CGAL/basic.h>
#include <CGAL/Search_traits_3.h>
#include <CGAL/Orthogonal_k_neighbor_search.h>

#include <iterator>
#include <list>

CGAL_BEGIN_NAMESPACE

/// Compute average spacing of one query point from K nearest neighbors.
///
/// Precondition: K >= 2.
///
/// @return Computed normal, model of OrientedNormal_3.
template < typename Kernel, ///< Geometric traits class.
           typename Tree 
>
typename Kernel::FT
average_spacing_3(const typename Kernel::Point_3& query, ///< 3D point whose normal we want to compute
								  Tree& tree, ///< KD-tree
								  const unsigned int K)
{
  // basic geometric types
  typedef typename Kernel::Point_3  Point;
  typedef typename Kernel::Plane_3  Plane;
  typedef typename Kernel::Vector_3 Vector;

	// types for K nearest neighbor search
  typedef typename CGAL::Search_traits_3<Kernel> Tree_traits;
  typedef typename CGAL::Orthogonal_k_neighbor_search<Tree_traits> Neighbor_search;
  typedef typename Neighbor_search::iterator Search_iterator;


	// performs K + 1 queries (if unique the query point is
	// output first). search may be aborted when K is greater
	// than number of input points
  Neighbor_search search(tree,query,K+1);
	Search_iterator search_iterator = search.begin();
	FT sum_distances = (FT)0.0;
	unsigned int i;
	for(i=0;i<(K+1);i++)
	{
		if(search_iterator == search.end())
			break; // premature ending

		Point p = search_iterator->first;
		sum_distances += std::sqrt(CGAL::squared_distance(query,p));
		search_iterator++;
	}

	// output average spacing
	return sum_distances / (FT)K;
}


/// Compute average spacing from K nearest neighbors.
/// This variant requires the kernel.
///
/// Precondition: K >= 2.
template < typename InputIterator, ///< InputIterator value_type is Point_3.
           typename Kernel ///< Geometric traits class.
>
typename Kernel::FT
average_spacing_3(InputIterator first,    ///< input points
                  InputIterator beyond,
								  const unsigned int K,   ///< number of neighbors
								  const Kernel& /*kernel*/)
{
	// types for K-nearest neighbor search structure
  typedef typename CGAL::Search_traits_3<Kernel> Tree_traits;
  typedef typename CGAL::Orthogonal_k_neighbor_search<Tree_traits> Neighbor_search;
  typedef typename Neighbor_search::Tree Tree;

  // precondition: at least one element in the container.
  // to fix: should have at least three distinct points
	// but this is costly to check
  CGAL_precondition(first != beyond);

	// precondition: at least 2 nearest neighbors
  CGAL_precondition(K >= 2);

	// instanciate a KD-tree search
  Tree tree(first,beyond);

	// iterate over input points, compute and output normal
	// vectors (already normalized)
	FT sum_spacings = (FT)0.0;
	unsigned int nb_points = 0;
	InputIterator it;
	for(it = first; it != beyond; it++)
	{
		sum_spacings += average_spacing_3<Kernel,Tree>(*it,tree,K);
		nb_points++;
	}

	// return average spacing
	return sum_spacings / (FT)nb_points;
}

/// Compute average spacing from K nearest neighbors.
/// This variant deduces the kernel from iterator types.
///
/// Precondition: K >= 2.
template < typename InputIterator, ///< InputIterator value_type is Point_3
           typename FT ///< number type
>
FT
average_spacing_3(InputIterator first,    ///< input points
                  InputIterator beyond,
									const unsigned int K)   ///< number of neighbors
{
	typedef typename std::iterator_traits<InputIterator>::value_type Value_type;
  typedef typename Kernel_traits<Value_type>::Kernel Kernel;
	return average_spacing_3(first,beyond,K,Kernel());
}

CGAL_END_NAMESPACE

#endif // CGAL_AVERAGE_SPACING_3_H

