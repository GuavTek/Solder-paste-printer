/* 
 * File:   stepper.c
 * Author: oletobiasmoen
 *
 * Created on February 28, 2020, 1:17 PM
 */

#include "Header.h"

st_block st;
StepCount Delta(StepCount steps, int laststeps, int coordmode);
enum DirSet StepDir(int steps);
/*StepDir(int *sp, int *pp);*/

void stepper_TCB_init(void)
{
    //enable TCB0
    //all counter registers set to 50% dutycycle
    TCB0.CTRLA = TCB_CLKSEL_CLKTCA_gc | TCB_ENABLE_bm;
	TCB0.CTRLB = TCB_CNTMODE_PWM8_gc;
	TCB0.CCMPL = 0xFF;
	TCB0.CCMPH = 0x80;
	
    //enable TCB1
    TCB1.CTRLA = TCB_CLKSEL_CLKTCA_gc | TCB_ENABLE_bm;
    TCB1.CTRLB |= TCB_CNTMODE_PWM8_gc;
    TCB1.CCMPL = 0xFF;
	TCB1.CCMPH = 0x80;
	
    //enable TCB2
    TCB2.CTRLA = TCB_CLKSEL_CLKTCA_gc;
    TCB2.CTRLB |= TCB_CNTMODE_PWM8_gc;
    TCB2.CTRLB |= TCB_CCMPEN_bm;
    TCB2.CCMP = 0x80FF;

	
	st.stepflag.line = 56;
	st.stepflag.ready = 0;
	
}

void prescale_select(uint8_t sel)
{
	TCA0.SINGLE.PER = 255; 
	
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
    
	
}


void PrepStep(void)
{
    gc_block GetLine;
    StepVector3 delta;
	
    uint8_t buffer_state = BlockBufferAvailable();
    
    if (buffer_state != BUFFER_EMPTY && st.stepflag.line == 56)
    {	
		
		
		
        GetLine = ReadBlockBuffer();
        /*prescale_select(prescale);*/
 
		
		delta.x = Delta(GetLine.pos.x, st.last_pos.x.full, GetLine.coordinateMode);
		delta.y = Delta(GetLine.pos.y, st.last_pos.y.full, GetLine.coordinateMode);
		delta.z = Delta(GetLine.pos.z, st.last_pos.z.full, GetLine.coordinateMode);
		st.direction.x_full = StepDir(delta.x.full);
		st.direction.y_full = StepDir(delta.y.full);
		
		st.step.x.full = abs(delta.x.full);
		st.step.x.micro = abs(delta.x.micro);
		if(st.step.x.full > 0)
		{
			st.stepflag.ready |= (1 << X_FSTEP_READY);
			st.last_pos.x.full = delta.x.full;
		}
		if(st.step.x.micro > 0)
		{
			st.stepflag.ready |= (1 << X_MSTEP_READY);
		}
		if(st.stepflag.ready & (1 << X_FSTEP_READY) || st.stepflag.ready & (1 << X_MSTEP_READY))
		{ 
			st.stepflag.line &= ~(1 << X_LINE_EXE);
			st.stepflag.line |= (1 << X_LINE_READY);
			st.counter.x.full = 0;
			st.counter.x.micro = 0;
			PORTA.DIRSET |= PIN2_bm;
			TCB0.CNT = 0;
			TCB0.CTRLB |= TCB_CCMPEN_bm;
			TCB0.INTCTRL |= TCB_CAPT_bm;
		}
		
		st.step.y.full = abs(delta.y.full);
		st.step.y.micro = abs(delta.y.micro);
		if(st.step.y.full > 0)
		{
			st.stepflag.ready |= (1 << Y_FSTEP_READY);
			st.last_pos.y.full = delta.y.full;
		}
		if(st.step.y.micro > 0)
		{
			st.stepflag.ready |= (1 << Y_MSTEP_READY);
		}
		if(st.stepflag.ready & (1 << Y_FSTEP_READY) || st.stepflag.ready & (1 << Y_MSTEP_READY))
		{
			st.stepflag.line &= ~(1 << Y_LINE_EXE);
			st.stepflag.line |= (1 << Y_LINE_READY);
			st.counter.y.full = 0;
			st.counter.y.micro = 0;
			PORTA.DIRSET |= PIN3_bm;
			TCB1.CNT = 0;
			TCB1.CTRLB |= TCB_CCMPEN_bm;
			TCB1.INTCTRL |= TCB_CAPT_bm;
		}
	}
}


