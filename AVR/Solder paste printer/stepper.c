/* 
 * File:   stepper.c
 * Author: oletobiasmoen
 *
 * Created on February 28, 2020, 1:17 PM
 */

#include "Header.h"



st_block st;

st.stepflag.line = 56;
st.stepflag.ready = 0;

void stepper_TCB_init()
{
    //enable TCB0
    //all counter registers set to 50% dutycycle
    TCB0.CTRLA = TCB_ENABLE_bm | TCB_CLKSEL_CLKTCA_gc;
    TCB0.CTRLB = TCB_CNTMODE_PWM8_gc;
    TCB0.CTRLB |= TCB_CCMPEN_bm;
    TCB0.CCMP = 0x80FF;
    
    
    //enable TCB1
    TCB1.CTRLA = TCB_ENABLE_bm | TCB_CLKSEL_CLKTCA_gc;
    TCB1.CTRLB = TCB_CNTMODE_PWM8_gc;
    TCB1.CTRLB |= TCB_CCMPEN_bm;
    TCB1.CCMP = 0x80FF;
    
    
    //enable TCB2
    TCB2.CTRLA = TCB_ENABLE_bm | TCB_CLKSEL_CLKTCA_gc;
    TCB2.CTRLB = TCB_CNTMODE_PWM8_gc;
    TCB2.CTRLB |= TCB_CCMPEN_bm;
    TCB2.CCMP = 0x80FF;
}

void prescale_select(uint8_t sel)
{
    switch (sel)
    {
        case 1:
            TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV2_gc;
            break;
        case 2:
            TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV4_gc;
            break;
        case 3:
            TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV8_gc;
            break;
        case 4:
            TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV16_gc;
            break;
        case 5:
            TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV64_gc;
            break;
        case 6: 
            TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV256_gc;   
        default:
            TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1024_gc;
    }
    
    TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm;
    TCA0.SINGLE.PER = 255;
    
}


