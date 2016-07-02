#ifndef DATA_TRANSFORMER_HPP
#define DATA_TRANSFORMER_HPP
#include <vector>

#include "protos/dragon.pb.h"
#include "blob.hpp"
#include "common.hpp"

#include <opencv2/opencv.hpp>
using namespace cv;

using namespace std;
template <typename Dtype>
class DataTransformer
{
public:
	DataTransformer(const TransformationParameter& param, Phase phase);
	vector<int> inferBlobShape(const Datum& datum);
	void transform(const Datum& datum, Blob<Dtype>* shadow_blob);
	void transform(const Datum& datum, Dtype* shadow_data);
	static void transform(IplImage* cv_img, Datum* datum);
	~DataTransformer() {}
	int rand(int n);
private:
	TransformationParameter param;
	Phase phase;
	Blob<Dtype> mean_blob;
	vector<Dtype> mean_vals;
};
#endif
