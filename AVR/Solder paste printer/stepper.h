/* 
 * File:   stepper.h
 * Author: oletob
 *
 * Created on February 28, 2020, 1:17 PM
 */

#ifndef STEPPER_H
#define	STEPPER_H

#ifdef	__cplusplus
extern "C" {
#endif


#ifdef	__cplusplus
}
#endif

#endif	/* STEPPER_H */

/*Preps the steps that is to be executet in X and Y axis*/
void PrepStep(void);
/*Calculetes the prescaling of TCA0 and period and pulse 
  lenght of the TCB compare registers from the value set from spindle value*/

//void FeedRateCalc(uint16_t speed, int x, int y, enum MotionModes movment);

/*Inits the TCB to run in PWM mode*/
void stepper_TCB_init(void);


/*Step ready flags */
#define X_MSTEP_READY    0
#define Y_MSTEP_READY    1
#define Z_MSTEP_READY    2
#define X_FSTEP_READY    3
#define Y_FSTEP_READY    4
#define Z_FSTEP_READY    5


/*Step line flags */

#define X_LINE_EXE      0
#define Y_LINE_EXE      1
#define Z_LINE_EXE      2

#define X_MSTEP_SET		0
#define Y_MSTEP_SET		1
#define X_FSTEP_SET		2
#define Y_FSTEP_SET		3


enum DirSet
{
    neg_dir, 
    pos_dir
};


typedef struct
{
	enum DirSet x_full;
	enum DirSet x_micro;
	enum DirSet y_full;
	enum DirSet y_micro;
}dirAxis;


typedef struct
{
    volatile uint32_t   full;
    volatile uint8_t    micro;
}count;


typedef struct
{
    count   x,
            y,
            z;
}st_count;


typedef struct
{
	uint8_t full_speed[2];
	uint8_t micro_speed[2];
}st_speedvect;



typedef struct
{
	uint8_t ready;
	uint8_t line;
	uint8_t settings;
}st_flag;

typedef struct
{
    StepVector3 steps;
    StepVector3 last_pos;
	st_speedvect step_speed;
    st_count    counter;
    st_flag     stepflag;
	dirAxis		direction;
	uint16_t	line_number;
	
}st_block;


	
	