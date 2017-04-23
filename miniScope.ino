/*  Arduino Mega 2560 + HX8357
 *  Mini scope with reading ADC
 *  Typically a clear screen for a 320 x 480 TFT will complete in only 12ms.
 *  Original code: 240 x 320 QVGA resolutions.
 */

#include <TFT_HX8357.h> // Hardware-specific library

/********************************************/
TFT_HX8357 tft = TFT_HX8357();       // Invoke custom library

#define LTBLUE    0xB6DF
#define LTTEAL    0xBF5F
#define LTGREEN   0xBFF7
#define LTCYAN    0xC7FF
#define LTRED     0xFD34
#define LTMAGENTA 0xFD5F
#define LTYELLOW  0xFFF8
#define LTORANGE  0xFE73
#define LTPINK    0xFDDF
#define LTPURPLE  0xCCFF
#define LTGREY    0xE71C

#define BLUE      0x001F
#define TEAL      0x0438
#define GREEN     0x07E0
#define CYAN      0x07FF
#define RED       0xF800
#define MAGENTA   0xF81F
#define YELLOW    0xFFE0
#define ORANGE    0xFC00
#define PINK      0xF81F
#define PURPLE    0x8010
#define GREY      0xC618
#define WHITE     0xFFFF
#define BLACK     0x0000

#define DKBLUE    0x000D
#define DKTEAL    0x020C
#define DKGREEN   0x03E0
#define DKCYAN    0x03EF
#define DKRED     0x6000
#define DKMAGENTA 0x8008
#define DKYELLOW  0x8400
#define DKORANGE  0x8200
#define DKPINK    0x9009
#define DKPURPLE  0x4010
#define DKGREY    0x4A49

#define COLOR_BACK     BLACK      	//backgrond color
#define COLOR_GRAPH    LTORANGE      //graph color

#define maxWIDTH		479
#define maxHEIGHT		319
#define grEND			420
#define btSTART			(grEND+5)
#define resGRID			70
#define ROW1			0
#define ROW2			(ROW1+resGRID)
#define ROW3			(ROW2+resGRID)
#define ROW4			(ROW3+resGRID)
#define ROW5            (ROW4+resGRID)
#define resBTCOL		resGRID
#define resBTROW		(maxWIDTH-btSTART-1)
#define resBTSELCOL		(resBTCOL-2)
#define resBTSELROW		(resBTROW-2)
#define valSTART		(btSTART+5)

char buf[12];
char recv_str[100];

int x,y;
int Input = 0;
byte Sample[grEND*2];
byte OldSample[grEND*2];
int Sum=0;
float SquareSum=0;
int StartSample = 0;
int EndSample = 0;
int Max = 100;
int Min = 100;
int mode = 0;
int dTime = 0;
int tmode = 0;
int Trigger = 0;
int SampleSize = 0;
int SampleTime = 0;
int dgvh;

int hpos = 45;      //set 0v on horizontal grid
int vsens = 35;     // vertical sensitivity,1/10

