/*****************************************************************************\
 *  cons_common.h - Common function interface for the select/cons_* plugins
 *****************************************************************************
 *  Copyright (C) 2019 SchedMD LLC
 *  Derived in large part from select/cons_[res|tres] plugins
 *
 *  This file is part of Slurm, a resource management program.
 *  For details, see <https://slurm.schedmd.com/>.
 *  Please also read the included file: DISCLAIMER.
 *
 *  Slurm is free software; you can redistribute it and/or modify it under
 *  the terms of the GNU General Public License as published by the Free
 *  Software Foundation; either version 2 of the License, or (at your option)
 *  any later version.
 *
 *  In addition, as a special exception, the copyright holders give permission
 *  to link the code of portions of this program with the OpenSSL library under
 *  certain conditions as described in each individual source file, and
 *  distribute linked combinations including the two. You must obey the GNU
 *  General Public License in all respects for all of the code used other than
 *  OpenSSL. If you modify file(s) with this exception, you may extend this
 *  exception to your version of the file(s), but you are not obligated to do
 *  so. If you do not wish to do so, delete this exception statement from your
 *  version.  If you delete this exception statement from all source files in
 *  the program, then also delete it here.
 *
 *  Slurm is distributed in the hope that it will be useful, but WITHOUT ANY
 *  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *  details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with Slurm; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA.
\*****************************************************************************/

#ifndef _CONS_COMMON_H
#define _CONS_COMMON_H

#include "core_array.h"
#include "src/common/gres.h"
#include "src/slurmctld/slurmctld.h"

/* a partition's per-row core allocation bitmap arrays (1 bitmap per node) */
struct part_row_data {
	struct job_resources **job_list;/* List of jobs in this row */
	uint32_t job_list_size;		/* Size of job_list array */
	uint32_t num_jobs;		/* Number of occupied entries in
					 * job_list array */
	bitstr_t **row_bitmap;		/* contains core bitmap for all jobs in
					 * this row, one bitstr_t for each node
					 * In cons_res only the first ptr is
					 * used.
					 */
};

/* partition core allocation bitmap arrays (1 bitmap per node) */
struct part_res_record {
	struct part_res_record *next; /* Ptr to next part_res_record */
	uint16_t num_rows;	      /* Number of elements in "row" array */
	struct part_record *part_ptr; /* controller part record pointer */
	struct part_row_data *row;    /* array of rows containing jobs */
};

/* per-node resource data */
struct node_res_record {
	uint16_t boards; 	      /* count of boards configured */
	uint16_t cores;		      /* count of cores per socket configured */
	uint16_t cpus;		      /* count of logical processors
				       * configured */
	uint32_t cume_cores;	      /* total cores for all nodes through us */
	uint64_t mem_spec_limit;      /* MB of specialized/system memory */
	struct node_record *node_ptr; /* ptr to the actual node */
	uint64_t real_memory;	      /* MB of real memory configured */
	uint16_t sockets;	      /* count of sockets per board configured*/
	uint16_t threads;	      /* count of hyperthreads per core */
	uint16_t tot_cores;	      /* total cores per node */
	uint16_t tot_sockets;	      /* total sockets per node */
	uint16_t vpus;		      /* count of virtual processors configure
				       * this could be the physical threads
				       * count or could be the core count if
				       * the node's cpu count matches the
				       * core count */
};

/* per-node resource usage record */
struct node_use_record {
	uint64_t alloc_memory;	      /* real memory reserved by already
				       * scheduled jobs */
	List gres_list;		      /* list of gres_node_state_t records as
				       * defined in in src/common/gres.h.
				       * Local data used only in state copy
				       * to emulate future node state */
	uint16_t node_state;	      /* see node_cr_state comments */
};

typedef struct avail_res {	/* Per-node resource availability */
	uint16_t avail_cpus;	/* Count of available CPUs */
	uint16_t avail_gpus;	/* Count of available GPUs */
	uint16_t avail_res_cnt;	/* Count of available CPUs + GPUs */
	uint16_t *avail_cores_per_sock;	/* Per-socket available core count */
	uint16_t max_cpus;	/* Maximum available CPUs */
	uint16_t min_cpus;	/* Minimum allocated CPUs */
	uint16_t sock_cnt;	/* Number of sockets on this node */
	List sock_gres_list;	/* Per-socket GRES availability, sock_gres_t */
	uint16_t spec_threads;	/* Specialized threads to be reserved */
	uint16_t vpus;		/* Virtual processors (CPUs) per core */
} avail_res_t;

struct select_nodeinfo {
	uint16_t magic;		/* magic number */
	uint16_t alloc_cpus;
	uint64_t alloc_memory;
	uint64_t *tres_alloc_cnt;	/* array of tres counts allocated.
					   NOT PACKED */
	char     *tres_alloc_fmt_str;	/* formatted str of allocated tres */
	double    tres_alloc_weighted;	/* weighted number of tres allocated. */
};

