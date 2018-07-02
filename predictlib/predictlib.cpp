// predictlib.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include <Python.h>  
#include <iostream>
#include <windows.h> 
#include <stdio.h>

using namespace std;  

#define EXPORT_DLL extern "C" __declspec(dllexport) 

struct PredictRes
{
public :
	int size;
	int *result;
};

PyObject *m_pInstance;

EXPORT_DLL int init(const char* pythonpath, const char* modelpath, const char* weightpath) {
	if (strlen(pythonpath) == 0 || strlen(modelpath) == 0 || strlen(weightpath) == 0)
	{
		return 1;
	}
	// python runtime initialization  
	Py_Initialize(); 
	// check the initialize status
	if (!Py_IsInitialized())
	{
		return 2;
	}

	// import sys module
	PyRun_SimpleString("import sys");
	string chdir_cmd = string("sys.path.append(\"");  
	chdir_cmd += pythonpath;
	chdir_cmd += "\")";  
	const char* cstr_cmd = chdir_cmd.c_str();  
	PyRun_SimpleString(cstr_cmd);

	// load the module object  
	PyObject* pModule = PyImport_ImportModule("predict_pcb");
	if (!pModule) {
		return 3;
	}

	PyObject *pDict;
	PyObject *pClass;
	PyObject *pArgs;

	pDict = PyModule_GetDict(pModule);
	pClass = PyDict_GetItemString(pDict, "PredictPCB");
	if(!pClass)
	{
		return 4;
	}

	pArgs = PyTuple_New(2);
	PyTuple_SetItem(pArgs, 0, Py_BuildValue("s", modelpath));
	PyTuple_SetItem(pArgs, 1, Py_BuildValue("s", weightpath));
	m_pInstance = PyEval_CallObject(pClass, pArgs);  
	if(!m_pInstance)
	{
		return 5;
	}

	//PyObject *pRes = PyObject_CallMethod(m_pInstance, "getpath", NULL, NULL);

	//if (!pRes)
	//{
	//	return -1;
	//}

	//int numx = int(PyList_Size(pRes));
	//
	//if (numx < 0)
	//{
	//	return -1;
	//}

	//for(int i = 0; i < numx; i++) {
	//	PyObject *ListItem = PyList_GetItem(pRes, i);
	//	char* result;
	//	PyArg_Parse(ListItem, "s", &result);
	//}
	return 0;
}

EXPORT_DLL void uninit() {
	//if (m_pModule)
	//	Py_XDECREF(m_pModule);
	Py_Finalize();
}

EXPORT_DLL void predict(const char* filePath, PredictRes& pr) {
	Py_Initialize();
	// check the initialize status
	if (!Py_IsInitialized())
	{
		return;
	}

	if (strlen(filePath) == 0) {
		return;
	}

	if (!m_pInstance)
	{
		//printf("%s\n", "Faild to load the Python m_pInstance.");
		return;
	}

	PyObject *pRes = PyObject_CallMethod(m_pInstance, "predict_fn", "s", filePath);

	if (!pRes)
	{
		//printf("%s\n", "Do not receive any result.");
		return;
	}

	int numx = 0;
	int numy = 0;

	numx = int(PyList_Size(pRes));
	if (numx > 0) {
		numy = int(PyList_Size(PyList_GetItem(pRes, 0)));
	} else {
		//printf("rect is empty\n");
		return;
	}

	pr.size = numx * numy;
	pr.result = (int *)malloc(sizeof(int)*pr.size);

	int count = 0;
	int** rect = new int*[numx];
	for(int i = 0; i < numx; i++) {
		PyObject *ListItem = PyList_GetItem(pRes, i);
		rect[i] = new int[numy]; 
		for(int j = 0; j < numy; j++) {
			PyObject *Item = PyList_GetItem(ListItem, j);
			int result;
			PyArg_Parse(Item, "i", &result);
			pr.result[count] = result;
			/*printf("item[%d][%d] = %d;\n", i, j, pr.result[count]);*/
			count += 1;
			//printf("item[%d][%d] = %d;\n", i, j, rect[i][j]);
		}
	}
}

//int _tmain(int argc, _TCHAR* argv[])  
//{  
//	string pythonfile = "D:/fc_example/python";
//	string modelfile = "D:/fc_example/models";
//	string weightpath = "D:/fc_example/weights";
//	init(pythonfile.c_str(), modelfile.c_str(), weightpath.c_str());
//	//string filepath = "D:/fc_example/test.jpg";
//	//int x = predict(filepath.c_str());
//	//   printf("x is: %i\n", x);
//	return 0;
//}  