// Define various ADC prescaler
const unsigned char PS_16 = (1 << ADPS2);
const unsigned char PS_32 = (1 << ADPS2) | (1 << ADPS0);
const unsigned char PS_64 = (1 << ADPS2) | (1 << ADPS1);
const unsigned char PS_128 = (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

//------------Start Subrutines------------------------------------
//--------draw buttons sub
void buttons(){
	tft.fillRoundRect (btSTART, ROW1, resBTROW, resBTCOL, 1, BLUE);
	tft.fillRoundRect (btSTART, ROW2, resBTROW, resBTCOL, 1, BLUE);
	tft.fillRoundRect (btSTART, ROW3, resBTROW, resBTCOL, 1, BLUE);
	tft.fillRoundRect (btSTART, ROW4, resBTROW, resBTCOL, 1, BLUE);
}

//receive message from Bluetooth with time out
int recvMsg(unsigned int timeout) {
    //wait for feedback
    unsigned int time = 0;
    unsigned char num;
    unsigned char i;

    //waiting for the first character with time out
    i = 0;
    while(1)
    {
        delay(50);
        if(Serial.available())
        {
            recv_str[i] = char(Serial.read());
            i++;
            break;
        }
        time++;
        if(time > (timeout / 50)) return -1;
    }

    //read other characters from uart buffer to string
    while(Serial.available() && (i < 100))
    {                                              
        recv_str[i] = char(Serial.read());
        i++;
    }
    recv_str[i] = '\0';

    return 0;
}

//-------touchscreen position sub
void touch(){
	if(Serial.available())
	{
		int delayIn=0, trigIn=0, hposIn=0;
        if(recvMsg(100) == 0)
        {
            if(strcmp((char *)recv_str, (char *)"DELAY") == 0)
				delayIn=1;
			else if (strcmp((char *)recv_str, (char *)"TRIG") == 0)
				trigIn=1;
			else if(strcmp((char *)recv_str, (char *)"HPOS") == 0)
				hposIn=1;
		}
		if(delayIn==1) {
            mode = mode+1;
			tft.drawRoundRect(btSTART+2, ROW1+2, resBTSELROW, resBTSELCOL, 2, MAGENTA); 
			
			// Select delay times
			if (mode == 0) dTime = 0;
			if (mode == 1) dTime = 1;
			if (mode == 2) dTime = 2;
			if (mode == 3) dTime = 5;
			if (mode == 4) dTime = 10;
			if (mode == 5) dTime = 20;
			if (mode == 6) dTime = 30;
			if (mode == 7) dTime = 50;
			if (mode == 8) dTime = 100;
			if (mode == 9) dTime = 200;
			if (mode == 10) dTime = 500;
			if (mode > 10) mode = 0; 
		}
		if(trigIn==1) {
			tmode= tmode+1;
			// Select Software trigger value
			tft.drawRoundRect (btSTART+2, ROW2+2, resBTSELROW, resBTSELCOL, 2, MAGENTA); 
			if (tmode == 1) Trigger = 0;
			if (tmode == 2) Trigger = 10;
			if (tmode == 3) Trigger = 20;
			if (tmode == 4) Trigger = 30;
			if (tmode == 5) Trigger = 50;
			if (tmode > 5) tmode = 0;
		}
		
		if(hposIn==1) {
			hpos= hpos+5;
			tft.drawRoundRect (btSTART+2, ROW3+2, resBTSELROW, resBTSELCOL, 2, MAGENTA);
			if (hpos > 80) hpos = 20;
		}
        delay(500);
        buttons(); tft.fillScreen(TFT_BLACK);
        Serial.print(dTime); Serial.print(Trigger);Serial.println(hpos);
	}
}

//----------draw grid sub
void DrawGrid(){
    for( dgvh = 0; dgvh < 5; dgvh ++ ){
        tft.drawLine( dgvh * resGRID, 0, dgvh * resGRID, ROW5, LTGREEN);            //vertical
        tft.drawLine(  0, dgvh * resGRID, grEND ,dgvh * resGRID, LTGREEN);        	//horizental
    }
    //tft.drawLine(  0, maxHEIGHT, grEND ,maxHEIGHT, LTGREEN); 						//bottom line
    tft.drawLine( 5* resGRID, 0, 5* resGRID, ROW5, LTGREEN);                        //6'th vertica
	tft.drawLine( 6 * resGRID, 0, 6 * resGRID, ROW5, LTGREEN);					    //7'th vertical
	
    tft.drawRoundRect(btSTART, ROW1, resGRID, resGRID, 1, WHITE);
    tft.drawRoundRect(btSTART, ROW2, resGRID, resGRID, 1, WHITE);
    tft.drawRoundRect(btSTART, ROW3, resGRID, resGRID, 1, WHITE);
    tft.drawRoundRect(btSTART, ROW4, resGRID, resGRID, 1, WHITE);
}

// ------ Wait for input to be greater than trigger sub
void trigger(){
	while (Input < Trigger)
		Input = analogRead(A0)*5/102;
}
//---------------End Subrutines ----------------------


void setup() {
    tft.begin();
    tft.fillScreen(TFT_BLACK);
    tft.setRotation(1);

	buttons();
	pinMode(0, INPUT);
	
	// set up the ADC
	ADCSRA &= ~PS_128; // remove bits set by Arduino library
	// you can choose a prescaler from below.
	// PS_16, PS_32, PS_64 or PS_128
	ADCSRA |= PS_64; // set our own prescaler   
	
	Serial.begin(115200); 
	Serial.println("miniOSC starts");  
}
  
void loop() {
	touch();
    DrawGrid();     //shoul run, graph destroy grid
	trigger();
    // Collect the analog data into an array

    StartSample = micros();
    
    for( int xpos = 0; xpos < grEND; xpos ++) { 
        Sample[ xpos] = analogRead(A0)*5/102;
        delayMicroseconds(dTime);
    }
    EndSample = micros();

    // Display the collected analog data from array
    for( int xpos = 0; xpos < grEND;  xpos ++)
    {
        // Erase previous display
        tft.drawLine (xpos + 1, 255-OldSample[xpos+1]*vsens/10 - hpos, xpos + 2, 255-OldSample[ xpos + 2]* vsens/10 - hpos, COLOR_BACK);
        if (xpos == 0) tft.drawLine(xpos + 1, 1, xpos + 1, maxHEIGHT, COLOR_BACK);
        //Draw the new data
        tft.drawLine (xpos, 255-Sample[xpos]*vsens/10 - hpos, xpos + 1, 255-Sample[ xpos + 1]* vsens /10 - hpos, COLOR_GRAPH);
    }
	// Determine sample voltage peak to peak
	Max = Sample[ 100];
	Min = Sample[ 100];    
    Sum = 0; SquareSum=0;
    for( int xpos = 0; xpos < grEND; xpos ++)
    {
        OldSample[ xpos] = Sample[ xpos];
        if (Sample[ xpos] > Max) Max = Sample[ xpos];
        if (Sample[ xpos] < Min) Min = Sample[ xpos];
        Sum = Sum + Sample[xpos];
        SquareSum = SquareSum + Sample[xpos]*Sample[xpos];
    }
    
    // display the sample time, delay time and trigger level
    tft.setTextSize(1);
    tft.setTextColor(WHITE,BLUE);  
    tft.drawString("Delay", valSTART, ROW1+5, 2);                	// 5+70*i
    tft.drawString(itoa ( dTime, buf, 10), valSTART, ROW1+20, 2);   // 20+70*i
    tft.drawString("Trig.", valSTART, ROW2+5, 2);
    tft.drawString(itoa( Trigger, buf, 10), valSTART, ROW2+20, 2);
	tft.drawString("H Pos.", valSTART, ROW3+5, 2);
	tft.drawString( itoa( hpos, buf, 10), valSTART, ROW3+20, 2);
    SampleTime =(EndSample-StartSample)/1000;
    tft.drawString("mSec.", valSTART, ROW4+5, 2);
    tft.drawFloat(SampleTime, 2, valSTART, ROW4+20, 2);

    // color setting of Vpp, Vrms, Vmean
    tft.setTextColor(CYAN, BLACK);
    //show Vpp
	SampleSize =(Max-Min)*100;
	tft.drawString("Vpp(mVolt)", 5, 290, 2);                        //y:290
    tft.drawString("          ", 5, 315, 2);                        //y:315, clear previous value
	tft.drawString(itoa(SampleSize, buf, 10),5, 315, 2);            //x:5

    //show Vmrs or V
    //tft.drawString("V(mVolt)", 5+100, 290, 2);
    //tft.drawString("          ", 5+100, 315, 2);                   
    //tft.drawString(itoa(analogRead(A0)*50.0/10.23, buf, 10),5+100 ,315, 2);
    tft.drawString("Vrms(mVolt)", 5+100, 290, 2);                   //y:290
    tft.drawString("          ", 5+100, 315, 2);                    //y:315, clear previous value
    tft.drawString(itoa(sqrt(SquareSum/grEND)*100, buf, 10),5+100 ,315, 2); //x:105

    //show Vmean
    tft.drawString("Vmean(mVolt)", 5+200, 290, 2);                  //y:290
    tft.drawString("          ", 5+200, 315, 2);                    //y:315, clear previous value
    tft.drawString( itoa((Sum/grEND*100), buf, 10),5+200 ,315, 2);  //x:205
}
