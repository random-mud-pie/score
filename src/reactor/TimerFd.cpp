#include "aliens/reactor/TimerFd.h"
#include <atomic>
#include <memory>
#include <cstdio>
#include <glog/logging.h>
#include "aliens/exceptions/macros.h"

namespace aliens { namespace reactor {


TimerFd::TimerFd(FileDescriptor &&desc, TimerFd::EventHandler *handler)
  : fd_(std::forward<FileDescriptor>(desc)),
    handler_(handler) {
  epollTask_.setParent(this);
}

void TimerFd::stop() {
  fd_.close();
}

void TimerFd::triggerRead() {
  uint64_t nTimeouts {0};
  CHECK(8 == read(getFdNo(), &nTimeouts, sizeof(uint64_t)));
  CHECK(!!handler_);

  // nTimeouts should almost always be 1.
  // are there edge cases?
  for (size_t i = 0; i < nTimeouts; i++) {
    handler_->onTick();
  }
}

TimerFd TimerFd::create(const TimerSettings& settings, TimerFd::EventHandler *handler) {
  int fd = timerfd_create(
    CLOCK_MONOTONIC,
    TFD_NONBLOCK | TFD_CLOEXEC
  );
  ALIENS_CHECK_SYSCALL2(fd, "TimerFd::create()");
  TimerFd instance(FileDescriptor::fromIntExcept(fd), handler);
  instance.settings_ = settings;
  itimerspec previousSettings;
  itimerspec *desiredSettings = instance.settings_.getTimerSpec();

  const int kRelativeTimerFlag = 0;
  ALIENS_CHECK_SYSCALL2(
    timerfd_settime(
      instance.getFdNo(),
      kRelativeTimerFlag,
      desiredSettings,
      &previousSettings
    ),
    "timerfd_settime()"
  );
  return instance;
}

std::shared_ptr<TimerFd> TimerFd::createShared(
    const TimerSettings& settings, TimerFd::EventHandler *handler) {
  return std::shared_ptr<TimerFd>(new TimerFd(create(
    settings, handler
  )));
}

void TimerFd::EpollTask::onReadable() {
  getParent()->triggerRead();
}

void TimerFd::EpollTask::onWritable() {
  LOG(INFO) << "TimerFd::EpollTask::onWritable()";
}

void TimerFd::EpollTask::onError() {
  LOG(INFO) << "TimerFd::EpollTask::onError()";
}

int TimerFd::EpollTask::getFd() {
  return parent_->getFdNo();
}

void TimerFd::EpollTask::setParent(TimerFd *parent) {
  parent_ = parent;
}

TimerFd* TimerFd::EpollTask::getParent() const {
  return parent_;
}

TimerFd::EpollTask* TimerFd::getEpollTask() {
  return &epollTask_;
}

int TimerFd::getFdNo() const {
  return fd_.getFdNo();
}


}} // aliens::reactor
