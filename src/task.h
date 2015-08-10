#pragma once

#include <condition_variable>
#include <map>
#include <mutex>
#include <list>
#include <thread>

typedef std::pair<int, int> TaskId;

struct Task {
  TaskId id;
  std::function<std::string ()> func;
};

struct TaskRunner {
  void RegisterTask(TaskId id, const std::function<std::string ()>& func);
  std::map<TaskId, std::string> RunAndWait(int cores);

  std::list<Task> tasks;
  std::mutex mtx1;

  std::map<TaskId, std::string> results;
  std::mutex mtx2;
};
