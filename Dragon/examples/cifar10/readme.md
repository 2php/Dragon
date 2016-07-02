# Dragon Examples: Cifar10

## Data
- download cifar10-binary files from [http://www.cs.toronto.edu/~kriz/cifar-10-binary.tar.gz](http://www.cs.toronto.edu/~kriz/cifar-10-binary.tar.gz)
- unzip data_batch_?.bin(1,2,3,4,5) and test_batch.bin to examples/cifar10/download
- run convert_cifar10.sh (Linux) or convert_cifar10.bat (Windows)

## Solutions
### cifar10_quick
- directly run_cifar10_quick.sh (Linux)  run_cifar10_quick.bat (Windows)

### cifar10_quick_mpi_2
we need mpi to run 2 workers to undertake this ensemble cifar10-networks

default you should have 2 GPUs, if you only have one , please configuare cifar10_quick_mpi_2/cifar10_quick_solver.prototxt
- set enable_rank_device: false

which will build the whole net at the default GPU (gpu0)

- Liunx
```Shell
mpiexec -n 2 run_cifar10_quick_mpi_2.sh
```

- Windows
```Shell
mpiexec -n 2 ../../bin/dragon.exe train --solver=./cifar10_quick/cifar10_quick_solver.prototxt --disable_info=false
```

- Helper

![](http://images.cnblogs.com/cnblogs_com/neopenx/690760/o_cifar10_mpi_2.png)



