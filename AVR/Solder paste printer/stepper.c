/* 
 * File:   stepper.c
 * Author: oletobiasmoen
 *
 * Created on February 28, 2020, 1:17 PM
 */

#include "Header.h"

st_block st;
step_counter counter;
step_counter counter_coard;
/*Stepper flags is set to seven by default, this because the first
 if statement in the getline function*/

void prescale_select(uint8_t sel)
{
    TCA0.CTRLD = (1 << SPLITM);
    
    switch (sel)
    {
        case 1:
            TCA0.CTRLA = TCA_SPLIT_CLKSEL_DIV2_gc;
            break;
        case 2:
            TCA0.CTRLA = TCA_SPLIT_CLKSEL_DIV4_gc;
            break;
        case 3:
            TCA0.CTRLA = TCA_SPLIT_CLKSEL_DIV8_gc;
            break;
        case 4:
            TCA0.CTRLA = TCA_SPLIT_CLKSEL_DIV16_gc;
            break;
        case 5:
            TCA0.CTRLA = TCA_SPLIT_CLKSEL_DIV64_gc;
            break;
        case 6: 
            TCA0.CTRLA = TCA_SPLIT_CLKSEL_DIV256_gc;   
        default:
            TCA0.CTRLA = TCA_SPLIT_CLKSEL_DIV1024_gc;
    }   
}


void stepper_TCB_init()
{
    //enable TCB0
    //all counter registers set to 50% dutycycle
    TCB0.CTRLA = TCB_ENABLE_bm | TCB_CLKSEL_CLKTCA_gc;
    TCB0.CTRLB = TCB_CNTMODE_PWM8_gc;
    TCB0.CCMPH = 127;
    TCB0.CCMPL = 255;
    
    //enable TCB1
    TCB1.CTRLA = TCB_ENABLE_bm | TCB_CLKSEL_CLKTCA_gc;
    TCB1.CTRLB = TCB_CNTMODE_PWM8_gc;
    TCB1.CCMPH = 127;
    TCB1.CCMPL = 255;
    
    //enable TCB2
    TCB2.CTRLA = TCB_ENABLE_bm | TCB_CLKSEL_CLKTCA_gc;
    TCB2.CTRLB = TCB_CNTMODE_PWM8_gc; 
    TCB2.CCMPH = 127;
    TCB2.CCMPL = 255;
}



void PrepStep(st_block *st)
{
    gc_block GetLine, *pp;
    uint8_t coordinate_mode;
    uint8_t prescale;
    uint8_t buffer_state = BlockBufferAvailable();
    
    pp = &GetLine;
    
    if ((buffer_state != BUFFER_EMPTY) && (st->stepper_flag == 7));
    {
        GetLine = ReadBlockBuffer();

        coordinate_mode = pp-> coordinateMode;
        prescale = pp-> motion;
        st->s_pos = pp-> pos;
        
        switch(coordinate_mode)
        {
            case(absolute):
                
                if(st->s_pos.x > st->s_count.x_c)
                {    
                    st->x_direction = pos_dir;
                }
                else
                {
                    st->x_direction = neg_dir;
                }
                
                if(st->s_pos.y > st->s_count.y_c)
                {    
                    st->y_direction = pos_dir;
                }
                else
                {
                    st->y_direction = neg_dir;
                }
                
                if(st->s_pos.z > st->s_count.z_c)
                {    
                    st->z_direction = pos_dir;
                }
                else
                {
                    st->z_direction = neg_dir;
                }
                
                st->s_delta = st->s_pos;
                st->s_pos = abs(st->s_delta - st->s_count);
                
                break;
                
            case(incremental):
                
                if(st->s_pos.x > 0)
                {
                    st->x_direction = pos_dir;
                }
                else
                {
                    st->x_direction = neg_dir;
                }
                
                if(st->s_pos.y > 0)
                {
                    st->y_direction = pos_dir;
                }
                else
                {
                    st->y_direction = neg_dir;
                }
                
                if(st->s_pos.z > 0)
                {
                    st->z_direction = pos_dir;
                }
                else
                {
                    st.z_direction = neg_dir;
                }
                
                st->s_pos = abs(st->s_pos);
                
                break;        
        }
        
        switch(prescale)
        {
            case(0):
                prescale_select(0);
                break;
            case(1):
                prescale_select(6);
                break;
            case(2):
                prescale_select(6);
                break;
            case(3):
                prescale_select(6);
                break;
            case(4):
                prescale_select(6);
                break;
            /*case(speed):
                prescale_select(6);
                break;*/
        }
                
                    
        if (st->s_pos.x > 0)
        {
            st->stepper_flag |= (1 << X_STEP_READY) & ~(1 << X_STEP_EXE);
            st->s_count = 0;
            TCB0.INTCTRL |= TCB_CAPT_bm; //enable interrupt 
            TCB0.CNT = 0; //reset counter
        }
                
        if (st->s_pos.y > 0)
        {
            st->stepper_flag |= (1 << Y_STEP_READY) & ~(1 << Y_STEP_EXE);
            TCB1.INTCTRL |= TCB_CAPT_bm;
            TCB1.CNT = 0;
        }
                
        if(st->s_pos.z > 0)
        {
            st->stepper_flag |= (1 << Z_STEP_READY) & ~(1 << Z_STEP_EXE);
            TCB2.INTCTRL |= TCB_CAPT_bm;
            TCB2.CNT = 0;
        }
    }
}


