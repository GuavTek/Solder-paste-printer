/* 
 * File:   stepper.c
 * Author: oletob
 *
 * Created on February 28, 2020, 1:17 PM
 */

#include "Header.h"

st_block st;

/*sets prescaling of the TCA0 clk*/
void prescale_select(uint8_t per);
/*calculates steps.*/
StepCount Delta(StepCount steps, StepCount laststeps, int coordmode);
/*Sets the direction of the steps*/
enum DirSet StepDir(int steps);

st_speedvect FeedRateCalc(uint16_t speed, StepCount x, StepCount y, enum MotionModes movment);

void PerCalc(float *temp, uint8_t prescale);
//void FeedRateCalc(uint16_t speed);
/*sets the period and pulse lenght of TCB PWM on X and Y axis*/ 
void PerSelect(uint8_t per, TCB_t *TCBn);

void LinFeedRateCalc(uint16_t speed, uint16_t x, uint16_t y, uint8_t prescale);

void FeedRateSet(uint16_t speed, StepCount x, StepCount y, enum MotionModes movment);

void MotorPrescaleSet(uint8_t motor_presc);

uint16_t current_line(uint16_t new_line, uint16_t last_line);

void PrepStep(void)
{
	/*steps get prepped here. calculates the amount of steps according 
	to how far it has stepped, sets the direction and stores the value 
	from the current block in last_steps*/
	
    StepVector3 delta;	  
    if(st.stepflag.line & (1 << X_LINE_EXE) && st.stepflag.line & (1 << Y_LINE_EXE)/* && currentState.blockFinished == true*/)
    {
		currentState.blockFinished = false;
		TCA0.SINGLE.CTRLA &= ~TCA_SINGLE_ENABLE_bm;
		delta.x = Delta(theCurrentBlock.pos.x, st.last_pos.x, theCurrentBlock.coordinateMode);
		delta.y = Delta(theCurrentBlock.pos.y, st.last_pos.y, theCurrentBlock.coordinateMode);
		st.direction.x_full = StepDir(delta.x.full);
		st.direction.y_full = StepDir(delta.y.full);
		st.direction.x_micro = StepDir(delta.x.micro);
		st.direction.y_micro = StepDir(delta.y.micro);
		st.line_number = current_line(theCurrentBlock.blockNumber, st.line_number);
		
		
		if(!(delta.x.full == 0))
		{
			st.steps.x.full = abs(delta.x.full);
			st.stepflag.ready |= (1 << X_FSTEP_READY);
			st.stepflag.settings |= (1 << X_FSTEP_SET);
			st.last_pos.x.full = theCurrentBlock.pos.x.full;
			st.counter.x.full = 1;
		}
		
		if(!(delta.x.micro == 0))
		{
			st.steps.x.micro = abs(delta.x.micro);
			st.stepflag.ready |= (1 << X_MSTEP_READY);
			st.stepflag.settings |= (1 << X_MSTEP_SET);
			st.last_pos.x.micro = theCurrentBlock.pos.x.micro;
			st.counter.x.micro = 1;
		}
		
		if(delta.y.full != 0)
		{
			st.steps.y.full = abs(delta.y.full);
			st.stepflag.ready |= (1 << Y_FSTEP_READY);
			st.stepflag.settings |= (1 << Y_FSTEP_SET);
			st.last_pos.y.full = theCurrentBlock.pos.y.full;
			st.counter.y.full = 1;
		}
		if(delta.y.micro != 0)
		{
			st.steps.y.micro = abs(delta.y.micro);
			st.stepflag.ready |= (1 << Y_MSTEP_READY);
			st.stepflag.settings |= (1 << Y_MSTEP_SET);
			st.last_pos.y.micro = theCurrentBlock.pos.y.micro;
			st.counter.y.micro = 1;
		}
		
		FeedRateSet(theCurrentBlock.moveSpeed, delta.x, delta.y, theCurrentBlock.motion);
		
		
		if(st.stepflag.ready & (1 << X_FSTEP_READY) || st.stepflag.ready & (1 << X_MSTEP_READY))
		{
			st.stepflag.line &= ~(1 << X_LINE_EXE);
		}
		
		if(st.stepflag.ready & (1 << Y_FSTEP_READY) || st.stepflag.ready & (1 << Y_MSTEP_READY))
		{
			st.stepflag.line &= ~(1 << Y_LINE_EXE);
		}
		
		
		TCB1.CNT = 0;
		TCB0.CNT = 0;

		TCB1.INTCTRL |= TCB_CAPT_bm;
		TCB0.INTCTRL |= TCB_CAPT_bm;
		TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm;
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
		
		default:
			temp.full = 0;
			temp.micro = 0;
		break;
	}
	return temp;
}


