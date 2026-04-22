// Solution for ACMOJ Problem 2994 - Memo
// Implements CustomNotifyLateEvent::GetNotification and the Memo class.

#ifndef SRC_HPP
#define SRC_HPP

#include <algorithm>
#include <iostream>
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
    Tracked t;
    t.ev = event;
    t.added_index = next_index_++;
    t.normal_notified = false;
    t.nb_pre_notified = false;
    t.nb_deadline_notified = false;
    t.late_last_n = -1;  // none sent yet
    events_.push_back(t);
  }

  // Advance one hour and emit due notifications (including any backlog not yet emitted)
  void Tick() {
    if (current_time_ < duration_) {
      ++current_time_;
    } else {
      // Even if duration reached, we still advance conceptual time to process late events, if any
      ++current_time_;
    }

    struct Item {
      int scheduled_time;
      int added_index;
      const Event *ev;
      int n;  // parameter to GetNotification
    };

    std::vector<Item> items;
    items.reserve(events_.size());

    for (auto &te : events_) {
      const Event *e = te.ev;
      if (!e) continue;
      if (e->IsComplete()) {
        // Do not notify completed events
        continue;
      }

      // Try NotifyBeforeEvent first (more specific than NormalEvent)
      if (auto nb = dynamic_cast<const NotifyBeforeEvent *>(e)) {
        int pre_time = e->GetDeadline() - nb->GetNotifyTime();
        if (!te.nb_pre_notified && pre_time >= 1 && pre_time <= current_time_) {
          items.push_back({pre_time, te.added_index, e, 0});
          te.nb_pre_notified = true;
        }
        if (!te.nb_deadline_notified && e->GetDeadline() <= current_time_) {
          items.push_back({e->GetDeadline(), te.added_index, e, 1});
          te.nb_deadline_notified = true;
        }
        continue;
      }

      // NotifyLateEvent (covers CustomNotifyLateEvent via inheritance)
      if (auto nl = dynamic_cast<const NotifyLateEvent *>(e)) {
        int dl = e->GetDeadline();
        int freq = nl->GetFrequency();
        if (freq <= 0) {
          // Defensive: treat as only deadline notification
          if (te.late_last_n < 0 && dl <= current_time_) {
            items.push_back({dl, te.added_index, e, 0});
            te.late_last_n = 0;
          }
        } else if (dl <= current_time_) {
          int max_n = (current_time_ - dl) / freq;
          // Ensure we also include n=0 when dl == current_time_
          if (max_n < 0) max_n = 0;
          for (int n = te.late_last_n + 1; n <= max_n; ++n) {
            int st = (n == 0) ? dl : (dl + n * freq);
            items.push_back({st, te.added_index, e, n});
          }
          te.late_last_n = std::max(te.late_last_n, max_n);
        }
        continue;
      }

      // NormalEvent (deadline-only)
      if (!te.normal_notified && e->GetDeadline() <= current_time_) {
        items.push_back({e->GetDeadline(), te.added_index, e, 0});
        te.normal_notified = true;
      }
    }

    std::sort(items.begin(), items.end(), [](const Item &a, const Item &b) {
      if (a.scheduled_time != b.scheduled_time) return a.scheduled_time < b.scheduled_time;
      return a.added_index < b.added_index;
    });

    for (const auto &it : items) {
      try {
        std::cout << it.ev->GetNotification(it.n) << std::endl;
      } catch (const std::exception &) {
        // If invalid n for the event type, skip silently to avoid runtime abort
      }
    }
  }

 private:
  struct Tracked {
    const Event *ev{nullptr};
    int added_index{0};
    // NormalEvent state
    bool normal_notified{false};
    // NotifyBeforeEvent state
    bool nb_pre_notified{false};
    bool nb_deadline_notified{false};
    // NotifyLateEvent state: last n already notified (starts at -1)
    int late_last_n{-1};
  };

  int duration_;
  int current_time_;
  int next_index_;
  std::vector<Tracked> events_;
};

#endif  // SRC_HPP

