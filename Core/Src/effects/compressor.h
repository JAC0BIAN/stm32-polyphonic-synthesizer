/*
 * compressor.h
 *
 *  Created on: Oct 27, 2026
 *      Author: Artur
 */

#ifndef SRC_EFFECTS_COMPRESSOR_H_
#define SRC_EFFECTS_COMPRESSOR_H_

#include <stdint.h>
#include <stddef.h>
#include "../filters/hpf.h"

#define	COMP_CTRL_RATE 		16u				// must divide MAX_BLOCK_FRAMES
#define COMP_DB_TO_LOG		(1.0f / 6.02f)	// 20log10(2) ~ 6.02 dB and log(2) is faster to calculate

typedef struct{
	HPF			cm_hpf;			// sidechain high pass filter

	// const params
	float		thresh;			// threshold of compression
	float 		knee;			// width of the compression knee - higher values smooth out the compression, 0 causes the compression to begin right at the threshold value
	float		slope;			// 1 - 1/ratio 	- how much the compressor takes from signals that exceed the threshold
	float		makeup_gain;	// compensates the overall volume

	float		a_rms;			// RMS averaging coefficient
	float		a_att;			// attach smoothing
	float		a_rel;			// release smoothing

	// state values
	float 		ms_acc;			// average from the next COMP_CTRL_RATE samples ^ 2
	float 		ms;				// smoothed mean square
	float		comp_gr;		// current compression value (always negative)
	float		gain;			// gain
	float		gain_step;		// per sample gain increment
	uint32_t	ctrl_cnt;		// sample counter
}Compressor;

void	compressor_init(Compressor *c);
float	compressor_process(Compressor *c, float x);


#endif /* SRC_EFFECTS_COMPRESSOR_H_ */
