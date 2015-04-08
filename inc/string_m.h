/*  string.h  */


// =======================================================================================================
// typy zmiennych
typedef enum tCompResult{DIFFERENT, EQUAL} tCompResult;
typedef enum tResult{OK, ERROR} tResult;



// =======================================================================================================
// funkcje
void CopyString(char pcSource[], char pcDestination[]);
tCompResult eCompareString(char pcStr1[], char pcStr2[]);
void AppendString (char pcSourceStr[], char pcDestinationStr[]);
void ReplaceCharactersInString(char pcString[], char cOldChar, char cNewChar);
void UIntToHexStr (unsigned int uiValue, char pcStr[]);
tResult eHexStringToUInt(char pcStr[], unsigned int *puiValue);
void AppendUIntToString(unsigned int uiValue, char pcDestinationStr[]);















