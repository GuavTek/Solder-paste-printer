/* 
 * File:   stepper.c
 * Author: oletob
 *
 * Created on February 28, 2020, 1:17 PM
 */

#include "Header.h"

st_block st;

uint8_t start_pos = (1 << START_POS_X) | (1 << START_POS_Y);

/*sets prescaling of the TCA0 clk*/
void prescale_select(uint8_t per);

/*calculates steps*/
StepCount Delta(StepCount steps, StepCount laststeps, enum CoordMode coordmode);

/*Sets the the output pins corresponding to the direction of the steps*/
enum DirSet StepDir(int steps);

/*Takes userdefined feedrate, moition modes and current steps that is to be executed as arguments. This is used to calculate the TCB counter period and TCA counter prescaling.
The step arguments is used in the function LinFeedRateCalc and PerCalc */
st_speedvect FeedRateCalc(uint16_t speed, StepCount x, StepCount y, enum MotionModes movment);

/*Function for calculating TCB counter period and TCA counter prescaling. Function is called from inside the FeedRateCalc*/
void PerCalc(float *temp, uint8_t prescale);

/*Sets the PWM period and duty cycle for the TCB counters on X and Y axis*/ 
void PerSelect(uint8_t per, TCB_t *TCBn);

/*Function for calculating the correct speed when executing Linear interpolation.*/
void LinFeedRateCalc(uint16_t speed, uint16_t x, uint16_t y, uint8_t prescale);

/*Sets the calculated speed*/
void FeedRateSet(uint16_t speed, StepCount x, StepCount y, enum MotionModes movment);

/*Sets the the stepper motor step lenght division factor*/  
void MotorPrescaleSet(uint8_t motor_presc);

/*Reports back when line is being executed*/
uint16_t current_line(uint16_t new_line, uint16_t last_line);

/*Before steps are being executed its needed prepare the amount of steps, set flags to distinguish what
type of steps that is to be executetd, set step direction and to reset the counter variables.
The feedrate get calculated coorespondig to the type of movement defined in the data structure 'theCurrentBlock'*/
void PrepStep(void)
{
    StepVector3 delta;	  
    if(st.stepflag.line & (1 << X_LINE_EXE) && st.stepflag.line & (1 << Y_LINE_EXE)); //&& currentState.blockFinished == true)
    {
        /*Set blockFinished to false, this ensure that the MCU only executes instruction sets one by one*/
		currentState.blockFinished = false;
        
        /*Calculate the the steps that is to be executed*/
		delta.x = Delta(theCurrentBlock.pos.x, st.last_pos.x, theCurrentBlock.coordinateMode);
		delta.y = Delta(theCurrentBlock.pos.y, st.last_pos.y, theCurrentBlock.coordinateMode);
		
        /*Report back line number*/
		st.line_number = current_line(theCurrentBlock.blockNumber, st.line_number);
		
        /*If the function Delta returns 0 on all step data its not necessary to store the data, 
        calculate the feedrate, set the stepping direction and set the corresponding step flags
        If delta returns non zero data sets, the steps to be excutetd gets stored in the 'st' block, steping direction is set,
        step flag is set, step data that comes directly from 'theCurrentBlock' gets stored as last position and the counter variables is reset to 1*/
        if(delta.x.full != 0 || delta.y.full != 0 || delta.x.micro != 0 || delta.y.micro != 0))
        {
            if(!(delta.x.full == 0))
            {
                st.steps.x.full = abs(delta.x.full);
                st.direction.x_full = StepDir(delta.x.full);
                st.stepflag.ready |= (1 << X_FSTEP_READY);
                st.stepflag.settings |= (1 << X_FSTEP_SET);
                st.last_pos.x.full = theCurrentBlock.pos.x.full;
                st.counter.x.full = 1;
            }
            if(!(delta.x.micro == 0))
            {
                st.steps.x.micro = abs(delta.x.micro);
                st.direction.x_micro = StepDir(delta.x.micro);
                st.stepflag.ready |= (1 << X_MSTEP_READY);
                st.stepflag.settings |= (1 << X_MSTEP_SET);
                st.counter.x.micro = 1;
            }
            if(delta.y.full != 0)
            { 
                st.steps.y.full = abs(delta.y.full);
                st.direction.y_full = StepDir(delta.y.full);
                st.stepflag.ready |= (1 << Y_FSTEP_READY);
                st.stepflag.settings |= (1 << Y_FSTEP_SET);
                st.last_pos.y.full = theCurrentBlock.pos.y.full;
                st.counter.y.full = 1;
            }
            if(delta.y.micro != 0)
            {
                st.steps.y.micro = abs(delta.y.micro);
                st.direction.y_micro = StepDir(delta.y.micro);
                st.stepflag.ready |= (1 << Y_MSTEP_READY);
                st.stepflag.settings |= (1 << Y_MSTEP_SET);
                st.counter.y.micro = 1;
            }
                   
            FeedRateSet(theCurrentBlock.moveSpeed, st.steps.x, st.steps.y, theCurrentBlock.motion);
            
            if(st.stepflag.ready & (1 << X_FSTEP_READY) || st.stepflag.ready & (1 << X_MSTEP_READY))
            {
                st.stepflag.line &= ~(1 << X_LINE_EXE);
            }
            if(st.stepflag.ready & (1 << Y_FSTEP_READY) || st.stepflag.ready & (1 << Y_MSTEP_READY))
            {
                st.stepflag.line &= ~(1 << Y_LINE_EXE);
            }   
        }
        
        TCB0.CNT = TCB0.CCMPL - 1; //to make the interupt trigger as soon as TCA is enabled
        TCB1.CNT = TCB1.CCMPL - 1;
		TCB1.INTCTRL |= TCB_CAPT_bm;
		TCB0.INTCTRL |= TCB_CAPT_bm;
		TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm;
	}
}


