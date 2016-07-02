#include <fstream>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#ifndef NO_MPI
#include <mpi/mpi.h>
#endif
#include "utils/io.hpp"
#include "solver.hpp"
#include "parallel/parameter_server.hpp"

template <typename Dtype>
Solver<Dtype>::Solver(const SolverParameter& param):param(param),mpi_size(-1),mpi_rank(-1){
	init();
}

template <typename Dtype>
Solver<Dtype>::Solver(const string& param_file) :mpi_size(-1), mpi_rank(-1){
	readSolverParamsFromTextFileOrDie(param_file, &this->param);
	init();
}

template <typename Dtype>
void Solver<Dtype>::initTrainNet(){
	const int num_train_nets = param.has_train_net();
	CHECK_EQ(num_train_nets, 1) << "Must specify a train net. ";

	NetParameter net_param;
	LOG_IF(INFO, Dragon::get_root_solver()) << "Create train net from train net file: " << param.train_net();
	readNetParamsFromTextFileOrDie(param.train_net(), &net_param);
	net_param.mutable_state()->set_phase(TRAIN);
	//	create and init net after parsing net_param
	net.reset(new Net<Dtype>(net_param));
}

template <typename Dtype>
void Solver<Dtype>::initTestNets(){
	const int num_test_nets = param.test_net_size();
	bool has_general_test = false;
	if (num_test_nets == 0 && param.test_iter_size() != 0) has_general_test = true;
	else CHECK_EQ(param.test_iter_size(), num_test_nets) << "Test iter must be specified for each test net.";
	vector<NetParameter> net_params(num_test_nets + has_general_test);
	test_nets.resize(num_test_nets + has_general_test);
	for (int i = 0; i < num_test_nets; i++){
		readNetParamsFromTextFileOrDie(param.test_net(i), &net_params[i]);
		net_params[i].mutable_state()->set_phase(TEST);
		LOG(INFO) << "Create test net #" << i << ": from " << param.test_net(i);
		//	create and init net after parsing net_param
		test_nets[i].reset(new Net<Dtype>(net_params[i]));
	}
	if (has_general_test){
		readNetParamsFromTextFileOrDie(param.train_net(), &net_params[0]);
		net_params[0].mutable_state()->set_phase(TEST);
		test_nets[0].reset(new Net<Dtype>(net_params[0]));
	}
}

template <typename Dtype>
void Solver<Dtype>::init(){
#ifndef NO_MPI
	if (param.enable_mpi_device()) Dragon::set_arch(Dragon::DEVICE);
	if (param.enable_mpi_ps()) Dragon::set_arch(Dragon::PS);
	if (Dragon::get_arch() != Dragon::NORMAL){
		//	we need mpi enable MPI_THREAD_MULTIPLE
		int provided;
		MPI_Init_thread(NULL, NULL, MPI_THREAD_MULTIPLE, &provided);
		CHECK_EQ(provided, MPI_THREAD_MULTIPLE) << "require multi thread MPI support";

		MPI_Comm_rank(MPI_COMM_WORLD, &this->mpi_rank);
		MPI_Comm_size(MPI_COMM_WORLD, &this->mpi_size);
		// reset the root solver if specify
		if (param.root_rank() != this->mpi_rank) Dragon::set_root_solver(false);
		else Dragon::set_root_solver(true);
	}
	if (param.enable_mpi_device()){
		CHECK_GE(this->mpi_rank, 0) << "please execute it with mpiexec if you want to use mpi device";
		Dragon::set_device(this->mpi_rank);
	}
#endif
	LOG_IF(INFO, Dragon::get_root_solver()) << "\n\nInitialize solver from parameters: "
		<< endl << param.DebugString();
	CHECK_GE(param.average_loss(), 1) << "average loss should greater equal than 1";
	checkSnapshotWritePermission(); 
	//	set seed for random_generator if necessary
	if (Dragon::get_root_solver() && param.random_seed() >= 0)
		Dragon::set_random_seed(param.random_seed());

	//	create and init a train net
	initTrainNet();
	// non root solver must init test net also
	initTestNets();
	LOG_IF(INFO, Dragon::get_root_solver()) << "solver initialization done";

	if (Dragon::get_arch() == Dragon::PS) enableParameterServer();

	iter = current_step = 0;
}

template <typename Dtype>
string Solver<Dtype>::snapshotFilename(const string extension){
	string filename(param.snapshot_prefix());
	char buffer[20];
#ifdef _WINDOWS_MSVC_
	_snprintf(buffer, 20, "_iter_%d", iter);
#else	
	snprintf(buffer, 20, "_iter_%d", iter);
#endif
	//	"xx_iter_yyy_rank_0.model"
	if (getMPISize() != -1)
		return filename + buffer + "_rank_" + lexical_cast<string>(getMPIRank()) + extension;
	return filename + buffer + extension;
}

