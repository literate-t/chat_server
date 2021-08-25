using System;
using System.Collections.Generic;
using System.Text;

namespace ChatClient {
    partial class MainForm {
        Dictionary<PacketId, Action<byte[]>> PacketFuncDic = new Dictionary<PacketId, Action<byte[]>>();
        void SetPacketHandler() {
            PacketFuncDic.Add(PacketId.LOGIN_RES, PacketProcess_LoginResponse);
            PacketFuncDic.Add(PacketId.LOGOFF_RES, PacketProcess_LogoffResponse);
            PacketFuncDic.Add(PacketId.LOBBY_ENTER_RES, PacketProcess_LobbyEnterResponse);
            PacketFuncDic.Add(PacketId.LOBBY_ENTER_USER_NTF, PacketProcess_LobbyEnterUserNotifyResponse);
            PacketFuncDic.Add(PacketId.LOBBY_ENTER_USER_INFO, PacketProcess_LobbyEnterUserInfoResponse);
            PacketFuncDic.Add(PacketId.LOBBY_LEAVE_RES, PacketProcess_LobbyLeaveResponse);
            PacketFuncDic.Add(PacketId.LOBBY_LEAVE_USER_NTF, PacketProcess_LobbyLeaveNotifyResponse);
            PacketFuncDic.Add(PacketId.ROOM_ENTER_RES, PacketProcess_RoomEnterResponse);
            PacketFuncDic.Add(PacketId.ROOM_ENTER_USER_NTF, PacketProcess_RoomEnterUserNotifyResponse);
            PacketFuncDic.Add(PacketId.ROOM_LEAVE_RES, PacketProcess_RoomLeaveResponse);
            PacketFuncDic.Add(PacketId.ROOM_LEAVE_USER_NTF, PacketProcess_RoomLeaveNotifyResponse);
            PacketFuncDic.Add(PacketId.ROOM_CHAT_RES, PacketProcess_RoomChatResponse);
            PacketFuncDic.Add(PacketId.NTF_SYS_CLOSE, PacketProcess_CloseSessionResponse);
        }

        void PacketProcess(PacketData packet) {
            var packetType = (PacketId)packet.PacketId;
            if (PacketFuncDic.ContainsKey(packetType))
                PacketFuncDic[packetType](packet.BodyData);
        }

        private void PacketProcess_LoginResponse(byte[] bodyData) {
            var errorCode = BitConverter.ToInt16(bodyData, 0);
            if (errorCode == (short)ErrorCode.NONE) {
                labelStatus.Text = string.Format($"로그인 완료");
                //buttonLogin.Enabled = false;
                buttonLogoff.Enabled = true;
                buttonEnterLobby.Enabled = true;
            } else if (errorCode == (short)ErrorCode.USER_MGR_ID_DUPLICATION) {
                labelStatus.Text = string.Format($"이미 로그인 되어 있는 아이디");
                //buttonLogin.Enabled = true;
                buttonLogoff.Enabled = false;
            } else {
                labelStatus.Text = string.Format($"아이디/비밀번호 불일치");
                //buttonLogin.Enabled = true;
                buttonLogoff.Enabled = false;
            }
        }

        private void PacketProcess_LogoffResponse(byte[] bodyData) {
            var errorCode = BitConverter.ToInt16(bodyData, 0);
            if (errorCode == (short)ErrorCode.NONE) {
                labelStatus.Text = string.Format($"로그오프 완료");
                buttonLogoff.Enabled = false;
                //buttonLogin.Enabled = true;
                buttonEnterLobby.Enabled = false;
                buttonConnect.Enabled = true;
            } else {
                labelStatus.Text = string.Format($"로그오프 실패");
            }
        }

        private void PacketProcess_CloseSessionResponse(byte[] bodyData) {
            var errorCode = BitConverter.ToInt16(bodyData, 0);
            if (errorCode == (short)ErrorCode.NONE) {
                labelStatus.Text = string.Format($"서버와 연결 종료");
            } else {
                labelStatus.Text = string.Format($"서버와 연결 종료 실패");
            }
        }

