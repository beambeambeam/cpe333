/*
 * faultgen - generate a known number of page faults, step by step.
 *
 * Usage: faultgen STEP...
 *   anon:MB     mmap MB of anonymous memory and touch every page
 *               -> adds 1 region, ~MB*256 minor faults (4 KiB pages)
 *   thp:MB      like anon, but ask for 2 MB transparent huge pages
 *               -> ~MB/2 minor faults
 *   file:PATH   drop PATH from the page cache, mmap it, read every page
 *               -> adds 1 region, ~1 major fault per page
 *   filewarm:PATH  same, but keep the page cache -> minor faults only
 *   fileseq:PATH   cold cache, readahead + fault-around left on
 *   free        munmap everything mapped so far
 *   sleep:SEC   sleep
 *   wait        wait for Enter
 *   loop        restart from the first step (runs until killed)
 *
 * Example: faultgen sleep:3 anon:64 sleep:3 file:/tmp/big.bin sleep:3 free wait
 */
#define _GNU_SOURCE
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_MAPS 64

static struct { void *addr; size_t len; } maps[MAX_MAPS];
static int nmaps;

static void report(const char *what)
{
	struct rusage ru;

	getrusage(RUSAGE_SELF, &ru);
	printf("[faultgen %d] %-28s minflt=%ld majflt=%ld\n",
	       getpid(), what, ru.ru_minflt, ru.ru_majflt);
	fflush(stdout);
}

static void track(void *addr, size_t len)
{
	if (nmaps == MAX_MAPS) {
		fprintf(stderr, "too many mappings\n");
		exit(1);
	}
	maps[nmaps].addr = addr;
	maps[nmaps].len = len;
	nmaps++;
}

static void do_anon(long mb, int thp)
{
	size_t len = (size_t)mb << 20, page = sysconf(_SC_PAGESIZE);
	char *p = mmap(NULL, len, PROT_READ | PROT_WRITE,
		       MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	char label[64];

	if (p == MAP_FAILED) {
		perror("mmap anon");
		exit(1);
	}
	/* Without THP each 4 KiB page is one minor fault. */
	madvise(p, len, thp ? MADV_HUGEPAGE : MADV_NOHUGEPAGE);
	for (size_t off = 0; off < len; off += page)
		p[off] = 1;
	track(p, len);
	snprintf(label, sizeof(label), "%s %ld MB (%zu pages)",
		 thp ? "thp" : "anon", mb, len / page);
	report(label);
}

static void do_file(const char *path, int evict, int random)
{
	size_t page = sysconf(_SC_PAGESIZE);
	volatile char sink = 0;
	struct stat st;
	char label[64];
	char *p;
	int fd = open(path, O_RDONLY);

	if (fd < 0 || fstat(fd, &st) < 0) {
		perror(path);
		exit(1);
	}
	/* Evict the file from the page cache so the next access must do I/O. */
	if (evict)
		posix_fadvise(fd, 0, 0, POSIX_FADV_DONTNEED);
	p = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	close(fd);
	if (p == MAP_FAILED) {
		perror("mmap file");
		exit(1);
	}
	/* Turn off readahead. Fault-around stays on, but with a cold cache it
	 * finds no neighbor pages to map, so each page takes one major fault. */
	if (random)
		madvise(p, st.st_size, MADV_RANDOM);
	for (off_t off = 0; off < st.st_size; off += page)
		sink += p[off];
	track(p, st.st_size);
	snprintf(label, sizeof(label), "file%s %lld KB (%lld pages)",
		 !evict ? "warm" : !random ? "seq" : "",
		 (long long)st.st_size >> 10, (long long)(st.st_size + page - 1) / page);
	report(label);
}

static void do_free(void)
{
	for (int i = 0; i < nmaps; i++)
		munmap(maps[i].addr, maps[i].len);
	nmaps = 0;
	report("free");
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		fprintf(stderr,
			"usage: %s anon:MB | thp:MB | file[warm|seq]:PATH | free | sleep:SEC | wait | loop ...\n",
			argv[0]);
		return 2;
	}
	report("start");

	for (int i = 1; i < argc; i++) {
		const char *s = argv[i];

		if (!strncmp(s, "anon:", 5)) {
			do_anon(atol(s + 5), 0);
		} else if (!strncmp(s, "thp:", 4)) {
			do_anon(atol(s + 4), 1);
		} else if (!strncmp(s, "file:", 5)) {
			do_file(s + 5, 1, 1);
		} else if (!strncmp(s, "filewarm:", 9)) {
			do_file(s + 9, 0, 1);
		} else if (!strncmp(s, "fileseq:", 8)) {
			do_file(s + 8, 1, 0);
		} else if (!strcmp(s, "free")) {
			do_free();
		} else if (!strncmp(s, "sleep:", 6)) {
			sleep(atoi(s + 6));
		} else if (!strcmp(s, "wait")) {
			printf("[faultgen %d] press Enter...\n", getpid());
			fflush(stdout);
			getchar();
		} else if (!strcmp(s, "loop")) {
			i = 0;
		} else {
			fprintf(stderr, "unknown step: %s\n", s);
			return 2;
		}
	}
	report("done");
	return 0;
}