template <typename Dtype>
void Solver<Dtype>::checkSnapshotWritePermission(){
	if (Dragon::get_root_solver() && param.snapshot_interval()){
		CHECK(param.has_snapshot_prefix()) << "must specify snapshot_prefix if need snapshot";
		if (!boost::filesystem::exists(param.snapshot_prefix())) boost::filesystem::create_directories(param.snapshot_prefix());
		string filename = snapshotFilename(".tmp");
		ofstream ofs(filename.c_str());
		//	test if write sucessfully
		if (ofs.good()){
			ofs.close();
			//	delete tmp file
			remove(filename.c_str());
		}
		else LOG(FATAL) << "Can not write to snapshot_prefix: " << param.snapshot_prefix() << ".";
	}
}

template <typename Dtype>
void Solver<Dtype>::snapshot(){
	string filename = snapshotFilename(".dragonmodel");
	LOG(PYTHON) << "Snapshot model to binary file: " << filename;
	NetParameter net_param;
	net->ToProto(&net_param, param.snapshot_diff());
	writeProtoToBinaryFile(net_param, filename.c_str());
}

template <typename Dtype>
void Solver<Dtype>::test(int net_id){
	LOG_IF(PYTHON, Dragon::get_root_solver()) << "Train iteration: " << iter << ", Test net #" << net_id << ": ";
	//	share params 
	test_nets[net_id]->shareTrainedLayerWith(net.get());
	vector<Dtype> test_score;
	vector<int> output_id;
	Net<Dtype>* test_net = test_nets[net_id].get();
	Dtype loss = 0;
	//	scan for all batches
	for (int i = 0; i < param.test_iter(net_id);i++){
		Dtype iter_loss;
		const vector<Blob<Dtype>*>& result = test_net->forward(&iter_loss);
		if (param.test_compute_loss()) loss += iter_loss;
		if (i == 0){
			for (int j = 0; j < result.size(); j++){
				const Dtype* base_output = result[j]->cpu_data();
				for (int k = 0; k < result[j]->count(); k++){
					//	fill output position
					test_score.push_back(base_output[k]);
					output_id.push_back(j);
				}
			}
		}else{
			int idx = 0;
			for (int j = 0; j < result.size(); j++){
				const Dtype* base_output = result[j]->cpu_data();
				for (int k = 0; k < result[j]->count(); k++) test_score[idx++] += base_output[k];
			}
		}
	}
	if (param.test_compute_loss()){
		loss /= param.test_iter(net_id);
		LOG_IF(PYTHON, Dragon::get_root_solver()) << "Test loss: " << loss;
	}
	for (int i = 0; i < test_score.size(); i++){
		const int blob_idx = test_net->getOutputBlobIdx()[output_id[i]];
		const string& output_name = test_net->getBlobNames()[blob_idx];
		const Dtype loss_weight = test_net->getBlobLossWeights()[blob_idx];
		ostringstream msg;
		//	per batch
		const Dtype mean_score = test_score[i] / param.test_iter(net_id);
		if (loss_weight)
			msg << " (* " << loss_weight << " = " << loss_weight*mean_score << " loss)";
		LOG_IF(PYTHON, Dragon::get_root_solver()) << "		Test net output #" << i << "(" << output_name << "): "
			<< mean_score << msg.str();
	}
} 

template <typename Dtype>
void Solver<Dtype>::testAll(){
	for (int net_id = 0; net_id < test_nets.size(); net_id++) test(net_id);
}

