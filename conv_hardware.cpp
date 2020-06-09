
#include "conv_hardware.h"
//image SIZES definitions
//hls::LineBuffer<3,4,int,0>  // A line buffer for the input image
//hls::LineBuffer<3,4,int,0>  // Another line buffer to convolute the input image
//hls::Window<3,3,int>

/*
 * Core that apply a 3x3(Configurable) 2d Convolution, Erode, Dilate on grayscale images
 * http://www.xilinx.com/support/documentation/sw_manuals/xilinx2014_1/ug902-vivado-high-level-synthesis.pdf
 * */

void conv_hardware (hls::stream<uint_8_side_channel> &input,hls::stream<uint_8_side_channel> &output,char kernel[KERNEL_DIM*KERNEL_DIM],int choice)
{
//#pragma HLS INTERFACE ap_ctrl_none port=return

#pragma HLS INTERFACE axis port=input
#pragma HLS INTERFACE axis port=output
#pragma HLS INTERFACE s_axilite port=return bundle=CRTL_BUS
#pragma HLS INTERFACE s_axilite port=choice bundle=CRTL_BUS //0x10 port
#pragma HLS INTERFACE s_axilite port = kernel bundle=KERNEL_BUS

#pragma HLS array_partition variable=kernel complete
//#pragma HLS ARRAY_RESHAPE variable=kernel complete
	hls::LineBuffer<KERNEL_DIM,IMG_COL,unsigned char> line_buffer;

#pragma HLS array_partition variable=line_buffer dim=2 complete

//#pragma HLS DEPENDENCE variable=line_buffer inter WAR false

	hls::Window<KERNEL_DIM,KERNEL_DIM,short> window;

//#pragma HLS array_partition variable line_buffer complete
	int idx_col=0, idx_row =0, pix_conv=0;

	//waiting parameters
	int count_wait=0;
	int sent_pixel=0;
	int wait_ticks= IMG_COL*(KERNEL_DIM-1)+KERNEL_DIM/2 ;

	uint_8_side_channel data_out;
	uint_8_side_channel curr_pix;

//	printf("I shall loop %d times\n ", IMG_COL*IMG_ROW);

	for(int idx_pix=0; idx_pix < (IMG_COL*IMG_ROW);idx_pix++)
	{
#pragma HLS PIPELINE II=2

	/*	printf("The current column is: %d \n", idx_col);
		printf("The current row is: %d \n ", idx_row);
		printf("The num of pixel convolved are: %d \n",pix_conv);
*/
		//Read and cache pixels of input image
	//	printf("I am reading the data\n");
		 curr_pix= input.read();

//get pixel data

		 unsigned char pixel_in= curr_pix.data;

		 /* 4 columns, 3 rows
		  *
		  *Kernel:
		  *Kernel[0:]:   1234
		  *Kernel[1:]:   1546
		  *Kernel[2:]:   1356
		  *
		  *
		  *0000
		  *0000
		  *0000
		  *
		  *0000
		  *0000
		  *1200 //one row of padding is added
		  *
		  *
		  *  //good enough:
		  *000   0
		  *123   4
		  *450   0
		  *
		  *
		  *
		  *0 1234567893 0 //Could be done with padding keeps the size of the image.
		  *0 1234566768 0
		  *0 2342546575 0
		  *0 2454266563 0
		  *
		  *
		  *
		  *
		  *1234
		  *5678
		  *6893
		  * */
//printf("I am doing the shifting\n");

		 line_buffer.shift_up(idx_col); //if idx_col is not a multiple of IMG_COL nothing happens

	//	 printf("I am doing the insertion of a top pixel\n");
		 line_buffer.insert_top(pixel_in,idx_col) ; //

//put data on the window and multiply by kernel

	//	 printf("The line buffers are in\n");

		 for (int idx_win_row=0; idx_win_row<KERNEL_DIM; idx_win_row++)
		 {
			 for (int idx_win_col=0; idx_win_col<KERNEL_DIM; idx_win_col++)
			 {
				 short val= (short) line_buffer.getval(idx_win_row,idx_win_col+pix_conv);
				 val=(short)kernel[idx_win_row*KERNEL_DIM+idx_win_col]*val;

				 //multiplication of value by kernel
				 //val=(short)kernel[idx_win_row*KERNEL_DIM+idx_win_col]*val;

				 window.insert(val,idx_win_row,idx_win_col);
			 }

		 }



	//	 printf("I need to make a decision\n");
		 short val_output=0;
		 count_wait++;
		 if((idx_row>=KERNEL_DIM-1) &&(idx_col>= KERNEL_DIM-1))
		 {

			 switch(choice){

			 	 case CONVOLUTION:
			 	 {
			 		 //printf("I am convoluting\n");
			 		 val_output=sum_window(&window);
			 		 pix_conv++;

			 		 break;
			 	 }

			 	 case MAX_POOL:
			 	 {
			 		 printf("I am max_pooling\n");
			 		 val_output=max_pool(&window);
			 		 break;
			 	 }

			 }

//Waiting time to wait for buffer to be full
				if((count_wait>wait_ticks))
				{
					 data_out.data = val_output;
					 data_out.keep = curr_pix.keep;
					 data_out.strb = curr_pix.strb;
					 data_out.user = curr_pix.user;

					 if(idx_pix>=IMG_COL*IMG_ROW-1)
					 	 data_out.last = 1;
					 else
						 data_out.last=0;
					 data_out.id = curr_pix.id;
					 data_out.dest = curr_pix.dest;

					 output.write(data_out);

					 sent_pixel++;
				}

		 }


		 if(idx_col<IMG_COL-1)
		 {
			 idx_col++;
		 }
		 else
		 {
			 idx_col=0;
		 	 idx_row++;
		 	 pix_conv=0;
		 }
	}


}



short sum_window(hls::Window<KERNEL_DIM,KERNEL_DIM,short>* window)
{
	short accumulator=0;

	for (int idx_row=0;idx_row<KERNEL_DIM; idx_row++)
	{
		for (int idx_col=0; idx_col<KERNEL_DIM; idx_col++)
		{
			accumulator+= (short) window->getval(idx_row,idx_col);
		}
	}

	return accumulator;
}

short max_pool(hls::Window<KERNEL_DIM,KERNEL_DIM,short>* window) //No multiplication for max_pool
{
	unsigned char max_val=0;
	unsigned char temp=0;
	for (int idx_row=0;idx_row<KERNEL_DIM; idx_row++)
		{
			for (int idx_col=0; idx_col<KERNEL_DIM; idx_col++)
			{
				temp=(unsigned char) window->getval(idx_row,idx_col);
				if(max_val>temp)
					max_val= temp;
			}
		}

	return max_val;

}






