#define FUSE_USE_VERSION 29

#include <fuse.h>
#include <Python.h>
#include <stdio.h>
#include <string.h>
#include "util.h"

struct User *user = NULL;
char buffer_path[1000] = "/tmp/thfs/";

int GetCourseFile(struct Course *course) {
    if(course->file_list != NULL) return 1;

    printf("Info: Get Course %s File Start\n", course->name);
    Py_Initialize();
    PyRun_SimpleString("import sys");
    PyRun_SimpleString("sys.path.append('./')");

    PyObject *pModule = PyImport_ImportModule("util");
    if (pModule == NULL) {
        printf("Error: PyImport ImportModule\n");
        return -1;
    }
    PyObject *pFunc = PyObject_GetAttrString(pModule, "GetCourseFile");
    if (pFunc == NULL) {
        printf("Error: PyObject GetAttrString\n");
        return -1;
    }
    PyObject *pArgs = PyTuple_New(3);
    PyTuple_SetItem(pArgs, 0, Py_BuildValue("s", course->id));
    PyTuple_SetItem(pArgs, 1, Py_BuildValue("s", user->id));
    PyTuple_SetItem(pArgs, 2, Py_BuildValue("s", user->password));
    PyObject *pReturn = PyEval_CallObject(pFunc, pArgs);
    if (pReturn == NULL) {
        printf("Error: PyEval CallObject\n");
        return -1;
    }
    int ret_num =  PyList_Size(pReturn);
    int i;
    for(i = 0; i < ret_num; i += 4) {
        struct File *file = (struct File *)malloc(sizeof(struct File));
        char *buffer;
        PyArg_Parse(PyList_GetItem(pReturn, i), "s", &buffer);    
        strcpy(file->link, buffer);
        PyArg_Parse(PyList_GetItem(pReturn, i + 1), "s", &buffer); 
        strcpy(file->type, buffer);
        PyArg_Parse(PyList_GetItem(pReturn, i + 2), "i", &file->size); 
        PyArg_Parse(PyList_GetItem(pReturn, i + 3), "s", &buffer);   
        strcpy(file->name, buffer);
        file->next = course->file_list;
        memset(file->path, 0, sizeof(file->path));
        course->file_list = file;
    }
    course->file_num = ret_num / 4;

    Py_Finalize();
    printf("Info: Get Course %s File End\n", course->name);
    return 0;
}

int GetCourseNote(struct Course *course) {
    if(course->note_list != NULL) return 1;
    
    printf("Info: Get Course %s Note Start\n", course->name);
    Py_Initialize();

    PyRun_SimpleString("import sys");
    PyRun_SimpleString("sys.path.append('./')");
    PyObject *pModule = PyImport_ImportModule("util");
    if (pModule == NULL) {
        printf("Error: PyImport ImportModule\n");
        return -1;
    }
    PyObject *pFunc = PyObject_GetAttrString(pModule, "GetCourseNote");
    if (pFunc == NULL) {
        printf("Error: PyObject GetAttrString\n");
        return -1;
    }
    PyObject *pArgs = PyTuple_New(4);
    PyTuple_SetItem(pArgs, 0, Py_BuildValue("s",buffer_path ));
    PyTuple_SetItem(pArgs, 1, Py_BuildValue("s",course->id ));
    PyTuple_SetItem(pArgs, 2, Py_BuildValue("s", user->id));
    PyTuple_SetItem(pArgs, 3, Py_BuildValue("s", user->password));
    PyObject *pReturn = PyEval_CallObject(pFunc, pArgs);
    if (pReturn == NULL) {
        printf("Error: PyEval CallObject\n");
        return -1;
    }
    int ret_num =  PyList_Size(pReturn);
    int i;
    for(i = 0; i < ret_num; i += 3) {
        struct File *file = (struct File*)malloc(sizeof(struct File));
        memset(file->path, 0, sizeof(file->path));
        memset(file->name, 0, sizeof(file->name));
        char *buffer;
        PyArg_Parse(PyList_GetItem(pReturn, i), "s", &buffer);    
        strcpy(file->path, buffer);
        PyArg_Parse(PyList_GetItem(pReturn, i + 1), "s", &buffer);   
        strcpy(file->name, buffer);
        PyArg_Parse(PyList_GetItem(pReturn, i + 2), "i", &file->size); 
        file->next = course->note_list;
        course->note_list = file;
    }
    course->note_num = ret_num / 3;

    Py_Finalize();
    printf("Info: Get Course %s Note End\n", course->name);
    return 0;
}

int GetCourseInfo(struct Course *course) {
    if(strlen(course->info) > 0) return 1;
    printf("Info: Get Course %s Info Start\n", course->name);
    
    Py_Initialize();
    PyRun_SimpleString("import sys");
    PyRun_SimpleString("sys.path.append('./')");
    PyObject *pModule = PyImport_ImportModule("util");
    if (pModule == NULL) {
        printf("Error: PyImport ImportModule\n");
        return -1;
    }
    PyObject *pFunc = PyObject_GetAttrString(pModule,"GetCourseInfo");
    if(pFunc == NULL) {
        printf("Error: PyObject GetAttrString\n");
        return -1;
    }
    PyObject *pArgs = PyTuple_New(3);
    PyTuple_SetItem(pArgs, 0, Py_BuildValue("s", course->id));
    PyTuple_SetItem(pArgs, 1, Py_BuildValue("s", user->id));
    PyTuple_SetItem(pArgs, 2, Py_BuildValue("s", user->password));
    PyObject *pReturn = PyEval_CallObject(pFunc, pArgs);
    if (pReturn == NULL) {
        printf("Error: PyEval CallObject\n");
        return -1;
    }
    char *buffer;
    int info_len = 0;
    PyArg_Parse(PyList_GetItem(pReturn, 0), "i", &info_len);
    PyArg_Parse(PyList_GetItem(pReturn, 1), "s", &buffer);
    if (info_len > sizeof(course->info)) info_len = sizeof(course->info);
    memcpy(course->info, buffer, info_len);
    course->info_len = info_len;
    Py_Finalize();
    printf("Info: Get Course %s Info End\n", course->name);
    return 0;
}

