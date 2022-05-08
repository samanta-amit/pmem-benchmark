#define _GNU_SOURCE
#include "read_bw.h"
#include "bw_benchmark.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libpmem.h>

#include <sys/sysinfo.h>
#include <pthread.h>
#include <sys/time.h>

#include "common.h"
#include "unrolled_instructions.h"

void read_nt_random_65536(struct thread_info* t_info){
    uint64_t ac = t_info->pmem_size / t_info->access_distance;
	uint64_t ad = t_info->access_distance;
	char *start_addr = t_info->thread_start_addr;
	char *mem_addr = start_addr;
	for (uint64_t i = 0; i < t_info->accesses_per_thread; i+=ACCESSES_PER_LOOP) {
		char *next_addr;
		UNROLL_OPT(
        asm volatile(
			READ_NT_8192_ASM
			READ_NEXT_ADDR
			READ_ADDR_ADD_8192
			READ_NT_8192_ASM
			READ_ADDR_ADD_8192
			READ_NT_8192_ASM
			READ_ADDR_ADD_8192
			READ_NT_8192_ASM
			READ_ADDR_ADD_8192
			READ_NT_8192_ASM
			READ_ADDR_ADD_8192
			READ_NT_8192_ASM
			READ_ADDR_ADD_8192
			READ_NT_8192_ASM
			READ_ADDR_ADD_8192
			READ_NT_8192_ASM
            : [next_addr] "=r" (next_addr), [addr] "+r" (mem_addr)
            :
            : "%zmm0", "%xmm0"
        );
		mem_addr = next_addr;
		)
	}
}

void read_nt_random(struct thread_info* t_info){
	pthread_barrier_wait(t_info->bar);

    switch (t_info->access_distance) {
        case 65536:
            read_nt_random_65536(t_info);
            return;
    };

    uint64_t ac = t_info->pmem_size / t_info->access_distance;
	uint64_t ad = t_info->access_distance;
	char *start_addr = t_info->thread_start_addr;
	char *mem_addr = start_addr;
	for (uint64_t i = 0; i < t_info->accesses_per_thread; i+=ACCESSES_PER_LOOP) {
		char *next_addr;
		uint64_t loops_per_access = ad / 65536;
		UNROLL_OPT(
		asm volatile(
			READ_NT_8192_ASM
			READ_NEXT_ADDR
			READ_NT_8192_ASM
			READ_NT_8192_ASM
			READ_NT_8192_ASM
			READ_NT_8192_ASM
			READ_NT_8192_ASM
			READ_NT_8192_ASM
			READ_NT_8192_ASM
			: [next_addr] "=r" (next_addr)
			: [addr] "r" (mem_addr)
			: "%zmm0", "%xmm0"
		);
		mem_addr = next_addr;
		for(int i = 1; i < loops_per_access; i++) {
			asm volatile(
				READ_NT_8192_ASM
				READ_NT_8192_ASM
				READ_NT_8192_ASM
				READ_NT_8192_ASM
				READ_NT_8192_ASM
				READ_NT_8192_ASM
				READ_NT_8192_ASM
				READ_NT_8192_ASM
				: [next_addr] "=r" (next_addr)
				: [addr] "r" (mem_addr)
				: "%zmm0", "%xmm0"
			);
		}
		)
	}
}
