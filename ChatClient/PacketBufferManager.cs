using System;

namespace ChatClient {
    class PacketBufferManager {
        int BufferSize = 0;
        int ReadPos = 0;
        int PrevReadPos = 0;
        int WritePos = 0;

        int HeaderSize = 0;
        int MaxPacketSize = 0;
        byte[] PacketData;
        byte[] PacketDataTemp;

        public bool Init(int size, int headerSize, int maxPacketSize) {
            if (size < (maxPacketSize * 2) || size < 1 || headerSize < 1 || maxPacketSize < 1)
                return false;

            BufferSize = size;
            HeaderSize = headerSize;
            MaxPacketSize = maxPacketSize;
            PacketData = new byte[BufferSize];
            PacketDataTemp = new byte[BufferSize];

            return true;
        }

        public bool Write(byte[] data, int pos, int size) {
            if (data == null || data.Length < size)
                return false;

            var remainBufferSize = BufferSize - WritePos;
            if (remainBufferSize < size)
                return false;

            Buffer.BlockCopy(data, pos, PacketData, WritePos, size);
            WritePos += size;

            if (HaveFreeSpace() == false)
                ResetBuffer();

            return true;
        }

        public ArraySegment<byte> Read() {
            var availableReadSize = WritePos - ReadPos;
            if (availableReadSize < HeaderSize)
                return new ArraySegment<byte>();

            var packetDataSize = BitConverter.ToInt16(PacketData, ReadPos);
            if (availableReadSize < packetDataSize)
                return new ArraySegment<byte>();

            var completePacketData = new ArraySegment<byte>(PacketData, ReadPos, packetDataSize);
            PrevReadPos = ReadPos;
            ReadPos += packetDataSize;
            return completePacketData;
        }

        public void SetReadPosToPrev() {
            ReadPos = PrevReadPos;
        }

        bool HaveFreeSpace() {
            var writableSize = BufferSize - WritePos;
            if (writableSize < MaxPacketSize)
                return false;

            return true;
        }

        void ResetBuffer() {
            var remainedToRead = WritePos - ReadPos;
            Buffer.BlockCopy(PacketData, ReadPos, PacketDataTemp, 0, remainedToRead);
            Buffer.BlockCopy(PacketDataTemp, 0, PacketData, 0, remainedToRead);
            ReadPos = 0;
            WritePos = remainedToRead;
        }
    }
}