StepCount Delta(StepCount steps, StepCount laststeps, enum CoordMode coordmode)
{
	StepCount temp; //return variable
	switch(coordmode)
	{
		case(absolute): //Calculates delta value of current step
			temp.full = steps.full - laststeps.full; //subtract last steps from current steps and store them in the return variable
			temp.micro = steps.micro; //store the microsteps in the return variable
            break;
		
		case(incremental): //Incremeantal coordinades needs no calculation.
            temp.full = steps.full;
            temp.micro = steps.micro;  
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
	start_pos = (1 << START_POS_X) | (1 << START_POS_Y);
	st.stepflag.ready = 0;
	st.stepflag.settings = 0;
	
	TCB0.CTRLB &= ~TCB_CCMPEN_bm;
	TCB1.CTRLB &= ~TCB_CCMPEN_bm;
	TCB0.INTCTRL &= ~TCB_CAPT_bm;
	TCB1.INTCTRL &= ~TCB_CAPT_bm;	
	
	PORTD.DIRSET |= PIN4_bm;
	PORTC.DIRSET |= PIN7_bm;
	PORTC.DIRSET |= PIN1_bm;
	
	PORTC.DIRSET |= PIN2_bm;
	PORTA.DIRSET |= PIN5_bm;
}


void HomingRoutine(enum MotionModes motion)
{
	if(motion == Home)
	{
		currentState.blockFinished = false;
		
		//Is end sensor X triggered?
		if (PORTD.IN & PIN2_bm)
		{
			PORTC.OUTSET = PIN2_bm;
		} else {
			PORTC.OUTCLR = PIN2_bm;
		}
		
		//Is end sensor Y triggered?
		if (PORTC.IN & PIN5_bm)
		{
			PORTA.OUTSET = PIN5_bm;
		} else {
			PORTA.OUTCLR = PIN5_bm;
		}
		
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
			PORTD.OUTCLR |= PIN4_bm;
			PORTC.OUTCLR |= PIN7_bm;
			PORTC.OUTCLR |= PIN1_bm;
			break;
		}
		case 2:
		{
			PORTD.OUTSET |= PIN4_bm;
			PORTC.OUTCLR |= PIN7_bm;
			PORTC.OUTCLR |=  PIN1_bm;
			break;
		}
		case 4:
		{
			PORTD.OUTCLR |= PIN4_bm;
			PORTC.OUTSET |= PIN7_bm;
			PORTC.OUTCLR |= PIN1_bm;
			break;
		}
		case 8:
		{
			PORTD.OUTSET |= PIN4_bm;
			PORTC.OUTSET |= PIN7_bm;
			PORTC.OUTCLR |= PIN1_bm;
			break;
		}
		case 16:
		{
			PORTD.OUTSET |= PIN4_bm;
			PORTC.OUTSET |= PIN7_bm;
			PORTC.OUTSET |= PIN1_bm;
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


/*Calculate the feed rate speed for the different motion modes.
This is done by finding the TCA prescaling and TCB period.
The equation for finding the feed rate speed is:
speed[mm/s] = step_lenght[mm] * (F_CPU/(6 * TCA_pres * TCB_period))[Hz]
In order to find the TCB period it's needed to find the appropriate
TCA prescaling first. This is done in the function PerCalc.
For linerar interpolation the feed rate is found by multiplying the defined
feed rate speed with the angluar differense betwen the axises*/
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
			PerCalc(calc_val, 0);					
			break;
		default:
			calc_val[0] = calc_val[1] = fCLK_MMS/3;
			PerCalc(calc_val, 1);
			PerCalc(calc_val, 0);
			break;
	}
}


void LinFeedRateCalc(uint16_t speed, uint16_t x, uint16_t y, uint8_t prescale)
{
	float temp[2];
	if (x == 0 || y == 0)
	{
		temp[0] = temp[1] = fCLK_MMS/speed;
	}
	else
	{
        //Find the angular argument between the axises
		float rad = atan2f(y,x); 
		temp[0] = fCLK_MMS/(speed * cos(rad));
		temp[1] = fCLK_MMS/(speed * sin(rad));
	}
	PerCalc(temp, prescale);
}


void PerCalc(float temp[2], uint8_t prescale)
{ 
	uint16_t comp[2];
	//Loop trough the TCA prescaling
	for(uint8_t i = 0; pow(2,i) <= 1024; i++)
	{
		for (uint8_t j = 0; j < 2; j++)
		{
			if (temp[j] != 0)
			{
                //insert the current TCA prescaling in to the feed rate speed equation
				comp[j] = (uint16_t)round(temp[j] / pow(2,i));
			}
		}
		//Check if the division is betwen 0 and 255
		if((comp[0] <= 255) && comp[1] <= 255 && !(i == 5 || i == 7 || i == 9))
		{	
			if (prescale == 1)
			{
                //Store the TCA prescale and TCB.CCMP value
				prescale_select(i);
				st.step_speed.full_speed[0] = comp[0];
				st.step_speed.full_speed[1] = comp[1];
			}
			else
			{
				st.step_speed.micro_speed[0] = comp[0];
				st.step_speed.micro_speed[1] = comp[1];
			}
		}
	}
}



void PerSelect(uint8_t per, TCB_t *TCBn)
{   //set the calulated TCB period. Divide by two to set duty cycle at 50%
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
	TCB0.CTRLA |= TCB_CLKSEL_CLKTCA_gc | TCB_ENABLE_bm;
	TCB0.CTRLB |= TCB_CNTMODE_PWM8_gc;
	TCB0.INTCTRL = TCB_CAPT_bm;
	TCB0.CCMPL = 0xFF;
	TCB0.CCMPH = 0x80;
	
	//enable TCB1
	TCB1.CTRLA |= TCB_CLKSEL_CLKTCA_gc | TCB_ENABLE_bm;
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

	
}


void prescale_select(uint8_t sel)
{
	TCA0.SINGLE.CTRLA &= ~TCA_SINGLE_ENABLE_bm;
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
    //Report back if the line number increases.
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
		if(st.stepflag.settings & (1 << X_FSTEP_SET)) //if statment to ensure that step settings is set once
		{
			MotorPrescaleSet(1); //Set the step motor step lenght
			
			if (st.direction.x_full == pos_dir) //Set the step direction
			{
				PORTC.OUTSET = PIN2_bm; //Positive direction
            }
			else
            {
				PORTC.OUTCLR = PIN2_bm; //Negative direction
			}
			
            st.stepflag.settings &= ~(1 << X_FSTEP_SET); //Clear the x-axis full step settings flag.
            PerSelect(st.step_speed.full_speed[0], &TCB0); //Set the PWM period and dutycycle.
            
            /*Reset the TCB0 counter register. 
            This ensures that counter value is smaller than the period value in CCMPL register*/
			TCB0.CNT = 0;
			TCB0.CTRLB |= TCB_CCMPEN_bm; //Enable TCB0 PWM output
		}
		
		if(st.counter.x.full == st.steps.x.full)
		{
			TCB0.CTRLB &= ~TCB_CCMPEN_bm; //Disable TCB0 PWM output
			st.stepflag.ready &= ~(1 << X_FSTEP_READY); // Clear the x-axis fullstep ready flag
		}
		else
		{
			st.counter.x.full++;
		}
	}
	/*Check if the full steps on the y-axis are done. Microsteps cant be executed while full steps are executed.
    This beacuse the motor steplenght is the same for all axises*/
	else if(st.stepflag.ready & (1 << X_MSTEP_READY) && !(st.stepflag.ready & (1 << Y_FSTEP_READY)))
	{
		if(st.stepflag.settings & (1 << X_MSTEP_SET))
		{
			
			MotorPrescaleSet(16); // Set the step motor step lenght to 1/16
			
			if (st.direction.x_micro == pos_dir) //Set the step direction
			{
				PORTC.OUTSET |= PIN2_bm; //Postive direction
            }
			else
            {
				PORTC.OUTCLR |= PIN2_bm; //Negative direction
			}
            
            PerSelect(st.step_speed.micro_speed[0], &TCB0);
			st.stepflag.settings &= ~(1 << X_MSTEP_SET);
            TCB0.CNT = 0;
			TCB0.CTRLB |= TCB_CCMPEN_bm;
		}
		if((st.counter.x.micro) == (st.steps.x.micro))
		{
			TCB0.CTRLB &= ~TCB_CCMPEN_bm;
			st.stepflag.ready &= ~(1 << X_MSTEP_READY);
		}
		else
		{
			st.counter.x.micro++;
		}
	}
	else if (!(st.stepflag.ready & (1 << X_FSTEP_READY) & (1 << X_MSTEP_READY))) //Check if all steps are executed
	{
		st.stepflag.line |= (1 << X_LINE_EXE); //Set the x-axis line executed flag
        /*All lines have been executed, set line exucuted flag for all axises and turn off all counter peripherals*/
		if(st.stepflag.line & (1 << X_LINE_EXE) && st.stepflag.line & (1 << Y_LINE_EXE) && currentState.blockFinished == false) 
		{
			currentState.blockFinished = true;
            TCB1.INTCTRL &= ~TCB_CAPT_bm;
			TCA0.SINGLE.CTRLA &= ~TCA_SINGLE_ENABLE_bm;
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
			MotorPrescaleSet(1);
			
			if(st.direction.y_full == pos_dir)
			{
				PORTA.OUTSET |= PIN5_bm;
            }
            else
            {
				PORTA.OUTCLR |= PIN5_bm;
			}
            st.stepflag.settings &= ~(1 << Y_FSTEP_SET);
            PerSelect(st.step_speed.full_speed[1], &TCB1);
            TCB1.CNT = 0;
			TCB1.CTRLB |= TCB_CCMPEN_bm;
		}
		
		if(st.counter.y.full == st.steps.y.full)
		{
			TCB1.CTRLB &= ~TCB_CCMPEN_bm;
			st.stepflag.ready &= ~(1 << Y_FSTEP_READY);
		}
		else
		{
			st.counter.y.full++;
		}
	}
	else if(st.stepflag.ready & (1 << Y_MSTEP_READY) && !(st.stepflag.ready & (1 << X_FSTEP_READY)))
	{
		if(st.stepflag.settings & (1 << Y_MSTEP_SET))
		{
			MotorPrescaleSet(16);
			
			if(st.direction.y_micro == pos_dir)
			{
				PORTA.OUTSET |= PIN5_bm;
            }
            else
            {
				PORTA.OUTCLR |= PIN5_bm;
			}
			
			st.stepflag.settings &= ~(1 << Y_MSTEP_SET);
            PerSelect(st.step_speed.micro_speed[1], &TCB1);
            TCB1.CNT = 0;
			TCB1.CTRLB |= TCB_CCMPEN_bm;
		}
        
		if(st.counter.y.micro == st.steps.y.micro)
		{
			TCB1.CTRLB &= ~TCB_CCMPEN_bm;
            st.stepflag.ready &= ~(1 << Y_MSTEP_READY);
		}
		else
		{
			st.counter.y.micro++;
		}
	}
	
	else if (!(st.stepflag.ready & (1 << Y_FSTEP_READY) & (1 << Y_MSTEP_READY)))
	{
		st.stepflag.line |= (1 << Y_LINE_EXE);
		
		if(st.stepflag.line & (1 << X_LINE_EXE) && st.stepflag.line & (1 << Y_LINE_EXE) && currentState.blockFinished == false)
		{
			currentState.blockFinished = true;
            TCB0.INTCTRL &= ~TCB_CAPT_bm;
			TCA0.SINGLE.CTRLA &= ~TCA_SINGLE_ENABLE_bm;
		}
        
		TCB1.INTCTRL &= ~TCB_CAPT_bm;
    }
}