typedef struct {
	avail_res_t *(*can_job_run_on_node)(struct job_record *job_ptr,
					    bitstr_t **core_map,
					    const uint32_t node_i,
					    uint32_t s_p_n,
					    struct node_use_record *node_usage,
					    uint16_t cr_type,
					    bool test_only,
					    bitstr_t **part_core_map);
	int (*choose_nodes)(struct job_record *job_ptr, bitstr_t *node_map,
			    bitstr_t **avail_core, uint32_t min_nodes,
			    uint32_t max_nodes, uint32_t req_nodes,
			    avail_res_t **avail_res_array, uint16_t cr_type,
			    bool prefer_alloc_nodes,
			    gres_mc_data_t *tres_mc_ptr);
	bitstr_t **(*mark_avail_cores)(bitstr_t *node_map, uint16_t core_spec);
	int (*dist_tasks_compute_c_b)(struct job_record *job_ptr,
				      uint32_t *gres_task_limit);
} cons_common_callbacks_t;

/* Global common variables */
extern bool     backfill_busy_nodes;
extern int      bf_window_scale;
extern cons_common_callbacks_t cons_common_callbacks;
extern int      core_array_size;
extern uint16_t cr_type;
extern bool     gang_mode;
extern bool     have_dragonfly;
extern bool     is_cons_tres;
extern const uint16_t nodeinfo_magic;
extern bool     pack_serial_at_end;
extern const uint32_t plugin_id;
extern const char *plugin_type;
extern bool     preempt_by_part;
extern bool     preempt_by_qos;
extern uint16_t priority_flags;
extern uint64_t select_debug_flags;
extern uint16_t select_fast_schedule;
extern int      select_node_cnt;
extern bool     spec_cores_first;
extern bool     topo_optional;
extern const char *plugin_type;

extern struct part_res_record *select_part_record;
extern struct node_res_record *select_node_record;
extern struct node_use_record *select_node_usage;

/* Delete the given partition row data */
extern void common_destroy_row_data(
	struct part_row_data *row, uint16_t num_rows);

extern void common_free_avail_res(avail_res_t *avail_res);
extern void common_free_avail_res_array(avail_res_t **avail_res_array);

/*
 * common_build_row_bitmaps: A job has been removed from the given partition,
 *                           so the row_bitmap(s) need to be reconstructed.
 *                           Optimize the jobs into the least number of rows,
 *                           and make the lower rows as dense as possible.
 *
 * IN p_ptr - the partition that has jobs to be optimized
 * IN job_ptr - pointer to single job removed, pass NULL to completely rebuild
 */
extern void common_build_row_bitmaps(struct part_res_record *p_ptr,
				     struct job_record *job_ptr);

/* Determine how many cpus per core we can use */
extern int common_cpus_per_core(struct job_details *details, int node_inx);

/*
 * Add job resource use to the partition data structure
 */
extern void common_add_job_to_row(struct job_resources *job,
				  struct part_row_data *r_ptr);

/*
 * allocate resources to the given job
 * - add 'struct job_resources' resources to 'struct part_res_record'
 * - add job's memory requirements to 'struct node_res_record'
 *
 * if action = 0 then add cores, memory + GRES (starting new job)
 * if action = 1 then add memory + GRES (adding suspended job at restart)
 * if action = 2 then only add cores (suspended job is resumed)
 *
 *
 * See also: common_rm_job_res()
 */
extern int common_add_job_to_res(struct job_record *job_ptr, int action);

/*
 * Deallocate resources previously allocated to the given job
 * - subtract 'struct job_resources' resources from 'struct part_res_record'
 * - subtract job's memory requirements from 'struct node_res_record'
 *
 * if action = 0 then subtract cores, memory + GRES (running job was terminated)
 * if action = 1 then subtract memory + GRES (suspended job was terminated)
 * if action = 2 then only subtract cores (job is suspended)
 * IN: job_fini - job fully terminating on this node (not just a test)
 *
 * RET SLURM_SUCCESS or error code
 *
 * See also: common_add_job_to_res()
 */
extern int common_rm_job_res(struct part_res_record *part_record_ptr,
			     struct node_use_record *node_usage,
			     struct job_record *job_ptr, int action,
			     bool job_fini);

/*
 * Add job resource allocation to record of resources allocated to all nodes
 * IN job_resrcs_ptr - resources allocated to a job
 * IN/OUT sys_resrcs_ptr - bitmap array (one per node) of available cores,
 *			   allocated as needed
 * NOTE: Patterned after add_job_to_cores() in src/common/job_resources.c
 */
extern void common_add_job_cores(job_resources_t *job_resrcs_ptr,
				 bitstr_t ***sys_resrcs_ptr);

/*
 * Remove job resource allocation to record of resources allocated to all nodes
 * IN job_resrcs_ptr - resources allocated to a job
 * IN/OUT full_bitmap - bitmap of available CPUs, allocate as needed
 * NOTE: Patterned after remove_job_from_cores() in src/common/job_resources.c
 */
extern void common_rm_job_cores(job_resources_t *job_resrcs_ptr,
				bitstr_t ***sys_resrcs_ptr);

/*
 * Test if job can fit into the given set of core_bitmaps
 * IN job_resrcs_ptr - resources allocated to a job
 * IN r_ptr - row we are trying to fit
 * RET 1 on success, 0 otherwise
 * NOTE: Patterned after job_fits_into_cores() in src/common/job_resources.c
 */
