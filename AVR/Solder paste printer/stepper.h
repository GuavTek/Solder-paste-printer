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


/*Stepper Flags */
#define X_STEP_EXE      0
#define Y_STEP_EXE      1
#define Z_STEP_EXE      2
#define X_STEP_READY    3
#define Y_STEP_READY    4
#define Z_STEP_READY    5


enum MoveSpeed 
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
    uint32_t    x,
                y,
                z; 
}s_vector;


typedef struct
{
    int32_t     x,
                y,
                z;
   
}temp_pos;


typedef struct
{
    volatile uint32_t   x_c = 0,
                        y_c = 0,
                        z_c = 0;
}step_counter;


typedef struct
{
    s_vector        s_pos; 
    temp_pos        s_delta;
    step_counter    s_count;
    uint8_t         stepper_flag = 7;
    enum DirSet     x_direction,
                    y_direction,
                    z_direction;
    
    enum MoveSpeed s_velosity;
}st_block;