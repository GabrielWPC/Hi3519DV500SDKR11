/*
  Copyright (c), 2001-2024, Shenshu Tech. Co., Ltd.
 */

#ifndef OT_MPI_SYS_MEM_INNER_H
#define OT_MPI_SYS_MEM_INNER_H

#include "ot_type.h"

#ifdef __cplusplus
extern "C" {
#endif

td_s32 ot_mpi_sys_mmz_check_phy_addr(td_phys_addr_t phys_addr, td_ulong size);

td_s32 ot_mpi_sys_mmz_alloc_only(td_phys_addr_t *phys_addr, const td_char *mmz_name,
    const td_char *buf_name, td_ulong size);

td_s32 ot_mpi_sys_mmz_free_only(td_phys_addr_t phys_addr, const td_void *virt_addr);

td_void *ot_mpi_sys_mmz_remap_nocache(td_phys_addr_t phys_addr, td_u32 size);

td_void *ot_mpi_sys_mmz_remap_cached(td_phys_addr_t phys_addr, td_u32 size);

td_s32 ot_mpi_sys_mmz_unmap(const td_void *virt_addr);

#ifdef __cplusplus
}
#endif

#endif /* OT_MPI_SYS_MEM_INNER_H */
