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

#define CHECKR(condition,message)\
        if(condition)\
        {\
            perror(message);\
            exit(EXIT_FAILURE);\
        }

#define INTEGRAL_START 0.0
#define INTEGRAL_END 300.0
#define INTEGRAL_DX 1e-6

#define INTEGRAL_MAXTHREADS CPU_SETSIZE

#define MAX(A, B) (( (A) > (B) ) ? (A) : (B))

struct thread_info
{
    double start;
    double end;
    double res;
};

double func(double x)
{
    return sin(x);
}

void *thread_cb(void *data)
{
    struct thread_info *tinfo = data;

    for (double x = tinfo->start; x < tinfo->end; x += INTEGRAL_DX)
        tinfo->res += func(x) * INTEGRAL_DX;

    return 0;
}

int main(int argc, char **argv)
{
    CHECKR((argc != 2), "wrong amount of args")

    char *endptr;
    unsigned long nthreads = strtoul(argv[1], &endptr, 10);
    CHECKR(*endptr, "wrong argument")

    double res = 0.0;
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

    double step = (INTEGRAL_END - INTEGRAL_START) / (double) nthreads;
    double start = 0.0;
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

        if (i < nthreads)
            res += ((struct thread_info *)(buf + i * alignment))->res;
    }

    printf("Result: %lg\n", res);

    free(buf);
    free(thread);
}
