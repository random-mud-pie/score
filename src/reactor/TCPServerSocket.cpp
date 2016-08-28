#include "aliens/reactor/TCPServerSocket.h"
#include "aliens/reactor/FdHandlerBase.h"
#include "aliens/exceptions/macros.h"
#include "aliens/ScopeGuard.h"
#include "aliens/macros.h"

#include "aliens/reactor/SocketAddr.h"
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <glog/logging.h>

using aliens::io::NonOwnedBufferPtr;
using aliens::async::ErrBack;
using aliens::exceptions::BaseError;

namespace aliens { namespace reactor {

TCPServerSocket::TCPServerSocket(FileDescriptor &&desc,
    EventHandler *handler,
    const SocketAddr& localAddr,
    const SocketAddr &remoteAddr)
  : FdHandlerBase<TCPServerSocket>(std::forward<FileDescriptor>(desc)),
    handler_(handler),
    localAddr_(localAddr),
    remoteAddr_(remoteAddr) {
  handler_->setParent(this);
}

void TCPServerSocket::readSome() {
  LOG(INFO) << "onReadable";
  const size_t kReadBuffSize = 128;
  for (;;) {
    void *buffPtr {nullptr};
    size_t buffLen {0};
    handler_->getReadBuffer(&buffPtr, &buffLen, kReadBuffSize);
    CHECK(!!buffPtr);
    ssize_t nr = read(getFdNo(), (char*) buffPtr, buffLen);
    if (nr == -1) {
      if (errno == EAGAIN) {
        handler_->onReadableStop();
        break;
      } else {
        ALIENS_CHECK_SYSCALL2(nr, "read()");
      }
    } else if (nr == 0) {
      handler_->onReadableStop();
      handler_->onEOF();
      break;
    } else {
      handler_->readBufferAvailable(buffPtr, nr);
    }
  }
}

void TCPServerSocket::triggerReadable() {
  handler_->onReadableStart();
}

void TCPServerSocket::triggerWritable() {
  handler_->onWritable();
}

void TCPServerSocket::triggerError() {
  LOG(INFO) << "onError [" << errno << "]";
}

TCPServerSocket TCPServerSocket::fromAccepted(
    FileDescriptor &&fd,
    TCPServerSocket::EventHandler *handler,
    const SocketAddr &localAddr,
    const SocketAddr &remoteAddr) {
  LOG(INFO) << "TCPServerSocket::fromAccepted()"
    << "\t{fd=" << fd.getFdNo() << "}";
  return TCPServerSocket(
    std::forward<FileDescriptor>(fd),
    handler,
    localAddr,
    remoteAddr
  );
}

void TCPServerSocket::EventHandler::sendBuff(
    NonOwnedBufferPtr buff, ErrBack &&cb) {
  getParent()->sendBuff(buff, std::forward<ErrBack>(cb));
}

void TCPServerSocket::sendBuff(
    NonOwnedBufferPtr buff, ErrBack &&cb) {
  auto buffStr = buff.copyToString();
  LOG(INFO) << "should send : [" << buffStr.size()
      << "] '" << buffStr << "'";
  auto fd = getFdNo();
  // ALIENS_UNUSED(fd);
  cb();
  ssize_t nr = ::write(fd, buff.vdata(), buff.size());
  ALIENS_CHECK_SYSCALL2(nr, "write()");
  if (nr == buff.size()) {
    cb();
  } else {
    cb(BaseError("couldn't write everything."));
  }
}

void TCPServerSocket::EventHandler::shutdown() {
  getParent()->shutdown();
}

void TCPServerSocket::shutdown() {
  LOG(INFO) << "TCPServerSocket::shutdown()";
  getFileDescriptor().close();
}
}} // aliens::reactor