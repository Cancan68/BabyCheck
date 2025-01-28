#ifndef BABYCHECK_H 
#define BABYCHECK_H

#include "sections.h"
#include "babycheck.rcp.h"

/*
 * Database structures
 * -------------------
 *
 */
typedef struct
{
  Char		name[26];
  TimeType	time;
} BabyDBTypeV1;

typedef struct
{
  Char          name[26];
  TimeType      time;
  Char		sexe;
} BabyDBType;

typedef struct
{
  UInt32	number;
  DateType      date;
  Char		code;
  Char          word;
} WordsDBType;

typedef struct
{
  UInt32	number;
  DateType      date;
  UInt16        weight;
  UInt16	height;
} MeasurDBType;

typedef struct
{
  UInt32	number;
  Char		code;
  DateType      date;
  Boolean	done;
  Char		comment;
} ShotsDBType;

typedef struct
{
  UInt32	number;
  DateType      date;
  TimeType	time;
  Boolean	breast;
  UInt16	duration;  
} SuckDBType;

/*
 * Databases variables
 * -------------------
 *
 */
#define appFileCreator		'CanC'
#define appDBType     		'DATA'
#define appBabyDBVersion	0x00
#define appPrefID		0

DmOpenRef  BabyDBRef;
DmOpenRef  WordsDBRef;
DmOpenRef  MeasurDBRef;
DmOpenRef  ShotDBRef;
DmOpenRef  SuckDBRef;

const Char *BabyDBName = "BabyCheckDB";
const Char *WordsDBName = "BabyCheckDBW";
const Char *MeasurDBName = "BabyCheckDBM";
const Char *ShotsDBName = "BabyCheckDBS";
const Char *SuckDBName = "BabyCheckDBT";

BabyDBType	WRecord;
WordsDBType	WWRecord;
MeasurDBType	WMRecord;
ShotsDBType	WSRecord;
SuckDBType	WTRecord;

typedef struct
{
  UInt8		Munit;
  UInt8		MyVersion;
} BabyLogPreferenceType;

BabyLogPreferenceType Prefs;
UInt16 prefsSize = sizeof(BabyLogPreferenceType);

/* 
 * Global Variables
 * ----------------
 *
 * uniqueID		Baby's record unique ID
 * romVersion		ROM version
 * appPrefVersion	Application preference version
 * listbaby             list of all the babies
 * num_baby             number of babies
 * new_record		new record boolean
 * Kmin			table of the Kg minimum values
 * Kmax			table of the Kg maximum values
 * Cmin			table of the Cm minimum values
 * Cmax			table of the Cm maximum values
 * Lmin			table of the Lbs minimum values
 * Lmax			table of the Lbs maximum values
 * Imin			table of the Inches minimum values
 * Imax			table of the Inches maximum values
 * BMIminb		table of the BMI minimum value for a boy
 * BMImaxb		table of the BMI maximum value for a boy
 * BMIming		table of the BMI minimum value for a girl
 * BMImaxg		table of the BMI maximum value for a girl
 * GraphType  		Graphic Type (1=Weight, 2=Height, 3=BMI)
 * TopRecord            The first record to be displayed on tables
 * NBRows 		Number of Rows of a table (first words or measurements)
 * NBRecs 		Number of Recs for a baby (first words or measurements)
 * RecordNum		Record number
 * dateBirth		Birthdate of the baby
 * dateMax		Max date to be processed for the graphs
 * dateLast		Last entered value
 * useColor		Can we use color for the graphs
 * Resolution		Screen Resolution
 * Resolx		Resolution in x 
 * Resoly		Resolution in y
 * Depth		Color Depth
 * graphMonth		Number of month to be displayed in the graph
 * WordsCode		Code to display words, actions or shots
 * ShotsTable		Table of the shots
 * IsOS5		OS5 compatible
 * IsOS4		OS4 compatible
 * IsHighDensity	HighDensity mode
 * BoolPrefs		Preferences was set
 */

static ListType	*listbaby;
static ListType	*listbabycomp;
 
static UInt32	Kmin[18]={250, 300, 330, 370, 430, 500, 570, 620, 710, 800, 850, 910, 970, 1000, 1030, 1080, 1110, 1150};
static UInt32	Kmax[18]={430, 510, 560, 600, 700, 800, 880, 960, 1090, 1210, 1300, 1390, 1480, 1550, 1610, 1670, 1750, 1800};
static UInt32	Cmin[18]={460, 480, 490, 510, 540, 570, 590, 620, 660, 700, 730, 750, 770, 790, 800, 830, 840, 860};
static UInt32	Cmax[18]={550, 580, 600, 610, 640, 660, 690, 720, 760, 800, 840, 870, 890, 910, 940, 960, 980, 1010};
static UInt32	Lmin[18]={550, 660, 730, 810, 950, 1100,  1250, 1360, 1560, 1760, 1870, 2000, 2130, 2200, 2270, 2380, 2440, 2530};
static UInt32	Lmax[18]={950, 1120, 1220, 1320, 1540, 1760, 1940, 2110, 2400, 2660, 2860, 3060, 3260, 3410, 3540, 3670, 3850, 3960};
static UInt32	Imin[18]={181, 189, 193, 201, 213, 224, 232, 244, 260, 276, 287, 295, 303, 311, 315, 327, 331, 339};
static UInt32	Imax[18]={217, 228, 236, 240, 252, 260, 272, 283, 299, 315, 331, 343, 350, 358, 370, 378, 386, 398};

static UInt32	BMIminb[18]={112, 123, 125, 129, 135, 138, 142, 145, 149, 151, 150, 148, 147, 145, 144, 142, 141, 140};
static UInt32	BMIming[18]={111 ,120, 124, 127, 131, 135, 139, 142, 147, 148, 147, 146, 143, 142, 141, 139, 138, 137};
static UInt32	BMImaxb[18]={158, 170, 175, 178, 185, 190, 195, 197, 203, 204, 201, 198, 195, 192, 189, 187, 185, 183};
static UInt32	BMImaxg[18]={152, 164, 168, 171, 177, 183, 188, 192, 198, 200, 198, 196, 193, 191, 188, 187, 185, 184};

static UInt8	graphMonth, GraphType;

Int16	TopRecord, NBRows, TNBRows, NBRecs, appPrefVersion;

static UInt16	RecordNum;
static UInt16	num_baby = 0;

UInt32	romVersion, uniqueID, uniqueID2, Resolution, Resolx, Resoly, Depth, Base, Wmin, Hmin;

static DateType	dateBirth, dateBirth2, dateMax, dateLast;

static Boolean	newBaby = false;
Boolean	BoolPrefs, useColor, IsOS4, IsOS5, IsHighDensity;

Char	WordsCode;

static Char	ShotsTable[30];

Char thoSep;
Char decSep;
TimeFormatType timeFormat;
DateFormatType dateFormat;
SystemPreferencesType syspref;
UInt8	oldMunit;

#define Item(x) FrmGetObjectPtr(FrmGetActiveForm(), FrmGetObjectIndex(FrmGetActiveForm(), (x)))

#endif
