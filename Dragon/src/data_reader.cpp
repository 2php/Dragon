#include <fstream>
#include "data_reader.hpp"
#include "data_transformer.hpp"

QueuePair::QueuePair(const int size){
	// set the upbound for a producter
	for (int i = 0; i < size; i++) free.push(new Datum());
}

QueuePair::~QueuePair(){
	// release and clear
	Datum *datum;
	while (free.try_pop(&datum)) delete datum;
	while (full.try_pop(&datum)) delete datum;
}

DataReader::DataReader(const LayerParameter& param) :param(param),cur_idx(0){
	pair.reset(new QueuePair(param.data_param().prefetch()*param.data_param().batch_size()));
	parseImageset();
	startThread();
}

DataReader::~DataReader() { stopThread(); }

void DataReader::parseImageset(){
	ifstream fin((param.data_param().imageset()).c_str());
	string line, filename;
	int label, idx = 0;
	Datum datum;
	ios::sync_with_stdio(false);
	while (getline(fin, line)){
		stringstream ss(line);
		ss >> filename >> label;
		datum.set_data(filename);
		datum.set_label(label);
		indices[idx++] = datum;
	}
}

void DataReader::read_one(){
	//	could block here when pre-buffer enough Datum
	Datum *datum = pair->free.pop();
	datum->Clear();
	DataParameter data_param = param.data_param();

	if (data_param.shuffle()) cur_idx = Dragon::get_random_value() % indices.size();
	else cur_idx = (cur_idx + 1) % indices.size();

	IplImage *img = cvLoadImage((data_param.source() + "/" + indices[cur_idx].data()).c_str());
	DataTransformer<float>::transform(img, datum);
	cvReleaseImage(&img);
	datum->set_label(indices[cur_idx].label());
	pair->full.push(datum);
}

void DataReader::interfaceKernel(){
	try{while (!must_stop()) read_one();
	} catch (boost::thread_interrupted&) {}
}
