#ifndef CL_HELPER_H_INCLUDED__
#define CL_HELPER_H_INCLUDED__

#include <stdlib.h>
#include <string.h>

#ifdef _OPENMP
#include <omp.h>
#else
#include <sys/time.h>
#endif

#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#ifdef __cplusplus
#include <cstdio>
#endif

typedef struct{
  int platformID;
  char platformName[2048];
  int deviceID;
  char deviceName[2048];
  long int max_mem_size;
  cl_device_type deviceType;
} clh_device_datas;

// wtime 
double clh_wtime(){
#ifdef _OPENMP
  return omp_get_wtime();
#else
  static int sec = -1;
  struct timeval tv;
  gettimeofday(&tv, NULL);
  if(sec < 0)
    sec = tv.tv_sec;
  return (tv.tv_sec - sec) + 1.0e-6 * tv.tv_usec;
#endif
}

// readfile
int clh_readFile(unsigned char **output, size_t *size, const char *name){
  FILE *fp= fopen(name, "rb");
  if(!fp){
    return -1;
  }
  fseek(fp, 0, SEEK_END);
  *size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  *output = (unsigned char *)malloc(*size);
  if(!*output){
    fclose(fp);
    return -1;
  }

  fread(*output, *size, 1, fp);
  fclose(fp);
  return 0;
}

clh_device_datas *clh_getDevices(int *all_num, int *r_p_num){
  int i, j, all_count=0;
  cl_int err;
  cl_uint p_num;
  clh_device_datas *devices;
  char name[1024];
  char name2[1024];
  cl_uint d_num;

  // count platform number
  err = clGetPlatformIDs(0, NULL, &p_num);
  // checkError(err, "count platforms");
  if(p_num == 0){
    printf("Found 0 platforms\n");
    exit(EXIT_FAILURE);
  }
  
  //alloc p_data
  cl_platform_id p_data[p_num];
  // get platform data
  err = clGetPlatformIDs(p_num, p_data, NULL);
  // checkError(err, "get platforms");
  
  *r_p_num = p_num;

  for(i=0;i<p_num;i++){
    cl_uint all_num;

    // get all the devices in this platform
    err = clGetDeviceIDs(p_data[i], CL_DEVICE_TYPE_ALL, 0, NULL, &all_num);

    if(err != -1){
      all_count += all_num;
    }
  }
  // alloc devices
  *all_num = all_count;
  devices = (clh_device_datas*)malloc(sizeof(clh_device_datas)*(*all_num)); 

  all_count--;
  for(i=0;i<p_num;i++){
    int inner_count = 0;
    // get this platform name
    err = clGetPlatformInfo(p_data[i], CL_PLATFORM_NAME, sizeof(name), &name, NULL);
    // checkError(err, "get platform name");

    //CPU
    err = clGetDeviceIDs(p_data[i], CL_DEVICE_TYPE_CPU, 0, NULL, &d_num);
    if(err != -1){
      cl_device_id cpu_data[d_num];
      cl_ulong mem_size;
      err = clGetDeviceIDs(p_data[i], CL_DEVICE_TYPE_CPU, d_num, cpu_data, NULL);
      // checkError(err, "get cpu");
      for(j=0;j<d_num;j++){
        err = clGetDeviceInfo(cpu_data[j], CL_DEVICE_NAME, sizeof(name2), &name2, NULL);
        // checkError(err, "get cpu name");
        err = clGetDeviceInfo(cpu_data[j], CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(cl_ulong), &mem_size, NULL);
        // checkError(err, "get cpu memsize");

        devices[all_count].platformID = i;
        strcpy(devices[all_count].platformName, name);
        devices[all_count].deviceID = inner_count;
        strcpy(devices[all_count].deviceName, name2);
        devices[all_count].deviceType = CL_DEVICE_TYPE_CPU;
        devices[all_count].max_mem_size = mem_size; 
        inner_count++;
        all_count--;
      }
    }

    //GPU
    err = clGetDeviceIDs(p_data[i], CL_DEVICE_TYPE_GPU, 0, NULL, &d_num);
    if(err != -1){
      cl_device_id gpu_data[d_num];
      cl_ulong mem_size;
      err = clGetDeviceIDs(p_data[i], CL_DEVICE_TYPE_GPU, d_num, gpu_data, NULL);
      // checkError(err, "get cpu");
      for(j=0;j<d_num;j++){
        err = clGetDeviceInfo(gpu_data[j], CL_DEVICE_NAME, sizeof(name2), &name2, NULL);
        // checkError(err, "get gpu name");
        err = clGetDeviceInfo(gpu_data[j], CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(cl_ulong), &mem_size, NULL);
        // checkError(err, "get cpu memsize");

        devices[all_count].platformID = i;
        strcpy(devices[all_count].platformName, name);
        devices[all_count].deviceID = inner_count;
        strcpy(devices[all_count].deviceName, name2);
        devices[all_count].deviceType = CL_DEVICE_TYPE_GPU;
        devices[all_count].max_mem_size = mem_size; 
        inner_count++;
        all_count--;
      }
    }

    //ACCELERATOR
    err = clGetDeviceIDs(p_data[i], CL_DEVICE_TYPE_ACCELERATOR, 0, NULL, &d_num);
    if(err != -1){
      cl_device_id act_data[d_num];
      cl_ulong mem_size;
      err = clGetDeviceIDs(p_data[i], CL_DEVICE_TYPE_GPU, d_num, act_data, NULL);
      // checkError(err, "get act");
      for(j=0;j<d_num;j++){
        err = clGetDeviceInfo(act_data[j], CL_DEVICE_NAME, sizeof(name2), &name2, NULL);
        // checkError(err, "get accelerator name");
        err = clGetDeviceInfo(act_data[j], CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(cl_ulong), &mem_size, NULL);
        // checkError(err, "get cpu memsize");

        devices[all_count].platformID = i;
        strcpy(devices[all_count].platformName, name);
        devices[all_count].deviceID = inner_count;
        strcpy(devices[all_count].deviceName, name2);
        devices[all_count].deviceType = CL_DEVICE_TYPE_ACCELERATOR;
        devices[all_count].max_mem_size = mem_size; 
        inner_count++;
        all_count--;
      }
    }
  }
  return (devices);
}

