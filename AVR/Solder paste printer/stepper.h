/* 
 * File:   stepper.h
 * Author: oletobiasmoen
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


void stepper_TCB_init(void);

void prescale_select(uint8_t sel);

/*Step ready flags */
#define X_MSTEP_READY    0
#define Y_MSTEP_READY    1
#define Z_MSTEP_READY    2
#define X_FSTEP_READY    3
#define Y_FSTEP_READY    4
#define Z_FSTEP_READY    5


/*Step line flags */
#define X_LINE_READY    0
#define Y_LINE_READY    1
#define Z_LINE_READY    2
#define X_LINE_EXE      3
#define Y_LINE_EXE      4
#define Z_LINE_EXE      5


enum stepper_pres 
{
    full_step,
    half_step,
    quarter_step,
    eight_step,
    sixteen_step
};

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
	uint8_t ready;
	uint8_t line;
}st_flag;

typedef struct
{
    StepVector3 steps;
    StepVector3 last_pos;
    st_count    counter;
    st_flag     stepflag;
	dirAxis		direction;
    enum stepper_pres s_velosity;
}st_block;


	
	