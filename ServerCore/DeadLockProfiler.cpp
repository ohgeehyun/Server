#include "pch.h"
#include "DeadLockProfiler.h"
/*-----------------------
	 DeadLockProfiler
------------------------*/

void DeadLockProfiler::PushLock(const char* name)
{
	LockGuard guard(_lock);

	//���̵� ã�ų� �߱��Ѵ�.
	int32 lockId = 0;
	//nameTold���� �̸��� �߰��� �ȴٸ� �̹� ���� �ܰ迡�� �߰ߵ� ������
	auto findIt = _nameTold.find(name);
	//�߰ߵ� ���� ���� ó�� �߰ߵ� ������
	if (findIt == _nameTold.end())
	{
		lockId = static_cast<int32>(_nameTold.size());
		_nameTold[name] = lockId;
		_idToName[lockId] = name;
	}
	else
	{

		lockId = findIt->second;
	}
	// ��� �ִ� ���� �־��ٸ�
	if (_lockStack.empty() == false)
	{
		//������ �߰ߵ��� ���� ���̽���� ����� ���� �ٽ� Ȯ��
		const int32 prevId = _lockStack.top();
		if (lockId != prevId) 
		{
			set<int32>& history = _lockHistory[prevId];
			if (history.find(lockId) == history.end())
			{
				//ó�� �߰��� ��
				history.insert(lockId);
				CheckCycle();
			}
		}

	}
	_lockStack.push(lockId);
}

void DeadLockProfiler::PopLock(const char* name)
{
	LockGuard guard(_lock);

	//������ ��������� ����� ������ �� ����Ǿ��ٴ� ��
	if (_lockStack.empty())
		CRASH("MULTIPLE_UNLOCK");

	//�ֱٿ� ���� �� ��ID�� ����ϴ� ���� ID�� ��ġ��������
	int32 lockId = _nameTold[name];
	if (_lockStack.top() != _nameTold[name])
	CRASH("INVALID_UNLOCK")

	_lockStack.pop();
}

void DeadLockProfiler::CheckCycle()
{
	//������� �߰ߵ� �� ī��Ʈ
	const int32 lockCount = static_cast<int32>(_nameTold.size());
	//�ʱ�ȭ
	_discoveredOrder = vector<int32>(lockCount, -1);//-1�� ������ ���� �湮�����ʾҴٶ�� ��
	_discoveredCount = 0;
	_finished = vector<bool>(lockCount, false);
	_parent = vector<int32>(lockCount, -1);
	for (int32 lockId = 0; lockId < lockCount; lockId++) {
		Dfs(lockId);
	}
	//������ �������� �����Ѵ�.
	_discoveredOrder.clear();
	_finished.clear();
	_parent.clear();

}

void DeadLockProfiler::Dfs(int32 here)
{
	if (_discoveredOrder[here] != -1)
		return;

	_discoveredOrder[here] = _discoveredCount++;
	//��� ������ ������ ��ȸ
	auto findIt = _lockHistory.find(here);
	if (findIt == _lockHistory.end())
	{
		//���� �������¿��� �ٸ� ���� ������ ���ٴ� ��.
		_finished[here] = true;
		return;
	}

	set<int32>& nextSet = findIt->second;

	for (int32 there : nextSet)
	{
		//���� �湮������ ���ٸ� �湮�Ѵ�.
		if (_discoveredOrder[there] == -1)
		{
			_parent[there] = here;
			Dfs(there);
			continue;
		}

		//here�� there���� ���� �߰ߵȴٸ� there�� here�� �ļ��̴�.(������)
		if (_discoveredOrder[here] < _discoveredOrder[there])
			continue;

		//�������� �ƴϰ� ,Dfs(there)�� ���� ������� �ʾҴٸ�, there�� here�� �����̴�.(������ ����)
		if (_finished[there]==false)
		{
			printf("%s -> %s\n", _idToName[here], _idToName[there]);

			int32 now = here;
			while (true)
			{
				printf("%s -> %s\n", _idToName[_parent[now]], _idToName[now]);
				now = _parent[now];
				if (now == there)
					break;
			}

			CRASH("DEADLOCK_DETECTED");
		}

		_finished[here] = true;
	}
}
