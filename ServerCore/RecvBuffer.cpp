#include "pch.h"
#include "RecvBuffer.h"
/*--------------------
	   RecvBuffer
---------------------*/

RecvBuffer::RecvBuffer(int32 bufferSize) :_bufferSize(bufferSize)
{
	_capacity = bufferSize * BUFFER_COUNT;
	_buffer.resize(_capacity);
}

RecvBuffer::~RecvBuffer()
{
}

void RecvBuffer::Clean()
{
	//read와 write가 같은 위치에있다면 데이터를 다읽었다는 뜻 다시 스타트지점으로 초기화
	//또는 r와 w를 버퍼의 끝까지 가서도 w와 r를 늘려주어야한다면 w와 r 그리고 사이에있는 데이터를 앞부분으로 옮겨줄 필요가있다.(순환 버퍼)
	int32 dataSize = DataSize();
	if (dataSize == 0)
	{
		//딱 마침 읽기 + 쓰기 커서가 동일한 위치라면 , 둘 다 리 셋.
		_readPos = _writePos = 0;
	}
	else
	{
		//여유 공간이 버퍼1개 크기 미만이면,데이터 앞으로 땅긴다.
		if (FreeSize() < _bufferSize)
		{
			memcpy(&_buffer[0], &_buffer[_readPos], dataSize);
			_readPos = 0;
			_writePos = dataSize;
		}
	}

}

bool RecvBuffer::OnRead(int32 numOfBytes)
{
	if (numOfBytes > DataSize())
		return false;

	_readPos += numOfBytes;

	return true;
}

bool RecvBuffer::OnWrite(int32 numOfBytes)
{
	if (numOfBytes > FreeSize())
		return false;

	_writePos += numOfBytes;
	return true;
}