void PrepStep(void)
{
    gc_block GetLine;
    StepVector3 delta;
    uint8_t prescale;
    uint8_t buffer_state = BlockBufferAvailable();
    
    if ((buffer_state != BUFFER_EMPTY) && (st.stepflag.line == 56));
    {
        GetLine = ReadBlockBuffer();
        
        switch(GetLine.coordinateMode)
        {
            case(absolute):
                delta.x.full = GetLine.pos.x.full - st.last_pos.x.full;
                delta.y.full = GetLine.pos.y.full - st.last_pos.y.full;
                delta.z.full = GetLine.pos.z.full - st.last_pos.z.full;
                delta.x.micro = GetLine.pos.x.micro;
                delta.y.micro = GetLine.pos.y.micro;
                delta.z.micro = GetLine.pos.z.micro;
                st.last_pos.x.full = GetLine.pos.x.full;
                st.last_pos.y.full = GetLine.pos.y.full;
                st.last_pos.z.full = GetLine.pos.z.full;
                break;
                
            case(incremental):
                delta = GetLine.pos;
                /*Incremental value gets added, 
                  to keep track on absolute position*/
                /*st->last_pos += delta;*/
                break;
        }
        
        if(delta.x.full != 0)
        {
            if(delta.x.full > 0)
            {    
                st.direction.x.full = pos_dir;
            }
            else
            {
                st.direction.x.full = neg_dir;     
            }
            st.step.x.full = abs(delta.x.full);
            st.counter.x.full = 0;
            st.stepflag.ready |= (1 << X_FSTEP_READY); 
        }

        if(delta.x.micro != 0)
        {
            if(delta.x.micro > 0)
            {
                st.direction.x.micro = pos_dir;
            }
            else
            {
                st.direction.x.micro = neg_dir;
            }
            
            st.step.x.micro = abs(delta.x.micro);
            st.counter.x.micro = 0;
            st.stepflag.ready |= (1 << X_MSTEP_READY);
        }
        
        if(delta.y.full != 0)
        {
            if(delta.y.full > 0)
            {    
                st.direction.y.full = pos_dir;
            }
            else
            {
                st.direction.y.full = neg_dir;
            }
            st.step.y.full = abs(delta.y.full);
            st.counter.y.full = 0;
            st.stepflag.ready |= (1 << Y_FSTEP_READY);
            
        }
        
        if(delta.y.micro != 0)
        {
            if(delta.y.micro > 0)
            {
                st.direction.y.micro = pos_dir;
            }
            else
            {
                st.direction.y.micro = neg_dir;
            }
            
            st.step.y.micro = abs(delta.y.micro);
            st.counter.y.micro = 0;
            st.stepflag.ready |= (1 << Y_MSTEP_READY);
        }
                
        if(delta.z.full != 0)
        {
            if(delta.z.full > 0)
            {    
                st.direction.z.full = pos_dir;
            }
            else
            {
                st.direction.z.full = neg_dir;
            }
            
            st.step.z.full = abs(delta.z.full);
            st.counter.z.full = 0;
            st.stepflag.ready |= (1 << Z_FSTEP_READY);
            
        }
        
        if(delta.z.micro != 0)
        {
            if(delta.z.micro > 0)
            {
                st.direction.z.micro = pos_dir;
            }
            else
            {
                st.direction.z.micro = neg_dir;
            }
            
            st.step.z.micro = abs(delta.y.micro);
            st.counter.z.micro = 0;
            st.stepflag.ready |= (1 << Z_MSTEP_READY);
        }
        
        prescale_select(st.s_velosity);
        
        if(st.stepflag.ready & ((1 << X_FSTEP_READY) | (1 << X_MSTEP_READY)))
        {   
            st.stepflag.line |= (1 << X_LINE_READY);
            st.stepflag.line &= ~(1 << X_LINE_EXE);
            TCB0.CNT = 0;
            TCB0.INTCTRL |= TCB_CAPT_bm; 
        }
        
        if(st.stepflag.ready & ((1 << Y_FSTEP_READY) | (1 << Y_MSTEP_READY)))
        {
            st.stepflag.line |= (1 << Y_LINE_READY);
            st.stepflag.line &= ~(1 << Y_LINE_EXE);
            TCB1.CNT = 0;
            TCB1.INTCTRL |= TCB_CAPT_bm; 
        }
        
        if(st.stepflag.ready & ((1 << Z_FSTEP_READY) | (1 << Z_MSTEP_READY)))
        {
            st.stepflag.line |= (1 << Z_LINE_READY);
            st.stepflag.line &= ~(1 << Z_LINE_EXE);
            TCB2.CNT = 0;
            TCB2.INTCTRL |= TCB_CAPT_bm;
        }
    }
}


ISR(TIMER0_COMPB_vect) //TCB0 vector
{   
    TCB0.INTFLAGS = TCB_CAPT_bm; // clear interrupt flag
    
    if(st.stepflag.line & (1 << X_LINE_READY))
    {
        switch(st.stepflag.ready)
        {
            case((1 << X_FSTEP_READY)):
                    
                PORTA.DIR |= PIN2_bm;
                PORTC.DIR |= (st.direction.x.full << PIN2);
        
                if((st.counter.x.full) == (st.step.x.full - 1))
                {
                    st.stepflag.ready &= ~(1 << X_FSTEP_READY);
                    PORTA.DIR &= ~PIN2_bm; //clear PA2 as output when line is done
          
                    return;
                }
                else
                {
                    st.counter.x.full++;
                }
                break;
                    
            case((1 << X_MSTEP_READY)):
                
                PORTA.DIR |= PIN2_bm;
                PORTC.DIR |= (st.direction.x.micro << PIN2);
                
                if((st.counter.x.micro) == (st.step.x.micro - 1))
                {   
                    st.stepflag.ready &= ~(1 << X_MSTEP_READY);
                    PORTA.DIR &= ~PIN2_bm; //clear PA2 as output when line is done
          
                    return;
                }
                else
                {
                    st.counter.x.micro++;
                }
                break;
                
            default:
                
                st.stepflag.line &= ~(1 << X_LINE_READY);
                st.stepflag.line = (1 << X_LINE_EXE);
                TCB0.INTCTRL &= ~TCB_CAPT_bm; 
                break;
        }
    }
}

