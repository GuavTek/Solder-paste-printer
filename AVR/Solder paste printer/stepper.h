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
#define EXE_X_LINE      0
#define EXE_Y_LINE      1
#define EXE_Z_LINE      2
#define X_STEP_READY    3
#define Y_STEP_READY    4
#define Z_STEP_READY    5


enum StepMode 
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
    uint32_t    x_s,
                y_s,
                z_s = 0;
    
}step_vector;


typedef struct
{
  volatile uint32_t     x_c,
                        y_c,
                        z_c;
}step_counter;


typedef struct
{   
    step_vector     s_axis;
    
   
    enum StepMode   x_prescale,
                    y_prescale,
                    z_prescale;
    
    
    
    enum DirSet     x_dir,
                    y_dir,
                    z_dir;
    
    
}st_block;