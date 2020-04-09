/* 
 * File:   stepper.c
 * Author: oletob
 *
 * Created on February 28, 2020, 1:17 PM
 */

#include "Header.h"

st_block st;

/*sets prescaling of the TCA0 clk*/
void prescale_select(uint8_t sel);
/*calculates steps.*/
StepCount Delta(StepCount steps, StepCount laststeps, int coordmode);
/*Sets the direction of the steps*/
enum DirSet StepDir(int steps);
/*sets the period and pulse lenght of TCB PWM on X and Y axis*/ 
void PerSelect(uint8_t per);

uint16_t current_line(uint16_t new_line, uint16_t last_line);

void PrepStep(void)
{
	
	/*steps get prepped here. calculates the amount of steps according 
	to how far it has stepped, sets the direction and stores the value 
	from the current block in last_steps*/
    StepVector3 delta;	  
    if(st.stepflag.line == 56 && currentState.blockFinished == true)
    {
		
		delta.x = Delta(theCurrentBlock.pos.x, st.last_pos.x, theCurrentBlock.coordinateMode);
		delta.y = Delta(theCurrentBlock.pos.y, st.last_pos.y, theCurrentBlock.coordinateMode);
		st.direction.x_full = StepDir(delta.x.full);
		st.direction.y_full = StepDir(delta.y.full);
		st.direction.x_micro = StepDir(delta.x.micro);
		st.direction.y_micro = StepDir(delta.y.micro);
		FeedRateCalc(theCurrentBlock.moveSpeed);
		st.line_number = current_line(theCurrentBlock.blockNumber, st.line_number);
		
		
		
		if(!(delta.x.full == 0))
		{
			st.steps.x.full = abs(delta.x.full);
			st.stepflag.ready |= (1 << X_FSTEP_READY);
			st.last_pos.x.full = theCurrentBlock.pos.x.full;
			st.counter.x.full = 0;
		}
		
		if(!(delta.x.micro == 0))
		{
			st.steps.x.micro = abs(delta.x.micro);
			st.stepflag.ready |= (1 << X_MSTEP_READY);
			st.last_pos.x.micro = theCurrentBlock.pos.x.micro;
			st.counter.x.micro = 0;
		}
		
		if(st.stepflag.ready & (1 << X_FSTEP_READY) || st.stepflag.ready & (1 << X_MSTEP_READY))
		{ 
			st.stepflag.line &= ~(1 << X_LINE_EXE);
			st.stepflag.line |= (1 << X_LINE_READY);
			TCB0.CNT = 0;
			TCB0.CTRLB |= TCB_CCMPEN_bm;
			TCB0.INTCTRL |= TCB_CAPT_bm;
		}
	
		if(delta.y.full != 0)
		{
			st.steps.y.full = abs(delta.y.full);
			st.stepflag.ready |= (1 << Y_FSTEP_READY);
			st.last_pos.y.full = theCurrentBlock.pos.y.full;
			st.counter.y.full = 0;
		}
		if(delta.y.micro != 0)
		{
			st.steps.y.micro = abs(delta.y.micro);
			st.stepflag.ready |= (1 << Y_MSTEP_READY);
			st.last_pos.y.micro = theCurrentBlock.pos.y.micro;
			st.counter.y.micro = 0;
		}
		if(st.stepflag.ready & (1 << Y_FSTEP_READY) || st.stepflag.ready & (1 << Y_MSTEP_READY))
		{
			st.stepflag.line &= ~(1 << Y_LINE_EXE);
			st.stepflag.line |= (1 << Y_LINE_READY);
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
			switch(st.direction.x_full)
			{
				case(pos_dir):
					PORTD.OUT |= PIN7_bm;
				break;
				
				case(neg_dir):
					PORTD.OUT &= ~PIN7_bm;
				break;
			}
					
            if((st.counter.x.full) == (st.steps.x.full - 1))
            {
				st.stepflag.ready &= ~(1 << X_FSTEP_READY);
				
				if(!(st.stepflag.ready & (1 << X_MSTEP_READY)))
				{
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
					PORTD.OUT |= PIN7_bm;
	            break;
				case(neg_dir):
					PORTD.OUT &= ~PIN7_bm;
	            break;
            }
                
			if((st.counter.x.micro) == (st.steps.x.micro - 1))
			{   
				st.stepflag.ready &= ~(1 << X_MSTEP_READY);
				if(!(st.stepflag.ready & (1 << X_FSTEP_READY)))
				{	
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
	
	else if(st.stepflag.line ==  56)
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
			switch(st.direction.y_full)
			{
				case(pos_dir):
				PORTC.OUT |= PIN6_bm;
				break;
				case(neg_dir):
				PORTC.OUT &= ~PIN6_bm;
				break;
			}
		 
			if((st.counter.y.full) == (st.steps.y.full - 1))
			{
				st.stepflag.ready &= ~(1 << Y_FSTEP_READY);
				
				if(!(st.stepflag.ready & (1 << Y_MSTEP_READY)))
				{
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
				PORTC.OUT |= PIN6_bm;
				break;
				case(neg_dir):
				PORTC.OUT &= ~PIN5_bm;
				break;
			}
		 
			if((st.counter.y.micro) == (st.steps.y.micro - 1))
			{
				st.stepflag.ready &= ~(1 << Y_MSTEP_READY);
				
				if(!(st.stepflag.ready & (1 << Y_FSTEP_READY)))
				{
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
	
	else if(st.stepflag.line  ==  56)
	{	
		currentState.blockFinished = true;
	}
}


StepCount Delta(StepCount steps, StepCount laststeps, int coordmode)
{
	StepCount temp;
	switch(coordmode)
	{
		case(absolute):
			
			temp.full = steps.full - laststeps.full;
			temp.micro = steps.micro - laststeps.micro;
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
		
void FeedRateCalc(uint16_t speed)
{	
	if(speed > 0)
	{
		uint8_t per = 0;
		uint16_t comp = 0;
		uint16_t temp = 0;
	
		if(speed > 778)
		{
			speed = 778;
		}
	
		temp = round(METRIC_STEP_LENGTH*(fCLK_PER/speed));
	
		for(uint8_t i = 0; pow(2,i) <= 1024; i++)
		{
			comp = round(temp / pow(2,i));
		
			if((comp <= 255) && !(i == 5 || i == 7 || i == 9))
			{
				per = comp;
				prescale_select(i);
				PerSelect(per);
				return;		
			}
		}
	}
	prescale_select(10);
	return;
}

void PerSelect(uint8_t per)
{
	uint8_t pulse = round(per/2);
	TCB0.CCMPL = per;
	TCB0.CCMPH = pulse;
	TCB1.CCMPL = per;
	TCB1.CCMPH = pulse;
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
// 	TCB2.CTRLA = TCB_CLKSEL_CLKTCA_gc;
// 	TCB2.CTRLB |= TCB_CNTMODE_PWM8_gc;
// 	TCB2.CCMP = 0x80FF;

	PORTD.DIRSET |= PIN7_bm;
	PORTC.DIRSET |= PIN6_bm;
	
	st.stepflag.line = 56;
}

void prescale_select(uint8_t sel)
{
	TCA0.SINGLE.PER |= 255;
	TCA0.SINGLE.CTRLA &= ~TCA_SINGLE_ENABLE_bm;
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
		break;
		case(10):
			TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1024_gc;
		break;
		default:
			TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1024_gc;
	}
	TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm;
}

uint16_t current_line(uint16_t new_line, uint16_t last_line)
{	
	static uint8_t line_state;
		
	if (new_line > last_line)
	{
		if (line_state)
		{
			TX_write('l');
			TX_write(last_line);
			line_state = 0;
		}
		
		TX_write('L');
		TX_write(new_line);
		last_line = new_line;
		line_state = 1;
	}
	return last_line;
}