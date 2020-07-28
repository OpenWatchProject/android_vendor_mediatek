/*******************************************************************************
 * Software Name : RCS IMS Stack
 *
 * Copyright (C) 2010 France Telecom S.A.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ******************************************************************************/

package com.orangelabs.rcs.platform.network;

import android.net.TrafficStats;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.Socket;

/**
 * Android socket connection
 *
 * @author jexa7410
 */
public class AndroidSocketConnection implements SocketConnection {
    /**
     * Socket connection
     */
    private Socket socket = null;

    /**
     * Constructor
     */
    public AndroidSocketConnection() {
    }

    /**
     * Constructor
     *
     * @param socket Socket
     */
    public AndroidSocketConnection(Socket socket) {
        this.socket = socket;
    }

    /**
     * Open the socket
     *
     * @param remoteAddr Remote address
     * @param remotePort Remote port
     * @throws IOException
     */
    public void open(String remoteAddr, int remotePort) throws IOException {
        final int oldTag = TrafficStats.getAndSetThreadStatsTag(TrafficStats.TAG_SYSTEM_APP);
        socket = new Socket(remoteAddr, remotePort);
        TrafficStats.setThreadStatsTag(oldTag);
    }

/**
     * Open the socket
     *
     * @param remoteAddr Remote address
     * @param remotePort Remote port
     * @throws IOException
     */
    public void open1(String remoteAddr, int remotePort, String localhost, int port) throws IOException {
        InetAddress  iAddress = InetAddress.getByName(localhost);
        final int oldTag = TrafficStats.getAndSetThreadStatsTag(TrafficStats.TAG_SYSTEM_APP);
        socket = new Socket(remoteAddr, remotePort,iAddress,port);
        TrafficStats.setThreadStatsTag(oldTag);
    }

    /**
     * Set the socket
     *
     * @param socket Socket
     */
    public void setSocket(Socket socket) {
        this.socket = socket;
    }

    /**
     * Get the socket
     *
     * @return Socket
     */
    public Socket getSocket() {
        return this.socket;
    }

    /**
     * Close the socket
     *
     * @throws IOException
     */
    public void close() throws IOException {
        if (socket != null) {
            socket.close();
            socket = null;
        }
    }

    /**
     * Returns the socket input stream
     *
     * @return Input stream
     * @throws IOException
     */
    public InputStream getInputStream() throws IOException {
        if (socket != null) {
            return socket.getInputStream();
        } else {
            throw new IOException("Connection not openned");
        }
    }

    /**
     * Returns the socket output stream
     *
     * @return Output stream
     * @throws IOException
     */
    public OutputStream getOutputStream() throws IOException {
        if (socket != null) {
            return socket.getOutputStream();
        } else {
            throw new IOException("Connection not openned");
        }
    }

    /**
     * Returns the remote address of the connection
     *
     * @return Address
     * @throws IOException
     */
    public String getRemoteAddress() throws IOException {
        if (socket != null) {
            return socket.getInetAddress().getHostAddress();
        } else {
            throw new IOException("Connection not openned");
        }
    }

    /**
     * Returns the remote port of the connection
     *
     * @return Port
     * @throws IOException
     */
    public int getRemotePort() throws IOException {
        if (socket != null) {
            return socket.getPort();
        } else {
            throw new IOException("Connection not openned");
        }
    }

    /**
     * Returns the local address of the connection
     *
     * @return Address
     * @throws IOException
     */
    public String getLocalAddress() throws IOException {
        if (socket != null) {
            return socket.getLocalAddress().getHostAddress();
        } else {
            throw new IOException("Connection not openned");
        }
    }

    /**
     * Returns the local port of the connection
     *
     * @return Port
     * @throws IOException
     */
    public int getLocalPort() throws IOException {
        if (socket != null) {
            return socket.getLocalPort();
        } else {
            throw new IOException("Connection not openned");
        }
    }

    /**
     * Get the timeout for this socket during which a reading
     * operation shall block while waiting for data
     *
     * @return Timeout in milliseconds
     * @throws IOException
     */
    public int getSoTimeout() throws IOException {
        if (socket != null) {
            return socket.getSoTimeout();
        } else {
            throw new IOException("Connection not openned");
        }
    }

    /**
     * Set the timeout for this socket during which a reading
     * operation shall block while waiting for data
     *
     * @param timeout Timeout in milliseconds
     * @throws IOException
     */
    public void setSoTimeout(int timeout) throws IOException {
        if (socket != null) {
            socket.setSoTimeout(timeout);
        } else {
            throw new IOException("Connection not openned");
        }
    }

    /**
     * Sets the {@link SocketOptions#SO_SNDBUF SO_SNDBUF} option to the
     * specified value for this {@code Socket}.
     * The {@link SocketOptions#SO_SNDBUF SO_SNDBUF} option is used by the
     * platform's networking code as a hint for the size to set the underlying
     * network I/O buffers.
     *
     * <p>Because {@link SocketOptions#SO_SNDBUF SO_SNDBUF} is a hint,
     * applications that want to verify what size the buffers were set to
     * should call {@link #getSendBufferSize()}.
     *
     * @exception SocketException if there is an error
     * in the underlying protocol, such as a TCP error.
     *
     * @param size the size to which to set the send buffer
     * size. This value must be greater than 0.
     *
     * @exception IllegalArgumentException if the
     * value is 0 or is negative.
     *
     * @see #getSendBufferSize()
     * @since 1.2
     */
    public void setSendBufferSize(int size) throws IOException {
        if (socket != null) {
            socket.setSendBufferSize(size);
        } else {
            throw new IOException("Connection not openned");
        }
    }

  /**
     * Sets the {@link SocketOptions#SO_RCVBUF SO_RCVBUF} option to the
     * specified value for this {@code Socket}. The
     * {@link SocketOptions#SO_RCVBUF SO_RCVBUF} option is
     * used by the platform's networking code as a hint for the size to set
     * the underlying network I/O buffers.
     *
     * <p>Increasing the receive buffer size can increase the performance of
     * network I/O for high-volume connection, while decreasing it can
     * help reduce the backlog of incoming data.
     *
     * <p>Because {@link SocketOptions#SO_RCVBUF SO_RCVBUF} is a hint,
     * applications that want to verify what size the buffers were set to
     * should call {@link #getReceiveBufferSize()}.
     *
     * <p>The value of {@link SocketOptions#SO_RCVBUF SO_RCVBUF} is also used
     * to set the TCP receive window that is advertized to the remote peer.
     * Generally, the window size can be modified at any time when a socket is
     * connected. However, if a receive window larger than 64K is required then
     * this must be requested <B>before</B> the socket is connected to the
     * remote peer. There are two cases to be aware of:
     * <ol>
     * <li>For sockets accepted from a ServerSocket, this must be done by calling
     * {@link ServerSocket#setReceiveBufferSize(int)} before the ServerSocket
     * is bound to a local address.<p></li>
     * <li>For client sockets, setReceiveBufferSize() must be called before
     * connecting the socket to its remote peer.</li></ol>
     * @param size the size to which to set the receive buffer
     * size. This value must be greater than 0.
     *
     * @exception IllegalArgumentException if the value is 0 or is
     * negative.
     *
     * @exception SocketException if there is an error
     * in the underlying protocol, such as a TCP error.
     *
     * @see #getReceiveBufferSize()
     * @see ServerSocket#setReceiveBufferSize(int)
     * @since 1.2
     */
    public void setReceiveBufferSize(int size) throws IOException {
        if (socket != null) {
            socket.setReceiveBufferSize(size);
        } else {
            throw new IOException("Connection not openned");
        }
    }

}
