#include "types.h"
#include "stat.h"
#include "user.h"

lock_t lock;
int counter = 0;

// 线程执行函数
void thread_func(void *arg1, void *arg2) {
  int i;
  int loop_cnt = *(int*)arg1;
  for(i = 0; i < loop_cnt; i++) {
    lock_acquire(&lock);
    counter++;
    lock_release(&lock);
  }
  exit();
}

int main(int argc, char *argv[]) {
  int loop = 1000;
  int thread_num = 4;
  int i;

  printf(1, "=== xv6 thread test start ===\n");
  lock_init(&lock);

  // 创建线程
  for(i = 0; i < thread_num; i++) {
    int pid = thread_create(thread_func, &loop, 0);
    if(pid < 0) {
      printf(1, "thread %d create failed\n", i);
      exit();
    }
    printf(1, "created thread %d, pid: %d\n", i, pid);
  }

  // 等待所有线程退出
  for(i = 0; i < thread_num; i++) {
    int pid = thread_join();
    if(pid < 0) {
      printf(1, "thread join failed\n");
      exit();
    }
    printf(1, "joined thread pid: %d\n", pid);
  }

  // 验证结果
  printf(1, "\nfinal counter: %d\n", counter);
  printf(1, "expected value: %d\n", loop * thread_num);
  if(counter == loop * thread_num) {
    printf(1, "=== test passed ===\n");
  } else {
    printf(1, "=== test failed ===\n");
  }

  exit();
}