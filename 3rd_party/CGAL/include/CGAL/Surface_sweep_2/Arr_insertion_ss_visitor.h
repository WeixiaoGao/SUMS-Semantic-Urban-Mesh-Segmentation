// Copyright (c) 2006,2007,2009,2010,2011 Tel-Aviv University (Israel).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
// You can redistribute it and/or modify it under the terms of the GNU
// General Public License as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.0-beta1/Arrangement_on_surface_2/include/CGAL/Surface_sweep_2/Arr_insertion_ss_visitor.h $
// $Id: Arr_insertion_ss_visitor.h 4581f1b %aI Andreas Fabri
// SPDX-License-Identifier: GPL-3.0+
//
// Author(s)     : Baruch Zukerman <baruchzu@post.tau.ac.il>
//                 Ron Wein <wein@post.tau.ac.il>
//                 Efi Fogel <efif@post.tau.ac.il>

#ifndef CGAL_ARR_INSERTION_SS_VISITOR_H
#define CGAL_ARR_INSERTION_SS_VISITOR_H

#include <CGAL/license/Arrangement_on_surface_2.h>

/*! \file
 *
 * Definition of the Arr_insertion_ss_visitor class-template. This class can be
 * further split into two, where one derives from the other, such that the
 * derived class handles the case of inserting curves into a non-empty
 * arrangement, and the base class handles the case of inserting curves into a
 * empty arrangement.
 */

#include <CGAL/Surface_sweep_2/Arr_no_intersection_insertion_ss_visitor.h>
#include <CGAL/Default.h>

namespace CGAL {

/*! \class Arr_insertion_ss_visitor
 *
 * A sweep-line visitor for inserting new curves into an existing arrangement
 * embedded on a surface.
 */
template <typename Helper_, typename Visitor_ = Default>
class Arr_insertion_ss_visitor :
  public Arr_no_intersection_insertion_ss_visitor<
    Helper_,
    typename Default::Get<Visitor_,
                          Arr_insertion_ss_visitor<Helper_, Visitor_> >::type>
{
public:
  typedef Helper_                                       Helper;

  typedef typename Helper::Geometry_traits_2            Geometry_traits_2;
  typedef typename Helper::Event                        Event;
  typedef typename Helper::Subcurve                     Subcurve;

private:
  typedef Geometry_traits_2                             Gt2;
  typedef Arr_insertion_ss_visitor<Helper, Visitor_>    Self;
  typedef typename Default::Get<Visitor_, Self>::type   Visitor;
  typedef Arr_no_intersection_insertion_ss_visitor<Helper, Visitor>
                                                        Base;

public:
  typedef typename Gt2::X_monotone_curve_2              X_monotone_curve_2;
  typedef typename Gt2::Point_2                         Point_2;

  typedef typename Helper::Arrangement_2                Arrangement_2;
  typedef typename Arrangement_2::Halfedge_handle       Halfedge_handle;

private:
  X_monotone_curve_2 sub_cv1;         // Auxiliary variables
  X_monotone_curve_2 sub_cv2;         // (used for splitting curves).

public:
  /*! A notification invoked when a new subcurve is created. */
  void add_subcurve(const X_monotone_curve_2& cv, Subcurve* sc);

  /*! Constructor. */
  Arr_insertion_ss_visitor(Arrangement_2* arr) : Base(arr) {}

  /// \name Edge-split functions (to be overridden by the child visitor).
  //@{

  /*! Check if the halfedge associated with the given subcurve will be split
   * at the given event.
   * \param sc The subcurve.
   * \param event The event.
   */
  virtual bool is_split_event(Subcurve* sc, Event* event);

  /*! Split the given edge edge.
   * \param he The edge to split.
   * \param sc The associated subcurve.
   * \param The split point.
   * \return A handle to the split edge.
   */
  virtual Halfedge_handle split_edge(Halfedge_handle he, Subcurve* sc,
                                     const Point_2& pt);
  //@}
};

//-----------------------------------------------------------------------------
// Member-function definitions:
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Check if the halfedge associated with the given subcurve will be split
// at the given event.
//
template <typename Hlpr, typename Vis>
bool Arr_insertion_ss_visitor<Hlpr, Vis>::
is_split_event(Subcurve* sc, Event* event)
{
  if (sc->last_curve().halfedge_handle() == Halfedge_handle(nullptr)) return false;

  if (! sc->originating_subcurve1())
    return (sc->left_event() != this->current_event());
  return (this->is_split_event(sc->originating_subcurve1(), event) ||
          this->is_split_event(sc->originating_subcurve2(), event));
  }

//-----------------------------------------------------------------------------
// Split an edge.
//
template <typename Hlpr, typename Vis>
typename Arr_insertion_ss_visitor<Hlpr, Vis>::Halfedge_handle
Arr_insertion_ss_visitor<Hlpr, Vis>::split_edge(Halfedge_handle he, Subcurve* sc,
                                                const Point_2& pt)
{
  // Make sure that the halfedge associated with sc is the directed from
  // right to left, since we always "look" above , and the incident face
  // is on the left of the  halfedge
  CGAL_assertion(he->direction() == ARR_RIGHT_TO_LEFT);

  this->traits()->split_2_object()(he->curve(), pt, sub_cv2, sub_cv1);
  Halfedge_handle new_he =
    this->m_arr_access.split_edge_ex(he, pt.base(),
                                     sub_cv1.base(), sub_cv2.base());
  Event* last_event_on_sc = sc->last_event();
  if (last_event_on_sc->halfedge_handle() == he)
    last_event_on_sc->set_halfedge_handle(new_he->next());

  return new_he;
}

//-----------------------------------------------------------------------------
// A notification invoked when a new subcurve is created.
//
template <typename Hlpr, typename Vis>
void Arr_insertion_ss_visitor<Hlpr, Vis>::
add_subcurve(const X_monotone_curve_2& cv, Subcurve* sc)
{
  if (Base::add_subcurve_(cv, sc)) return;

  // sc is an overlap Subcurve of existing edge and new curve,
  // which means that the edeg will have to be modified
  if (sc->originating_subcurve1()) {
    this->m_arr->modify_edge
      (this->current_event()->halfedge_handle()->next()->twin(), cv.base());
  }

  Halfedge_handle next_ccw_he =
    this->current_event()->halfedge_handle()->next()->twin();
  this->current_event()->set_halfedge_handle(next_ccw_he);
}

} // namespace CGAL

#endif