void StepperInit(void)
{
	st.counter.x.full = 0;
	st.counter.x.micro = 0;
	st.last_pos.x.full = 0;
	st.last_pos.x.micro = 0;
	st.steps.x.full = 0;
	st.steps.x.micro = 0;
	
	st.counter.y.full = 0;
	st.counter.y.micro = 0;
	st.last_pos.y.full = 0;
	st.last_pos.y.micro = 0;
	st.steps.y.full = 0;
	st.steps.y.micro = 0;
	
	st.stepflag.line =  (1 << X_LINE_EXE) | (1 << Y_LINE_EXE);
	st.stepflag.ready = 0;
	st.stepflag.settings = 0;
	
	TCB0.CTRLB &= ~TCB_CCMPEN_bm;
	TCB1.CTRLB &= ~TCB_CCMPEN_bm;
	TCB0.INTCTRL &= ~TCB_CAPT_bm;
	TCB1.INTCTRL &= ~TCB_CAPT_bm;	
}


void HomingRoutine(enum MotionModes motion)
{
	if(motion == Home)
	{
		currentState.blockFinished = false;
		
		PORTC.OUT &= ~PIN2_bm;
		PORTA.OUT &= ~PIN5_bm;
		
		prescale_select(8);
		PerSelect(255, &TCB0);
		PerSelect(255, &TCB1);
		
		MotorPrescaleSet(1);
		
		StepperInit();
		
		TCB0.CTRLB |= TCB_CCMPEN_bm;
		TCB1.CTRLB |= TCB_CCMPEN_bm;
		TCB0.INTCTRL &= ~TCB_CAPT_bm;
		TCB1.INTCTRL &= ~TCB_CAPT_bm;
	}
}


