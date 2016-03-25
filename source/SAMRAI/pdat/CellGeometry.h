/*************************************************************************
 *
 * This file is part of the SAMRAI distribution.  For full copyright
 * information, see COPYRIGHT and COPYING.LESSER.
 *
 * Copyright:     (c) 1997-2011 Lawrence Livermore National Security, LLC
 * Description:   hier
 *
 ************************************************************************/

#ifndef included_pdat_CellGeometry
#define included_pdat_CellGeometry

#include "SAMRAI/SAMRAI_config.h"

#include "SAMRAI/pdat/CellIndex.h"
#include "SAMRAI/pdat/CellOverlap.h"
#include "SAMRAI/hier/Box.h"
#include "SAMRAI/hier/BoxGeometry.h"
#include "SAMRAI/hier/BoxOverlap.h"
#include "SAMRAI/hier/IntVector.h"
#include "SAMRAI/tbox/Pointer.h"

namespace SAMRAI {
namespace pdat {

/*!
 * Class CellGeometry manages the mapping between the AMR index space
 * and the cell-centered geometry index space.  It is a subclass of
 * hier::BoxGeometry and it computes intersections between cell-
 * centered box geometries for communication operations.
 *
 * See header file for CellData<DIM> class for a more detailed
 * description of the data layout.
 *
 * @see hier::BoxGeometry
 * @see pdat::CellOverlap
 */

class CellGeometry:public hier::BoxGeometry
{
public:
   /*!
    * The BoxOverlap implemenation for this geometry.
    */
   typedef CellOverlap Overlap;

   /*!
    * @brief Convert an AMR index box space box into a cell geometry box.
    * A cell geometry box is the same as the given AMR index box space box.
    */
   static hier::Box
   toCellBox(
      const hier::Box& box);

   /*!
    * @brief Transform a CellIndex.
    *
    * This static method applies a coordinate system transformation to the
    * given CellIndex.
    *
    * @param[in,out]  index
    * @param[in]      transformation
    */
   static void
   transform(
      pdat::CellIndex& index,
      const hier::Transformation& transformation);

   /*!
    * @brief Construct the cell geometry object given an AMR index
    * space box and ghost cell width.
    */
   explicit CellGeometry(
      const hier::Box& box,
      const hier::IntVector& ghosts);

   /*!
    * @brief The virtual destructor does nothing interesting.
    */
   virtual ~CellGeometry();

   /*!
    * @brief Compute the overlap in cell-centered index space between
    * the source box geometry and the destination box geometry.
    */
   virtual tbox::Pointer<hier::BoxOverlap>
   calculateOverlap(
      const hier::BoxGeometry& dst_geometry,
      const hier::BoxGeometry& src_geometry,
      const hier::Box& src_mask,
      const hier::Box& fill_box,
      const bool overwrite_interior,
      const hier::Transformation& transformation,
      const bool retry,
      const hier::BoxContainer& dst_restrict_boxes = hier::BoxContainer()) const;

   /*!
    * @brief Compute the cell-centered destination boxes that represent
    * the overlap between the source box geometry and the destination
    * box geometry.
    */
   void
   computeDestinationBoxes(
      hier::BoxContainer& dst_boxes,
      const CellGeometry& src_geometry,
      const hier::Box& src_mask,
      const hier::Box& fill_box,
      const bool overwrite_interior,
      const hier::Transformation& transformation,
      const hier::BoxContainer& dst_restrict_boxes) const;

   /*!
    * @brief Set up a CellOverlap object that consists simply of the given
    * boxes and the transformation.
    */
   virtual tbox::Pointer<hier::BoxOverlap>
   setUpOverlap(
      const hier::BoxContainer& boxes,
      const hier::Transformation& transformation) const;

   /*!
    * @brief Return the box for this cell centered box geometry
    * object.
    */
   const hier::Box&
   getBox() const;

   /*!
    * @brief Return the ghost cell width for this cell centered box
    * geometry object.
    */
   const hier::IntVector&
   getGhosts() const;

private:
   /**
    * Function doOverlap() is the function that computes the overlap
    * between the source and destination objects, where both box geometry
    * objects are guaranteed to have cell centered geometry.
    */
   static tbox::Pointer<hier::BoxOverlap>
   doOverlap(
      const CellGeometry& dst_geometry,
      const CellGeometry& src_geometry,
      const hier::Box& src_mask,
      const hier::Box& fill_box,
      const bool overwrite_interior,
      const hier::Transformation& transformation,
      const hier::BoxContainer& dst_restrict_boxes);

   static void
   rotateAboutAxis(
      pdat::CellIndex& index,
      const int axis,
      const int num_rotations);

   CellGeometry(
      const CellGeometry&);             // not implemented
   void
   operator = (
      const CellGeometry&);                     // not implemented

   hier::Box d_box;
   hier::IntVector d_ghosts;

};

}
}
#ifdef SAMRAI_INLINE
#include "SAMRAI/pdat/CellGeometry.I"
#endif
#endif