void InitEndSensors(){
    /*Init interupt capture on event routine, and enable pullup resistor to ensure non floating condition for the coresponding end sensors input pins*/ 
	PORTC.PIN5CTRL = PORT_ISC_BOTHEDGES_gc | PORT_PULLUPEN_bm;
	PORTD.PIN1CTRL = PORT_ISC_BOTHEDGES_gc | PORT_PULLUPEN_bm;
	PORTD.PIN2CTRL = PORT_ISC_BOTHEDGES_gc | PORT_PULLUPEN_bm;
	PORTD.PIN5CTRL = PORT_ISC_BOTHEDGES_gc | PORT_PULLUPEN_bm;
}


ISR(PORTC_PORT_vect) //Capture on event interupt vector
{
	if (PORTC.INTFLAGS & PIN5_bm)
	{
		//Y-axis end detected
		if (PORTC.IN & PIN5_bm)
		{
			//Entering end-sensor
			TCB1.CTRLB &= ~TCB_CCMPEN_bm;
			start_pos |= (1 << START_POS_Y);
 			
			//Reverse Y direction
			
		} 
		else
		{
			//Leaving end-sensor
			//Stop Y Stepping
			start_pos &= ~(1 << START_POS_Y);
			TCB1.CTRLB &= ~TCB_CCMPEN_bm;
		}
			
		//Log unexpected end trigger, and halt printing
		if (currentState.task != Home)
		{
			currentState.noError = false;
			ReportEvent(UNEXPECTED_EDGE, 'Y');
			TCB1.CTRLB &= ~TCB_CCMPEN_bm;
		} else {
			if(start_pos == 0) {
				currentState.blockFinished = true;
			}
		}
		PORTC.INTFLAGS |= PIN5_bm;
	}
}



