/*
 * Feedback.h
 *
 * Created: 02-Mar-20 09:53:50
 *  Author: mikda
 */ 


#ifndef FEEDBACK_H_
#define FEEDBACK_H_

//Sends error statuses to pc
void ReportEvent(ReturnCodes code, int num);

//Dumps status of printer to PC
void ReportStatus();

#endif /* FEEDBACK_H_ */