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
        byte[] UserId = new byte[PacketDefine.MaxUserIdLength + 1];
        byte[] UserPw = new byte[PacketDefine.MaxUserPwLength + 1];

        public void SetData(string userId, string userPw) {
            Encoding.UTF8.GetBytes(userId).CopyTo(UserId, 0);
            Encoding.UTF8.GetBytes(userPw).CopyTo(UserPw, 0);
        }

        public byte[] ToBytes() {
            List<byte> data = new List<byte>();
            data.AddRange(UserId);
            data.AddRange(UserPw);
            return data.ToArray();
        }
    }

    public class LoginResponsePacket {
        byte[] UserId = new byte[PacketDefine.MaxUserIdLength];
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
