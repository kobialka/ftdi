/*  string.c  */

#define NULL 0
#define TetraMask_bm 0x0000000f



#include "../inc/string_m.h"




void CopyString(char pcSource[], char pcDestination[])
{	
	unsigned char ucCharacterCounter;

	for(ucCharacterCounter=0; pcSource[ucCharacterCounter] != NULL; ucCharacterCounter++)
	{
		pcDestination[ucCharacterCounter] = pcSource[ucCharacterCounter];
	}
	pcDestination[ucCharacterCounter] = NULL;
}


//---------------------------------------------------------------------------------------
tCompResult eCompareString(char pcStr1[], char pcStr2[])
{
	unsigned char ucCharacterCounter;
	
	for(ucCharacterCounter=0; (pcStr1[ucCharacterCounter] == pcStr2[ucCharacterCounter]); ucCharacterCounter++)
	{
		if(pcStr1[ucCharacterCounter] == NULL)
		{
			return EQUAL;
		}
	}
	return DIFFERENT;
}


//---------------------------------------------------------------------------------------
void AppendString (char pcSourceStr[], char pcDestinationStr[])
{
	unsigned char ucCharacterCounter;
	
	for(ucCharacterCounter=0; pcDestinationStr[ucCharacterCounter] != NULL; ucCharacterCounter++){};
	
	CopyString(pcSourceStr, pcDestinationStr+ucCharacterCounter);
}


//---------------------------------------------------------------------------------------
void ReplaceCharactersInString(char pcString[], char cOldChar, char cNewChar)
{
	unsigned char ucCharacterCounter;
	
	for(ucCharacterCounter=0; pcString[ucCharacterCounter] != NULL; ucCharacterCounter++)
	{
		if(pcString[ucCharacterCounter] == cOldChar)
		{
			pcString[ucCharacterCounter] = cNewChar;
		}
	}
}


//---------------------------------------------------------------------------------------
void UIntToHexStr (unsigned int uiValue, char pcStr[])
{
	unsigned char uiTetraCounter;
	unsigned char uiValueBuff;
	
	for (uiTetraCounter = 0; uiTetraCounter < 8; uiTetraCounter++)
	{
		uiValueBuff = ( (uiValue >> (uiTetraCounter*4)) & TetraMask_bm );
		if (uiValueBuff > 9)
		{
			pcStr[9 - uiTetraCounter] = uiValueBuff + ('A' - 10);
		}
		else
		{
			pcStr[9 - uiTetraCounter] = uiValueBuff + '0';
		}
	}
	pcStr[1] = 'x';
	pcStr[0] = '0';
}


//---------------------------------------------------------------------------------------
tResult eHexStringToUInt(char pcStr[], unsigned int *puiValue)
{
	unsigned char ucCharacterCounter; 
	unsigned char ucCharacterBuff;
	unsigned int  uiValueBuff = 0;
	
	
	if ((pcStr[0] != '0') || (pcStr[1] != 'x') || (pcStr[2] == NULL))
	{
		return ERROR;
	}
		
	for (ucCharacterCounter = 2; pcStr[ucCharacterCounter] != NULL; ucCharacterCounter++)
	{
		if (ucCharacterCounter > 5)
		{
			return ERROR;
		}
		
		uiValueBuff = uiValueBuff << 4;	
		if( (ucCharacterBuff = pcStr[ucCharacterCounter]) > ('a'-1) )  // 0x60 => '''
		{
			ucCharacterBuff = ucCharacterBuff -'a'+10;
		}
		else if( ucCharacterBuff > '@')
		{
			ucCharacterBuff = ucCharacterBuff -'A'+10;
		}
		else
		{
			ucCharacterBuff = ucCharacterBuff -'0';
		}
		uiValueBuff = uiValueBuff + ucCharacterBuff;
	}		
	*puiValue = uiValueBuff;
	return OK;
}




//---------------------------------------------------------------------------------------
void AppendUIntToString(unsigned int uiValue, char pcDestinationStr[])
{
	unsigned char ucCharacterCounter;
	
	for (ucCharacterCounter = 0; pcDestinationStr[ucCharacterCounter] != NULL; ucCharacterCounter++){};
	UIntToHexStr((uiValue), pcDestinationStr + ucCharacterCounter);
}
