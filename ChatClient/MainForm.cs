using System;
using System.Collections.Generic;
using System.Threading;
using System.Windows.Forms;
using System.Windows.Threading;
using System.Text;

namespace ChatClient {
    public partial class MainForm : Form {
        SimpleNetwork Network = new SimpleNetwork();

        bool IsNetworkThreadRunning = false;
        bool IsRoom = false;

        Thread NetworkReadThread = null;
        Thread NetworkSendThread = null;

        PacketBufferManager PacketBuffer = new PacketBufferManager();
        Queue<PacketData> RecvPacketQueue = new Queue<PacketData>();
        Queue<byte[]> SendPacketQueue = new Queue<byte[]>();

        DispatcherTimer dispatcherTimer;

        public MainForm() {
            InitializeComponent();
        }

        private void MainForm_Load(object sender, EventArgs e) {
            PacketBuffer.Init(8096 * 10, PacketDefine.kPacketHeaderSize, 1024);

            IsNetworkThreadRunning = true;
            NetworkReadThread = new Thread(NetworkReadProcess);
            NetworkReadThread.Start();
            NetworkSendThread = new Thread(NetworkSendProcess);
            NetworkSendThread.Start();

            dispatcherTimer = new DispatcherTimer();
            dispatcherTimer.Tick += new EventHandler(BackGroundProcess);
            dispatcherTimer.Interval = new TimeSpan(0, 0, 0, 0, 100);
            dispatcherTimer.Start();

            buttonLogoff.Enabled = false;
            SetPacketHandler();
        }

        void NetworkReadProcess() {
            while (IsNetworkThreadRunning) {
                Thread.Sleep(1);
                if (Network.IsConnected() == false) {
                    continue;
                }

                var recvData = Network.Receive();
                if (recvData != null) {
                    PacketBuffer.Write(recvData.Item2, 0, recvData.Item1);
                    while (true) {
                        var data = PacketBuffer.Read();
                        if (data.Count < 1) {
                            break;
                        }
                        var packetSize = BitConverter.ToInt16(data.Array, data.Offset);

                        if (data.Count < packetSize) {
                            PacketBuffer.SetReadPosToPrev();
                            break;
                        }

                        var packet = new PacketData();
                        packet.DataSize = (short)(data.Count - PacketDefine.kPacketHeaderSize);
                        packet.PacketId = BitConverter.ToInt16(data.Array, data.Offset + 2);
                        packet.BodyData = new byte[packet.DataSize];
                        Buffer.BlockCopy(data.Array, data.Offset + PacketDefine.kPacketHeaderSize, packet.BodyData, 0, packet.DataSize);
                        lock (((System.Collections.ICollection)RecvPacketQueue).SyncRoot) {
                            RecvPacketQueue.Enqueue(packet);
                        }
                    }
                }
            }
        }

        void NetworkSendProcess() {
            while (IsNetworkThreadRunning) {
                Thread.Sleep(1);

                if (Network.IsConnected() == false)
                    continue;

                lock (((System.Collections.ICollection)SendPacketQueue).SyncRoot) {
                    if (SendPacketQueue.Count > 0) {
                        var packet = SendPacketQueue.Dequeue();
                        Network.Send(packet);
                    }
                }
            }
        }

        void BackGroundProcess(object sender, EventArgs e) {
            try {
                lock (((System.Collections.ICollection)RecvPacketQueue).SyncRoot) {
                    if (RecvPacketQueue.Count > 0) {
                        var packet = RecvPacketQueue.Dequeue();
                        if (packet.PacketId != 0)
                            PacketProcess(packet);
                    }
                }

            } catch (Exception ex) {
                MessageBox.Show(string.Format("Read Packet Queue Process Error:{0}", ex.Message));
            }
        }

