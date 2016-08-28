#pragma once
#include <glog/logging.h>
#include "aliens/reactor/FdHandlerBase.h"
#include "aliens/reactor/SocketAddr.h"
#include "aliens/reactor/ParentHaving.h"
#include "aliens/async/ErrBack.h"
#include "aliens/io/NonOwnedBufferPtr.h"

namespace aliens { namespace reactor {

class TCPServerSocket : public FdHandlerBase<TCPServerSocket> {
 public:
  class EventHandler : public ParentHaving<TCPServerSocket> {
   public:
    virtual void onReadableStart() = 0;
    virtual void onReadableStop() = 0;
    virtual void getReadBuffer(void **buff, size_t* buffLen, size_t hint) = 0;
    virtual void readBufferAvailable(void *buff, size_t buffLen) = 0;
    virtual void onWritable() = 0;
    virtual void onEOF() = 0;
    void readSome();
    void sendBuff(io::NonOwnedBufferPtr, async::ErrBack &&errback);
    void shutdown();
    virtual ~EventHandler() = default;
  };
  friend class EventHandler;
 protected:
  EventHandler *handler_ {nullptr};
  SocketAddr localAddr_;
  SocketAddr remoteAddr_;
  TCPServerSocket(FileDescriptor &&, EventHandler*,
    const SocketAddr &localAddr, const SocketAddr &remoteAddr
  );
  void shutdown();
 public:
  void triggerReadable();
  void triggerWritable();
  void triggerError();
  void sendBuff(io::NonOwnedBufferPtr, async::ErrBack &&errback);
  static TCPServerSocket fromAccepted(
    FileDescriptor&&, EventHandler*,
    const SocketAddr&, const SocketAddr&
  );
};

}} // aliens::reactor