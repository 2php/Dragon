#ifndef SOLVER_HPP
#define SOLVER_HPP

#include "net.hpp"

template <typename Dtype>
class Solver{
public:
	Solver(const SolverParameter& param);
	Solver(const string& param_file);
	void init();
	~Solver() { }
	void initTrainNet();
	void initTestNets();
	string snapshotFilename(const string extension);
	void checkSnapshotWritePermission();
	void solve();
	void snapshot();
	void restore(const string& filename,int iter);
	void test(int net_id);
	void testAll();
	void step(int iters);
	//	implemented by different ways
	virtual void applyUpdate() = 0;
	boost::shared_ptr<Net<Dtype> > getTrainNet() { return net; }
	const vector<boost::shared_ptr<Net<Dtype> > >& getTestNets() { return test_nets; }
	int getIter() { return iter; }
	void setIter(int iter) { this->iter = iter; }
	void enableParameterServer();
	void shareWeights();
	int getMPIRank() { return mpi_rank; }
	int getMPISize() { return mpi_size; }
protected:
	SolverParameter param;
	boost::shared_ptr<Net<Dtype> > net;
#ifndef NO_MPI
	boost::shared_ptr<ParameterServer<Blob<Dtype> > > parameter_server;
#endif 
	vector<boost::shared_ptr<Net<Dtype> > > test_nets;
	int iter, current_step, mpi_rank, mpi_size;
};
#endif