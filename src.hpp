// Solution for ACMOJ Problem 2994 - Memo
// Implements CustomNotifyLateEvent::GetNotification and the Memo class.

#ifndef SRC_HPP
#define SRC_HPP

#include <algorithm>
#include <iostream>
#include <queue>
#include <string>
#include <utility>
#include <vector>

#include "event.h"

// Implement CustomNotifyLateEvent notification by delegating to base then appending generator
inline std::string CustomNotifyLateEvent::GetNotification(int n) const {
  // Follow the required three-step generation described in event.h
  // 1) Call base class NotifyLateEvent::GetNotification(n)
  std::string base = NotifyLateEvent::GetNotification(n);
  // 2) Call the generator function pointer
  std::string extra = generator_(n);
  // 3) Concatenate the two parts and return
  return base + extra;
}

class Memo {
 public:
  Memo() = delete;

  // Simulate hours 1..duration (inclusive)
  explicit Memo(int duration) : duration_(duration), current_time_(0), next_index_(0) {}

  // Ensure no leaks in internal structures (STL containers manage memory)
  ~Memo() = default;

  // Add an event to the memo. The memo does not own the event pointer.
  void AddEvent(const Event *event) {
    int idx = next_index_++;
    schedule_initial(event, idx);
  }

  // Advance one hour and emit due notifications (including any backlog not yet emitted)
  void Tick() {
    ++current_time_;

    // Pop and process all notifications whose time <= current_time_
    while (!pq_.empty()) {
      const Node top = pq_.top();
      if (top.time > current_time_) break;
      pq_.pop();

      const Event *e = top.ev;
      if (!e) continue;
      if (e->IsComplete()) {
        // Do not emit or reschedule completed events
        continue;
      }

      try {
        std::cout << e->GetNotification(top.n) << std::endl;
      } catch (const std::exception &) {
        // Invalid n for event type; skip emission and rescheduling
        continue;
      }

      // Reschedule for NotifyLateEvent types after emission
      if (auto nl = dynamic_cast<const NotifyLateEvent *>(e)) {
        int freq = nl->GetFrequency();
        if (freq > 0) {
          Node next{top.time + freq, top.added_index, e, top.n + 1};
          pq_.push(next);
        }
      }
    }
  }

 private:
  struct Node {
    int time;
    int added_index;
    const Event *ev;
    int n;
  };

  struct Cmp {
    bool operator()(const Node &a, const Node &b) const {
      if (a.time != b.time) return a.time > b.time; // min-heap by time
      // Prefer pre-reminder (n==0 for NotifyBeforeEvent) before deadline (n==1)
      if (a.time == b.time) {
        if ((a.n == 0) != (b.n == 0)) {
          return !(a.n == 0); // put n==0 ahead
        }
      }
      if (a.added_index != b.added_index) return a.added_index > b.added_index;
      return a.n > b.n; // ensure stable order for same time
    }
  };

  void schedule_initial(const Event *e, int added_index) {
    if (!e) return;
    if (auto nb = dynamic_cast<const NotifyBeforeEvent *>(e)) {
      int pre_time = e->GetDeadline() - nb->GetNotifyTime();
      // Schedule pre-notification if its time is valid; backlog will be emitted on next ticks
      if (pre_time >= 1) {
        pq_.push(Node{pre_time, added_index, e, 0});
      }
      pq_.push(Node{e->GetDeadline(), added_index, e, 1});
    } else if (auto nl = dynamic_cast<const NotifyLateEvent *>(e)) {
      pq_.push(Node{e->GetDeadline(), added_index, e, 0});
    } else {
      // NormalEvent
      pq_.push(Node{e->GetDeadline(), added_index, e, 0});
    }
  }

  int duration_;
  int current_time_;
  int next_index_;
  std::priority_queue<Node, std::vector<Node>, Cmp> pq_;
};

#endif  // SRC_HPP