ISR(TIMER1_COMPB_vect) //TCB1 vector
{
    TCB1.INTFLAGS = TCB_CAPT_bm; // clear interrupt flag
     
    if(st.stepflag.line & (1 << Y_LINE_READY))
    {
        switch(st.stepflag.ready)
        {
            case((1 << Y_FSTEP_READY)):
                    
                PORTA.DIR |= PIN3_bm;
                PORTA.DIR |= (st.direction.y.full << PIN5);
        
                if((st.counter.y.full) == (st.step.y.full - 1))
                {
                    st.stepflag.ready &= ~(1 << Y_FSTEP_READY);
                    PORTA.DIR &= ~PIN3_bm; //clear PA2 as output when all steps is done
          
                    return;
                }
                else
                {
                    st.counter.y.full++;
                }
                break;
                    
            case((1 << Y_MSTEP_READY)):
                
                PORTA.DIR |= PIN3_bm;
                PORTA.DIR |= (st.direction.y.micro << PIN5);
                
                if((st.counter.y.micro) == (st.step.y.micro - 1))
                {   
                    st.stepflag.ready &= ~(1 << Y_MSTEP_READY);
                    PORTA.DIR &= ~PIN3_bm; //clear PA3 as output when all steps is done
                    return;
                }
                else
                {
                    st.counter.y.micro++;
                }
                break;
                
            default:
                
                st.stepflag.line &= ~(1 << Y_LINE_READY);
                st.stepflag.line = (1 << Y_LINE_EXE);
                TCB1.INTCTRL &= ~TCB_CAPT_bm; 
                break;
        }
    }
}


/*ISR(TIMER2_COMPB_vect) //TCB2 vector
{   
    if(st.stepflag.line & (1 << Z_LINE_READY))
    {
        switch(st.stepflag.ready)
        {
            case((1 << Z_FSTEP_READY)):
                    
                PORTC.DIR |= PIN0_bm;
                PORTA.DIR |= (st.direction.z.full << PIN6);
        
                if((st.counter.z.full) == (st.step.z.full - 1))
                {
                    st.stepflag.ready &= ~(1 << Z_FSTEP_READY);
                    PORTC.DIR &= ~PIN0_bm; //clear PC0 as output when line is done
          
                    return;
                }
                else
                {
                    st.counter.z.full++;
                }
                break;
                    
            case((1 << Z_MSTEP_READY)):
                
                PORTA.DIR |= PIN0_bm;
                PORTA.DIR |= (st.direction.z.micro << PIN6);
                
                if((st.counter.z.micro) == (st.step.z.micro - 1))
                {   
                    st.stepflag.ready &= ~(1 << Z_MSTEP_READY);
                    PORTC.DIR &= ~PIN0_bm; //clear PC0 as output when line is done
          
                    return;
                }
                else
                {
                    st.counter.z.micro++;
                }
                break;
                
            case((1 << Z_LINE_RET)):
                
                if(st.stepflag.ret & (1 << Z_FSTEP_RET))
                {    
                    PORTA.DIR |= PIN0_bm;
                    PORTA.DIR ^= (st.direction.z.full << PIN6);
                
                    if((st.counter.z.full) == (st.step.z.full_ret))
                    {
                        PORTA.DIR &= ~PIN0_bm;
                        st.stepflag.ret &= ~(1 << Z_FSTEP_RET);
                        
                        return;
                    }
                    else
                    {
                        st.counter.z.full--;
                    }
                }
                
                else if(st.stepflag.ret & (1 << Z_MSTEP_RET))
                {    
                    PORTA.DIR |= PIN0_bm;
                    PORTA.DIR ^= (st.direction.z.micro << PIN6);
                
                    if((st.counter.z.micro) == (st.step.z.micro_ret))
                    {
                        PORTA.DIR &= ~PIN0_bm;
                        st.stepflag.ret &= ~(1 << Z_MSTEP_RET);
                        
                        return;
                    }
                    else
                    {
                        st.counter.z.micro--;
                    }
                }
                break;
        }
    }
    
    TCB2.INTFLAGS = TCB_CAPT_bm;// clear interrupt flag
}*/