ISR(TIMER0_COMPB_vect) //TCB0 vector
{   
    if(st.stepper_flag & (1 << X_STEP_READY))
    {
        PORTA.DIR |= PIN2_bm;
        PORTC.DIR |= (st.x_direction << PIN2);
        
        if((st.s_count.x_c) == (st.s_pos.x - 1))
        {
            st.stepper_flag |= (1 << X_STEP_EXE) & ~(1 << X_STEP_READY);
            PORTA.DIR &= ~PIN2_bm; //clear PA2 as output when line is done
            TCB0.INTCTRL &=  ~TCB_CAPT_bm; // disable interrupt when line is done
            TCB0.INTFLAGS = TCB_CAPT_bm; // clear interrupt flag
            return 0;
        }
        else
        {
            st.s_count.x_c++;
        }
    }  
    TCB0.INTFLAGS = TCB_CAPT_bm; // clear interrupt flag
}

ISR(TIMER1_COMPB_vect) //TCB1 vector
{
    if(st.stepper_flag & (1 << Y_STEP_READY))
    {
        PORTA.DIR |= PIN3_bm;
        PORTA.DIR |= (st.y_direction << PIN5);
        
        if(st.s_count.y_c == st.s_pos.y - 1)
        {
            st.stepper_flag |= (1 << Y_STEP_EXE) & ~(1 << Y_STEP_READY);
            PORTA.DIR &= ~PIN3_bm; //clear PA2 as output when line is done
            TCB1.INTCTRL &=  ~TCB_CAPT_bm; // disable interrupt when line is done
            TCB1.INTFLAGS = TCB_CAPT_bm; // clear interrupt flag
            return 0;
        }
        else
        {
            st.s_count.y_c++;
        }
    }
    
    TCB1.INTFLAGS = TCB_CAPT_bm; // clear interrupt flag
}

ISR(TIMER2_COMPB_vect) //TCB2 vector
{       
    if(st.stepper_flag & (1 << Z_STEP_READY))
    {
        PORTC.DIR |= PIN0_bm;
        PORTA.DIR |= (st.z_direction << PIN6);
        
        if(st.s_count.z_c == st.s_pos.z - 1)
        {
            st.stepper_flag |= (1 << Z_STEP_EXE) & ~(1 << Z_STEP_READY);
            PORTC.DIR &= ~PIN0_bm; //clear PINC0 as output when line is done
            TCB2.INTCTRL &= ~TCB_CAPT_bm; // disable interrupt when line is done
            return 0;
        }
        else
        {
            st.s_count.z_c++;
        }
    }    
    TCB2.INTFLAGS = TCB_CAPT_bm; // clear interrupt flag   
}