#ifndef EVENT_H
#define EVENT_H

#include <stdexcept>
#include <string>

class Event {
public:
  Event() = delete;
  Event(const std::string &name, int deadline) : name_(name), deadline_(deadline), complete_(false) {}
  Event(const Event &other) = delete;
  Event(Event &&other) = delete;
  Event &operator=(const Event &other) = delete;
  Event &operator=(Event &&other) = delete;
  virtual ~Event() = default;
  const std::string &GetName() const { return name_; }
  void SetComplete() { complete_ = true; }
  bool IsComplete() const { return complete_; }
  int GetDeadline() const { return deadline_; }
  virtual std::string GetNotification(int n) const = 0;
private:
  std::string name_;
  int deadline_;
  bool complete_;
};

class NormalEvent final : public Event {
public:
  NormalEvent(const std::string &name, int deadline) : Event(name, deadline) {}
  std::string GetNotification(int n) const override {
    if (n != 0) {
      throw std::runtime_error("Notification argument is invalid for Normal Events!");
    }
    return "Normal Event \"" + GetName() + "\" is over.";
  }
};

class NotifyBeforeEvent final : public Event {
public:
  NotifyBeforeEvent(const std::string &name, int deadline, int notify_time)
      : Event(name, deadline), notify_time_(notify_time) {}
  int GetNotifyTime() const { return notify_time_; }
  std::string GetNotification(int n) const override {
    if (n == 0) {
      return "Notify Before Event \"" + GetName() + "\" is about to end. Please hurry!";
    }
    if (n == 1) {
      return "Notify Before Event \"" + GetName() + "\" is over.";
    }
    throw std::runtime_error("Notification argument is invalid for Notify Before Events!");
  }

private:
  int notify_time_;
};

class NotifyLateEvent : public Event {
public:
  NotifyLateEvent(const std::string &name, int deadline, int frequency)
      : Event(name, deadline), frequency_(frequency) {}
  int GetFrequency() const { return frequency_; }
  std::string GetNotification(int n) const override {
    if (n == 0) {
      return "Notify Late Event \"" + GetName() + "\" is over.";
    }
    if (n > 0) {
      return "Notify Late Event \"" + GetName() + "\" is late for " + std::to_string(frequency_ * n) + " hours. ";
    }
    throw std::runtime_error("Notification argument is invalid for Notify Late Events!");
  }

private:
  int frequency_;
};

class CustomNotifyLateEvent final : public NotifyLateEvent {
public:
  CustomNotifyLateEvent(const std::string &name, int deadline, int frequency, std::string (*generator)(int))
      : NotifyLateEvent(name, deadline, frequency), generator_(generator) {}
  std::string GetNotification(int n) const override;

private:
  std::string (*generator_)(int);
};

#endif // EVENT_H

