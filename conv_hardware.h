
#define IMG_ROW 10
#define IMG_COL 10
#define KERNEL_DIM 3
#define INPUT_SIZE 1000

enum choice {CONVOLUTION, MAX_POOL,RELU};


#include <stdint.h>
#include <hls_stream.h>
#include <ap_axi_sdata.h>
#include <hls_video.h>



typedef ap_axiu<8,2,5,6> uint_8_side_channel;
typedef ap_axis<8,2,5,6> int_8_side_channel;

short sum_window(hls::Window<KERNEL_DIM,KERNEL_DIM,short>* window);

short max_pool(hls::Window<KERNEL_DIM,KERNEL_DIM,short>* window);
void conv_hardware (hls::stream<uint_8_side_channel> &input,hls::stream<uint_8_side_channel> &output,char kernel[KERNEL_DIM*KERNEL_DIM],int choice);
