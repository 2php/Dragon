#include <fstream>
#include <string>
#include <opencv2/core.hpp>
#include <opencv/highgui.h>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include "utils/logging.hpp"
#include "utils/flags.hpp"
#include "protos/dragon.pb.h"
using namespace std;
using namespace boost::filesystem;

DEFINE_string(input_folder, "", "");
DEFINE_string(output_folder, "", "");

const int kCIFARSize = 32;
const int kCIFARImageNBytes = 3072;
const int kCIFARBatchSize = 10000;
const int kCIFARTrainBatches = 5;

void read_image(std::ifstream* file, int* label, unsigned char* buffer) {
	char label_char;
	file->read(&label_char, 1);
	*label = label_char;
	file->read((char*)buffer, kCIFARImageNBytes);
	return;
}

std::string i_to_s(int n) { std::stringstream ss; ss << n; return ss.str(); }

string genId(int bits, int id){
	std::stringstream ss; ss << id;
	string t(bits - ss.str().size(), '0');
	return (t += ss.str());
}

void genImage(char* img,int height,int width, cv::Mat* mat){
	for (int h = 0; h < height; h++) {
		uchar* ptr = mat->ptr<uchar>(h);
		int img_index = 0;
		for (int w = 0; w < width; w++) {
			int r = h*width + w, g = r + height*width, b = g + height*width;
			ptr[img_index++] = static_cast<uchar>(img[b]);
			ptr[img_index++] = static_cast<uchar>(img[g]);
			ptr[img_index++] = static_cast<uchar>(img[r]);
		}
	}
}

void convert(const string& input_folder, const string& output_folder) {
	// Data buffer
	int label, height = kCIFARSize, width = kCIFARSize;
	char str_buffer[kCIFARImageNBytes];
	string img_dir = output_folder + "/JPEGImages", imgset_dir = output_folder + "/ImageSets",out_path;
	if (!exists(img_dir)) create_directories(img_dir);
	if (!exists(imgset_dir)) create_directories(imgset_dir);
	ofstream train_txt(imgset_dir + "/train.txt"), test_txt(imgset_dir + "/test.txt");

	LOG(INFO) << "convert training data......";
	for (int fileid = 0; fileid < kCIFARTrainBatches; ++fileid) {
		LOG(INFO) << "training batch " << fileid + 1;
		ifstream data_file((input_folder + "/data_batch_" + i_to_s(fileid + 1)+".bin").c_str(), ios::in | ios::binary);
		CHECK(data_file) << "unable to open train file #" << fileid + 1;
		for (int itemid = 0; itemid < kCIFARBatchSize; ++itemid) {
			read_image(&data_file, &label, (unsigned char*)str_buffer);
			cv::Mat mat = cv::Mat(kCIFARSize, kCIFARSize, CV_8UC3);
			genImage(str_buffer, height, width, &mat);
			IplImage img(mat);
			out_path = img_dir + "/" + genId(5, fileid * kCIFARBatchSize + itemid) + ".jpg";
			if (itemid != 0) train_txt << endl;
			train_txt << genId(5, fileid * kCIFARBatchSize + itemid) + ".jpg" << " " << label;
			cvSaveImage(out_path.c_str(), &img);
		}
	}
	
	LOG(INFO) << "convert testing data......";
	ifstream data_file((input_folder + "/test_batch.bin").c_str(), ios::in | ios::binary);
	CHECK(data_file) << "unable to open test file";
	for (int itemid = 0; itemid < kCIFARBatchSize; ++itemid) {
		read_image(&data_file, &label, (unsigned char*)str_buffer);
		cv::Mat mat = cv::Mat(kCIFARSize, kCIFARSize, CV_8UC3);
		genImage(str_buffer, height, width, &mat);
		IplImage img(mat);
		out_path = img_dir + "/" + genId(5, kCIFARTrainBatches * kCIFARBatchSize + itemid) + ".jpg";
		if (itemid != 0) test_txt << endl;
		test_txt << genId(5, kCIFARTrainBatches * kCIFARBatchSize + itemid) + ".jpg" << " " << label;
		cvSaveImage(out_path.c_str(), &img);
	}
}

int main(int argc, char** argv) {

	ParseCommandLineFlags(&argc, &argv);
	CHECK_GT(FLAGS_input_folder.size(),0) << "please specify a input folder.";
	CHECK_GT(FLAGS_output_folder.size(), 0) << "please specify a output folder.";
	convert(FLAGS_input_folder, FLAGS_output_folder);
	return 0;

}