int DownloadFile(struct File *file) {
    Py_Initialize();
    printf("Info: Get File %s Start\n", file->name);

    PyRun_SimpleString("import sys");
    PyRun_SimpleString("sys.path.append('./')");
    PyObject *pModule = PyImport_ImportModule("util");
    if (pModule == NULL) {
        printf("Error: PyImport ImportModule\n");
        return -1;
    }
    PyObject *pFunc = PyObject_GetAttrString(pModule, "DownloadFile");
    if (pFunc == NULL) {
        printf("Error: PyObject GetAttrString\n");
        return -1;
    }
    PyObject *pArgs = PyTuple_New(4);
    PyTuple_SetItem(pArgs, 0, Py_BuildValue("s", file->path ));
    PyTuple_SetItem(pArgs, 1, Py_BuildValue("s", file->link ));
    PyTuple_SetItem(pArgs, 2, Py_BuildValue("s", user->id));
    PyTuple_SetItem(pArgs, 3, Py_BuildValue("s", user->password));
    PyObject *pReturn = PyEval_CallObject(pFunc, pArgs);
    if (pReturn == NULL) {
        printf("Error: PyEval CallObject\n");
        return -1;
    }
    Py_Finalize();
    printf("Info: Get File %s End\n", file->name);
    return 0;
}

int UploadFile(struct File *file) {
    Py_Initialize();
    printf("Info: Upload File %s Start\n", file->name);

    PyRun_SimpleString("import sys");
    PyRun_SimpleString("sys.path.append('./')");
    PyObject *pModule = PyImport_ImportModule("util");
    if (pModule == NULL) {
        printf("Error: PyImport ImportModule\n");
        return -1;
    }
    PyObject *pFunc = PyObject_GetAttrString(pModule, "UploadFile");
    if (pFunc == NULL) {
        printf("Error: PyObject GetAttrString\n");
        return -1;
    }
    PyObject *pArgs = PyTuple_New(3);
    PyTuple_SetItem(pArgs, 0, Py_BuildValue("s", file->path));
    PyTuple_SetItem(pArgs, 1, Py_BuildValue("s", user->id));
    PyTuple_SetItem(pArgs, 2, Py_BuildValue("s", user->password));
    PyObject *pReturn = PyEval_CallObject(pFunc, pArgs);
    if (pReturn == NULL) {
        printf("Error: PyEval CallObject\n");
        return -1;
    }
    Py_Finalize();
    printf("Info: Upload File %s End\n", file->name);
    return 0;
}

int GetUserCourse() {
    printf("Info: Get User Courses Start\n");
    Py_Initialize();

    PyRun_SimpleString("import sys");
    PyRun_SimpleString("sys.path.append('./')");
    PyObject *pModule = PyImport_ImportModule("util");
    if (pModule == NULL) {
        printf("Error: PyImport ImportModule\n");
        return -1;
    }
    PyObject *pFunc = PyObject_GetAttrString(pModule, "GetUserCourse");
    if(pFunc == NULL) {
        printf("Error: PyObject GetAttrString\n");
        return -1;
    }
    PyObject *pArgs = PyTuple_New(2);
    PyTuple_SetItem(pArgs, 0, Py_BuildValue("s", user->id));
    PyTuple_SetItem(pArgs, 1, Py_BuildValue("s", user->password));
    PyObject *pReturn = PyEval_CallObject(pFunc, pArgs);
    if (pReturn == NULL) {
        printf("Error: PyEval CallObject\n");
        return -1;
    }
    int retNum =  PyList_Size(pReturn);
    int i;
    for(i = 0; i < retNum; i += 2) {
        struct Course *course = (struct Course*)malloc(sizeof(struct Course));
        memset(course, 0, sizeof(struct Course));
        char *buffer;
        PyArg_Parse(PyList_GetItem(pReturn, i), "s", &buffer);
        strcpy(course->name, buffer);
        PyArg_Parse(PyList_GetItem(pReturn, i + 1), "s", &buffer);
        strcpy(course->id, buffer);
        course->next = user->course_list;
        course->file_list = NULL;
        course->note_list = NULL;
        course->info_len = 0;
        GetCourseInfo(course);
        user->course_list = course;
    }
    user->course_num = retNum / 2;

    Py_Finalize();
    printf("Info: Get User Courses End\n");
    return 0;
}

struct File* SearchFile(const char *path, struct Course *course, char *child, struct File *file_list) {
    const char *filename = path + 1 + strlen(course->name) + 1 + strlen(child) + 1;
    struct File *file = file_list;
    while (file) {
        if (strcmp(file->name, filename) == 0)
            return file;
        file = file->next;
    }
    return NULL;
}