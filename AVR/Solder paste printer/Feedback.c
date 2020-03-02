/*
 * Feedback.c
 *
 * Created: 02-Mar-20 09:53:19
 *  Author: mikda
 */ 

void ReportStatus(ReturnCodes code){
	if (code == BUFFER_FULL)
	{
		TX_receive('F');
	} else if (code == BUFFER_AVAILABLE)
	{
		TX_receive('A');
	} else if (code == STOP_DETECTED)
	{
		TX_receive('S');
	} else if (code == NOT_RECOGNIZED)
	{
		TX_receive('N');
	}
	TX_receive('\r');
}

void ReportStatus(ReturnCodes code, int num){
	if (code == NOT_RECOGNIZED)
	{
		TX_receive('N');
		TX_receive(num);	//Should be letter of G-code, is ASCII
	} else {
		ReportStatus(code);
		return;
	}
	TX_receive('\r')
}