template <typename Dtype>
void Solver<Dtype>::step(int iters){
	const int start_iter = iter, stop_iter = iter + iters;
	int average_loss = param.average_loss();
	vector<Dtype> loss_vec;
	Dtype smoothed_loss = 0;
	while (iter < stop_iter){
		// clear accumulative diffs in last iter
		net->clearParamDiffs();
		//	cross vaildation or test
		if (param.test_interval() && iter%param.test_interval() == 0)
			if ((iter == 0 && param.test_before_train()) || iter != 0) testAll();

		const bool display = param.display() && iter%param.display() == 0;
		Dtype loss = 0;
		for (int i = 0; i < param.iter_size(); i++) loss += net->forwardBackward();
		loss /= param.iter_size();
		//	smoothed_loss use the last num_(average_loss) iters to average
		//	default use last iter(average_loss=1)
		if (loss_vec.size() < average_loss){
			//	fill
			loss_vec.push_back(loss);
			int size = loss_vec.size();
			smoothed_loss = (smoothed_loss*(size - 1) + loss) / size;
		}
		else{
			//replace
			int idx = (iter - start_iter) % average_loss;
			smoothed_loss += ((loss - loss_vec[idx]) / average_loss);
			loss_vec[idx] = loss;
		}
		applyUpdate();
		if (display&&Dragon::get_root_solver()){
			LOG(PYTHON) << ", loss = " << smoothed_loss;
			int score_idx = 0;
			const vector<Blob<Dtype>*>& result = net->getOutputBlobs();
			for (int i = 0; i < result.size(); i++){
				const Dtype* res_vec = result[i]->cpu_data();
				const string& output_name = net->getBlobNames()[net->getOutputBlobIdx()[i]];
				const Dtype loss_weight = net->getBlobLossWeights()[net->getOutputBlobIdx()[i]];
				for (int j = 0; j < result[i]->count(); j++){
					ostringstream msg;
					if (loss_weight)
						msg << " (* " << loss_weight << " = " << loss_weight*res_vec[j] << " loss)";
					LOG(PYTHON) << "		Train net output #" << i << "(" << output_name << "): " << res_vec[j] << msg.str();
				}
			}
		}
		iter++;
		// snapshot if at the time or necessary
		if ((param.snapshot_interval() && iter%param.snapshot_interval() == 0)) snapshot();
	}
}


template <typename Dtype>
void Solver<Dtype>::restore(const string& filename,int iter = 0){
	CHECK(net.get()) << "must create a net before restore.";
	LOG_IF(INFO, Dragon::get_root_solver()) << "restore previous solver status from" << filename;
	net->copyTrainedLayerFrom(filename);
	// set the current_iter
	this->iter = iter;
	// compute for the history current_step
	if (param.lr_policy() == "multistep"){
		for (int i = 0; i < param.step_value_size(); i++){
			if (iter >= param.step_value(i)) current_step++;
			else break;
		}
	}
}

template <typename Dtype>
void Solver<Dtype>::solve(){
	LOG_IF(INFO, Dragon::get_root_solver()) << "solve: " << net->getNetName();
	//cout<<this->mpi_rank<<" "<<"solve: " << net->getNetName()<<endl;
	LOG_IF(INFO, Dragon::get_root_solver()) << "learning rate policy: " << param.lr_policy();
	//	step
	step(param.max_iter() - iter);
	//	snapshot after train if necessary
	if (param.snapshot_after_train() && (!param.snapshot_interval() || iter%param.snapshot_interval() != 0)) snapshot();
	LOG_IF(INFO, Dragon::get_root_solver()) << "Optimization Done.";
	if(this->mpi_size!=-1) MPI_Finalize();
}

template <typename Dtype>
void Solver<Dtype>::enableParameterServer(){
	if (Dragon::get_arch() != Dragon::PS) return;
#ifndef NO_MPI
	//	init for ParameterServer
	//	must init mpi before
	parameter_server.reset(new ParameterServer<Blob<Dtype> >());

	//	share weights with ParameterServer
	shareWeights();

	int rank = parameter_server->getRank();

	//	enable ParameterServer
	if (rank == 0){
		int processors;
		MPI_Comm_size(MPI_COMM_WORLD, &processors);
		LOG(PYTHON) << "[Server]: Run " << processors << " Parallel Processors.\n"
			<< "                     Server at rank " << rank << ".";
		// run listening service
		if (Dragon::get_arch() == Dragon::PS){
			LOG(PYTHON) << "                     Architecture: ParameterServer";
			LOG(PYTHON) << "                     Listening Service: Enable";
			parameter_server->listen(true);
		}
		else{
			LOG(PYTHON) << "                     Architecture: Others";
			LOG(PYTHON) << "                     Listening Service: Disable";
		}
	}
#endif
}

template <typename Dtype>
void Solver<Dtype>::shareWeights(){
#ifndef NO_MPI
	if (Dragon::get_arch() != Dragon::PS) return;

	const vector < boost::shared_ptr<Blob<Dtype> > >& params = net->getParams();
	//	share params with ParameterServer
	parameter_server->set_params(params);
	//  share ParameterServer with Blobs
	for (int i = 0; i < params.size(); i++){
		params[i]->setParameterServer(parameter_server);
		//	drag from server and all solvers start with the same weights
		if (parameter_server->getRank() != 0) parameter_server->update(i);
	}
#endif
}

INSTANTIATE_CLASS(Solver);