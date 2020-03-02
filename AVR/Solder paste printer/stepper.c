#include "Header.h"


st_block st_data[block_buffer];
step_counter counter;
step_counter counter_coard;
uint8_t steper_flag;


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
            TCA0.CTRLA = TCA_SPLIT_CLKSEL_DIV1024_gc
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



void GetLine(gc_block *blockBuffer, st_block *st_data)
{
    
    if (data_flag & (1 << NEW_DATA))
    {    
        if ((steper_flag & (1 << EXE_X_LINE)) && (data_flag & (1 << NEW_DATA_X)))
        {
                    
            for(uint32_t count = 0 ; count <= blockBufferTail ; count++)
                    
            {
                st_data -> s_axis.x_s[count] = blockBuffer -> pos.x[count];
                st_data -> x_prescale[count] = blockBuffer -> moveSpeed[count];
                st_data -> x_dir[count] = XDir;
            }        
        
            steper_flag |= (1 << X_STEP_READY);
            TCB0.INTCTRL |= TCB_CAPT_bm; //enable interrupt 
            TCB0.CNT = 0; //reset counter
                                                   
        }
        
        if ((steper_flag & (1 << EXE_Y_LINE)) && (data_flag & (1 << NEW_DATA_Y)))
        {
            for(uint32_t count = 0 ; count <= blockBufferTail  ; count++)
                    
            {
                st_data -> s_axis.y_s[count] = blockBuffer[count] -> pos.y;
                st_data -> y_prescale[count] = blockBuffer[count] -> moveSpeed;
                st_data -> y_dir[count] = ZDir; // needs to be config
            }
            
            step_flag |= (1 << Y_STEP_READY)
            TCB1.INTCTRL |= TCB_CAPT_bm; //enable interrupt 
            TCB1.CNT = 0; //reset counter
            
        }
        
        if ((step_flags & (1 << EXE_Y_LINE)) && (NEW_DATA_Y == 1))
        {
            for(uint32_t count = 0 ; count > blockBufferTail ; count++)
                    
            {
                st_data -> s_axis.z_s[count] = blockBuffer[count] -> pos.y;
                st_data -> z_prescale[count] = blockBuffer[count] -> moveSpeed;
                st_data -> z_dir[count] = ZDir; // needs to be config
            }
            
            steper_flag = (1 << Z_STEP_READY);
            TCB2.INTCTRL |= TCB_CAPT_bm; //enable interrupt 
            TCB2.CNT = 0; //reset counter
        } 
    }
}


ISR() //TCB0 vector
{   
    
    uint32_t compare = st_data ->s_axis.x_s[counter_coard.x_c];

    if(steper_flag & (1 << X_STEP_READY))
    {
        PORTA.DIR |= PIN2_bm;
        
        if((counter.x_c) == (compare - 1))
        {
            counter.x_c  = 0;
            counter_coard.x_c++;
        }
        else
        {
            counter.x_c++;
        }
        
        if(counter_coard.x_c == blockBufferTail)
        {
            counter_coard.x_c = 0;
            steper_flag |= (1 << EXE_X_LINE) & ~(1 << X_STEP_READY);
            PORTA.DIR &= ~PIN2_bm; //clear PA2 as output when line is done
            TCB0.INTCTRL &=  ~TCB_CAPT_bm; // disable interrupt when line is done
        }
    } 
    
    TCB0.INTFLAGS = TCB_CAPT_bm; // clear interrupt flag
}

ISR() //TCB1 vector
{
    uint32_t compare = st_data ->s_axis.y_s[counter_coard.y_c];

    if(steper_flag & (1 << Y_STEP_READY))
    {
        PORTA.DIR |= PIN2_bm;
        
        if((counter.y_c) == (compare - 1))
        {
            counter.y_c  = 0;
            counter_coard.y_c++;
        }
        else
        {
            counter.y_c++;
        }
        
        if(counter_coard.y_c == blockBufferTail)
        {
            counter_coard.y_c = 0;
            steper_flag |= (1 << EXE_Y_LINE) & ~(1 << Y_STEP_READY);
            PORTA.DIR &= ~PIN3_bm; //clear PA2 as output when line is done
            TCB1.INTCTRL &=  ~TCB_CAPT_bm; // disable interrupt when line is done
        }
    
    
    TCB1.INTFLAGS = TCB_CAPT_bm; // clear interrupt flag
}

ISR() //TCB2 vector
{       
    uint32_t compare = st_data ->s_axis.z_s[counter_coard.z_c];

    if(steper_flag & (1 << Z_STEP_READY))
    {
        PORTC.DIR |= PIN0_bm;
        
        if((counter.y_c) == (compare - 1))
        {
            counter.y_c  = 0;
            counter_coard.y_c++;
        }
        else
        {
            counter.y_c++;
        }
        
        if(counter_coard.y_c == blockBufferTail)
        {
            counter_coard.y_c = 0;
            steper_flag |= (1 << EXE_Y_LINE) & ~(1 << Y_STEP_READY);
            PORTC.DIR &= ~PIN0_bm; //clear PINC0 as output when line is done
            TCB2.INTCTRL &=  ~TCB_CAPT_bm; // disable interrupt when line is done
        }
    TCB2.INTFLAGS = TCB_CAPT_bm; // clear interrupt flag   
}

