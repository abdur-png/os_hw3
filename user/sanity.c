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
  // Your code can start here
  exit();
}
