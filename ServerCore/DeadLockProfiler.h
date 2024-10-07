#pragma once
#include<stack>
#include<map>
#include<vector>

/*-----------------------
	 DeadLockProfiler
------------------------*/

class DeadLockProfiler
{

public:
	void PushLock(const char* name);
	void PopLock(const char* name);
	void CheckCycle();

private:
	void Dfs(int32 index);
private:
	//내부적으로 연산을할때에는 char보다 int가 빠르기때문에 2가지 를 만듬.
	unordered_map<const char*, int32> _nameTold;
	unordered_map<int32, const char*> _idToName;

	
	//정점들이 어느 정점으로 간지 기록
	map<int32, set<int32>>			  _lockHistory;

	Mutex _lock;
private:
	vector<int32> _discoveredOrder;//정점이 발견된 순서를 기록하는 배열
	int32 _discoveredCount = 0;//정점이 발견된 순서
	vector<bool> _finished; //Dfs(i)가 종료 되었는지 여부
	vector<int32> _parent; //발견된 정점의 부모 정점

};

