using System;
using System.Text;
using System.Collections.Generic;

namespace ChatClient {
    struct PacketData {
        public Int16 DataSize;
        public Int16 PacketId;
        public byte[] BodyData;
    }

    public class LoginRequestPacket {
        byte[] _userId = new byte[PacketDefine.kMaxUserIdLength + 1];
        byte[] _userPw = new byte[PacketDefine.kMaxUserPwLength + 1];

        public void SetData(string userId) {
            Encoding.UTF8.GetBytes(userId).CopyTo(_userId, 0);
            //Encoding.UTF8.GetBytes(userPw).CopyTo(_userPw, 0);
        }

        public byte[] ToBytes() {
            List<byte> data = new List<byte>();
            data.AddRange(_userId);
            //data.AddRange(_userPw);
            return data.ToArray();
        }
    }

    public class LoginResponsePacket {
        byte[] UserId = new byte[PacketDefine.kMaxUserIdLength];
    }

    public class PacketBasicEnterLeaveReq {
        public short Index;
        public byte[] ToBytes() {
            List<byte> data = new List<byte>();
            data.AddRange(BitConverter.GetBytes(Index));
            return data.ToArray();
        }
    }
}
