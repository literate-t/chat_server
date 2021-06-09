using System;
using System.Collections.Generic;
using System.Threading;
using System.Windows.Forms;
using System.Windows.Threading;
using System.Text;

namespace ChatClient {
    public partial class MainForm : Form {
        SimpleNetwork _network = new SimpleNetwork();

        bool _isNetworkThreadRunning = false;
        bool _isRoom = false;

        Thread _networkReadThread = null;
        Thread _networkSendThread = null;

        PacketBufferManager _packetBuffer = new PacketBufferManager();
        Queue<PacketData> _recvPacketQueue = new Queue<PacketData>();
        Queue<byte[]> _sendPacketQueue = new Queue<byte[]>();

        DispatcherTimer _dispatcherTimer;

        public MainForm() {
            InitializeComponent();
        }

        private void MainForm_Load(object sender, EventArgs e) {
            _packetBuffer.Init(8096 * 10, PacketDefine.kPacketHeaderSize, 1024);

            _isNetworkThreadRunning = true;
            _networkReadThread = new Thread(NetworkReadProcess);
            _networkReadThread.Start();
            _networkSendThread = new Thread(NetworkSendProcess);
            _networkSendThread.Start();

            _dispatcherTimer = new DispatcherTimer();
            _dispatcherTimer.Tick += new EventHandler(BackGroundProcess);
            _dispatcherTimer.Interval = new TimeSpan(0, 0, 0, 0, 100);
            _dispatcherTimer.Start();

            buttonLogoff.Enabled = false;
            SetPacketHandler();
        }

        void NetworkReadProcess() {
            while (_isNetworkThreadRunning) {
                Thread.Sleep(1);
                if (_network.IsConnected() == false) {
                    continue;
                }

                var recvData = _network.Receive();
                if (recvData != null) {
                    _packetBuffer.Write(recvData.Item2, 0, recvData.Item1);
                    while (true) {
                        var data = _packetBuffer.Read();
                        if (data.Count < 1) {
                            break;
                        }
                        var packetSize = BitConverter.ToInt16(data.Array, data.Offset);

                        if (data.Count < packetSize) {
                            _packetBuffer.SetReadPosToPrev();
                            break;
                        }

                        var packet = new PacketData();
                        packet.DataSize = (short)(data.Count - PacketDefine.kPacketHeaderSize);
                        packet.PacketId = BitConverter.ToInt16(data.Array, data.Offset + 2);
                        packet.BodyData = new byte[packet.DataSize];
                        Buffer.BlockCopy(data.Array, data.Offset + PacketDefine.kPacketHeaderSize, packet.BodyData, 0, packet.DataSize);
                        lock (((System.Collections.ICollection)_recvPacketQueue).SyncRoot) {
                            _recvPacketQueue.Enqueue(packet);
                        }
                    }
                }
            }
        }

        void NetworkSendProcess() {
            while (_isNetworkThreadRunning) {
                Thread.Sleep(1);

                if (_network.IsConnected() == false)
                    continue;

                lock (((System.Collections.ICollection)_sendPacketQueue).SyncRoot) {
                    if (_sendPacketQueue.Count > 0) {
                        var packet = _sendPacketQueue.Dequeue();
                        _network.Send(packet);
                    }
                }
            }
        }

        void BackGroundProcess(object sender, EventArgs e) {
            try {
                lock (((System.Collections.ICollection)_recvPacketQueue).SyncRoot) {
                    if (_recvPacketQueue.Count > 0) {
                        var packet = _recvPacketQueue.Dequeue();
                        if (packet.PacketId != 0)
                            PacketProcess(packet);
                    }
                }

            } catch (Exception ex) {
                MessageBox.Show(string.Format("Read Packet Queue Process Error:{0}", ex.Message));
            }
        }

        private bool PostSend(PacketId packetId, byte[] bodyData) {
            if (_network.IsConnected() == false) {
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
            data.AddRange(bodyData);

            _sendPacketQueue.Enqueue(data.ToArray());
            return true;
        }

        private void ButtonConnect_Click(object sender, EventArgs e) {
            string ip = "127.0.0.1";
            int port = 32452;
            if (_network.Connect(ip, port)) {
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
            _network.Close();
            Close();
        }

        private void MainForm_FormClosing(object sender, FormClosingEventArgs e) {
            _network.Close();
            _isNetworkThreadRunning = false;
            _networkSendThread.Join();
            _networkReadThread.Join();
            _dispatcherTimer.Stop();
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
            if (_isRoom == true && length > 0) {
                buttonSendMsg.Enabled = true;
            } else {
                buttonSendMsg.Enabled = false;
            }
        }
    }
}