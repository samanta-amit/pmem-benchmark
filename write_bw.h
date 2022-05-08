#ifndef SCHEDULING_H
#define SCHEDULING_H

#include "common.h"

#ifdef SFENCE_AFTER_STORE
	#define OPT_FENCE "sfence \n"
#endif

void write_nt(struct thread_info* t_info);

#endif