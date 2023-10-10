#include "user.h"

// Fibonacci sequence
int fib(int n) {
  if(n <= 1) {
    return 1;
  }
  return fib(n - 1) + fib(n - 2);
}

// Use this for long computation
// fib(39) takes roughly 30 secs
void longComputation() {
  fib(39);
}


int main(int argc, char* argv[]) {
  int NCHILD = 20;
  int pid;
  int creation_time, termination_time, ready_time, running_time, sleep_time;
  int total_wait_time = 0, total_run_time = 0, total_sleep_time = 0, total_turnaround_time = 0;

  for (int i = 0; i < NCHILD; i++) {
    pid = fork();
    if (pid == 0) { // Child process
      longComputation();
      exit();
    }
  }

  for (int i = 0; i < NCHILD; i++) {
    pid = wait_stat(&creation_time, &termination_time, &ready_time, &running_time, &sleep_time);
    printf("PID[%d] Creation[%d] Termination[%d] Ready Time[%d] Running Time[%d] Sleep Time[%d]\n",
           pid, creation_time, termination_time, ready_time, running_time, sleep_time);
    total_wait_time += ready_time;
    total_run_time += running_time;
    total_sleep_time += sleep_time;
    total_turnaround_time += (termination_time - creation_time);
  }

  printf("Avg. Wait Time[%d] Avg. Running Time[%d] Avg. Sleep Time[%d] Avg. Turnaround Time[%d]\n",
         total_wait_time / NCHILD, total_run_time / NCHILD, total_sleep_time / NCHILD, total_turnaround_time / NCHILD);

  exit();
}