        private bool PostSend(PacketId packetId, byte[] bodyData) {
            if (Network.IsConnected() == false) {
                labelStatus.Text = string.Format($"서버와의 연결이 끊어졌습니다.");
                return false;
            }

            Int16 bodyDataSize = 0;
            if (bodyData != null)
                bodyDataSize = (Int16)bodyData.Length;

            var packetSize = PacketDefine.kPacketHeaderSize + bodyDataSize;
            List<byte> data = new List<byte>();
            data.AddRange(BitConverter.GetBytes((Int16)packetSize));
            data.AddRange(BitConverter.GetBytes((Int16)packetId));

            if (bodyData != null)
                data.AddRange(bodyData);

            SendPacketQueue.Enqueue(data.ToArray());
            return true;
        }

        private void ButtonConnect_Click(object sender, EventArgs e) {
            string ip = "127.0.0.1";
            int port = 32452;
            if (Network.Connect(ip, port)) {
                labelStatus.Text = string.Format($"{DateTime.Now} / 서버 접속 완료");
                buttonConnect.Enabled = false;
            } else {
                labelStatus.Text = string.Format($"{DateTime.Now} / 서버 접속 실패");
            }
        }

        private void ButtonLogin_Click(object sender, EventArgs e) {
            var loginRequest = new LoginRequestPacket();
            loginRequest.SetData(textBoxId.Text, textBoxPw.Text);
            PostSend(PacketId.LOGIN_REQ, loginRequest.ToBytes());
            labelStatus.Text = string.Format($"로그인 요청");
        }

        private void buttonLogoff_Click(object sender, EventArgs e) {
            Network.Close();
            Close();
        }

        private void MainForm_FormClosing(object sender, FormClosingEventArgs e) {
            Network.Close();
            IsNetworkThreadRunning = false;
            NetworkSendThread.Join();
            NetworkReadThread.Join();
            dispatcherTimer.Stop();
        }

        private void buttonEnterLobby_Click(object sender, EventArgs e) {
            // 로비가 지금은 하나니까 디폴트로 0
            var packetReq = new PacketBasicEnterLeaveReq();
            packetReq.Index = 0;
            PostSend(PacketId.LOBBY_ENTER_REQ, packetReq.ToBytes());
            labelStatus.Text = string.Format($"로비 입장 요청");
        }

        private void buttonLeaveLobby_Click(object sender, EventArgs e) {
            var packetReq = new PacketBasicEnterLeaveReq();
            packetReq.Index = 0;
            var result = PostSend(PacketId.LOBBY_LEAVE_REQ, packetReq.ToBytes());
            if (result == true)
                labelStatus.Text = string.Format($"로비 퇴장 요청");
        }

        private void buttonEnterRoom_Click(object sender, EventArgs e) {
            var packetReq = new PacketBasicEnterLeaveReq();
            packetReq.Index = Int16.Parse(textBoxRoomNumber.Text);
            PostSend(PacketId.ROOM_ENTER_REQ, packetReq.ToBytes());
            labelStatus.Text = string.Format($"방 입장 요청");
        }

        private void buttonLeaveRoom_Click(object sender, EventArgs e) {
            var packetReq = new PacketBasicEnterLeaveReq();
            packetReq.Index = Int16.Parse(textBoxRoomNumber.Text);
            PostSend(PacketId.ROOM_LEAVE_REQ, packetReq.ToBytes());
            labelStatus.Text = string.Format($"방 퇴장 요청");

        }

        private void buttonSendMsg_Click(object sender, EventArgs e) {
            var msg = textBoxChat.Text;
            PostSend(PacketId.ROOM_CHAT_REQ, Encoding.UTF8.GetBytes(textBoxChat.Text));
            textBoxChat.Clear();
            labelStatus.Text = string.Format($"채팅 중");
        }

        private void textBoxChat_TextChanged(object sender, EventArgs e) {
            var length = textBoxChat.Text.Length;
            if (IsRoom == true && length > 0) {
                buttonSendMsg.Enabled = true;
            } else {
                buttonSendMsg.Enabled = false;
            }
        }
    }
}