extern int common_job_fit_in_row(job_resources_t *job_resrcs_ptr,
				 struct part_row_data *r_ptr);

/* Log contents of partition structure */
extern void common_dump_parts(struct part_res_record *p_ptr);

/* sort the rows of a partition from "most allocated" to "least allocated" */
extern void common_sort_part_rows(struct part_res_record *p_ptr);

/* Create a duplicate part_row_data struct */
extern struct part_row_data *common_dup_row_data(struct part_row_data *orig_row,
						 uint16_t num_rows);

extern void common_init(void);
extern void common_fini(void);

/*
 * Bit a core bitmap array of available cores
 * node_bitmap IN - Nodes available for use
 * core_spec IN - Specialized core specification, NO_VAL16 if none
 * RET core bitmap array, one per node. Use free_core_array() to release memory
 */
extern bitstr_t **common_mark_avail_cores(
	bitstr_t *node_bitmap, uint16_t core_spec);

/*
 * common_allocate_cores - Given the job requirements, determine which cores
 *                   from the given node can be allocated (if any) to this
 *                   job. Returns the number of cpus that can be used by
 *                   this node AND a bitmap of the selected cores.
 *
 * IN job_ptr       - pointer to job requirements
 * IN/OUT core_map  - core_bitmap of available cores on this node
 * IN part_core_map - bitmap of cores already allocated on this partition/node
 * IN node_i        - index of node to be evaluated
 * IN/OUT cpu_alloc_size - minimum allocation size, in CPUs
 * IN cpu_type      - if true, allocate CPUs rather than cores
 * IN req_sock_map - OPTIONAL bitmap of required sockets
 * RET resource availability structure, call _free_avail_res() to free
 */
extern avail_res_t *common_allocate_cores(struct job_record *job_ptr,
					  bitstr_t *core_map,
					  bitstr_t *part_core_map,
					  const uint32_t node_i,
					  int *cpu_alloc_size,
					  bool cpu_type,
					  bitstr_t *req_sock_map);

/*
 * common_allocate_sockets - Given the job requirements, determine which sockets
 *                     from the given node can be allocated (if any) to this
 *                     job. Returns the number of cpus that can be used by
 *                     this node AND a core-level bitmap of the selected
 *                     sockets.
 *
 * IN job_ptr       - pointer to job requirements
 * IN/OUT core_map  - core_bitmap of available cores on this node
 * IN part_core_map - bitmap of cores already allocated on this partition/node
 * IN node_i        - index of node to be evaluated
 * IN/OUT cpu_alloc_size - minimum allocation size, in CPUs
 * IN req_sock_map - OPTIONAL bitmap of required sockets
 * RET resource availability structure, call _free_avail_res() to free
 */
extern avail_res_t *common_allocate_sockets(struct job_record *job_ptr,
					    bitstr_t *core_map,
					    bitstr_t *part_core_map,
					    const uint32_t node_i,
					    int *cpu_alloc_size,
					    bitstr_t *req_sock_map);

/*
 * common_job_test - Given a specification of scheduling requirements,
 *	identify the nodes which "best" satisfy the request.
 * 	"best" is defined as either a minimal number of consecutive nodes
 *	or if sharing resources then sharing them with a job of similar size.
 * IN/OUT job_ptr - pointer to job being considered for initiation,
 *                  set's start_time when job expected to start
 * IN/OUT bitmap - usable nodes are set on input, nodes not required to
 *	satisfy the request are cleared, other left set
 * IN min_nodes - minimum count of nodes
 * IN req_nodes - requested (or desired) count of nodes
 * IN max_nodes - maximum count of nodes (0==don't care)
 * IN mode - SELECT_MODE_RUN_NOW   (0): try to schedule job now
 *           SELECT_MODE_TEST_ONLY (1): test if job can ever run
 *           SELECT_MODE_WILL_RUN  (2): determine when and where job can run
 * IN preemptee_candidates - List of pointers to jobs which can be preempted.
 * IN/OUT preemptee_job_list - Pointer to list of job pointers. These are the
 *		jobs to be preempted to initiate the pending job. Not set
 *		if mode=SELECT_MODE_TEST_ONLY or input pointer is NULL.
 * RET zero on success, EINVAL otherwise
 * globals (passed via select_p_node_init):
 *	node_record_count - count of nodes configured
 *	node_record_table_ptr - pointer to global node table
 * NOTE: the job information that is considered for scheduling includes:
 *	req_node_bitmap: bitmap of specific nodes required by the job
 *	contiguous: allocated nodes must be sequentially located
 *	num_cpus: minimum number of processors required by the job
 * NOTE: bitmap must be a superset of req_nodes at the time that
 *	select_p_job_test is called
 */
extern int common_job_test(struct job_record *job_ptr, bitstr_t * bitmap,
			   uint32_t min_nodes, uint32_t max_nodes,
			   uint32_t req_nodes, uint16_t mode,
			   List preemptee_candidates,
			   List *preemptee_job_list,
			   bitstr_t **exc_cores);

#endif /* _CONS_COMMON_H */