        private void PacketProcess_LobbyEnterResponse(byte[] bodyData) {
            var errorCode = BitConverter.ToInt16(bodyData, 0);
            if (errorCode == (short)ErrorCode.NONE) {
                labelStatus.Text = string.Format($"로비 입장");
                buttonEnterLobby.Enabled = false;
                buttonLeaveLobby.Enabled = true;
                buttonEnterRoom.Enabled = true;
                buttonLogoff.Enabled = false;
            } else if (errorCode == (short)ErrorCode.LOBBY_ENTER_INVALID_LOBBY_INDEX) {
                labelStatus.Text = string.Format($"요청한 로비 인덱스가 없습니다.");
            } else if (errorCode == (short)ErrorCode.LOBBY_ENTER_USER_DUPLICATION) {
                labelStatus.Text = string.Format($"로비에 이미 입장했습니다.");
            } else if (errorCode == (short)ErrorCode.LOBBY_ENTER_MAX_USER_COUNT) {
                labelStatus.Text = string.Format($"로비에 인원이 다 찼습니다.");
            } else {
                labelStatus.Text = string.Format($"로비 입장 실패");
            }
        }

        private void PacketProcess_LobbyEnterUserInfoResponse(byte[] bodyData) {
            var errorCode = BitConverter.ToInt16(bodyData, 0);
            if (errorCode == (short)ErrorCode.NONE) {
                var usersCount = BitConverter.ToInt16(bodyData, 2);
                int offset = 4;
                for (int i = 0; i < usersCount; ++i) {
                    var length = BitConverter.ToInt16(bodyData, offset);
                    offset += 2;
                    var id = Encoding.UTF8.GetString(bodyData, offset, length);
                    listBoxLobby.Items.Add(id);
                    offset += length;
                }
            }
        }

        private void PacketProcess_LobbyEnterUserNotifyResponse(byte[] bodyData) {
            var errorCode = BitConverter.ToInt16(bodyData, 0);
            int offset = 2;
            if (errorCode == (short)ErrorCode.NONE) {
                var length = BitConverter.ToInt16(bodyData, offset);
                offset += 2;
                var id = Encoding.UTF8.GetString(bodyData, offset, length);
                listBoxLobby.Items.Add(id);
            }
        }

        private void PacketProcess_LobbyLeaveResponse(byte[] bodyData) {
            var errorCode = BitConverter.ToInt16(bodyData, 0);
            if (errorCode == (short)ErrorCode.NONE) {
                labelStatus.Text = string.Format($"로비 퇴장");
                listBoxLobby.Items.Clear();
                buttonLeaveLobby.Enabled = false;
                buttonEnterLobby.Enabled = true;
                buttonLogoff.Enabled = true;
            } else if (errorCode == (short)ErrorCode.LOBBY_LEAVE_INVALID_DOMAIN) {
                labelStatus.Text = string.Format($"사용자가 로비에 있지 않습니다");
            } else if (errorCode == (short)ErrorCode.LOBBY_LEAVE_USER_INVALID) {
                labelStatus.Text = string.Format($"사용자가 접속되어 있지 않습니다.");
            } else if (errorCode == (short)ErrorCode.USER_MGR_INVALID_SESSION_INDEX) {
                labelStatus.Text = string.Format($"사용자가 접속되어 있지 않습니다.");
            } else if (errorCode == (short)ErrorCode.ROOM_LEAVE_NOT_MEMBER) {
                labelStatus.Text = string.Format($"사용자가 방에 없습니다.");
            } else {
                labelStatus.Text = string.Format($"로비 퇴장 실패");
            }
        }

        private void PacketProcess_LobbyLeaveNotifyResponse(byte[] bodyData) {
            var errorCode = BitConverter.ToInt16(bodyData, 0);
            int offset = 2;
            if (errorCode == (short)ErrorCode.NONE) {
                var length = BitConverter.ToInt16(bodyData, offset);
                offset += 2;
                var id = Encoding.UTF8.GetString(bodyData, offset, length);
                listBoxLobby.Items.Remove(id);
            }
        }

