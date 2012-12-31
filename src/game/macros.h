
#ifndef __MACROS_H__
#define __MACROS_H__

#define ARRAY_LENGTH(arr) ((sizeof (arr)) / (sizeof ((arr)[0])))

#ifndef MIN
#define MIN(a, b) (((a) <= (b)) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b) (((a) >= (b)) ? (a) : (b))
#endif

#define STRINGIFY(x) _STRINGIFY(x)
#define _STRINGIFY(x) #x

#define IRBox(a,b,c,d,e)           {InBox((a),(b),(c),(d));fill_rectangle((a)+1,(b)+1,(c)-1,(d)-1,(e));}
#define ORBox(a,b,c,d,e)           {OutBox((a),(b),(c),(d));fill_rectangle((a)+1,(b)+1,(c)-1,(d)-1,(e));}


// f==g
#define pButton(a,b,c,d,e,f,g) if ((x>=(a) && y>=(b) && x<=(c) && y<=(d) && mousebuttons==1) || ((f)==(g)))\
                            { \
                            InBox((a),(b),(c),(d)); \
                            WaitForMouseUp(); \
                            (e);\
                        if((f)>0) delay(150); \
                            OutBox((a),(b),(c),(d));\
                            };

#define Button2(a,b,c,d,e,f,g) if ((x>=(a) && y>=(b) && x<=(c) && y<=(d) && mousebuttons==1) || ((f)==(g)))\
                            { \
                            WaitForMouseUp(); \
                            (e); \
                            };

// AI related macro
#define PrestigeCheck(a,b) (Data->Prestige[(b)].Place==(a) || Data->Prestige[(b)].mPlace==(a))

#define IsHumanPlayer(a) ( (plr[(a)]==0 || plr[(a)]==1) )


#endif

