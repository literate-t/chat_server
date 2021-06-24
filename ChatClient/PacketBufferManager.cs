using System;

namespace ChatClient {
    class PacketBufferManager {
        int _bufferSize = 0;
        int _readPos = 0;
        int _prevReadPos = 0;
        int _writePos = 0;

        int _headerSize = 0;
        int _maxPacketSize = 0;
        byte[] _packetData;
        byte[] _packetDataTemp;

        public bool Init(int size, int headerSize, int maxPacketSize) {
            if (size < (maxPacketSize * 2) || size < 1 || headerSize < 1 || maxPacketSize < 1)
                return false;

            _bufferSize = size;
            _headerSize = headerSize;
            _maxPacketSize = maxPacketSize;
            _packetData = new byte[_bufferSize];
            _packetDataTemp = new byte[_bufferSize];

            return true;
        }

        public bool Write(byte[] data, int pos, int size) {
            if (null  == data || data.Length < size)
                return false;

            var remainBufferSize = _bufferSize - _writePos;
            if (remainBufferSize < size)
                return false;

            Buffer.BlockCopy(data, pos, _packetData, _writePos, size);
            _writePos += size;

            if (false == HaveFreeSpace())
                ResetBuffer();

            return true;
        }

        public ArraySegment<byte> Read() {
            var availableReadSize = _writePos - _readPos;
            if (availableReadSize < _headerSize)
                return new ArraySegment<byte>();

            var packetDataSize = BitConverter.ToInt16(_packetData, _readPos);
            if (availableReadSize < packetDataSize)
                return new ArraySegment<byte>();

            var completePacketData = new ArraySegment<byte>(_packetData, _readPos, packetDataSize);
            _prevReadPos = _readPos;
            _readPos += packetDataSize;
            return completePacketData;
        }

        public void SetReadPosToPrev() {
            _readPos = _prevReadPos;
        }

        bool HaveFreeSpace() {
            var writableSize = _bufferSize - _writePos;
            if (writableSize < _maxPacketSize)
                return false;

            return true;
        }

        void ResetBuffer() {
            var remainedToRead = _writePos - _readPos;
            Buffer.BlockCopy(_packetData, _readPos, _packetDataTemp, 0, remainedToRead);
            Buffer.BlockCopy(_packetDataTemp, 0, _packetData, 0, remainedToRead);
            _readPos = 0;
            _writePos = remainedToRead;
        }
    }
}