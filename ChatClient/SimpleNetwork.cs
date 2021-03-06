﻿using System;
using System.Net.Sockets;
using System.Diagnostics;

namespace ChatClient {
    class SimpleNetwork {
        public Socket ClientSocket = null;
        public string ErrorMessage = "";

        public bool Connect(string ip, int port) {
            try {
                ClientSocket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
                ClientSocket.ReceiveTimeout = 5000;
                ClientSocket.Connect(ip, port);

                if (null == ClientSocket || false == ClientSocket.Connected)
                    return false;

                return true;
            } catch (Exception ex) {
                ErrorMessage = ex.Message;
                return false;
            }
        }

        public bool Disconnect() {
            try {
                ClientSocket.Shutdown(SocketShutdown.Both);
                ClientSocket.Disconnect(true);
                return true;
            } catch (Exception ex) {
                ErrorMessage = ex.Message;
                return false;
            }
        }

        public Tuple<int, byte[]> Receive() {
            try {
                byte[] readBuffer = new byte[PacketDefine.kMaxPacketSize];
                // 서버에서 보낸 데이터 수와 실제 데이터 크기가 안 맞으면 오류 발생
                var recvByte = ClientSocket.Receive(readBuffer, 0, readBuffer.Length, SocketFlags.None);
                if (recvByte == 0)
                    return null;

                return Tuple.Create(recvByte, readBuffer);
            } catch (Exception ex) {
                ErrorMessage = ex.Message;
                return null;
            }
        }

        public void Send(byte[] sendData) {
            try {
                if (null != ClientSocket && true == ClientSocket.Connected)
                    ClientSocket.Send(sendData, 0, sendData.Length, SocketFlags.None);

                else
                    ErrorMessage = "채팅 서버에 접속하세요";
            } catch (Exception ex) {
                ErrorMessage = ex.Message;
            }
        }

        public void Close() {
            if (null != ClientSocket  && true == ClientSocket.Connected) {
                ClientSocket.Shutdown(SocketShutdown.Both);
                ClientSocket.Close();
            }
        }

        public void ShutDown(SocketShutdown how)
        {
            ClientSocket.Shutdown(how);
        }

        public bool IsConnected() { return (null != ClientSocket && true == ClientSocket.Connected) ? true : false; }
    }
}