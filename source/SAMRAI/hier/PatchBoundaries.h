/*************************************************************************
 *
 * This file is part of the SAMRAI distribution.  For full copyright
 * information, see COPYRIGHT and COPYING.LESSER.
 *
 * Copyright:     (c) 1997-2011 Lawrence Livermore National Security, LLC
 * Description:   Describes boundaries for a patch
 *
 ************************************************************************/

#ifndef included_hier_PatchBoundaries
#define included_hier_PatchBoundaries

#include "SAMRAI/SAMRAI_config.h"

#include "SAMRAI/hier/BoundaryBox.h"
#include "SAMRAI/tbox/Array.h"
#include "SAMRAI/tbox/Utilities.h"

#include <map>

#if !defined(__BGL_FAMILY__) && defined(__xlC__)
/*
 * Suppress XLC warnings
 */
#pragma report(disable, CPPC5334)
#pragma report(disable, CPPC5328)
#endif

namespace SAMRAI {
namespace hier {

/*!
 * @brief Class PatchBoundaries is a container class for storing
 * BoundaryBox objects for a single patch.
 *
 * @see hier::BoundaryBox
 */

class PatchBoundaries
{
public:
   /*!
    * @brief Explicit constructor requires dimension argument.
    *
    * @param[in] dim
    */
   explicit PatchBoundaries(
      const tbox::Dimension& dim);

   /*!
    * @brief Copy constructor.
    *
    * @param[in] r  Patchboundaries object to be copied in constructor.
    */
   PatchBoundaries(
      const PatchBoundaries& r);

   /*!
    * @brief Assignment operator.
    *
    * @param[in] r  Patchboundaries object to be copied in assignment.
    */
   const PatchBoundaries&
   operator = (
      const PatchBoundaries& r);

   /*!
    * @brief Array access operator.
    *
    * @param[in] i  Array index.
    */
   tbox::Array<hier::BoundaryBox>&
   operator [] (
      unsigned int i);

   /*!
    * @brief Const Array access operator.
    *
    * @param[in] i  Array index.
    */
   const tbox::Array<hier::BoundaryBox>&
   operator [] (
      unsigned int i) const;

   /*!
    * @brief Get copy of the internal arrays.
    *
    * @return  Copy of the internal arrays.
    */
   tbox::Array<tbox::Array<hier::BoundaryBox> >
   getArrays();

   /*!
    * @brief Get const copy of the internal arrays.
    *
    * @return  Const copy of the internal arrays.
    */
   const tbox::Array<tbox::Array<hier::BoundaryBox> >
   getArrays() const;

   /*!
    * @brief friend declaration
    */
   friend class::std::map<int, SAMRAI::hier::PatchBoundaries>;

private:
   /*!
    * @brief Private default constructor.
    *
    * This constructor is need by the brain dead STL and should not be used
    * for any other purpose.
    */
   PatchBoundaries();

   /*!
    * @brief Dimension of the object.
    */
   const tbox::Dimension d_dim;

   /*
    * @brief Internal arrays of BoundaryBox
    */
   tbox::Array<tbox::Array<hier::BoundaryBox> > d_array_of_bboxes;
};

} // SAMRAI namespace
} // hier namespace

#if !defined(__BGL_FAMILY__) && defined(__xlC__)
/*
 * Suppress XLC warnings
 */
#pragma report(enable, CPPC5334)
#pragma report(enable, CPPC5328)
#endif

#ifdef SAMRAI_INLINE
#include "SAMRAI/hier/PatchBoundaries.I"
#endif

#endif