ISR(TCB0_INT_vect) //TCB0 vector
{   
	
    TCB0.INTFLAGS = TCB_CAPT_bm; // clear interrupt flag
    
	if(st.stepflag.line & (1 << X_LINE_READY))
    {
		if(st.stepflag.ready & (1 << X_FSTEP_READY))
		{
            PORTC.OUT |= (st.direction.x_full << 2);
			
            if((st.counter.x.full) == (st.step.x.full - 1))
            {
				st.stepflag.ready &= ~(1 << X_FSTEP_READY);
					
				if(!(st.stepflag.ready & (1 << X_MSTEP_READY)))
				{
					PORTA.DIRCLR |= PIN2_bm;
					st.stepflag.line &= ~(1 << X_LINE_READY);
					st.stepflag.line |= (1 << X_LINE_EXE);
					TCB0.CTRLB &= ~TCB_CCMPEN_bm;
					TCB0.INTCTRL &= ~TCB_CAPT_bm;
					return;	
				}
            }
            else
            {
				st.counter.x.full++;
            }
		}
        else if(st.stepflag.ready & (1 << X_MSTEP_READY))           
		{        
            PORTC.OUT |= (st.direction.x_micro << 2);
                
			if((st.counter.x.micro) == (st.step.x.micro - 1))
			{   
				st.stepflag.ready &= ~(1 << X_MSTEP_READY);
				
				if(!(st.stepflag.ready & (1 << X_FSTEP_READY)))
				{	
					PORTA.DIRCLR |= PIN2_bm;
					st.stepflag.line &= ~(1 << X_LINE_READY);
					st.stepflag.line |= (1 << X_LINE_EXE);
					TCB0.CTRLB &= ~TCB_CCMPEN_bm;
					TCB0.INTCTRL &= ~TCB_CAPT_bm;
					return;
				}
            }
            else
            {
				st.counter.x.micro++;
            }
			
		}
	}
}

ISR(TCB1_INT_vect) //TCB1 vector
{
 TCB1.INTFLAGS = TCB_CAPT_bm; // clear interrupt flag
 
 if(st.stepflag.line & (1 << Y_LINE_READY))
 {
	 if(st.stepflag.ready & (1 << Y_FSTEP_READY))
	 {
		 PORTA.OUT |= (st.direction.y_full << 5);
		 
		 if((st.counter.y.full) == (st.step.y.full - 1))
		 {
			 st.stepflag.ready &= ~(1 << Y_FSTEP_READY);
			 
			 if(!(st.stepflag.ready & (1 << Y_MSTEP_READY)))
			 {
				 PORTA.DIRCLR |= PIN3_bm;
				 st.stepflag.line &= ~(1 << Y_LINE_READY);
				 st.stepflag.line |= (1 << Y_LINE_EXE);
				 TCB1.CTRLB &= ~TCB_CCMPEN_bm;
				 TCB1.INTCTRL &= ~TCB_CAPT_bm;
				 return;
			 }
		 }
		 else
		 {
			 st.counter.y.full++;
		 }
	 }
	 else if(st.stepflag.ready & (1 << Y_MSTEP_READY))
	 {
		 PORTA.OUT |= (st.direction.y_micro << 5);
		 
		 if((st.counter.y.micro) == (st.step.y.micro - 1))
		 {
			 st.stepflag.ready &= ~(1 << Y_MSTEP_READY);
			 
			 if(!(st.stepflag.ready & (1 << Y_MSTEP_READY)))
			 {
				 PORTA.DIRCLR |= PIN3_bm;
				 st.stepflag.line &= ~(1 << Y_LINE_READY);
				 st.stepflag.line |= (1 << Y_LINE_EXE);
				 TCB1.CTRLB &= ~TCB_CCMPEN_bm;
				 TCB1.INTCTRL &= ~TCB_CAPT_bm;
				 return;
			 }
		 }
		 else
		 {
			 st.counter.y.micro++;
		 }
		 
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

StepCount Delta(StepCount steps, int laststeps, int coordmode)
{
	StepCount temp;
	switch(coordmode)
	{
		case(absolute):
			if(steps.full != laststeps)
			{
				
				temp.full = steps.full - laststeps;
				temp.micro = steps.micro;
				
				return temp;
				
			}
			else
			{
				temp.full = 0;
				temp.micro = 0;
				
				return temp;
			}
			
		break;
		
		case(incremental):
			if(steps.full != 0 || steps.micro != 0)
			{
				laststeps += steps.full;
				return steps;
			}
			else
			{
				temp.full = 0;
				temp.micro = 0;
				
				return temp;
			}
		break;
	}
	
	return temp;
}

enum DirSet StepDir(int step)
{	
	if(step != 0)
	{
		if(step > 0)
		{
			return pos_dir;	
		}
		else
		{
			return neg_dir;
		}
	}
	
	return 0;
}
		