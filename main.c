#include <msp430.h>
#include "rotary-encoder.h"
#include "nokia_5110.h"

//global Variable
char x = '0';
int menu_number = 0; 
int menu_number_prev = 0;
int cursor = 1;

//Variable to mark the need of updating the display
int LCD_Change = 1;

//ANALOG INPUT
char AI_Max[5] = { '0','3', ',', '0','0', '\0'}; //Maximum value 
char AI_Min[5] = { '0', '0', ',', '0','0', '\0'}; //Minimum value
char AI_Value[6]; //Actual value
long int  AI_Max_Int = 300; 
long int AI_Min_Int = 0; 
long int AI_Value_Int; 
long AI_Percentage;	
int comma = 3; //Position of the comma

//The position of the comma for max and min value
int AI_Max_Komma = 2;
int AI_Min_Komma = 2;

//PREFIX AND UNIT
int prefix_sel = 2;
int unit_sel = 2;
char *prefix[9] = {" Mega", " Kilo", "Hecto", " Deka", "     ", " Deci", "Centi", " Mili", "Micro"};
char *prefix_symbol[9] = { "    M", "    k","    h","   da","     ", "    d","    c","    m","    {"};
char *unit[4] = {"Ampere ", "Volt   " , "Ohm    ", "Celcius"};
char *unit_symbol[4] = {"A      ","V      ","}      ","°C     "};

//ERROR FLAG
int error_flag = 0; //  0  means no error


//CALIBRATION
char KAL_ADC10[5] = {'0','0','0','0','\0'}; //Aktual value of the Reg. ADC10MEM
char KAL_4mA_char[5] = { '0', '2', '5', '0', '\0' };
char KAL_20mA_char[5] = { '0', '9', '3', '2', '\0' };
int KAL_20mA = 932 ; //Value of the Reg. ADC10MEM at 4 mA
int KAL_4mA = 250; //Value of the Reg. ADC10MEM at 20 mA

//Digital Filter - Averaging
long int ADC10_Mem[20] = {250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250, 250};

//Digitaler Filter
void ADC10_New(long int ADC10Copy)
{
    int i = 19;

    for (i = 19; i > 0 ; i--)
    {
        ADC10_Mem[i] = ADC10_Mem[i-1];
    }
    ADC10_Mem[0] = ADC10Copy;
}

long int ADC10_Avg()
{
    long int sum = 0;
    int i = 0;
    for (i = 0; i < 20; i++)
    {
        sum = sum + ADC10_Mem[i];
    }

    return sum/20;
}

void ADC10_Init(long int ADC10Copy)
{
    int i;
    for (i = 19; i > 0; i--)
    {
        ADC10_Mem[i] = ADC10Copy;
    }
    ADC10_Mem[0] = ADC10Copy;
}

//Calibration
//Copy ADC10MEM-Copy in array
void KAL_Getchar(long int ADC10Copy)
{
    int i = 0;
    long int result;
    long int rest;
    do{
        result = ADC10Copy/1000;
        KAL_ADC10[i] = result + '0';
        rest = ADC10Copy%1000;
        ADC10Copy = rest*10;
        i++;
    }while (i<4);

   AI_Value[4] = '\0';
}

void KAL_4mA_Getchar(long int KAL_4mA)
{
    int i = 0;
    long int result;
    long int rest;
    do{
        result = KAL_4mA/1000;
        KAL_4mA_char[i] = result + '0';
        rest = KAL_4mA%1000;
        KAL_4mA = rest*10;
        i++;
    }while (i<4);

   AI_Value[4] = '\0';
}
void KAL_20mA_Getchar(long int KAL_20mA)
{
    int i = 0;
    long int result;
    long int rest;
    do{
        result = KAL_20mA/1000;
        KAL_20mA_char[i] = result + '0';
        rest = KAL_20mA%1000;
        KAL_20mA = rest*10;
        i++;
    }while (i<4);

   AI_Value[4] = '\0';
}