ISR(PORTD_PORT_vect){
	
	if (PORTD.INTFLAGS & PIN2_bm)
	{
		//X-axis end detected
		if (PORTD.IN & PIN2_bm)
		{
			//Entering edge sensor
			//Disable TCB0 PWM output on PA2
			TCB0.CTRLB &= ~TCB_CCMPEN_bm;
			//Reset the start position flag
			start_pos |= (1 << START_POS_X);
		}
		//Leaving edge sensor
		else
		{
			//Disable TCB0 PWM output on PA2
			TCB0.CTRLB &= ~TCB_CCMPEN_bm;
			//Clear the start position flag
			start_pos &= ~(1 << START_POS_X);
				
		}

		//Log unexpected end trigger, and halt printing
		if (currentState.task != Home)
		{
			currentState.noError = false;
			ReportEvent(UNEXPECTED_EDGE, 'X');
			//Disable TCB0 PWM output on PA2
			TCB0.CTRLB &= ~TCB_CCMPEN_bm;
			
		} else {
			if(start_pos == 0) {
				currentState.blockFinished = true;
			}
		}
		PORTD.INTFLAGS |= PIN2_bm;
	}
	if (PORTD.INTFLAGS & PIN5_bm)
	{
		//Z-axis end detected
		if (PORTD.IN & PIN5_bm)
		{
			//Reverse Z direction
		}
		else
		{
			//Stop Z stepping
			//Z pos = 0
		}

		//Log unexpected end trigger, and halt printing
		if (currentState.task != Home)
		{
			currentState.noError = false;
			ReportEvent(UNEXPECTED_EDGE, 'Z');
		}
		PORTD.INTFLAGS |= PIN5_bm;
	}
	if (PORTD.INTFLAGS & PIN1_bm)
	{
		//Currently unused
		
		PORTD.INTFLAGS |= PIN1_bm;
	}
}