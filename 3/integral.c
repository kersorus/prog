#define _GNU_SOURCE
#include <sys/sysinfo.h>
#include <fcntl.h>
#include <error.h>
#include <unistd.h>
#include <sched.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include <errno.h>
#include <math.h>

#include "config.h"
#include "debug.h"

#define MAX(A, B) (( (A) > (B) ) ? (A) : (B))

struct thread_info
{
    double start;
    double end;
    double res;
};

static void *thread_cb(void *data)
{
    struct thread_info *tinfo = data;

    for (double x = tinfo->start; x < tinfo->end; x += DX)
        tinfo->res += FUNC(x) * DX;

    return 0;
}

int compute_integral(double integral_start, double integral_end, size_t nthreads, double *result)
{
    assert(result);

    unsigned long nprocs = get_nprocs();
    errno = 0;

    pthread_t *thread = calloc(MAX(nprocs, nthreads), sizeof(*thread));
    CHECKR(errno, "calloc() error")

    pthread_attr_t attr;

    errno = pthread_attr_init(&attr);
    CHECKR(errno, "pthread_attr_init() error")

    size_t alignment = sysconf(_SC_PAGESIZE);
    uint8_t *buf = NULL;
    errno = posix_memalign((void **) &buf, alignment, alignment * MAX(nprocs, nthreads));
    CHECKR(errno, "posix_memalign() error")

    double step = (integral_end - integral_start) / (double) nthreads;
    double start = integral_start;
    for (unsigned long i = 0; i < MAX(nthreads, nprocs); i++, start += step)
    {
        cpu_set_t set;

        unsigned long cpu = 0;
        if (((i * 2) / nprocs) % 2)
            cpu = ((i * 2) + 1) % nprocs;
        else
            cpu = (i * 2) % nprocs;

        CPU_ZERO(&set);
        CPU_SET(cpu, &set);

        errno = pthread_attr_setaffinity_np(&attr, sizeof(set), &set);
        CHECKR(errno, "pthread_attr_setaffinity_np() error")

        struct thread_info *tinfo = (struct thread_info *)(buf + i * alignment);
        tinfo->res = 0;
        tinfo->start = start;
        tinfo->end = start + step;

        errno = pthread_create(thread + i, &attr, thread_cb, tinfo);
        CHECKR(errno, "pthread_create() error")
    }

    errno = pthread_attr_destroy(&attr);
    CHECKR(errno, "pthread_attr_destroy() error")

    for (unsigned long i = 0; i < MAX(nthreads, nprocs); i++)
    {
        errno = pthread_join(thread[i], NULL);
        CHECKR(errno, "pthread_join() error")

        struct thread_info *tinfo = (struct thread_info *) (buf + i * alignment);

        printf("[%lg, %lg] = %lg\n", tinfo->start, tinfo->end, tinfo->res);

        if (i < nthreads)
            *result += tinfo->res;
    }

    free(buf);
    free(thread);

    return 0;
}
