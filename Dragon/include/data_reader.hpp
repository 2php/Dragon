 #ifndef DATA_READER_HPP
#define DATA_READER_HPP

#include "dragon_thread.hpp"
#include "protos/dragon.pb.h"
#include "utils/blocking_queue.hpp"

// QueuePair is the basic applicated DataStructure
// it is equal to a producter/consumer model
// free&full can be regard as a memory queue with Semaphore

class QueuePair{
public:
	QueuePair(const int size);
	~QueuePair();
	BlockingQueue<Datum*> free; // as producter queue
	BlockingQueue<Datum*> full; // as consumer queue
};


class DataReader: public DragonThread
{
public:
	DataReader(const LayerParameter& param);
	~DataReader();
	BlockingQueue<Datum*>& free() const  { return pair->free; }
	BlockingQueue<Datum*>& full() const  { return pair->full; }
	void interfaceKernel();
	void parseImageset();
	void read_one();
	void readImage(const string& filename);
	map<int, Datum> indices;
	int cur_idx;
private:
	LayerParameter param;
	boost::shared_ptr<QueuePair> pair;
};



#endif

