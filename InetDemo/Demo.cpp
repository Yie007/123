#include"Demo.h"

void Demo::sendRequest(const char* ip, const char* port,const char* request)
{
	int iResult;
	Client it;
	iResult = it.init();
	if (iResult == -1)
		return;
	iResult = it.connectTo(ip, port);
	if (iResult == -1)
		return;
	//�����ַ�ʽ���������ڴ�����ʱ���Զ�����==����
	unique_lock<mutex>unilock(m);
	//����ӡ����������ʱ��ֹͣ��������
	while (Q.size() > MAX_CACHE_LENGTH)
		condInput.wait(unilock);//�ȴ����ѣ��Զ�����m.unlock()
	//��������ѣ��Զ�����m.lock()
	iResult = it.sendMessage(request);
	if (iResult == -1)
		return;
	it.close();

	CacheData temp;
	temp.id = ID++;
	temp.data = it.getRecv();
	Q.push(temp);//������ѹ���ӡ����
	condOutput.notify_one();//���Ѵ�ӡ����������ӡ

	//���˳���ʱ���Զ�����
	//ͨ�������Ӧ�����Ƚ����ٻ��������̵߳�
}

void Demo::printData()
{
	//����
	unique_lock<mutex>unilock(m);
	while (Q.empty())
		condOutput.wait(unilock);//�ȴ���ӡ�����г��ִ���ӡ����
	CacheData temp = Q.front();
	Q.pop();
	printf("Data %d is printing:  ", temp.id);
	printf("%s\n", temp.data.c_str());
	//Ϊ�����ֶ��̣߳������ӡ��Ҫ1����
	unilock.unlock();//�ֶ�����
	condInput.notify_one();//���ѷ���������߳�
	Sleep(1000);
}

void Demo::printerAction()
{
	while (flag)
		printData();
}

void Demo::printStart()
{
	//�����߳�
	t = thread(&Demo::printerAction, this);
}

void Demo::powerOn()
{
	flag = true;
}

void Demo::powerOff()
{
	flag = false;
}

