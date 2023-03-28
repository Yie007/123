#include"Demo.h"

void Demo::sendRequest(const char* ip, const char* port,const char* request)
{
	Client it;
	it.init();
	it.connectTo(ip, port);
	unique_lock<mutex>lock(m);
	while (Q.size() > MAX_CACHE_LENGTH)
		condInput.wait(lock);
	it.sendMessage(request);
	it.close();

	CacheData temp;
	temp.id = ID++;
	temp.data = it.getRecv();
	Q.push(temp);
	condOutput.notify_one();
}

void Demo::printData()
{
	unique_lock<mutex>lock(m);
	while (Q.empty())
		condOutput.wait(lock);
	CacheData temp = Q.front();
	Q.pop();
	printf("Data %d is printing:\n", temp.id);
	printf("--%s--\n", temp.data.c_str());
	condInput.notify_one();
}