void clh_showInfo(const clh_device_datas *devices, const int d_num, const int p_num){
  int i;
  for(i=d_num-1;i>=0;i--){
    printf("P[%d]->%s\n", devices[i].platformID, devices[i].platformName);
    printf("     D[%d]->%s [%li]", devices[i].deviceID, devices[i].deviceName, devices[i].max_mem_size);
    if(devices[i].deviceType == CL_DEVICE_TYPE_CPU){
      printf(" [CPU]\n");
    }else if(devices[i].deviceType == CL_DEVICE_TYPE_GPU){
      printf(" [GPU]\n");
    }else if(devices[i].deviceType == CL_DEVICE_TYPE_ACCELERATOR){
      printf(" [ACCELERATOR]\n");
    }
  }
}

int clh_selectDevice(const clh_device_datas *devices, 
                      const int d_num, const int p_num, const int d_cel, const int p_cel,
                      char *name1, char *name2, long int *max_mem){
  int pass_flag = 0, i;
  for(i=0; i<d_num; i++){
    int p_tmp = devices[i].platformID;
    if(p_tmp == p_cel){
      int d_tmp = devices[i].deviceID;
      if(d_tmp == d_cel){
        strcpy(name1, devices[i].platformName);
        strcpy(name2, devices[i].deviceName);
        *max_mem = devices[i].max_mem_size;
        pass_flag = 1;
      }
    }
  }
  if(!pass_flag){
    printf("Platform Device error\n");
    return -1;
  }
  return 0;

}

#endif //CL_HELPER_H_INCLUDED__

