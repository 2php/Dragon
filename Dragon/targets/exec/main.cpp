#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>   
#include "common.hpp"
#include "layer_factory.hpp"
#include "utils/io.hpp"
#include "utils/flags.hpp"
#include "solvers/gradient_solver.hpp"
#pragma warning(disable:4099)

//	define format(name , default value, help string)
DEFINE_string(gpu, "",
	"Optional; run in GPU mode on given device IDs separated by ','."
	"Use '-gpu all' to run on all available GPUs. The effective training "
	"batch size is multiplied by the number of devices.");
DEFINE_string(solver, "", "The solver definition protocol buffer text file.");
DEFINE_string(model, "", "The model definition protocol buffer text file.");
DEFINE_string(snapshot, "","Optional; the snapshot solver state to resume training.");
DEFINE_string(weights, "","Optional; the pretrained weights to initialize finetuning.")
DEFINE_int32(current_iter, 0, "Optional; set the start iter if you are using a snapshot.");
DEFINE_bool(disable_info, false, "Optional; disable printing the infos.");

typedef int (*FUNC)();
typedef map<string, FUNC> ArgFactory;
ArgFactory arg_factory;

#define RegisterArgFunction(func) \
class Registerer_##func { \
 public: \
  Registerer_##func() { \
    arg_factory[#func] = &func; \
  } \
};\
Registerer_##func g_registerer_##func

static FUNC getArgFunction(const string& name){
	if (arg_factory.count(name)) return  arg_factory[name];
	else LOG(FATAL) << "Unknown action: " << name;
}



int train(){

	CHECK_GT(FLAGS_solver.size(), 0) << "need a solver to be specified.";
	CHECK(!FLAGS_snapshot.size() || !FLAGS_weights.size()) << "snapshot and weights can not be specified both.";

	SolverParameter solver_param;
	readSolverParamsFromTextFileOrDie(FLAGS_solver, &solver_param);

	if (solver_param.solver_mode()==SolverParameter_SolverMode_CPU){
		LOG(INFO) << "use CPU.";
		Dragon::set_mode(Dragon::CPU);
	}else{
		LOG(INFO) << "use GPU.";
		Dragon::set_device(0);
		Dragon::set_mode(Dragon::GPU);
	}

	Dragon::set_random_seed(solver_param.random_seed());

	//	simple but not use Solver Factory
	boost::shared_ptr<Solver<float> > solver(new SGDSolver<float>(solver_param));

	//	resume or finetune ?
	if (FLAGS_snapshot.size()){
		LOG(INFO) << "resume from: " << FLAGS_snapshot;
		solver->restore(FLAGS_snapshot,FLAGS_current_iter);
	}else if (FLAGS_weights.size()){
		LOG(INFO) << "finetune from: " << FLAGS_weights;
		solver->restore(FLAGS_weights,0);
	}

	LOG_IF(INFO, Dragon::get_root_solver()) << "start optimization.";
	solver->solve();
	LOG_IF(INFO, Dragon::get_root_solver()) << "optimization done.";
	return 0;
}

RegisterArgFunction(train);

int main(int argc,char* argv[]){

	// parse flags
	ParseCommandLineFlags(&argc, &argv);
	// disable printing infos if necessary
	if (FLAGS_disable_info) SetLogDestination(WARNING);
	if (argc == 2) return getArgFunction(string(argv[1]))();

}
