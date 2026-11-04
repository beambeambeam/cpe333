/*
 * uffdblock - keep one page fault blocked inside handle_mm_fault().
 * A thread touches a page registered with userfaultfd, and nobody serves
 * the fault, so the thread waits in the kernel until the process is killed.
 * Used by tests/regress.sh to unload pfcount while a fault is in flight.
 * Needs root (or vm.unprivileged_userfaultfd=1).
 */
#include <fcntl.h>
#include <linux/userfaultfd.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <unistd.h>

static void *toucher(void *p)
{
	*(volatile char *)p = 1;	/* blocks: missing page, no handler */
	return NULL;
}

int main(void)
{
	long page = sysconf(_SC_PAGESIZE);
	int uffd = syscall(SYS_userfaultfd, O_CLOEXEC);
	struct uffdio_api api = { .api = UFFD_API };
	char *p = mmap(NULL, page, PROT_READ | PROT_WRITE,
		       MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	struct uffdio_register reg = {
		.range = { .start = (unsigned long)p, .len = page },
		.mode = UFFDIO_REGISTER_MODE_MISSING,
	};
	pthread_t t;

	if (uffd < 0 || ioctl(uffd, UFFDIO_API, &api) ||
	    ioctl(uffd, UFFDIO_REGISTER, &reg)) {
		perror("userfaultfd");
		return 1;
	}
	usleep(200000);	/* time for the caller to select this PID */
	pthread_create(&t, NULL, toucher, p);
	pause();
	return 0;
}
