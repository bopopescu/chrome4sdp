/*
* Copyright (c) 2014, 2015, The Linux Foundation. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are
* met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above
*       copyright notice, this list of conditions and the following
*       disclaimer in the documentation and/or other materials provided
*       with the distribution.
*     * Neither the name of The Linux Foundation nor the names of its
*       contributors may be used to endorse or promote products derived
*       from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
* ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
* BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
* OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
* IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ALT_SOCKET_TCP_CLIENT_SOCKET_H_
#define ALT_SOCKET_TCP_CLIENT_SOCKET_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "net/base/address_list.h"
#include "net/base/completion_callback.h"
#include "net/base/net_export.h"
#include "net/log/net_log.h"
#include "net/socket/connection_attempts.h"
#include "net/socket/stream_socket.h"

namespace net {

class AltTransportDataBuf;
class AltTransaction;

class NET_EXPORT AltClientSocket : public StreamSocket {
 public:

  AltClientSocket(AltTransaction* alt_transction, StreamSocket* original_socket);

  virtual ~AltClientSocket();

  // StreamSocket implementation.
  virtual int Connect(const CompletionCallback& callback) override;
  virtual void Disconnect() override;
  virtual bool IsConnected() const override;
  virtual bool IsConnectedAndIdle() const override;
  virtual int GetPeerAddress(IPEndPoint* address) const override;
  virtual int GetLocalAddress(IPEndPoint* address) const override;
  virtual const BoundNetLog& NetLog() const override;
  virtual void SetSubresourceSpeculation() override;
  virtual void SetOmniboxSpeculation() override;
  virtual bool WasEverUsed() const override;
  virtual bool UsingTCPFastOpen() const override;
  virtual bool WasNpnNegotiated() const override;
  virtual NextProto GetNegotiatedProtocol() const override;
  virtual bool GetSSLInfo(SSLInfo* ssl_info) override;

  // Socket implementation.
  virtual int Read(IOBuffer* buf, int buf_len,
                   const CompletionCallback& callback) override;
  virtual int Write(IOBuffer* buf, int buf_len,
                    const CompletionCallback& callback) override;
  virtual int SetReceiveBufferSize(int32 size) override;
  virtual int SetSendBufferSize(int32 size) override;

  //Alternative Transport interface
  scoped_ptr<StreamSocket> PassOriginalSocket();
  void DidCompleteRead(int result);

  void GetConnectionAttempts(ConnectionAttempts* out) const override;
  void ClearConnectionAttempts() override;
  void AddConnectionAttempts(const ConnectionAttempts& attempts) override;

  virtual void SetStatHubParentId(unsigned int stat_hub_parent_id) {
       stat_hub_parent_id_ = stat_hub_parent_id;
   }
 private:
  AltTransaction* alt_transction_;

  scoped_ptr<StreamSocket> original_socket_;

  AltTransportDataBuf* data_to_read_;

  bool read_buf_empty_;

  // The buffer used for reads.
  scoped_refptr<IOBuffer> read_buf_;
  uint32_t read_buf_len_;

  // External callback; called when read is complete.
  CompletionCallback read_callback_;

  base::Lock lock_;

  // Failed connection attempts made while trying to connect this socket.
  ConnectionAttempts connection_attempts_;

  unsigned int stat_hub_parent_id_;

  DISALLOW_COPY_AND_ASSIGN(AltClientSocket);
};

}  // namespace net

#endif  // ALT_SOCKET_TCP_CLIENT_SOCKET_H_
