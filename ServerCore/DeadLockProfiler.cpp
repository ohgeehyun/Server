#include "pch.h"
#include "DeadLockProfiler.h"
/*-----------------------
	 DeadLockProfiler
------------------------*/

void DeadLockProfiler::PushLock(const char* name)
{
	LockGuard guard(_lock);

	//아이디를 찾거나 발급한다.
	int32 lockId = 0;
	//nameTold에서 이름을 발견이 된다면 이미 이전 단계에서 발견된 스레드
	auto findIt = _nameTold.find(name);
	//발견된 적이 없어 처음 발견된 스레드
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
	// 잡고 있는 락이 있었다면
	if (_lockStack.empty() == false)
	{
		//기존에 발견되지 않은 케이스라면 데드락 여부 다시 확인
		const int32 prevId = _lockStack.top();
		if (lockId != prevId) 
		{
			set<int32>& history = _lockHistory[prevId];
			if (history.find(lockId) == history.end())
			{
				//처음 발견한 락
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

	//스택이 비어있으면 언락이 락보다 더 실행되었다는 뜻
	if (_lockStack.empty())
		CRASH("MULTIPLE_UNLOCK");

	//최근에 락을 건 락ID와 언락하는 락의 ID가 일치하지않음
	int32 lockId = _nameTold[name];
	if (_lockStack.top() != _nameTold[name])
	CRASH("INVALID_UNLOCK")

	_lockStack.pop();
}

void DeadLockProfiler::CheckCycle()
{
	//현재까지 발견된 락 카운트
	const int32 lockCount = static_cast<int32>(_nameTold.size());
	//초기화
	_discoveredOrder = vector<int32>(lockCount, -1);//-1은 정점이 아직 방문되지않았다라는 뜻
	_discoveredCount = 0;
	_finished = vector<bool>(lockCount, false);
	_parent = vector<int32>(lockCount, -1);
	for (int32 lockId = 0; lockId < lockCount; lockId++) {
		Dfs(lockId);
	}
	//연산이 끝났으면 정리한다.
	_discoveredOrder.clear();
	_finished.clear();
	_parent.clear();

}

void DeadLockProfiler::Dfs(int32 here)
{
	if (_discoveredOrder[here] != -1)
		return;

	_discoveredOrder[here] = _discoveredCount++;
	//모든 인접한 정점을 순회
	auto findIt = _lockHistory.find(here);
	if (findIt == _lockHistory.end())
	{
		//락을 잡은상태에서 다른 락에 간적이 없다는 뜻.
		_finished[here] = true;
		return;
	}

	set<int32>& nextSet = findIt->second;

	for (int32 there : nextSet)
	{
		//아직 방문한적이 없다면 방문한다.
		if (_discoveredOrder[there] == -1)
		{
			_parent[there] = here;
			Dfs(there);
			continue;
		}

		//here가 there보다 먼저 발견된다면 there는 here의 후손이다.(순방향)
		if (_discoveredOrder[here] < _discoveredOrder[there])
			continue;

		//순방향이 아니고 ,Dfs(there)가 아직 종료되지 않았다면, there는 here의 선조이다.(역방향 간선)
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
