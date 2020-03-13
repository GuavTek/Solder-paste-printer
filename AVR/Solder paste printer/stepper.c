/* 
 * File:   stepper.c
 * Author: oletobiasmoen
 *
 * Created on February 28, 2020, 1:17 PM
 */

#include "Header.h"

st_block st;
void PrepStep(void);
StepCount Delta(StepCount steps, StepCount laststeps, int coordmode);
enum DirSet StepDir(int steps);
void FeedRateCalc(uint16_t speed);
void PerSelect(uint8_t per);

void PrepStep(void)
{
    StepVector3 delta;	  
    if(st.stepflag.line == 56  && currentState.blockFinished == true)
    {
		
		delta.x = Delta(theCurrentBlock.pos.x, st.last_pos.x, theCurrentBlock.coordinateMode);
		delta.y = Delta(theCurrentBlock.pos.y, st.last_pos.y, theCurrentBlock.coordinateMode);
		delta.z = Delta(theCurrentBlock.pos.z, st.last_pos.z, theCurrentBlock.coordinateMode);
		st.direction.x_full = StepDir(delta.x.full);
		st.direction.y_full = StepDir(delta.y.full);
		st.direction.x_micro = StepDir(delta.x.micro);
		
		st.steps.x.full = abs(delta.x.full);
		st.steps.x.micro = abs(delta.x.micro);
		if(st.steps.x.full > 0)
		{
			st.stepflag.ready |= (1 << X_FSTEP_READY);
			st.last_pos.x.full = abs(theCurrentBlock.pos.x.full);
		}
		if(st.steps.x.micro > 0)
		{
			st.stepflag.ready |= (1 << X_MSTEP_READY);
			st.last_pos.x.micro = abs(theCurrentBlock.pos.x.micro);
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
		
		st.steps.y.full = abs(delta.y.full);
		st.steps.y.micro = abs(delta.y.micro);
		if(st.steps.y.full > 0)
		{
			st.stepflag.ready |= (1 << Y_FSTEP_READY);
			st.last_pos.y.full = abs(theCurrentBlock.pos.x.full);
		}
		if(st.steps.y.micro > 0)
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
	else
	{
		TCB0.CTRLB &= ~TCB_CCMPEN_bm;
		TCB1.CTRLB &= ~TCB_CCMPEN_bm;
	}
}




ISR(TCB0_INT_vect) //TCB0 vector
{   
	
    TCB0.INTFLAGS = TCB_CAPT_bm; // clear interrupt flag
    
	if(st.stepflag.line & (1 << X_LINE_READY))
    {
		if(st.stepflag.ready & (1 << X_FSTEP_READY))
		{	
			switch(st.direction.x_full)
			{
				case(pos_dir):
					PORTC.OUT |= PIN3_bm;
				break;
				case(neg_dir):
					PORTC.OUT &= ~PIN3_bm;
				break;
			}
			
			
		
            if((st.counter.x.full) == (st.steps.x.full - 1))
            {
				st.stepflag.ready &= ~(1 << X_FSTEP_READY);
				
					if(st.counter.x.full == 6)
					{
						TX_write('+');
					}
				
					
				if(!(st.stepflag.ready & (1 << X_MSTEP_READY)))
				{
					PORTA.DIRCLR |= PIN2_bm;
					st.stepflag.line &= ~(1 << X_LINE_READY);
					st.stepflag.line |= (1 << X_LINE_EXE);
					TCB0.CTRLB &= ~TCB_CCMPEN_bm;
					
				}
            }
            else
            {
				st.counter.x.full++;
            }
		}
        else if(st.stepflag.ready & (1 << X_MSTEP_READY))           
		{        
            switch(st.direction.x_micro)
            {
	            case(pos_dir):
					PORTC.OUT |= PIN3_bm;
	            break;
				case(neg_dir):
					PORTC.OUT &= ~PIN3_bm;
	            break;
            }
                
			if((st.counter.x.micro) == (st.steps.x.micro - 1))
			{   
				st.stepflag.ready &= ~(1 << X_MSTEP_READY);
				
				if(!(st.stepflag.ready & (1 << X_FSTEP_READY)))
				{	
					PORTA.DIRCLR |= PIN2_bm;
					st.stepflag.line &= ~(1 << X_LINE_READY);
					st.stepflag.line |= (1 << X_LINE_EXE);
					TCB0.CTRLB &= ~TCB_CCMPEN_bm;
				}
            }
            else
            {
				st.counter.x.micro++;
            }
		}
	}
	
	if(st.stepflag.line ==  56)
	{	
		currentState.blockFinished = true;
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
			 
			 switch(st.direction.y_full)
			 {
				 case(pos_dir):
				 PORTA.OUT |= PIN5_bm;
				 break;
				 case(neg_dir):
				 PORTA.OUT &= ~PIN5_bm;
				 break;
			 }
		 
			if((st.counter.y.full) == (st.steps.y.full - 1))
			{
				st.stepflag.ready &= ~(1 << Y_FSTEP_READY);
				
				if(!(st.stepflag.ready & (1 << Y_MSTEP_READY)))
				{
					PORTA.DIRCLR |= PIN3_bm;
					st.stepflag.line &= ~(1 << Y_LINE_READY);
					st.stepflag.line |= (1 << Y_LINE_EXE);
					TCB1.CTRLB &= ~TCB_CCMPEN_bm;
				}
			}
			else
			 {
				st.counter.y.full++;
			}
		}
		else if(st.stepflag.ready & (1 << Y_MSTEP_READY))
		{
			switch(st.direction.y_micro)
			{
				case(pos_dir):
				PORTA.OUT |= PIN5_bm;
				break;
				case(neg_dir):
				PORTA.OUT &= ~PIN5_bm;
				break;
			}
		 
			if((st.counter.y.micro) == (st.steps.y.micro - 1))
			{
				st.stepflag.ready &= ~(1 << Y_MSTEP_READY);
				
				if(!(st.stepflag.ready & (1 << Y_FSTEP_READY)))
				{
					PORTA.DIRCLR |= PIN3_bm;
					st.stepflag.line &= ~(1 << Y_LINE_READY);
					st.stepflag.line |= (1 << Y_LINE_EXE);
					TCB1.CTRLB &= ~TCB_CCMPEN_bm;
				}
			}
			else
			{
				st.counter.y.micro++;
			}
		}
	}
	
	if(st.stepflag.line  ==  56)
	{	
		currentState.blockFinished = true;
		return;
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

StepCount Delta(StepCount steps, StepCount laststeps, int coordmode)
{
	StepCount temp;
	switch(coordmode)
	{
		case(absolute):
			if(steps.full != laststeps.full)
			{
				temp.full = steps.full - laststeps.full;
			}
			else
			{
				temp.full = 0;
			}
			if(steps.micro != laststeps.micro)
			{
				temp.micro = steps.micro - laststeps.micro;
			}
			else
			{
				temp.micro = 0;
			}
			return temp;
		break;
		
		case(incremental):
			if(steps.full != 0 || steps.micro != 0)
			{
				laststeps.full += steps.full;
				laststeps.micro += steps.micro;
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
		
void FeedRateCalc(uint16_t speed)
{
	if(speed > 0)
	{
		uint8_t per;
		float comp;
		float temp;
	
		if(speed > 778)
		{
			speed = 778;
		}
	
		temp = METRIC_STEP_LENGTH*(fCLK_PER/speed);
		comp = temp;
	
		for(uint8_t i = 0; pow(2,i) <= 1024; i++)
		{
			comp /= pow(2,i);
			comp = floor(comp); 
		
			if((comp <= 255) && (i != 9 || i != 5 || i != 7))
			{
				per = comp;
				prescale_select(i);
				PerSelect(per);
				return;		
			}
			comp = temp;
		}
	}
	else
	{
		prescale_select(10);
	}
}	

void stepper_TCB_init(void)
{
	//enable TCB0
	//all counter registers set to 50% dutycycle
	TCB0.CTRLA = TCB_CLKSEL_CLKTCA_gc | TCB_ENABLE_bm;
	TCB0.CTRLB = TCB_CNTMODE_PWM8_gc;
	TCB0.INTCTRL = TCB_CAPT_bm;
	TCB0.CCMPL = 0xFF;
	TCB0.CCMPH = 0x80;
	
	//enable TCB1
	TCB1.CTRLA = TCB_CLKSEL_CLKTCA_gc | TCB_ENABLE_bm;
	TCB1.CTRLB |= TCB_CNTMODE_PWM8_gc;
	TCB1.INTCTRL = TCB_CAPT_bm;
	TCB1.CCMPL = 0xFF;
	TCB1.CCMPH = 0x80;
	
	//enable TCB2
	TCB2.CTRLA = TCB_CLKSEL_CLKTCA_gc;
	TCB2.CTRLB |= TCB_CNTMODE_PWM8_gc;
	TCB2.CCMP = 0x80FF;

	PORTA.DIRSET |= PIN7_bm;
	PORTC.DIRSET |= PIN3_bm;
	st.stepflag.line = 56;
	st.stepflag.ready = 0;
	st.steps.x.full = 0;
	st.steps.x.micro = 0;
	st.last_pos.x.full = 0;
	st.last_pos.x.micro = 0;
}

void PerSelect(uint8_t per)
{
	uint8_t pulse = floor(per/2);
	
	TCB0.CCMPL = per;
	TCB0.CCMPH = pulse;
	TCB1.CCMPL = per;
	TCB1.CCMPH = pulse;
}

void prescale_select(uint8_t sel)
{
	TCA0.SINGLE.PER = 255;
	
	switch (sel)
	{
		case(0):
			TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1_gc;
		break;
		case(1):
			TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV2_gc;
		break;
		case(2):
			TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV4_gc;
		break;
		case(3):
			TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV8_gc;
		break;
		case(4):
			TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV16_gc;
		break;
		case(6):
			TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV64_gc;
		break;
		case(8):
			TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV256_gc;
		case(10):
			TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1024_gc;
		break;
		default:
			TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1024_gc;
	}
	TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm;
}