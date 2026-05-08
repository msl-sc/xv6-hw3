#include "types.h"
#include "stat.h"
#include "fcntl.h"
#include "user.h"
#include "x86.h"

#define PGSIZE 4096
char*
strcpy(char *s, const char *t)
{
  char *os;

  os = s;
  while((*s++ = *t++) != 0)
    ;
  return os;
}

int
strcmp(const char *p, const char *q)
{
  while(*p && *p == *q)
    p++, q++;
  return (uchar)*p - (uchar)*q;
}

uint
strlen(const char *s)
{
  int n;

  for(n = 0; s[n]; n++)
    ;
  return n;
}

void*
memset(void *dst, int c, uint n)
{
  stosb(dst, c, n);
  return dst;
}

char*
strchr(const char *s, char c)
{
  for(; *s; s++)
    if(*s == c)
      return (char*)s;
  return 0;
}

char*
gets(char *buf, int max)
{
  int i, cc;
  char c;

  for(i=0; i+1 < max; ){
    cc = read(0, &c, 1);
    if(cc < 1)
      break;
    buf[i++] = c;
    if(c == '\n' || c == '\r')
      break;
  }
  buf[i] = '\0';
  return buf;
}

int
stat(const char *n, struct stat *st)
{
  int fd;
  int r;

  fd = open(n, O_RDONLY);
  if(fd < 0)
    return -1;
  r = fstat(fd, st);
  close(fd);
  return r;
}

int
atoi(const char *s)
{
  int n;

  n = 0;
  while('0' <= *s && *s <= '9')
    n = n*10 + *s++ - '0';
  return n;
}

void*
memmove(void *vdst, const void *vsrc, int n)
{
  char *dst;
  const char *src;

  dst = vdst;
  src = vsrc;
  while(n-- > 0)
    *dst++ = *src++;
  return vdst;
}


void lock_init(lock_t *lock) {
  lock->flag = 0;//0 is unlocked, 1 is locked
}

void lock_acquire(lock_t *lock) {
  while(xchg(&lock->flag, 1) != 0)
    ; // spin
}

void lock_release(lock_t *lock) {
  asm volatile("movl $0, %0" : "+m" (lock->flag) : : "cc");
}

int thread_create(void(*start_routine)(void *, void *), void *arg1, void *arg2) {
  void *stack = malloc(2 * PGSIZE); // 2 pages for stack
  if(stack == 0){
    return -1; // allocation failed
  }
  void *aligned_stack = (void *)(((uint)stack + PGSIZE - 1) & ~(PGSIZE - 1)); // align to page boundary
  *(void **)aligned_stack = stack; // save original pointer for free in thread_exit
  
int pid = clone(start_routine, arg1, arg2, aligned_stack);
  if(pid < 0){
    free(stack); // free on failure
    return -1;
  }
  return pid;
}

int thread_join(void){
  void *stack_top;
  int pid = join(&stack_top);
  if(pid < 0){
    return -1; // join failed
  }
  if(stack_top != 0){
    // stack_top points to the aligned stack used by the thread, which is at the top of the allocated 2-page region
    void *aligned_stack = stack_top - PGSIZE;
    void *malloc_addr = *(void**)aligned_stack;
    // free the original malloc'd address, which is the start of the 2-page region
    free(malloc_addr);
  }
  return pid;
}
