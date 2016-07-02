# Dragon: A Light Deep Learning Framework

## Specifics !
- Light 3rdparty: Protobuf / Boost and Install Within 10 Minutes

- CaffeModels Support

- Distributed Architectures Support
 	- Device Parallelism: Use muti-GPUs to run a huge Neural Network(e.g. Res-Net)
![](http://images.cnblogs.com/cnblogs_com/neopenx/690760/o_mpi.png)
	- Data Parallelism: Update several Neural Networks with a ParameterServer
	- Python Parallelism: Be compatible with Python

- Symbols Support (in development)

- Cross Platforms (Linux / Windows) and United Deployment


![](http://images.cnblogs.com/cnblogs_com/neopenx/690760/o_cross.jpg)

## How to Install ?
### Requirements
1. CUDA
2. Anaconda [Optional]
3. Microsoft MPI / OpenMPI [Optional]

### Installation 
1. clone this repository

2. download and install [CUDA](https://developer.nvidia.com/cuda-toolkit)
	- we recommend install CUDA8 or higher to support GCC5/VS2015

3. download 3rdparty.zip and unzip to any path
	- [Win64_VS2013_Release](http://pan.baidu.com/s/1c28xTsc)
	- [Win64_VS2015_Release](http://pan.baidu.com/s/1dFuiDTv)
	- Linux64_GCC4 (Use GCC5 but compile libprotobuf with GCC4)
	- [Linux64_GCC5](http://pan.baidu.com/s/1geZfRHL)
	- we recommond unzip to /home/xxx/3rdparty (Linux) or C:/3rdparty (Windows)

4. install [Anaconda](https://www.continuum.io/downloads) with x64-py2.7  [Optional]
	- allow importing environment variable into bashrc (Linux) / PATH (Windows)
	- install protobuf
	```Shell
	pip install protobuf
	```

5. configure Dragon/CMakeLists.txt
	- select optional libraries [PYTHON / MPI / CUDA_AWARE_MPI]
	- set 3rdparty path
	- set python path  [Optional]
	- set cuda compiling architectures if necessary

6. set environment variables
	- Linux:
		- create dragon.conf
		```Shell
		sudo gedit /etc/ld.so.conf.d/dragon.conf
		```
		- append 2 lines with ld dirs for your 3rdparty, e.g: 
		 	- /home/xxx/3rdparty/bin
		 	- /home/xxx/3rdparty/lib
		- rebuild the scaning cache
		```Shell 
		sudo ldconfig
		```
	- Windows
		- add 3rdparty/bin to system environment variables, e.g:
			- PATH=........;C:\3rdparty\bin;

7. install MPI [Optional]
	- Linux:
		- we use OpenMPI which support "cuda-aware-mpi"
		- see more: 
			- https://devblogs.nvidia.com/parallelforall/introduction-cuda-aware-mpi/
			- https://www.open-mpi.org/faq/?category=buildcuda
		- configure 3rdparty/setup_mpi.sh
			- disable cuda-aware support if necessary
		- run 3rdparty/setup_mpi.sh
		```Shell
		sudo ./setup_mpi.sh
		```
	- Windows:
		- we use Microsoft MPI which can perfectly run at lastest Windows10
		- Microsoft MPI is intergrated into 3rdparty and you should do nothing

8. compile
	- Linux: 
		- install cmake
		```Shell
  		sudo apt-get install cmake
  		```
  		- configure Dragon/main_install.sh
			- set 3rdparty path
		- run Dragon/main_install.sh
		```Shell
		sudo ./main_install.sh
		```
	- Windows:
	 	- run Dragon/bin/gen_proto.bat
		- install cmake-gui
		- mkdir Dragon/build
		- configure and generate MSVC project in Dragon/build
		- open Dragon/build/Dragon.sln
		- compile and generate for "INSTALL" solution

9. install PyDragon [Optional]
	- Linux:
		- configure Dragon/py_install.sh
			- set 3rdparty path
			- set anaconda path
		- run Dragon/py_install.sh
		```Shell
  		sudo ./py_install.sh
  		```
	- Windows:
		- configure Dragon/py_install.bat
			- set 3rdparty path
			- set anaconda path
		- run Dragon/py_install.bat

## Hints

Developer: PanTing, HeFei University of Technology at Xuancheng

We will change the codes frequently

This repo is also regarded as the Developer's Bachelor Graduation Project

## License and Citation

Dragon is a distribution of the [Caffe](https://github.com/BVLC/caffe)(BVLC)、[MPI-Caffe](https://github.com/steflee/mpi-caffe)(Stefan Lee)

Please cite their projects firstly in your publications if it helps your research