void MotorPrescaleSet(uint8_t motor_presc)
{
	switch(motor_presc)
	{
		case 1:
		{
			PORTD.OUT &= ~PIN4_bm;
			PORTC.OUT &= ~PIN7_bm;
			PORTC.OUT &= ~PIN1_bm;
			break;
		}
		case 2:
		{
			PORTD.OUT |= PIN4_bm;
			PORTC.OUT &= ~PIN7_bm;
			PORTC.OUT &= ~PIN1_bm;
			break;
		}
		case 4:
		{
			PORTD.OUT &= ~PIN4_bm;
			PORTC.OUT |= PIN7_bm;
			PORTC.OUT &= ~PIN1_bm;
			break;
		}
		case 8:
		{
			PORTD.OUT |= PIN4_bm;
			PORTC.OUT |= PIN7_bm;
			PORTC.OUT &= ~PIN1_bm;
			break;
		}
		case 16:
		{
			PORTD.OUT |= PIN4_bm;
			PORTC.OUT |= PIN7_bm;
			PORTC.OUT |= PIN1_bm;
			break;
		}
		default:
		{
			break;
		}
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



void LinFeedRateCalc(uint16_t speed, uint16_t x, uint16_t y, uint8_t prescale)
{
	float temp[2];
	x = abs(x);
	y = abs(y);
	if (x == 0 || y == 0)
	{
		temp[0] = temp[1] = fCLK_MMS/speed;
	}
	else
	{
		float rad = atan2f(y,x); 
		temp[0] = fCLK_MMS/(speed * cos(rad));
		temp[1] = fCLK_MMS/(speed * sin(rad));
	}
	
	PerCalc(temp, prescale);
}


void PerCalc(float *temp, uint8_t prescale)
{ 
	uint16_t comp[2];
	
	for(uint8_t i = 0; pow(2,i) <= 1024; i++)
	{
		for (uint8_t j = 0; j < 2; j++)
		{
			if (temp[j] != 0)
			{
				comp[j] = (uint16_t)round(temp[j] / pow(2,i));
			}
		}
		
		if((comp[0] <= 255) && comp[1] <= 255 && !(i == 5 || i == 7 || i == 9))
		{	
			if (prescale == 1)
			{
				prescale_select(i);
				st.step_speed.full_speed[0] = comp[0];
				st.step_speed.full_speed[1] = comp[1];
			}
			else
			{
				st.step_speed.micro_speed[0] = comp[0];
				st.step_speed.micro_speed[1] = comp[1];
			}
			return;
		}
	}
}


void FeedRateSet(uint16_t speed, StepCount x, StepCount y, enum MotionModes movment)
{	
	float calc_val[2];
	

	if(speed > 778)
	{
		speed = 778;
	}
		
	
	switch(movment)
	{
		case(Linear_interpolation):
			LinFeedRateCalc(speed, x.full, y.full, 1);
			LinFeedRateCalc(speed, x.micro, y.micro, 0);
			break;
		case(Rapid_position):
			calc_val[0] = calc_val[1] = fCLK_MMS/speed;
			PerCalc(calc_val, 1);					
			break;
		default:
			calc_val[0] = calc_val[1] = fCLK_MMS/1;
			PerCalc(calc_val, 1);
			break;
	}
}


void PerSelect(uint8_t per, TCB_t *TCBn)
{
	
	if (per != 0)
	{
		uint8_t pulse = round(per/2);
		TCBn->CCMPL = per;
		TCBn->CCMPH = pulse;
	}
}	


void stepper_TCB_init(void)
{
	//enable TCB0
	//all counter registers set to 50% dutycycle
	TCB0.CTRLA = TCB_CLKSEL_CLKTCA_gc | TCB_ENABLE_bm;
	TCB0.CTRLB |= TCB_CNTMODE_PWM8_gc;
	TCB0.INTCTRL = TCB_CAPT_bm;
	TCB0.CCMPL = 0xFF;
	TCB0.CCMPH = 0x80;
	
	//enable TCB1
	TCB1.CTRLA = TCB_CLKSEL_CLKTCA_gc | TCB_ENABLE_bm;
	TCB1.CTRLB |= TCB_CNTMODE_PWM8_gc;
	TCB1.INTCTRL = TCB_CAPT_bm;
	TCB1.CCMPL = 0xFF;
	TCB1.CCMPH = 0x80;
	
// 	TCA0.SINGLE.PER |= 255;
// 	TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV256_gc;
// 	TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm;
// 	
	//enable TCB2
// 	TCB2.CTRLA = TCB_CLKSEL_CLKTCA_gc;
// 	TCB2.CTRLB |= TCB_CNTMODE_PWM8_gc;
// 	TCB2.CCMP = 0x80FF;

	PORTC.DIRSET |= PIN2_bm;
	PORTA.DIRSET |= PIN5_bm;
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
	if (new_line > last_line)
	{
		TX_write('L');
		SendInt(new_line);
		last_line = new_line;
	}
	return last_line;
}


ISR(TCB0_INT_vect) //TCB0 vector
{
	TCB0.INTFLAGS = TCB_CAPT_bm; // clear interrupt flag
	
	if(st.stepflag.ready & (1 << X_FSTEP_READY))
	{
		if(st.stepflag.settings & (1 << X_FSTEP_SET))
		{
			PerSelect(st.step_speed.full_speed[0], &TCB0);
			
			MotorPrescaleSet(1);
			
			switch(st.direction.x_full)
			{
				case(pos_dir):
				PORTC.OUT |= PIN2_bm;
				break;
				
				case(neg_dir):
				PORTC.OUT &= ~PIN2_bm;
				break;
			}
			
			st.stepflag.settings &= ~(1 << X_FSTEP_SET);
			/*TCB0.CNT = TCB1.CNT;*/
			TCB0.CTRLB |= TCB_CCMPEN_bm;
		}
		
		if(st.counter.x.full == st.steps.x.full)
		{
			TCB0.CTRLB &= ~TCB_CCMPEN_bm;
			if(st.counter.y.full == st.steps.y.full)
			{
				TCB1.CTRLB &= ~TCB_CCMPEN_bm;
			}
			st.stepflag.ready &= ~(1 << X_FSTEP_READY);
		}
		else
		{
			st.counter.x.full++;
			
		}
	}
	
	else if(st.stepflag.ready & (1 << X_MSTEP_READY))
	{
		if(st.stepflag.settings & (1 << X_MSTEP_SET))
		{
			PerSelect(st.step_speed.micro_speed[0], &TCB0);
			
			MotorPrescaleSet(16);
			
			switch(st.direction.x_full)
			{
				case(pos_dir):
				PORTC.OUT |= PIN2_bm;
				break;
				
				case(neg_dir):
				PORTC.OUT &= ~PIN2_bm;
				break;
			}
			
			st.stepflag.settings &= ~(1 << X_MSTEP_SET);
			/*TCB0.CNT = TCB1.CNT;*/
			TCB0.CTRLB |= TCB_CCMPEN_bm;
		}
		
		if((st.counter.x.micro) == (st.steps.x.micro))
		{
			TCB0.CTRLB &= ~TCB_CCMPEN_bm;
			if(st.counter.y.micro == st.steps.y.micro)
			{
				TCB1.CTRLB &= ~TCB_CCMPEN_bm;
			}
			st.stepflag.ready &= ~(1 << X_MSTEP_READY);
			
		}
		else
		{
			st.counter.x.micro++;
		}
	}
	else
	{
		st.stepflag.line |= (1 << X_LINE_EXE);
		if(st.stepflag.line & (1 << X_LINE_EXE) && st.stepflag.line & (1 << Y_LINE_EXE) && currentState.blockFinished == false)
		{
			currentState.blockFinished = true;
		}
		
		TCB0.INTCTRL &= ~TCB_CAPT_bm;
	}
}


ISR(TCB1_INT_vect) //TCB1 vector
{
	TCB1.INTFLAGS = TCB_CAPT_bm; // clear interrupt flag
	
	if(st.stepflag.ready & (1 << Y_FSTEP_READY))
	{
		if(st.stepflag.settings & (1 << Y_FSTEP_SET))
		{
			PerSelect(st.step_speed.full_speed[1], &TCB1);
			
			MotorPrescaleSet(1);
			
			switch(st.direction.y_full)
			{
				case(pos_dir):
				PORTA.OUT |= PIN5_bm;
				break;
				case(neg_dir):
				PORTA.OUT &= ~PIN5_bm;
				break;
			}
			
			st.stepflag.settings &= ~(1 << Y_FSTEP_SET);
			TCB1.CTRLB |= TCB_CCMPEN_bm;
		}
		
		if(st.counter.y.full == st.steps.y.full)
		{
			TCB1.CTRLB &= ~TCB_CCMPEN_bm;
			if(st.counter.x.full == st.steps.x.full)
			{
				TCB0.CTRLB &= ~TCB_CCMPEN_bm;
			}
			st.stepflag.ready &= ~(1 << Y_FSTEP_READY);
		}
		else
		{
			st.counter.y.full++;
		}
	}
	else if(st.stepflag.ready & (1 << Y_MSTEP_READY))
	{
		if(st.stepflag.settings & (1 << Y_MSTEP_SET))
		{
			PerSelect(st.step_speed.micro_speed[1], &TCB1);
			
			MotorPrescaleSet(16);
			
			switch(st.direction.y_full)
			{
				case(pos_dir):
				PORTA.OUT |= PIN5_bm;
				break;
				case(neg_dir):
				PORTA.OUT &= ~PIN5_bm;
				break;
			}
			
			st.stepflag.settings &= ~(1 << Y_MSTEP_SET);
			/*TCB1.CNT = TCB0.CNT;*/
			TCB1.CTRLB |= TCB_CCMPEN_bm;
		}
		if(st.counter.y.micro == st.steps.y.micro)
		{
			TCB1.CTRLB &= ~TCB_CCMPEN_bm;
			if(st.counter.x.micro == st.steps.x.micro)
			{
				TCB0.CTRLB &= ~TCB_CCMPEN_bm;
			}
			st.stepflag.ready &= ~(1 << Y_MSTEP_READY);
		}
		else
		{
			st.counter.y.micro++;
		}
	}
	
	else
	{
		st.stepflag.line |= (1 << Y_LINE_EXE);
		
		if(st.stepflag.line & (1 << X_LINE_EXE) && st.stepflag.line & (1 << Y_LINE_EXE) && currentState.blockFinished == false)
		{
			currentState.blockFinished = true;
		}
		TCB1.INTCTRL &= ~TCB_CAPT_bm;
	}
}
