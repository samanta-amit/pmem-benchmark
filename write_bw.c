 #define _GNU_SOURCE
#include "write_bw.h"
#include "bw_benchmark.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libpmem.h>

#include <sys/sysinfo.h>
#include <pthread.h>

#include "common.h"
#include "unrolled_instructions.h"

void write_nt_65536(struct thread_info* t_info){
    struct min_access_info mai;
    set_access_pattern(&mai, t_info);

    const char memdata[64] __attribute__((aligned(64))) = "!!HansGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+-";

    char *t;
    for (t = mai.pmem_start; t < mai.pmem_end && t + t_info->access_distance <= mai.pmem_end; t += mai.stride) {
        char *memaddr = t;
        asm volatile(
        "vmovntdqa  (%[data]), %%zmm0 \n"
        EXEC_8_TIMES(
        WRITE_NT_8192_ASM
        "add $8192, %[addr]\n"
        )
		OPT_FENCE
        : [addr] "+r" (memaddr)
        : [data] "r" (memdata)
        : "%zmm0"
        );
    }
}

void write_nt(struct thread_info* t_info){
	pthread_barrier_wait(t_info->bar);

    switch (t_info->access_distance) {
        case 65536:
            write_nt_65536(t_info);
            return;
    };
    struct min_access_info mai;
    set_access_pattern(&mai, t_info);

    const char memdata[64] __attribute__((aligned(64))) = "!!HansGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+-";

    char *t;
	uint64_t loops = t_info->access_distance / 131072;
	uint64_t count = 0;
    for (t = mai.pmem_start; t < mai.pmem_end && t + t_info->access_distance <= mai.pmem_end; t += mai.stride) {
        char *memaddr = t;

        for (uint64_t i = 0; i < loops; i++) {
            // write 512 Bit / 64 Byte
			if (memaddr+131072 > mai.pmem_end)
				break;
			asm volatile(
            "vmovntdqa  (%[data]), %%zmm0 \n"
            EXEC_16_TIMES(
            WRITE_NT_8192_ASM
            "add $8192, %[addr]\n"
            )
            : [addr] "+r" (memaddr)
            : [data] "r" (memdata)
            : "%zmm0"
            );
			memaddr+=131072;
			count++;
        }
		asm volatile(
			OPT_FENCE
		: : :
		);
    }
}