//Check if the set value for 4 mA < 20 mA
int KAL_Check()
{
	if (KAL_4mA > KAL_20mA)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

//return array KAL_ADC10 as Integer
int KAL_Getint()
{
    int i = 0;
    int wert = 0;
    int faktor = 1000;
    for ( i = 0; i < 4 ; i++)
    {
        wert = wert + (KAL_ADC10[i] - '0')*faktor;
        faktor /= 10; //Valency at the next position is 1/10
    }

    return wert;
}

//Accept the set value for 4 mA
void KAL_4mA_Set()
{
	KAL_4mA = 0;
	int i = 0;
	unsigned int multiplier = 1000;
	do
	{
		KAL_4mA = KAL_4mA + ((KAL_4mA_char[i]) - '0')*multiplier;
		multiplier = multiplier / 10;
		i++;
	} while (i < 4);
}

//Accept the set value for 20 mA
void KAL_20mA_Set()
{
	KAL_20mA = 0;
	int i = 0;
	unsigned int multiplier = 1000;
	do
	{
		KAL_20mA = KAL_20mA + ((KAL_20mA_char[i]) - '0')*multiplier;
		multiplier = multiplier / 10;
		i++;
	} while (i < 4);
}

//Check if Minimum < Maximum, otherwise -> Error flag
int AI_Limit_Check()
{
    if (AI_Min_Int > AI_Max_Int)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

//The position of the comma on both maximum and minimum have to be the same
void AI_Min_Set_Komma()
{
    int i = 0;
    AI_Min_Komma = AI_Max_Komma;
    long int AI_Min_Int_Copy = AI_Min_Int;
    long int result;
    long int rest;
    do
    {
        if (i == AI_Min_Komma)
        {
            AI_Min[i] = ',';
            i++;
        }
        result = AI_Min_Int_Copy / 10000;
        AI_Min[i] = result + '0';
        rest = AI_Min_Int_Copy % 10000;

        AI_Min_Int_Copy = rest * 10;

        i++;

    }
    while (i < 5);
}

//Scaling Analog Input
void AI_Scale(unsigned int ADC10_Copy)
{

    AI_Percentage = (10000*(long)(ADC10_Copy - KAL_4mA))/(long)(KAL_20mA - KAL_4mA);
    AI_Value_Int = (int)((((AI_Max_Int-AI_Min_Int)*AI_Percentage)/10000+AI_Min_Int));
}

//Check if AI_Scale not under or over the value that has been set
int KAL_Scale_Check(unsigned int ADC10_Copy)
{
    if (ADC10_Copy < KAL_4mA || ADC10_Copy > KAL_20mA)
    {
        return 1; // Fehler . Messwert außerhalb Messbereich

    }
    else
    {
        return 0; //fehlerfrei
    }
}

void AI_Getchar()
{
    int i = 0;
    long int result;
    long int rest;
    long int AI_Value_Copy = AI_Value_Int;
    do{

        if(i == AI_Max_Komma)
        {
            AI_Value[i] = ',';
            i++;
        }

        result = AI_Value_Copy/1000;
        AI_Value[i] = result + '0';
        rest = AI_Value_Copy%1000;

        AI_Value_Copy = rest*10;


        i++;
    }while (i<5);

   AI_Value[5] = '\0';
}

void AI_Max_Set()
{
    AI_Max_Int = 0;
    int i = 0;
    unsigned int multiplier = 1000;
    do
    {
        if(i == AI_Max_Komma)
        {
            i++;
        }

        AI_Max_Int = AI_Max_Int + ((AI_Max[i])-'0')*multiplier;
        multiplier = multiplier/10;
        i++;
    }while ( i < 4);
}

void AI_Min_Set()
{
    AI_Min_Int = 0;
    int i = 0;
    unsigned int multiplier = 1000;
    do
    {
        if(i == AI_Min_Komma)
        {
            i++;
        }

        AI_Min_Int = AI_Min_Int + ((AI_Min[i])-'0')*multiplier;
        multiplier = multiplier/10;
        i++;
    }while ( i < 4);
}

//Incremental Decoder --------------------------------------------------------------------------------------------------------------------Incremental Decoder--------
//StepCCW
void stepCCW()
{


	switch (menu_number)
	{
	case (0):
	{
	  //int cursor_min = 1;
		if (cursor != 1)
		{
			cursor--;
		}

		LCD_Change = 1;
		break;

	}
	case (2):
	{

	  //int cursor_min = 1;
		if (cursor != 1)
		{
			cursor--;
		}

		LCD_Change = 1;
		break;

	}
	case (3):
	{

	  //int cursor_min = 1;
		if (cursor != 1)
		{
			cursor--;
		}

		LCD_Change = 1;
		break;

	}

	case (4):
	{

	  //int cursor_min = 0;
		if (cursor != 0)
		{
			cursor--;
			if (cursor == AI_Min_Komma)
			{
				cursor--;
			}
		}

		LCD_Change = 1;
		break;
	}
	case (5):
	{
	  //int cursor_min = 0;
		if (cursor != 0)
		{
			cursor--;
		}

		LCD_Change = 1;
		break;
	}
	case (6):
	{

		if (cursor == 1)
		{
			if (prefix_sel != 0) //Min Value = 0
			{
				prefix_sel--;
			}
		}
		else if (cursor == 2)
		{
			if (unit_sel != 0) //Min Value = 0
			{
				unit_sel--;
			}
		}

		LCD_Change = 1;
		break;
	}
	case (7):
	{

		if (AI_Min[cursor] != '0') //Min Number : 0
		{
			AI_Min[cursor]--;

		}
		LCD_Change = 1;
		break;
	}

	case (8):
	{ //Endwert ändern

		if (AI_Max[cursor] == '0') //Min Number: 0
		{

		}
		else if (AI_Max[cursor] == ',')
		{
			if (cursor > 1)
			{
				char help = 0;
				help = AI_Max[(cursor - 1)];
				AI_Max[(cursor - 1)] = ',';
				AI_Max[cursor] = help;
				cursor--;
				AI_Max_Komma = cursor;
			}
			else if (cursor == 1)
			{

			}
		}
		else
		{
			AI_Max[cursor]--;
		}
		LCD_Change = 1;
		break;
	}
	case (10):
	{

	  //int cursor min = 1
		if (cursor != 1)
		{
			cursor--;

		}
		LCD_Change = 1;
		break;
	}
	case (15):
	{ //Calibration at 4mA

	  //int cursor min = 1
		if (cursor != 1)
		{
			cursor--;

		}
		LCD_Change = 1;
		break;
	}
	case (16):
	{ //Calibration at 20mA

	  //int cursor min = 1
		if (cursor != 1)
		{
			cursor--;

		}
		LCD_Change = 1;
		break;
	}
	case (17):
	{ //Manual Calibration at 4 maA 

	  //int cursor_min = 0;
		if (cursor != 0)
		{
			cursor--;
		}

		LCD_Change = 1;
		break;
	}
	case (18):
	{ //Manual Calibration at 20 maA 

	  //int cursor_min = 0;
		if (cursor != 0)
		{
			cursor--;
		}

		LCD_Change = 1;
		break;
	}
	case (20):
	{ //Manual Calibration at 4 mA - inverted

		if (KAL_4mA_char[cursor] != '0') //Min Number : 0
		{
			KAL_4mA_char[cursor]--;

		}
		LCD_Change = 1;
		break;
	}
	case (21):
	{ //Manual Calibration at 20 mA - inverted

		if (KAL_20mA_char[cursor] != '0') //Min Number : 0
		{
			KAL_20mA_char[cursor]--;

		}
		LCD_Change = 1;
		break;
	}
	default: break;
	}

}

//StepCW
void stepCW()
{

	switch (menu_number)
	{
	case (0):
	{

	  //int cursor_max = 2;
		if (cursor != 2)
		{
			cursor++;
		}

		LCD_Change = 1;
		break;

	}
	case (2):
	{

	  //int cursor_max = 5;
		if (cursor != 5)
		{
			cursor++;
		}

		LCD_Change = 1;
		break;

	}
	case (3):
	{

	  //int cursor_min = 5;
		if (cursor != 3)
		{
			cursor++;
		}

		LCD_Change = 1;
		break;

	}
	case (4):
	{

	  //int cursor_max = 5;
		if (cursor != 5)
		{
			cursor++;
			if (cursor == AI_Min_Komma)
			{
				cursor++;
			}

		}

		LCD_Change = 1;
		break;
	}
	case (5):
	{

	  //int cursor_max = 5;
		if (cursor != 5)
		{
			cursor++;
		}

		LCD_Change = 1;
		break;
	}
	case (6):
	{

		if (cursor == 1)
		{
			if (prefix_sel != 8) //Höchstes Element = 8			
			{
				prefix_sel++;
			}
		}
		else if (cursor == 2)
		{
			if (unit_sel != 3) //Höchstes Element = 3			
			{
				unit_sel++;
			}
		}

		LCD_Change = 1;
		break;
	}
	case (7):
	{

		if (AI_Min[cursor] != '9')	//max Number : 9
		{
			AI_Min[cursor]++;
		}
		LCD_Change = 1;
		break;
	}
	case (8):
	{

		if (AI_Max[cursor] == '9') //max Number: 9
		{

		}
		else if (AI_Max[cursor] == ',')
		{
			if (cursor < 3)
			{
				char help = 0;
				help = AI_Max[(cursor + 1)];
				AI_Max[(cursor + 1)] = ',';
				AI_Max[cursor] = help;
				cursor++;
				AI_Max_Komma = cursor;
			}
		}
		else
		{
			AI_Max[cursor]++;
		}
		LCD_Change = 1;
		break;
	}
	case (10):
	{

	  //int cursor_max = 4
		if (cursor != 4)
		{
			cursor++;

		}
		LCD_Change = 1;
		break;
	}
	case (15):
	{ //Calibration at 4mA

	  //int cursor_max = 3
		if (cursor != 3)
		{
			cursor++;

		}
		LCD_Change = 1;
		break;
	}
	case (16):
	{ //Calibration at 20mA

	  //int cursor_max = 3
		if (cursor != 3)
		{
			cursor++;

		}
		LCD_Change = 1;
		break;
	}
	case (17):
	{ //Manual Calibration at 4 mA

	  //int cursor_max = 4;
		if (cursor != 4)
		{
			cursor++;
		}

		LCD_Change = 1;
		break;
	}
	case (18):
	{ //Manual Calibration at 20 mA

	  //int cursor_max = 4;
		if (cursor != 4)
		{
			cursor++;
		}

		LCD_Change = 1;
		break;
	}
	case (20):
	{ //Manual Calibration at 4 mA - inverted

		if (KAL_4mA_char[cursor] != '9')	//max Number : 9
		{
			KAL_4mA_char[cursor]++;
		}
		LCD_Change = 1;
		break;
	}
	case (21):
	{ //Manual Calibration at 20 mA - inverted

		if (KAL_20mA_char[cursor] != '9')	//max Number Zahl : 9
		{
			KAL_20mA_char[cursor]++;
		}
		LCD_Change = 1;
		break;
	}
	default:
		break;
	}

}

void stepPush()
{
	switch (menu_number)
	{
	case (0):
	{

		switch (cursor)
		{
		case (1):
		{
			LCD_clear();

			//Analog Input
			ADC10CTL0 = ADC10ON + ADC10IE + SREF_1 + REFON + REF2_5V;
			ADC10CTL1 = ADC10DIV_5;
			ADC10AE0 = INCH_0;
			ADC10CTL0 |= ENC + ADC10SC;

			menu_number = 1;

			break;
		}
		case (2):
		{

			menu_number = 2;
			break;
		}
		default:
			break;
		}

		cursor = 1;
		LCD_Change = 1;
		break;

	}
	case (1):
	{
		menu_number = 0;

		cursor = 1;
		LCD_Change = 1;
		break;

	}
	case (2):
	{

		switch (cursor)
		{
		case (1):
		{

			menu_number = 3;
			cursor = 1;
			break;
		}
		case (2):
		{

			menu_number = 4;
			cursor = 0;
			break;
		}
		case (3):
		{

			menu_number = 5;
			cursor = 0;
			break;
		}
		case (4):
		{

			LCD_clear();

			menu_number = 10;
			cursor = 1;
			break;
		}
		case (5):
		{

			menu_number = 0;
			cursor = 2;
			break;
		}

		default:
			break;
		}


		LCD_Change = 1;
		break;

	}
	case (3):
	{
		if (cursor < 3)
		{
			menu_number = 6;
		}
		else if (cursor == 3)
		{
			menu_number = 2;
			cursor = 1;
		}

		LCD_Change = 1;
		break;

	}
	case (4):
	{
		if (cursor == 5)
		{

			menu_number = 2;
			cursor = 2;
		}
		else if (cursor < 5)
		{

			menu_number = 7;
		}


		LCD_Change = 1;
		break;

	}
	case (5):
	{

		if (cursor == 5)
		{

			menu_number = 2;
			cursor = 3;
		}
		else
		{

			menu_number = 8;
		}

		LCD_Change = 1;
		break;

	}
	case (6):
	{
		menu_number = 3;
		LCD_Change = 1;
		break;

	}
	case (7):
	{

		AI_Min_Set();
		int error = AI_Limit_Check();

		if (error == 0)
		{
			menu_number = 4;
		}
		else
		{
			menu_number = 13;
		}


		LCD_Change = 1;
		break;

	}
	case (8):
	{

		AI_Max_Set();
		int error = AI_Limit_Check();
		AI_Min_Set_Komma();

		if (error == 0)
		{
			menu_number = 5;
		}
		else
		{
			menu_number = 9;
		}

		LCD_Change = 1;
		break;

	}
	case (9):
	{
		menu_number = 8;
		LCD_clear();
		LCD_Change = 1;
		break;
	}
	case (10):
	{
		switch (cursor)
		{
		case (1):
		{

			menu_number = 15;
			cursor = 1;
			LCD_Change = 1;
			break;
		}
		case (2):
		{

			menu_number = 16;
			cursor = 1;
			LCD_Change = 1;
			LCD_clear();
			break;
		}
		case (3):
		{

			KAL_4mA = 250;
			KAL_20mA = 932;


			menu_number = 19;
			cursor = 3;
			LCD_Change = 1;
			break;
		}
		case (4):
		{

			menu_number = 2;
			cursor = 4;
			LCD_Change = 1;
			break;
		}
		default: break;
		}

		break;
	}
	case (11):
	{
		KAL_4mA = KAL_Getint();

		int error = KAL_Check();
		if (error == 1)
		{
		    menu_number = 24;
		    LCD_Change = 1;
		}
		else
		{
		    KAL_4mA_Getchar(KAL_4mA);
		    menu_number = 10;
		    LCD_Change = 1;
		}
		break;
	}
	case (12):
	{
		KAL_20mA = KAL_Getint();

		int error = KAL_Check();
        if (error == 1)
        {
            menu_number = 25;
            LCD_Change = 1;
        }
        else
        {
            KAL_4mA_Getchar(KAL_4mA);
            menu_number = 10;
            LCD_Change = 1;
        }
        break;
		break;
	}
	case (13):
	{ //Error: Anfangswert > Endwert


		menu_number = 7;
		LCD_Change = 1;
		break;
	}
	case (14):
	{ //Error: Messwert außerhalb Messbereich

		menu_number = 0;
		LCD_Change = 1;
		break;
	}
	case (15):
	{		//Caliibration at 4 mA
		switch (cursor)
		{
		case(1):
		{

			menu_number = 17;
			LCD_Change = 1;
			break;
		}
		case (2):
		{
			//Analog Input
			ADC10CTL0 = ADC10ON + ADC10IE + SREF_1 + REFON + REF2_5V;
			ADC10CTL1 = ADC10DIV_5;
			ADC10AE0 = INCH_0;
			ADC10CTL0 |= ENC + ADC10SC;

			menu_number = 11;
			LCD_Change = 1;
			LCD_clear();
			break;
		}
		case(3):
		{

			menu_number = 10;
			cursor = 1;
			LCD_Change = 1;
		}
		default: break;
		}
		break;
	}
	case (16):
	{
		switch (cursor)
		{
		case(1):
		{

			menu_number = 18;
			LCD_Change = 1;
			break;
		}
		case (2):
		{
			//Analog Input
			ADC10CTL0 = ADC10ON + ADC10IE + SREF_1 + REFON + REF2_5V;
			ADC10CTL1 = ADC10DIV_5;
			ADC10AE0 = INCH_0;
			ADC10CTL0 |= ENC + ADC10SC;


			menu_number = 12;
			LCD_Change = 1;
			LCD_clear();
			break;
		}
		case(3):
		{
			menu_number = 10;
			cursor = 1;
			LCD_Change = 1;
		}
		default: break;
		}
		break;
	}
	case (17):
	{

		if (cursor == 4)
		{
			menu_number = 15;
			cursor = 1;
		}
		else
		{

			menu_number = 20;
		}

		LCD_Change = 1;
		break;

	}
	case (18):
	{

		if (cursor == 4)
		{

			menu_number = 16;
			cursor = 1;
		}
		else
		{
			menu_number = 21;
		}

		LCD_Change = 1;
		break;

	}
	case (19):
	{
		menu_number = 10;
		cursor = 3;
		LCD_Change = 1;
		break;
	}
	case (20):
	{

		KAL_4mA_Set();
		int error = KAL_Check();

		if (error == 0)
		{

			menu_number = 17;
		}
		else
		{
			menu_number = 22;
		}


		LCD_Change = 1;
		break;

	}
	case (21):
	{

		KAL_20mA_Set();
		int error = KAL_Check();

		if (error == 0)
		{

			menu_number = 18;
		}
		else
		{
			menu_number = 23;
		}

		LCD_Change = 1;
		break;

	}
	case (22):
	{
		menu_number = 20;
		LCD_Change = 1;
		break;
	}
	case (23):
	{
		menu_number = 21;
		LCD_Change = 1;
		break;
	}
	case (24):
    {

        menu_number = 11;
        LCD_clear();
        LCD_Change = 1;
        break;
    }
    case (25):
    {
        menu_number = 12;
        LCD_clear();
        LCD_Change = 1;
        break;
    }

	default:
		break;
	}

}

//LCD Update
void LCD_Update()
{
	switch (menu_number)
	{
	case (0):
	{
		LCD_clear();

		int i = 1;
		char *inhalt[2] = { "Messen", "Einstellg." };

		LCD_write_string(0, 0, "Messgerät");
        for (i = 1; i < 3; i++)
        {
            if (cursor == i)
            {
                LCD_write_string(0, i + '0', ">");
            }
            LCD_write_string(7, i + '0', inhalt[i - 1]);
        }

		LCD_Change = 0;
		break;

	}
	case (1):
	{

		LCD_write_string(0, 0, "Wertanzeige");
		LCD_write_string(0, 3, &AI_Value);
		LCD_write_string(23, 3, prefix_symbol[prefix_sel]);
		LCD_write_string(54, 3, unit_symbol[unit_sel]);
		LCD_write_string(0, 5, ">");
		LCD_write_string(7, 5, "Zurück");
		break;
	}
	case (2):
	{

		LCD_clear();

		int i = 1;
		char *inhalt[5] = { "Einheit", "Anfangswert", "Endwert", "Kalibrieren",
			"Zurück" };


		LCD_write_string(0, 0, "Einstellungen");
        for (i = 1; i < 6; i++)
        {
            if (cursor == i)
            {
                LCD_write_string(0, i + '0', ">");
            }
            LCD_write_string(7, i + '0', inhalt[i - 1]);

        }
		LCD_Change = 0;
		break;
	}
	case (3):
	{

		LCD_clear();

		int i = 1;

		LCD_write_string(0, 0, "Einheiten");


		if (cursor == 1)
		{
			LCD_write_string(35, 1, "v");
		}
		if (cursor == 2)
		{
			LCD_write_string(41, 1, "v");
		}

		LCD_write_string(10, 2, prefix_symbol[prefix_sel]);
		LCD_write_string(41, 2, unit_symbol[unit_sel]);
		LCD_write_string(10, 3, prefix[prefix_sel]);
		LCD_write_string(41, 3, unit[unit_sel]);


		if (cursor == 3)
		{
			LCD_write_string(0, 5, ">");
		}
		LCD_write_string(7, 5, "Zurück");

		LCD_Change = 0;
		break;

	}

	case (4):
	{

		LCD_clear();

		LCD_write_string(0, 0, "Anfangswert");

		LCD_write_string(23, 2, &AI_Min);


		if (cursor == 5)
		{
			LCD_write_string(0, 5, ">");
		}
		else
		{
			int cursor_help;
			cursor_help = (cursor * 6) + 23;
			LCD_write_string(cursor_help, 3, "^");
		}
		LCD_write_string(7, 5, "Zurück");
		LCD_Change = 0;
		break;
	}

	case (5):
	{

		LCD_clear();	

		LCD_write_string(0, 0, "Endwert");		
		LCD_write_string(23, 2, &AI_Max);
		if (cursor == 5)
		{
			LCD_write_string(0, 5, ">");
		}
		else
		{
			int cursor_help;
			cursor_help = (cursor * 6) + 23;
			LCD_write_string(cursor_help, 3, "^");
		}
		LCD_write_string(7, 5, "Zurück");
		LCD_Change = 0;
		break;
	}
	case (6):
	{

		LCD_clear();
		int i = 1;

		LCD_write_string(0, 0, "Einheit");

		if (cursor == 1)
		{
			LCD_write_string(35, 1, "v");
			LCD_write_string_hl(10, 2, prefix_symbol[prefix_sel]);
			LCD_write_string(41, 2, unit_symbol[unit_sel]);
			LCD_write_string_hl(10, 3, prefix[prefix_sel]);

			LCD_write_string(41, 3, unit[unit_sel]);
		}
		if (cursor == 2)
		{
			LCD_write_string(41, 1, "v");
			LCD_write_string(10, 2, prefix_symbol[prefix_sel]);
			LCD_write_string_hl(41, 2, unit_symbol[unit_sel]);
			LCD_write_string(10, 3, prefix[prefix_sel]);
			LCD_write_string_hl(41, 3, unit[unit_sel]);
		}

		LCD_write_string(7, 5, "Zurück");

		LCD_Change = 0;
		break;
	}
	case (7):
	{


		int i = 0;
		int X = 23;

		LCD_clear();

		LCD_write_string(0, 0, "Anfangswert");


		for (i = 0; i < 5; i++)
		{
			LCD_set_XY(X, 2);
			if (i == cursor)
			{
				LCD_write_char_hl(AI_Min[i]);
			}
			else
			{
				LCD_write_char(AI_Min[i]);
			}

			X += 6;
		}

		if (cursor == 5)
		{
			LCD_write_string(0, 5, ">");
		}
		else
		{
			int cursor_help;
			cursor_help = (cursor * 6) + 23;
			LCD_write_string(cursor_help, 3, "^");
		}
		LCD_write_string(7, 5, "Zurück");
		LCD_Change = 0;
		break;
	}
	case (8):
	{


		int i = 0;
		int X = 23;

		LCD_clear();

		LCD_write_string(0, 0, "Endwert");


		for (i = 0; i < 5; i++)
		{
			LCD_set_XY(X, 2);
			if (i == cursor)
			{
				LCD_write_char_hl(AI_Max[i]);
			}
			else
			{
				LCD_write_char(AI_Max[i]);
			}
			X += 6;
		}
		if (cursor == 5)
		{
			LCD_write_string(0, 5, ">");
		}
		else
		{
			int cursor_help;
			cursor_help = (cursor * 6) + 23;
			LCD_write_string(cursor_help, 3, "^");
		}
		LCD_write_string(7, 5, "Zurück");
		LCD_Change = 0;
		break;
	}
	case (9):
	{

		LCD_clear();
		LCD_write_string(0, 0, "Fehler");
		LCD_write_string(0, 1, "Endwert < Anfangswert");
		LCD_write_string(0, 5, ">");
		LCD_write_string(7, 5, "Zurück");
		LCD_Change = 0;
		break;
	}
	case (10):
	{

	    LCD_clear();
		int i;
		char* inhalt[4] = { "4 mA", "20 mA", "Rücksetzen", "Zurück" };

		LCD_write_string(0, 0, "Kalibrieren");
		for (i = 1; i < 5; i++)
		{
			if (cursor == i)
			{
			    LCD_write_string(0, i, ">");
			}
			LCD_write_string(7, i, inhalt[i - 1]);
		}
		LCD_Change = 0;
		break;
	}
	case (11):
	{
		LCD_write_string(0, 0, "4 mA - Auto");


		LCD_write_string(23, 2, &KAL_ADC10);
		LCD_write_string(0, 5, ">");
		LCD_write_string(7, 5, "Set u. Zurück");


		break;

	}
	case (12):
	{
	    LCD_write_string(0, 0, "20 mA - Auto");


		LCD_write_string(23, 2, &KAL_ADC10);
		LCD_write_string(0, 5, ">");
		LCD_write_string(7, 5, "Set u. Zurück");


		break;
	}
	case (13):
	{
	    LCD_clear();
		LCD_write_string(0, 0, "Fehler");
		LCD_write_string(0, 1, "Anfangswert >");
		LCD_write_string(0, 2, "Endwert");
		LCD_write_string(0, 5, ">");
		LCD_write_string(7, 5, "Zurück");

		LCD_Change = 0;
		break;
	}
	case (14):
	{
	    LCD_clear();
		LCD_write_string(0, 0, "Fehler");
		LCD_write_string(0, 1, "Messwert");
		LCD_write_string(0, 2, "außerhalb");
		LCD_write_string(0, 3, "Messbereich");
		LCD_write_string(0, 5, ">");
		LCD_write_string(7, 5, "Zurück");

		LCD_Change = 0;
		break;
	}
	case (15):
	{

		LCD_clear();

		LCD_write_string(0,0, "4 mA");

		char *text[3] = { "Manual", "Auto", "Zurück" };
		int i = 1;

		for (i; i < 4; i++)
		{
			if (cursor == i)
			{
				LCD_write_string(0, i, ">");
			}
			LCD_write_string(7, i, text[i-1]);
		}

		LCD_Change = 0;
		break;
	}
	case (16):
	{

		LCD_clear();

		LCD_write_string(0, 0, "20 mA");

		char *text[3] = { "Manual", "Auto", "Zurück" };
		int i = 1;

		for (i; i < 4; i++)
		{
			if (cursor == i)
			{
				LCD_write_string(0, i, ">");
			}
			LCD_write_string(7, i, text[i - 1]);
		}

		LCD_Change = 0;
		break;
	}
	case (17):
	{

		LCD_clear();

		LCD_write_string(0, 0, "4 mA - Manual");
		LCD_write_string(23, 2, &KAL_4mA_char);
		if (cursor == 4)
		{
			LCD_write_string(0, 5, ">");
		}
		else
		{
			int cursor_help;
			cursor_help = (cursor * 6) + 23; //Position of the cursor
			LCD_write_string(cursor_help, 3, "^");
		}
		LCD_write_string(7, 5, "Zurück");
		LCD_Change = 0;
		break;
	}
	case (18):
	{
		//Calibration at 20 mA - Manual
		LCD_clear();

		LCD_write_string(0, 0, "20 mA - Manual");
		LCD_write_string(23, 2, &KAL_20mA_char);
		if (cursor == 4)
		{
			LCD_write_string(0, 5, ">");
		}
		else
		{
			int cursor_help;
			cursor_help = (cursor * 6) + 23; //Position of the cursor
			LCD_write_string(cursor_help, 3, "^");
		}
		LCD_write_string(7, 5, "Zurück");
		LCD_Change = 0;
		break;
	}
	case (19):
	{
		//Calibration is reset
		LCD_clear();
		LCD_write_string(0, 0, "Kalibration");
		LCD_write_string(0, 1, "ist");
		LCD_write_string(0, 2, "zurück-");
		LCD_write_string(0, 3, "gesetzt");
		LCD_write_string(0, 5, ">");
		LCD_write_string(7, 5, "Zurück");

		LCD_Change = 0;
		break;
	}
	case (20):
	{


		int i = 0;
		int X = 23;

		LCD_clear();

		LCD_write_string(0, 0, "20 mA - Manual");


		for (i = 0; i < 4; i++)
		{
			LCD_set_XY(X, 2);
			if (i == cursor)
			{
				LCD_write_char_hl(KAL_4mA_char[i]);
			}
			else
			{
				LCD_write_char(KAL_4mA_char[i]);
			}

			X += 6;
		}

		if (cursor == 4)
		{
			LCD_write_string(0, 5, ">");
		}
		else
		{
			int cursor_help;
			cursor_help = (cursor * 6) + 23; //Position of the cursor
			LCD_write_string(cursor_help, 3, "^");
		}
		LCD_write_string(7, 5, "Zurück");
		LCD_Change = 0;
		break;
	}
	case (21):
	{ //Manual Calibration at 20 mA - inverted	


		int i = 0;
		int X = 23;

		LCD_clear();

		LCD_write_string(0, 0, "4 mA - Manual");

		//Inverted Bits
		for (i = 0; i < 4; i++)
		{
			LCD_set_XY(X, 2);
			if (i == cursor)
			{
				LCD_write_char_hl(KAL_20mA_char[i]);
			}
			else
			{
				LCD_write_char(KAL_20mA_char[i]);
			}

			X += 6;
		}

		if (cursor == 4)
		{
			LCD_write_string(0, 5, ">");
		}
		else
		{
			int cursor_help;
			cursor_help = (cursor * 6) + 23; //Ermittlung der Kursorposition			
			LCD_write_string(cursor_help, 3, "^");
		}
		LCD_write_string(7, 5, "Zurück");
		LCD_Change = 0;
		break;
	}
	case (22):
	{ //Error: Calibration at 4 mA > 20 mA
		LCD_clear();
		LCD_write_string(0, 0, "Fehler");
		LCD_write_string(0, 1, "Kalibration");
		LCD_write_string(0, 2, "4 mA > 20 mA");
		LCD_write_string(0, 5, ">");
		LCD_write_string(7, 5, "Zurück");

		LCD_Change = 0;
		break;
	}
	case (23):
	{ //Error: Calibration at 4 mA > 20 mA
		LCD_clear();
		LCD_write_string(0, 0, "Fehler");
		LCD_write_string(0, 1, "Kalibration");
		LCD_write_string(0, 2, "4 mA > 20 mA");
		LCD_write_string(0, 5, ">");
		LCD_write_string(7, 5, "Zurück");

		LCD_Change = 0;
		break;
	}
	case (24):
    { //Error: Calibration at 4 mA > 20 mA
        LCD_clear();
        LCD_write_string(0, 0, "Fehler");
        LCD_write_string(0, 1, "Kalibration");
        LCD_write_string(0, 2, "4 mA > 20 mA");
        LCD_write_string(0, 5, ">");
        LCD_write_string(7, 5, "Zurück");

        LCD_Change = 0;
        break;
    }
	case (25):
    { //Error: Calibration at 4 mA > 20 mA
        LCD_clear();
        LCD_write_string(0, 0, "Fehler");
        LCD_write_string(0, 1, "Kalibration");
        LCD_write_string(0, 2, "20 mA > 4 mA");
        LCD_write_string(0, 5, ">");
        LCD_write_string(7, 5, "Zurück");

        LCD_Change = 0;
        break;
    }
		default:
		break;
	}
	// LCD_Change = 0;
}



int main(void) {

     WDTCTL = WDTPW + WDTHOLD;

    //LCD Initialisation
    LCD_init();
    LCD_clear();
    LCD_LED_set(1);

    //Encoder Initialisation
    encoderInit();

    __enable_interrupt(); //set low power mode 4

    while(1)
    {
        if (LCD_Change == 1)
        {
            LCD_Update();


        }
        if (menu_number == 1 || menu_number == 11 || menu_number == 12)
        {
            ADC10CTL0 |= ENC + ADC10SC;
        }

    }




}


#pragma vector = ADC10_VECTOR
__interrupt void adc10_interrupt(void)
{
    unsigned int adc_value = 0;

    ADC10_New(ADC10MEM);
    adc_value = ADC10_Avg();

    if (menu_number == 1)
    {
        AI_Scale(adc_value); //Scaling
        int error = KAL_Scale_Check(adc_value); //Check if the scaling is ok
        if (error == 1)
        {
            menu_number = 14;
            LCD_Change = 1;
        }
        AI_Getchar();
    }
    else if(menu_number == 11 || menu_number == 12)
    {
        KAL_Getchar(adc_value);

    }




}
