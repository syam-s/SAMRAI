/*************************************************************************
 *
 * This file is part of the SAMRAI distribution.  For full copyright
 * information, see COPYRIGHT and COPYING.LESSER.
 *
 * Copyright:     (c) 1997-2013 Lawrence Livermore National Security, LLC
 * Description:   Node in asynchronous Berger-Rigoutsos dendogram
 *
 ************************************************************************/
#ifndef included_mesh_BergerRigoutsosNode
#define included_mesh_BergerRigoutsosNode

#include "SAMRAI/SAMRAI_config.h"

#include "SAMRAI/tbox/AsyncCommGroup.h"
#include "SAMRAI/hier/BlockId.h"
#include "SAMRAI/hier/BoxLevel.h"
#include "SAMRAI/hier/Connector.h"
#include "SAMRAI/hier/PatchLevel.h"

#include <set>
#include <list>
#include <vector>

namespace SAMRAI {
namespace mesh {

/*!
 * @brief Node in the asynchronous Berger-Rigoutsos (BR) dendogram.
 * Do not directly use this class; for clustering, use BergerRigoutsos
 * instead.
 *
 * In mesh generation, the BR algorithm can be used to cluster
 * tagged cells into boxes.
 * This algorithm is described in Berger and Rigoutsos,
 * IEEE Trans. on Sys, Man, and Cyber (21)5:1278-1286.
 *
 * This class implements the BR algorithm to execute
 * in a non-recursive way, in order to improve parallel
 * efficiency over recursive implementations.
 * To facilitate a non-recursive implementation,
 * data in the recursive tree is maintained in a "BR dendogram",
 * nodes of which are instances of this class.
 *
 * Clarification on the uses of the word "node":
 * - Dendogram node: Node in the BR dendogram (this class).
 * - Graph node: Node in a box graph.  The box graph is the form
 *   of the outputs of this class.  Each output graph node
 *   corresponds to a box generated by the BR algorithm.
 * - Processor: MPI process id.  This is called a node in some
 *   context.  For clarity, we avoid this use of "node".
 *
 * Each dendogram node is associated with a candidate box,
 * an owner process coordinating distributed computations on the box
 * and a group of processors participating in those computations.
 * Should the candidate box be one of the final output boxes,
 * the owner also owns the graph node associated with the box.
 *
 * To use this class:
 * -# Construct the root dendogram node, an object of type
 *    BergerRigoutsosNode.
 * -# Finetune the algorithm settings using the methods under
 *    "Algorithm settings".
 * -# Start clustering by calling clusterAndComputeRelationships().
 *
 * The 2 primary outputs of this implementation are:
 * -# A BoxLevel of Boxes containing input tags.  Each node
 *    corresponds to an output box.
 * -# Connector between the tag BoxLevel and the new BoxLevel.
 *
 * TODO:
 * -# Implement MOST_TAGS ownership option.  This may be an
 *    improvement over the MOST_OVERLAP and is easy to do
 *    because the number of local tags in the candidate box
 *    is already computed.
 */

class BergerRigoutsosNode:
   private tbox::AsyncCommStage::Handler
{

public:
   enum OwnerMode { SINGLE_OWNER = 0,
                    MOST_OVERLAP = 1,
                    FEWEST_OWNED = 2,
                    LEAST_ACTIVE = 3 };

   /*!
    * @brief Method for advancing the algorithm.
    *
    * Each corresponds to a choice permitted by setAlgorithmAdvanceMode().
    */
   enum AlgoAdvanceMode { ADVANCE_ANY,
                          ADVANCE_SOME,
                          SYNCHRONOUS };


   /*!
    * @brief Data structure for clustering set-up, initiation, and
    * output.
    *
    * This structure contains data input, output, set-up data shared
    * by the dendogram nodes and the BergerRigoutsos object initiating
    * the clustering.  Also contains parameters shared among all nodes
    * in a dendogram and collectively managed by those nodes.
    *
    * In the implementation of the BR algorithm, some parameters are
    * to be shared among all nodes in the dendogram, either for
    * efficiency or coordinating the dendogram nodes.  All such
    * parameters are contained in a single CommonParams object.
    *
    * @param[in] tag_level
    * @param[in] tag_data_index
    * @param[in] tag_val
    * @param[in] min_box
    * @param[in] efficiency_tol
    * @param[in] combine_tol
    * @param[in] max_box_size
    * @param[in] max_inflection_cut_from_center Limit the Laplace cut to this
    *   fraction of the distance from the center plane to the end.
    *   Zero means cut only at the center plane.  One means unlimited.
    *   Under most situations, one is fine.
    *
    * @param[in] inflection_cut_threshold_ar
    */
   class CommonParams
   {
public:
      explicit CommonParams(
         const boost::shared_ptr<hier::PatchLevel> &tag_level,
         const int tag_data_index,
         const int tag_val,
         const hier::IntVector min_box,
         const double efficiency_tol,
         const double combine_tol,
         const hier::IntVector& max_box_size,
         const double max_inflection_cut_from_center,
         const double inflection_cut_threshold_ar);

      //@{
      //! @name Algorithm mode settings

      /*!
       * @brief Set the mode for advancing the asynchronous implementation.
       *
       * Choices are:
       * - @b "SYNCHRONOUS" --> wait for each communication stage to complete
       *   before moving on, thus resulting in synchronous execution.
       * - @b "ADVANCE_ANY" --> advance an dendogram node through its
       *   communication stage by using tbox::AsyncCommStage::advanceAny().
       * - @b "ADVANCE_SOME" --> advance an dendogram node through its
       *   communication stage by using tbox::AsyncCommStage::advanceSome().
       *
       * The default is "ADVANCE_SOME".
       *
       * Asynchronous modes are NOT guaranteed to compute the output
       * graph nodes in any particular order.  The order depends on
       * the ordering of message completion, which is not deterministic.
       * If you require consistent outputs, we suggest you have a scheme
       * for reordering the output boxes.
       *
       * @pre (algo_advance_mode == "ADVANCE_ANY") ||
       *      (algo_advance_mode == "ADVANCE_SOME") ||
       *      (algo_advance_mode == "SYNCHRONOUS")
       */
      void
      setAlgorithmAdvanceMode(
         const std::string& algo_advance_mode);

      /*!
       * @brief Set the method for choosing the owner.
       * Choices:
       * - "MOST_OVERLAP"
       *   Ownership is given to the processor with the most
       *   overlap on the candidate box.  Default.
       * - "SINGLE_OWNER"
       *   In single-owner mode, the initial owner (process 0)
       *   always participates and owns all dendogram nodes.
       * - "FEWEST_OWNED"
       *   Choose the processor that owns the fewest dendogram
       *   nodes when the choice is made.  This is meant to
       *   relieve bottle-necks caused by excessive ownership.
       *   This option may lead to non-deterministic ownerships.
       * - "LEAST_ACTIVE"
       *   Choose the processor that participates in the fewest
       *   number of dendogram nodes when the choice is made.
       *   This is meant to relieve bottle-necks caused by
       *   excessive participation. This option may lead to
       *   non-deterministic ownerships.
       *
       * Experiments show that "MOST_OVERLAP" gives the best
       * clustering speed, while "SINGLE_OWNER" may give a faster
       * output globalization (since you don't need an all-gather).
       *
       * @pre (mode == "SINGLE_OWNER") ||(mode == "MOST_OVERLAP") ||
       *      (mode == "FEWEST_OWNED") ||(mode == "LEAST_ACTIVE")
       */
      void
      setOwnerMode(
         const std::string& mode);

      /*!
       * @brief Relationship computation flag.
       *
       * Valid mode values to set are:
       *
       * - "NONE" = No relationship computation.
       *
       * - "TAG_TO_NEW": Compute directed relationships from input (tagged) to
       * output (new) graph nodes.  With this option, it is possible to
       * determine output nodes neighboring any input nodes, but not
       * possible to determine input nodes neighboring a specific output
       * node.
       *
       * - "BIDIRECTIONAL": Compute directed relationships from input (tagged) to
       * output (new) graph nodes as well as the reverse.  With this
       * option, it is possible to determine output nodes neighboring any
       * input nodes, as well as input nodes neighboring any output node.
       * This is accomplished using an additional relationship-sharing
       * communication after all graph nodes have been created.
       *
       * The ghost_cell_width specifies the growth for the overlap
       * checks.  Overlap checking is done to determine nearest-neighbor
       * relationships when generating connectivity to new graph nodes.
       * If a box grown by this ammount intersects another box, the two
       * boxes are considered neighbors.
       *
       * By default, compute bidirectional relationships with a ghost cell width
       * of 1.
       *
       * @pre (mode == "NONE") || (mode == "TAG_TO_NEW") ||
       *      (mode == "BIDIRECTIONAL")
       * @pre ghost_cell_width >= hier::IntVector::getZero(d_common->getDim())
       */
      void
      setComputeRelationships(
         const std::string mode,
         const hier::IntVector& ghost_cell_width);

      /*!
       * @brief Set the minimum box size constraint when making cuts.
       *
       * This parameter is not in the the BoxGeneratorStrategy interface so it
       * has to be set here.
       */
      void
      setMinBoxSizeFromCutting(
         const hier::IntVector& min_box_size_from_cutting);

      //@}

      /*!
       * @brief Run the clustering algorithm to generate the new BoxLevel
       * and compute relationships (if specified by setComputeRelationships()).
       *
       * If relationships computation is not specified, the Connectors are
       * unchanged.
       *
       * @param new_box_level
       * @param tag_to_new
       * @param bound_boxes Contains one global bounding box for each
       *                    block with a patch in tag_level.
       * @param tag_level
       * @param mpi_object Alternative SAMRAI_MPI object.  If given,
       *   must be congruent with the tag box_level's MPI communicator.
       *   Specify tbox::SAMRAI_MPI::commNull if unused.  Highly recommend
       *   using an isolated communicator to prevent message mix-ups.
       *
       * @pre !bound_boxes.isEmpty()
       * @pre d_parent == 0
       * @pre (d_common->getDim() == (*(bound_boxes.begin())).getDim()) &&
       *      (d_common->getDim() == tag_level->getDim())
       */
      void
      clusterAndComputeRelationships(
         boost::shared_ptr<hier::BoxLevel>& new_box_level,
         boost::shared_ptr<hier::Connector>& tag_to_new,
         const hier::BoxContainer& bound_boxes);


      /*!
       * @brief Duplicate given MPI communicator for private use
       * and various dependent parameters.
       *
       * This method overrides the MPI object from the tag level,
       * which is set in the constructor.  Calling this method
       * guarantees that an exclusive MPI Communicator is used for
       * clustering, making the execution immune to stray messages
       * from un-related code.
       */
      void setMPI( const tbox::SAMRAI_MPI& mpi );

      /*!
       * @brief Setup names of timers.
       *
       * By default, timers are named
       * "mesh::BergerRigoutsosNode::*", where the third field is
       * the specific steps performed by the BergerRigoutsosNode.
       * You can override the first two fields with this method.
       * Conforming to the timer naming convention, timer_prefix should
       * have the form "*::*".
       */
      void
      setTimerPrefix(
         const std::string& timer_prefix);

      const tbox::Dimension &getDim() const {
         return d_tag_level->getDim();
      }

      //! @brief Global number of tags in clusters.
      int
      getNumTags() const
         {
            return d_num_tags_in_all_nodes;
         }

      //! @brief Max number of tags owned.
      int
      getMaxTagsOwned() const
         {
            return d_max_tags_owned;
         }

      //! @brief Max number of local nodes for dendogram.
      int
      getMaxNodes() const
         {
            return d_max_nodes_allocated;
         }

      //! @brief max generation count for the local nodes in the dendogram.
      int
      getMaxGeneration() const
         {
            return d_max_generation;
         }

      //! @brief Max number of locally owned nodes in the dendogram.
      int
      getMaxOwnership() const
         {
            return d_max_nodes_owned;
         }

      //! @brief Average number of continuations for local nodes in dendogram.
      double
      getAvgNumberOfCont() const
         {
            if (d_num_nodes_completed > 0) {
               return (double)d_num_conts_to_complete
                  / d_num_nodes_completed;
            }
            return 0;
         }

      //! @brief Max number of continuations for local nodes in dendogram.
      int
      getMaxNumberOfCont() const
         {
            return d_max_conts_to_complete;
         }

      /*!
       * @brief Number of boxes generated (but not necessarily owned)
       * on the local process.
       */
      int
      getNumBoxesGenerated() const
         {
            return d_num_boxes_generated;
         }

      /*!
       * @brief Set whether to log dendogram node action history
       * (useful for debugging).
       */
      void
      setLogNodeHistory(
         bool flag)
         {
            d_log_node_history = flag;
         }


      //@{
      //! @name Timer data for this class.

      /*
       * @brief Structure of timers used by this class.
       *
       * Each object can set its own timer names through
       * setTimerPrefix().  This leads to many timer look-ups.  Because
       * it is expensive to look up timers, this class caches the timers
       * that has been looked up.  Each TimerStruct stores the timers
       * corresponding to a prefix.
       */
      struct TimerStruct {
         boost::shared_ptr<tbox::Timer> t_cluster;
         boost::shared_ptr<tbox::Timer> t_cluster_and_compute_relationships;
         boost::shared_ptr<tbox::Timer> t_continue_algorithm;
         boost::shared_ptr<tbox::Timer> t_compute;
         boost::shared_ptr<tbox::Timer> t_comm_wait;
         boost::shared_ptr<tbox::Timer> t_MPI_wait;
         boost::shared_ptr<tbox::Timer> t_compute_new_graph_relationships;
         boost::shared_ptr<tbox::Timer> t_share_new_relationships;
         boost::shared_ptr<tbox::Timer> t_share_new_relationships_send;
         boost::shared_ptr<tbox::Timer> t_share_new_relationships_recv;
         boost::shared_ptr<tbox::Timer> t_share_new_relationships_unpack;
         boost::shared_ptr<tbox::Timer> t_local_tasks;
         boost::shared_ptr<tbox::Timer> t_local_histogram;
         /*
          * Multi-stage timers.  These are used in continueAlgorithm()
          * instead of the methods they time, because what they time may
          * include waiting for messages.  They are included in the
          * timer t_continue_algorithm.  They provide timing breakdown
          * for the different stages.
          */
         boost::shared_ptr<tbox::Timer> t_reduce_histogram;
         boost::shared_ptr<tbox::Timer> t_bcast_acceptability;
         boost::shared_ptr<tbox::Timer> t_gather_grouping_criteria;
         boost::shared_ptr<tbox::Timer> t_bcast_child_groups;
         boost::shared_ptr<tbox::Timer> t_bcast_to_dropouts;
      };

      //! @brief Default prefix for Timers.
      static const std::string s_default_timer_prefix;

      /*!
       * @brief Static container of timers that have been looked up.
       */
      static std::map<std::string, TimerStruct> s_static_timers;

      /*!
       * @brief Structure of timers in s_static_timers, matching this
       * object's timer prefix.
       */
      TimerStruct* d_object_timers;

      /*!
       * @brief Set d_object_timers.  The timers are named with the
       * given prefix.
       */
      void
      setObjectTimers(
         const std::string& timer_prefix);

      //@}

      void incNumNodesCommWait() {
         ++d_num_nodes_commwait;
         d_max_nodes_commwait =
            tbox::MathUtilities<int>::Max(d_num_nodes_commwait,
                                          d_max_nodes_commwait);
      }
      void decNumNodesCommWait() {
         --d_num_nodes_commwait;
      }
      void writeCounters() {
         tbox::plog << d_num_nodes_allocated << "-alloc  "
                    << d_num_nodes_active << "-act  "
                    << d_num_nodes_owned << "-owned  "
                    << d_num_nodes_completed << "-done  "
                    << d_relaunch_queue.size() << "-qd  "
                    << d_num_nodes_commwait << "-wait  ";
      }

      /*!
       * @brief Check the congruency between d_mpi and d_tag_level's MPI.
       */
      bool checkMPICongruency() const;

      /*!
       * @brief Set up data that depend on the MPI communicator being
       * used.
       */
      void setupMPIDependentData();

      //! @brief Participants send new relationship data to graph node owners.
      void shareNewNeighborhoodSetsWithOwners();

      //! @brief Shorthand for a sorted, possibly incontiguous, set of integers.
      typedef std::set<int> IntSet;

      /*!
       * @brief Shorthand for std::vector<int> for internal use.
       */
      typedef std::vector<int> VectorOfInts;

      /*!
       * @brief Queue on which to append jobs to be
       * launched or relaunched.
       */
      std::list<BergerRigoutsosNode *> d_relaunch_queue;

      /*!
       * @brief Stage handling multiple asynchronous communication groups.
       */
      tbox::AsyncCommStage d_comm_stage;

      AlgoAdvanceMode d_algo_advance_mode;

      /*!
       * @brief Level where tags live.
       */
      boost::shared_ptr<hier::PatchLevel> d_tag_level;

      /*!
       * @brief New BoxLevel generated by BR.
       *
       * This is where we store the boxes as we progress in the BR algorithm.
       *
       * This is set in the public clusterAndComputeRelationships method.
       */
      boost::shared_ptr<hier::BoxLevel> d_new_box_level;

      /*!
       * @brief Connector from tag_box_level to new_box_level.
       *
       * This is where we store the relationships resulting from the BR
       * algorithm.  The relationships are created locally for local nodes in
       * tag_box_level.
       *
       * This is set in the public clusterAndComputeRelationships method.
       */
      boost::shared_ptr<hier::Connector> d_tag_to_new;

      /*!
       * @brief Initial boxes for top-down clustering.
       */
      hier::BoxContainer d_root_boxes;

      /*!
       * @brief
       */
      double d_max_inflection_cut_from_center;

      /*
       * @brief Threshold for favoring thicker directions for Laplace
       * cuts.
       *
       * The higher the value, the more we tolerate high aspect
       * ratios.  Box directions corresponding to aspect ratios lower
       * than this will not be subject to Laplace cuts (except for the
       * thickest direction).  Set to 0 to always cut the thickest
       * direction.  Set to huge value to disregard high aspect
       * ratios.
       */
      double d_inflection_cut_threshold_ar;

      /*!
       * @brief If a candidate box does not fit in this limit,
       * it will be split.
       *
       * Boxes will not be recombined (@see combine_tol) if the
       * combination breaks this limit.
       *
       * This is meant to prevent huge boxes that degrades worst-case
       * performances in when later processing the box.
       */
      hier::IntVector d_max_box_size;

      /*!
       * @brief Alternate minimum box size applying to inflection
       * point cuts.
       *
       * This size can be greater than the absolute min_size
       * specified by the
       * BoxGeneratorStrategy::findBoxesContainingTags() abstract
       * interface.
       */
      hier::IntVector d_min_box_size_from_cutting;

      //@{
      //@name Parameters from clustering algorithm interface
      int d_tag_data_index;
      int d_tag_val;
      hier::IntVector d_min_box;
      double d_efficiency_tol;
      double d_combine_tol;
      //@}

      /*!
       * @brief Relationship computation flag.
       *
       * See setComputeRelationships().
       */
      int d_compute_relationships;

      /*!
       * @brief List of processes that will send neighbor data
       * for locally owned boxes after the BR algorithm completes.
       */
      IntSet d_relationship_senders;

      /*!
       * @brief Outgoing messages to be sent to graph node owners
       * describing new relationships found by local process.
       */
      std::map<int, CommonParams::VectorOfInts> d_relationship_messages;

      //! @brief Ammount to grow a box when checking for overlap.
      hier::IntVector d_max_gcw;

      //! @brief How to chose the group's owner.
      OwnerMode d_owner_mode;

      //@{
      //! @name Communication parameters
      /*!
       * @brief MPI communicator used in all communications in
       * the dendogram.
       */
      tbox::SAMRAI_MPI d_mpi_object;
      //! @brief Upperbound of valid tags.
      int d_tag_upper_bound;
      //! @brief Smallest unclaimed MPI tag in pool given to local process.
      int d_available_mpi_tag;
      //@}

      //@{
      //! @name Auxiliary data for analysis and debugging.

      //TODO: Are these counters multiblock?  If not, which block?  Make them consistent.

      //! @brief Whether to log major actions of dendogram node.
      bool d_log_node_history;
      //! @brief Number of tags.
      int d_num_tags_in_all_nodes;
      //! @brief Max number of tags owned.
      int d_max_tags_owned;
      //! @brief Current number of dendogram nodes allocated.
      int d_num_nodes_allocated;
      //! @brief Highest number of dendogram nodes allocated.
      int d_max_nodes_allocated;
      //! @brief Current number of dendogram nodes active.
      int d_num_nodes_active;
      //! @brief Highest number of dendogram nodes active.
      int d_max_nodes_active;
      //! @brief Current number of dendogram nodes owned.
      int d_num_nodes_owned;
      //! @brief Highest number of dendogram nodes owned.
      int d_max_nodes_owned;
      //! @brief Current number of dendogram nodes in communication wait.
      int d_num_nodes_commwait;
      //! @brief Highest number of dendogram nodes in communication wait.
      int d_max_nodes_commwait;
      //! @brief Current number of completed.
      int d_num_nodes_completed;
      //! @brief Highest number of generation.
      int d_max_generation;
      //! @brief Current number of boxes generated.
      int d_num_boxes_generated;
      //! @brief Number of continueAlgorithm calls for to complete nodes.
      int d_num_conts_to_complete;
      //! @brief Highest number of continueAlgorithm calls to complete nodes.
      int d_max_conts_to_complete;

      int d_num_nodes_existing;
      //@}
   };

   /*!
    * @brief Construct a root node for a single block.
    *
    * @param common_params  Parameters shares by all nodes in clustering
    * @param box            Global bounding box for a single block
    */
   BergerRigoutsosNode(
      CommonParams* common_params,
      const hier::Box& box);

#if 0
   /*!
    * @brief Construct the root node of a BR dendogram.
    *
    * The root node is used to run the BR algorithm and
    * obtain outputs.
    */
   BergerRigoutsosNode(
      const tbox::Dimension& dim);
#endif

   /*!
    * @brief Destructor.
    *
    * Deallocate internal data.
    */
   virtual ~BergerRigoutsosNode();

   const tbox::Dimension &getDim() const {
      return d_box.getDim();
   }

   //@{

   //! @name Developer's methods for analysis and debugging this class.
   virtual void
   printClassData(
      std::ostream& os,
      int detail_level = 0) const;

   //@}

private:
   /*
    * Static integer constant defining value corresponding to a bad integer.
    */
   static const int BAD_INTEGER;

   /*!
    * @brief Shorthand for the box-graph node corresponding
    * to boxes.
    */
   typedef hier::Box Box;

   //! @brief Shorthand for a container of graph-nodes.
   typedef hier::BoxContainer BoxContainer;

private:

   /*!
    * @brief Construct a non-root node.
    *
    * This is private because the object requires setting up
    * after constructing.  Nodes constructed this way are
    * only meant for internal use by the recursion mechanism.
    */
   BergerRigoutsosNode(
      CommonParams* common_params,
      BergerRigoutsosNode* parent,
      const int child_number);

   /*!
    * @brief Names of algorithmic phases while outside of
    * continueAlgorithm().
    *
    * "For_data_only" phase is when the dendogram node is only used to
    * store data. If the node is to be executed, it enters the
    * "to_be_launched" phase.
    *
    * All names beginning with "reduce", "gather" or "bcast"
    * refer to communication phases, where control is
    * returned before the algorithm completes.
    *
    * The "children" phase does not explicitly contain communication,
    * but the children may perform communication.
    *
    * The "completed" phase is when the algorithm has run to completion.
    * This is where the recursive implementation would return.
    *
    * The "deallocated" phase is for debugging.  This phase is
    * set by the destructor, just to help find dendogram nodes that
    * are deallocated but somehow was referenced.
    */
   enum WaitPhase { for_data_only,
                    to_be_launched,
                    reduce_histogram,
                    bcast_acceptability,
                    gather_grouping_criteria,
                    bcast_child_groups,
                    run_children,
                    bcast_to_dropouts,
                    completed,
                    deallocated };

   /*!
    * @brief MPI tags identifying messages.
    *
    * Each message tag is the d_mpi_tag plus a PhaseTag.
    * Originally, there were different tags for different
    * communication phases, determined by d_mpi_tag plus
    * a PhaseTag.  But this is not really needed,
    * so all phases use the tag d_mpi_tag.  The PhaseTag
    * type is just here in case we have to go back to using
    * them.
    */
   enum PhaseTag { reduce_histogram_tag = 0,
                   bcast_acceptability_tag = 0,
                   gather_grouping_criteria_tag = 0,
                   bcast_child_groups_tag = 0,
                   bcast_to_dropouts_tag = 0,
                   total_phase_tags = 1 };

   /*!
    * @brief Continue the the BR algorithm.
    *
    * Parameters for finding boxes are internal.
    * They should be set in the constructor.
    *
    * In parallel, this the method may return before
    * algorithm is completed.  In serial, no communication
    * is done, so the algorithm IS completed when this
    * method returns.  The method is completed if it
    * returns WaitPhase::completed.  This method may
    * and @em should be called multiple times as long as
    * the algorithm has not completed.
    *
    * If this method returns before the algorithm is
    * complete, this object will have put itself on
    * the leaf queue to be checked for completion later.
    *
    * @return The communication phase currently running.
    *
    * @pre (d_parent == 0) || (d_parent->d_wait_phase != completed)
    * @pre inRelaunchQueue(this) == d_common->d_relaunch_queue.end()
    */
   WaitPhase
   continueAlgorithm();

   /*!
    * @brief Candidate box acceptance state.
    *
    * Note that accepted values are odd and rejected
    * and undetermined values are even!  See boxAccepted(),
    * boxRejected() and boxHasNoTag().
    *
    * It is not critical to have all values shown,
    * but the values help in debugging.
    *
    * Meaning of values:
    * - "hasnotag_by_owner": histogram is truly empty (after sum reduction).
    *   We don't accept the box, but we don't split it either.
    *   (This can only happen at the root dendogram node, as child
    *   boxes are guaranteed to have tags.)
    * - "(rejected|accepted)_by_calculation": decision by calculation
    *   on the owner process.
    * - "(rejected|accepted)_by_owner": decision by owner process,
    *   broadcast to participants.
    * - "(rejected|accepted)_by_recombination": decision by recombination
    *   on local process.
    * - "(rejected|accepted)_by_dropout_bcast": decision by participant group,
    *   broadcast
    *    to the dropout group.
    */
   enum BoxAcceptance { undetermined = -2,
                        hasnotag_by_owner = -1,
                        rejected_by_calculation = 0,
                        accepted_by_calculation = 1,
                        rejected_by_owner = 2,
                        accepted_by_owner = 3,
                        rejected_by_recombination = 4,
                        accepted_by_recombination = 5,
                        rejected_by_dropout_bcast = 6,
                        accepted_by_dropout_bcast = 7 };

   //@{
   //! @name Delegated tasks for various phases of running algorithm.
   void
   makeLocalTagHistogram();

   void
   reduceHistogram_start();

   bool
   reduceHistogram_check();

   void
   computeMinimalBoundingBoxForTags();

   void
   acceptOrSplitBox();

   void
   broadcastAcceptability_start();

   bool
   broadcastAcceptability_check();

   void
   countOverlapWithLocalPatches();

   void
   gatherGroupingCriteria_start();

   bool
   gatherGroupingCriteria_check()
   {
      if (d_group.size() == 1) {
         return true;
      }
      d_comm_group->checkGather();
      /*
       * Do nothing yet with the overlap data d_recv_msg.
       * We extract it in formChildGroups().
       */
      return d_comm_group->isDone();
   }

   //! @brief Form child groups from gathered overlap counts.
   // @pre d_common->d_rank == d_owner
   // @pre d_recv_msg.size() == 4 * d_group.size()
   void
   formChildGroups();

   //! @brief Form child groups from local copy of all level boxes.
   void
   broadcastChildGroups_start();

   bool
   broadcastChildGroups_check();

   void
   runChildren_start();

   bool
   runChildren_check();

   void
   broadcastToDropouts_start();

   bool
   broadcastToDropouts_check();

   void
   createBox();

   void
   eraseBox();

   //! @brief Compute new graph relationships touching local tag nodes.
   // @pre d_common->d_compute_relationships > 0
   // @pre d_accepted_box.getLocalId() >= 0
   // @pre boxAccepted()
   // @pre d_box_acceptance != accepted_by_dropout_bcast
   // @pre (d_parent == 0) || (d_box.numberCells() >= d_common->d_min_box)
   // @pre d_box_acceptance != accepted_by_dropout_bcast
   void
   computeNewNeighborhoodSets();
   //@}

   //@{
   //! @name Utilities for implementing algorithm

   //! @brief Find the index of the owner in the group.
   int
   findOwnerInGroup(
      int owner,
      const CommonParams::VectorOfInts& group) const
   {
      for (unsigned int i = 0; i < group.size(); ++i) {
         if (group[i] == owner) {
            return i;
         }
      }
      return -1;
   }

   //! @brief Claim a unique tag from process's available tag pool.
   // @pre d_mpi_tag < 0
   void
   claimMPITag();

   /*!
    * @brief Heuristically determine "best" tree degree for
    * communication group size.
    */
   int
   computeCommunicationTreeDegree(
      int group_size) const
   {
      int tree_deg = 2;
      int shifted_size = group_size >> 3;
      while (shifted_size > 0) {
         shifted_size >>= 3;
         ++tree_deg;
      }
      return tree_deg;
   }

   void computeGlobalTagDependentVariables();

   bool
   findZeroCutSwath(
      int& cut_lo,
      int& cut_hi,
      const int dim);

   void
   cutAtInflection(
      int& cut_pt,
      int& inflection,
      const int dim);

   int
   getHistogramBufferSize(
      const hier::Box& box) const
   {
      int size = box.numberCells(0);
      int dim_val = d_common->getDim().getValue();
      for (int d = 1; d < dim_val; ++d) {
         size += box.numberCells(d);
      }
      return size;
   }

   int *
   putHistogramToBuffer(
      int* buffer);

   int *
   getHistogramFromBuffer(
      int* buffer);

   int *
   putBoxToBuffer(
      const hier::Box& box,
      int* buffer) const;

   int *
   getBoxFromBuffer(
      hier::Box& box,
      int* buffer) const;

   //! @brief Compute list of non-participating processes.
   // @pre main_group.size() >= sub_group.size()
   void
   computeDropoutGroup(
      const CommonParams::VectorOfInts& main_group,
      const CommonParams::VectorOfInts& sub_group,
      CommonParams::VectorOfInts& dropouts,
      const int add_group) const;

   BoxAcceptance
   intToBoxAcceptance(
      int i) const;

   bool
   boxAccepted() const
   {
      return bool(d_box_acceptance >= 0 && d_box_acceptance % 2);
   }

   bool
   boxRejected() const
   {
      return bool(d_box_acceptance >= 0 && d_box_acceptance % 2 == 0);
   }

   bool
   boxHasNoTag() const
   {
      return bool(d_box_acceptance == -1);
   }
   //@}

   //@{
   //! @name Utilities to help analysis and debugging
   std::list<BergerRigoutsosNode *>::const_iterator
   inRelaunchQueue(
      BergerRigoutsosNode* node_ptr) const
   {
      std::list<BergerRigoutsosNode *>::const_iterator li =
         std::find(d_common->d_relaunch_queue.begin(),
            d_common->d_relaunch_queue.end(),
            node_ptr);
      return li;
   }

   bool
   inGroup(
      CommonParams::VectorOfInts& group,
      int rank = -1) const
   {
      if (rank < 0) {
         rank = d_common->d_mpi_object.getRank();
      }
      for (size_t i = 0; i < group.size(); ++i) {
         if (rank == group[i]) {
            return true;
         }
      }
      return false;
   }

   void
   printState(
      std::ostream& co) const;

   void
   printDendogramState(
      std::ostream& co,
      const std::string& border) const;
   //@}

   /*!
    * @brief Unique id in the binary dendogram.
    *
    * - To have succinct formula, the root dendogram node has d_pos of 1.
    * - Parent id is d_pos/2
    * - Left child id is 2*d_pos
    * - Right child id is 2*d_pos+1
    * - Generation number is ln(d_pos)
    *
    * This parameter is only used for debugging.
    *
    * The id of a node grows exponentially with each generation.
    * If the position in the binary tree is too big to be represented
    * by an integer, d_pos is set to -1 for a left child and -2 for a
    * right child.
    */
   const int d_pos;

   /*!
    * @brief Common parameters shared with descendents and ancestors.
    *
    * Only the root of the tree allocates the common parameters.
    * For all others, this pointer is set by the parent.
    */
   CommonParams* d_common;

   //@{
   /*!
    * @name Tree-related data
    */

   //! @brief Parent node (or NULL for the root node).
   BergerRigoutsosNode* d_parent;

   //! @brief Left child.
   BergerRigoutsosNode* d_lft_child;

   //! @brief Right child.
   BergerRigoutsosNode* d_rht_child;

   //@}

   //@{
   /*!
    * @name Data for one recursion of the BR algorithm
    */

   /*
    * These parameters are listed roughly in order of usage.
    */

   hier::Box d_box;

   /*!
    * @name Id of participating processes.
    */
   CommonParams::VectorOfInts d_group;

   /*!
    * @brief MPI tag for message within a dendogram node.
    *
    * The tag is determined by on the process that owns the parent
    * when the parent decides to split its box.  The tags are broadcasted
    * along with the children boxes.
    */
   int d_mpi_tag;

   /*!
    * @brief Overlap count with d_box.
    */
   int d_overlap;

   /*!
    * @brief Whether and how box is accepted.
    *
    * @see BoxAcceptance.
    */
   BoxAcceptance d_box_acceptance;

   /*!
    * @brief Histogram for all directions of box d_box.
    *
    * If local process is owner, this is initially the
    * local histogram, then later, the reduced histogram.
    * If not, it is just the local histogram.
    */
   CommonParams::VectorOfInts d_histogram[SAMRAI::MAX_DIM_VAL];

   /*!
    * @brief Number of tags in the candidate box.
    */
   int d_num_tags;

   /*!
    * @brief Distributed graph node corresponding to an accepted box.
    *
    * On the owner process, this belongs in a hier::BoxLevel
    * object.  On contributor nodes, this is used to identify the
    * Box assigned by the owner.  The Box is important for
    * computing neighbor data.
    */
   hier::Box d_accepted_box;  // Try to use d_box for this.

   /*!
    * @brief Box iterator corresponding to an accepted box on
    * the owner.
    *
    * This is relevant only on the owner, where the d_box is
    * in a container.  On contributors, the graph node is non-local
    * and stands alone.
    */
   BoxContainer::const_iterator d_box_iterator;

   /*!
    * @brief Name of wait phase when continueAlgorithm()
    * exits before completion.
    */
   WaitPhase d_wait_phase;

   //@}

   //@{
   /*!
    * @name Lower-level parameters for communication.
    */

   //! @brief Buffer for organizing outgoing data.
   CommonParams::VectorOfInts d_send_msg;
   //! @brief Buffer for organizing incoming data.
   CommonParams::VectorOfInts d_recv_msg;

   tbox::AsyncCommGroup* d_comm_group;
   //@}


   //@{
   //! @name Deubgging aid

   /*!
    * @brief Generation number.
    *
    * The generation number is the parent's generation number plus 1.
    * The root has generation number 1.
    */
   const int d_generation;

   //! @brief Number of times continueAlgorithm was called.
   int d_n_cont;

   //@}

   /*
    * Static initialization and cleanup handler.
    */

   static tbox::StartupShutdownManager::Handler
      s_initialize_handler;
};

}
}


#endif  // included_mesh_BergerRigoutsosNode
