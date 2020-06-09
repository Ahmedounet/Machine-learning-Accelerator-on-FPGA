#include <stdint.h>
#include <hls_stream.h>
#include <ap_axi_sdata.h>

//#define INPUT_SIZE 10

typedef ap_axis<32,2,5,6> intSdCh;

void conv_pp (hls::stream<intSdCh> &input,hls::stream<intSdCh> &output,int gain,int in_size)
{


#pragma HLS INTERFACE ap_ctrl_none port=return


#pragma HLS INTERFACE axis port=input
#pragma HLS INTERFACE axis port=output
#pragma HLS INTERFACE s_axilite port=gain bundle=CRTL_BUS //0x10 port
#pragma HLS INTERFACE s_axilite port=in_size bundle=CRTL_BUS
#pragma HLS INTERFACE s_axilite port=return bundle=CRTL_BUS
//#pragma HLS DATAFLOW

	for (int i=0; i< in_size; i++)
	{
#pragma HLS PIPELINE

		intSdCh val_in=input.read();
		intSdCh val_out;

		val_out.data=val_in.data*gain;

		//copy the channel param to the ouptut side

		val_out.keep=val_in.keep;
		val_out.strb=val_in.strb;
		val_out.user=val_in.user;
		val_out.last=val_in.last;
		val_out.id=val_in.id;
		val_out.dest=val_in.dest;

		//send stream to ouptut
		output.write(val_out);


		//IDLE register
	}
/*
	bool last=0;
	int count=0;

	  while(!last)
	  {
			intSdCh val_in=input.read();
			intSdCh val_out;

	  		input.read();

	  		val_out.data=val_in.data*gain; //

	   	val_out.keep=val_in.keep;
			val_out.strb=val_in.strb;
			val_out.user=val_in.user;
			val_out.id=val_in.id;
			val_out.dest=val_in.dest;


			if(count<in_size)
				val_out.last=val_in.last;
			else
				val_out.last=1;


	  		output.write(val_out);

	  		last=val_in.last;
	  		count++;
	  }
*/



	/*
	 * Transfer two rows and then process
	 *
	 * Store 3 rows internally, then process them and while doing so add in another row (
	 *
	 * Do relu
	 *
	 * Do maxpool, transfer 2 rows ouput 1 row for these 2 rows and transfer them to the system.
	 *
	 * 3 loops: recieve, process and send back
	 *
	 */


}
