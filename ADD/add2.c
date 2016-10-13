#include <CL/cl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "err_code.h"
#include "cl_helper.h"

int main(int argc, char const* argv[])
{
  const size_t num_elems = 1000000;
  const size_t buf_size = sizeof(cl_int) * num_elems;

  cl_int err;
  clh_device_datas *devices;
  int d_num=0, p_num=0, i;
  devices = clh_getDevices(&d_num, &p_num);

  int d_cel, p_cel;
  char name1_cel[1024];
  char name2_cel[1024];
  long int max_mem;


  printf("--------------------------\n");
  clh_showInfo(devices, d_num, p_num);
  printf("--------------------------\n");
  
  
  printf("which Platform : ");
  scanf("%d", &p_cel);
  printf("which Device : ");
  scanf("%d", &d_cel);
  printf("--------------------------\n");

  err = clh_selectDevice(devices, d_num, p_num, d_cel, p_cel, name1_cel, name2_cel, &max_mem);
  if(err!=0){
    printf("Select Device Error\n");
    exit(EXIT_FAILURE);
  }

  printf("Select Platform -> [%d] %s\n", p_cel, name1_cel);
  printf("Select Device -> [%d] %s [%li]\n", d_cel, name2_cel, max_mem);
  printf("--------------------------\n");

//----------------------------------------------
  
  cl_int *data = (cl_int *)malloc(buf_size);
  for(i=0; i<num_elems; i++){
    data[i] = i;
  }

  cl_platform_id platform[p_num];
  err = clGetPlatformIDs(p_num, platform, NULL);
  checkError(err, "get platform");

  cl_device_id device[d_num];
  err = clGetDeviceIDs(platform[p_cel], CL_DEVICE_TYPE_ALL, d_num, device, NULL);
  checkError(err, "get device");

  const cl_context_properties prop[]={
    CL_CONTEXT_PLATFORM, (cl_context_properties)platform[p_cel],
    0
  };

  //Create context
  cl_context ctx = clCreateContext(prop, 1, &device[d_cel], NULL, NULL, &err);
  checkError(err, "create context");

  //Create program
  unsigned char* program_file = NULL;
  size_t program_size = 0;
  clh_readFile(&program_file, &program_size, "vec_add.cl");

  cl_program program = clCreateProgramWithSource(ctx, 1, (const char **)&program_file, &program_size, &err);
  checkError(err, "create program");

  //build program
  err = clBuildProgram(program, 1, &device[d_cel], NULL, NULL, NULL);
  checkError(err, "create program");

  free(program_file);

  //Allocate 
  cl_mem a = clCreateBuffer(ctx, CL_MEM_READ_ONLY, buf_size, NULL, &err);
  checkError(err, "alloc");
  cl_mem b = clCreateBuffer(ctx, CL_MEM_READ_ONLY, buf_size, NULL, &err);
  checkError(err, "alloc");
  cl_mem c = clCreateBuffer(ctx, CL_MEM_WRITE_ONLY, buf_size, NULL, &err);
  checkError(err, "alloc");

  //craete command queue
  cl_command_queue queue = clCreateCommandQueueWithProperties(ctx, device[d_cel], NULL, &err);
  checkError(err, "command queue");

  //enqueue the write buffer commands
  cl_event wb_events[2];

  err = clEnqueueWriteBuffer(queue, a, CL_FALSE, 0, buf_size, data, 0, NULL, &wb_events[0]);
  checkError(err, "enqueue 1");
  err = clEnqueueWriteBuffer(queue, b, CL_FALSE, 0, buf_size, data, 0, NULL, &wb_events[1]);
  checkError(err, "enqueue 2");

  //enqueue kernel execution command
  cl_kernel kernel = clCreateKernel(program, "vec_add", &err);
  checkError(err, "kernel");
  err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &c);
  checkError(err, "kernel arg");
  err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &a);
  checkError(err, "kernel arg");
  err = clSetKernelArg(kernel, 2, sizeof(cl_mem), &b);
  checkError(err, "kernel arg");

  double s_time = clh_wtime();
  
  //start queue
  const size_t global_offset = 0;
  cl_event kernel_event;
  err = clEnqueueNDRangeKernel(queue, kernel, 1, &global_offset, &num_elems, NULL, 2, wb_events, &kernel_event);
  checkError(err, "run kernel ");

  err = clFinish(queue);
  checkError(err, "finish");

  double e_time = clh_wtime();
  double times = e_time - s_time;

  err = clEnqueueReadBuffer(queue, c, CL_TRUE, 0, buf_size, data, 1, &kernel_event, NULL);
  checkError(err, "read buffer");

  err = clFinish(queue);
  checkError(err, "finish");

  // Release the resources
  clReleaseMemObject(a);
  clReleaseMemObject(b);
  clReleaseMemObject(c);
  clReleaseKernel(kernel);
  clReleaseProgram(program);
  clReleaseCommandQueue(queue);
  clReleaseContext(ctx);
  clReleaseDevice(device[d_cel]);

  for(i = 0; i < num_elems; ++i) {
    if (data[i] != 2 * i) {
      fprintf(stderr, "Failed: %u\n", (unsigned)i);
    }
  }
  printf("Times=%8f\n", times);

  printf("Done!\n");
  return 0;
}

