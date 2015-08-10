#include "task.h"
#include <vector>
using namespace std;

void TaskRunner::RegisterTask(TaskId id, const std::function<std::string ()>& func) {
  unique_lock<mutex> lock(mtx1);
  Task t = { id, func };
  tasks.push_back(t);
}

map<TaskId, string> TaskRunner::RunAndWait(int cores) {
  std::vector<std::thread> threads(cores);

  auto f = [this]() {
    for (;;) {
      Task task;

      // pop task
      {
        unique_lock<mutex> lock(mtx1);
        if (tasks.empty())
          return;
        task = tasks.front(); tasks.pop_front();
      }

      // run task
      string ret = task.func();

      // save result
      {
        unique_lock<mutex> lock(mtx2);
        results[task.id] = ret;
      }
    }
  };

  for (int i = 0; i < cores; ++i) {
    threads[i] = thread(f);
  }

  for (auto& th : threads) {
    th.join();
  }

  {
    unique_lock<mutex> lock(mtx2);
    return results;
  }
}