        private void PacketProcess_RoomEnterResponse(byte[] bodyData) {
            var errorCode = BitConverter.ToInt16(bodyData, 0);
            if ((short)ErrorCode.NONE == errorCode) {
                var usersCount = BitConverter.ToInt16(bodyData, 2);
                int offset = 4;
                for (int i = 0; i < usersCount; ++i) {
                    var length = BitConverter.ToInt16(bodyData, offset);
                    offset += 2;
                    var id = Encoding.UTF8.GetString(bodyData, offset, length);
                    listBoxRoomUser.Items.Add(id);
                    offset += length;
                }
                labelStatus.Text = string.Format($"{textBoxRoomNumber.Text}번 방 입장");
                buttonEnterLobby.Enabled = false;
                buttonLeaveLobby.Enabled = false;
                buttonEnterRoom.Enabled = false;
                buttonLeaveRoom.Enabled = true;
                buttonLogoff.Enabled = false;
                _isRoom = true;
            } else if((short)ErrorCode.ROOM_CHAT_INVALID_ROOM_INDEX == errorCode){
                labelStatus.Text = string.Format($"방 번호는 0 ~ 4 범위에서 가능합니다");
            }
        }

        private void PacketProcess_RoomEnterUserNotifyResponse(byte[] bodyData) {
            var errorCode = BitConverter.ToInt16(bodyData, 0);
            int offset = 2;
            if (errorCode == (short)ErrorCode.NONE) {
                var length = BitConverter.ToInt16(bodyData, offset);
                offset += 2;
                var id = Encoding.UTF8.GetString(bodyData, offset, length);
                listBoxChat.Items.Add(String.Format("[{0}]님이 입장했습니다", id));
                listBoxRoomUser.Items.Add(id);
            }
        }

        private void PacketProcess_RoomLeaveResponse(byte[] bodyData) {
            var errorCode = BitConverter.ToInt16(bodyData, 0);
            if ((short)ErrorCode.NONE == errorCode) {
                labelStatus.Text = string.Format($"{textBoxRoomNumber.Text}번 방 퇴장");
                listBoxRoomUser.Items.Clear();
                buttonEnterLobby.Enabled = false;
                buttonLeaveLobby.Enabled = true;
                buttonLeaveRoom.Enabled = false;
                buttonLogoff.Enabled = true;
                buttonSendMsg.Enabled = false;
                listBoxChat.Items.Clear();
                textBoxChat.Clear();
            } else if (errorCode == (short)ErrorCode.ROOM_LEAVE_INVALID_DOMAIN) {
                labelStatus.Text = string.Format($"이미 방을 나간 상태입니다");
            } else {
                labelStatus.Text = string.Format($"방 퇴장 실패");
            }
        }

        private void PacketProcess_RoomLeaveNotifyResponse(byte[] bodyData) {
            var errorCode = BitConverter.ToInt16(bodyData, 0);
            int offset = 2;
            if (errorCode == (short)ErrorCode.NONE) {
                var length = BitConverter.ToInt16(bodyData, offset);
                offset += 2;
                var id = Encoding.UTF8.GetString(bodyData, offset, length);
                if (id != null) {
                    listBoxChat.Items.Add(String.Format("[{0}]님이 방을 나갔습니다", id));
                    listBoxRoomUser.Items.Remove(id);
                }
            }
        }

        private void PacketProcess_RoomChatResponse(byte[] bodyData) {
            var errorCode = BitConverter.ToInt16(bodyData, 0);
            if (errorCode == (short)ErrorCode.NONE) {
                var userIdSize = BitConverter.ToInt16(bodyData, 2);
                var userId = Encoding.UTF8.GetString(bodyData, 4, userIdSize);
                if (userId == _id) { 
                    userId = "나";
                }

                var msgSize = BitConverter.ToInt16(bodyData, 4 + PacketDefine.kMaxUserIdLength);
                var msg = Encoding.UTF8.GetString(bodyData, 6 + PacketDefine.kMaxUserIdLength, msgSize);
                msg = String.Format("[{0}]{1}", userId, msg);
                listBoxChat.Items.Add(msg);
            } else if (errorCode == (short)ErrorCode.USER_MGR_NOT_SET_USER) {
                labelStatus.Text = string.Format($"올바른 유저가 아닙니다");
            }
        }
    }
}