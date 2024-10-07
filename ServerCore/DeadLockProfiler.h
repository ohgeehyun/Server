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
	//���������� �������Ҷ����� char���� int�� �����⶧���� 2���� �� ����.
	unordered_map<const char*, int32> _nameTold;
	unordered_map<int32, const char*> _idToName;

	
	//�������� ��� �������� ���� ���
	map<int32, set<int32>>			  _lockHistory;

	Mutex _lock;
private:
	vector<int32> _discoveredOrder;//������ �߰ߵ� ������ ����ϴ� �迭
	int32 _discoveredCount = 0;//������ �߰ߵ� ����
	vector<bool> _finished; //Dfs(i)�� ���� �Ǿ����� ����
	vector<int32> _parent; //�߰ߵ� ������ �θ� ����

};

