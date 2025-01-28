#include "DateDB.h"

/*
 * this is a modified version of the original module provided
 * by Palm Inc. with their SDK 4.0
 *
 * we concentrate here only on inserting untimed records into 
 * the datebook database
 *
 * 25.dec.01 - pete.
 */


/***********************************************************************
 *
 *	Internal Routines
 *
 ***********************************************************************/

static Int16 		TimeCompare (TimeType t1, TimeType t2);

static Int16 		DateCompare (DateType d1, DateType d2);

static UInt16 	ApptPackedSize (ApptDBRecordPtr r);

static void 		ApptPack(ApptDBRecordPtr s, ApptPackedDBRecordPtr d);

static UInt16 	ApptFindSortPosition(DmOpenRef dbP, ApptPackedDBRecordPtr newRecord);

static Int16 		ApptComparePackedRecords (ApptPackedDBRecordPtr r1, ApptPackedDBRecordPtr r2, 
	 												Int16 extra, SortRecordInfoPtr info1, SortRecordInfoPtr info2, 
													MemHandle appInfoH);

static Err			ApptAppInfoInit(DmOpenRef dbP);

static void 		SetDBBackupBit(DmOpenRef dbP);

static Boolean 	FindNextRepeat (ApptDBRecordPtr apptRec, DatePtr dateP, Boolean searchForward);

static Boolean IsException (ApptDBRecordPtr apptRec, DateType date);

/***********************************************************************
 *
 * FUNCTION:    FindNextRepeat
 *
 * DESCRIPTION: This routine computes the date of the next 
 *              occurrence of a repeating appointment.
 *
 * PARAMETERS:  apptRec - a pointer to an appointment record
 *              date    - passed:   date to start from
 *                        returned: date of next occurrence             
 *              searchForward - search for the next occurrence before or after the 
 *						  specified date
 *
 * RETURNED:    true if an occurrence was found
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	6/14/95	Initial Revision
 *			gap	09/25/00	Add capability to search for the most recent previous occurrence
 *								(needed for attention manager support)
 *
 ***********************************************************************/
static Boolean FindNextRepeat (ApptDBRecordPtr apptRec, DatePtr dateP, Boolean searchForward)
{
	Int16  i;
	Int32  adjust;
	Int32  daysTilNext;
	Int32  monthsTilNext;
	UInt16 day;
	UInt16 freq;
	UInt16 year;
	UInt16 weeksDiff;
	UInt16 monthsDiff;
	UInt16 daysInMonth;
	UInt16 dayOfWeek;
	UInt16 apptWeekDay;
	UInt16 firstDayOfWeek;
	UInt32 dateInDays;
	UInt32 startInDays;
	DateType start;
	DateType date;
	DateType next;
	

	date = *dateP;
	
	if (searchForward)
		{
		// Is the date passed after the end date of the appointment?
		if (DateCompare (date, apptRec->repeat->repeatEndDate) > 0)
			return (false);
		
		// Is the date passed before the start date of the appointment?
		if (DateCompare (date, apptRec->when->date) < 0)
			date = apptRec->when->date;
		}
	else
		{
		// Is the date passed is before the start date of the appointment? 
		// return false now
		if (DateCompare (date, apptRec->when->date) < 0)
			return (false);

		// Is the date passed after the end date of the appointment?
		// search backwards from repeat end date for first valid occurrence.
		if (DateCompare (date, apptRec->repeat->repeatEndDate) > 0)
			date = apptRec->repeat->repeatEndDate;
		}

	// apptRec->repeat->repeatEndDate can be passed into this routine
	// or be set in the else case above.  Since apptNoEndDate is not a 
	// valid date (month is 15) set it must be set to the last date 
	// support by the current OS  12/31/31
	if ( DateToInt(date) == apptNoEndDate)
		date.month = 12;

	// Get the frequency on occurrecne (ex: every 2nd day, every 3rd month, etc).  
	freq = apptRec->repeat->repeatFrequency;
	
	// Get the date of the first occurrecne of the appointment.
	start = apptRec->when->date;	

	switch (apptRec->repeat->repeatType)
		{
		// Daily repeating appointment.
		case repeatDaily:
			dateInDays = DateToDays (date);
			startInDays = DateToDays (start);
			if (searchForward)
				daysTilNext = (dateInDays - startInDays + freq - 1) / freq * freq;
			else
				daysTilNext = (dateInDays - startInDays) / freq * freq;
			if (startInDays + daysTilNext > (UInt32) maxDays)
				return (false);
			DateDaysToDate (startInDays + daysTilNext, &next);
			break;
			


		// Weekly repeating appointment (ex: every Monday and Friday). 
		// Yes, weekly repeating appointment can occur more then once a
		// week.
		case repeatWeekly:
			dateInDays = DateToDays (date);
			startInDays = DateToDays (start);

			firstDayOfWeek = (DayOfWeek (1, 1, firstYear) - 
				apptRec->repeat->repeatStartOfWeek + daysInWeek) % daysInWeek;

			dayOfWeek = DayOfWeek (date.month, date.day, date.year+firstYear);
			apptWeekDay = (dayOfWeek - apptRec->repeat->repeatStartOfWeek +
				daysInWeek) % daysInWeek;

			// Are we in a week in which the appointment occurrs, if not 
			// move to that start of the next week in which the appointment
			// does occur.
			weeksDiff = (((dateInDays + firstDayOfWeek) / daysInWeek) - 
							 ((startInDays + firstDayOfWeek) / daysInWeek)) %freq;
			if (weeksDiff)
				{
				if (searchForward)
					{
					adjust = ((freq - weeksDiff) * daysInWeek) - apptWeekDay;
					apptWeekDay = 0;
					dayOfWeek = (dayOfWeek + adjust) % daysInWeek;
					}
				else
					{
					adjust = (weeksDiff * daysInWeek) + (daysInWeek-1 - apptWeekDay);
					apptWeekDay = 6;
					dayOfWeek = (dayOfWeek - adjust) % daysInWeek;
					}
				}
			else
				adjust = 0;
			
			// Find the next day on which the appointment repeats.
			if (searchForward)
				{
				for (i = 0; i < daysInWeek; i++)
					{
					if (apptRec->repeat->repeatOn & (1 << dayOfWeek)) break;
					adjust++;
					if (++dayOfWeek == daysInWeek)
						dayOfWeek = 0;
					if (++apptWeekDay == daysInWeek)
						adjust += (freq - 1) * daysInWeek;
					}

				if (dateInDays + adjust > (UInt32) maxDays)
					return (false);
				DateDaysToDate (dateInDays + adjust, &next);
				}
			else
				{
				for (i = 0; i < daysInWeek; i++)
					{
					if (apptRec->repeat->repeatOn & (1 << dayOfWeek)) break;
					adjust++;
					if (--dayOfWeek == 0)
						dayOfWeek = daysInWeek-1;
					if (--apptWeekDay == 0)
						adjust += (freq - 1) * daysInWeek;
					}		
				
				// determine if date goes past first day (unsigned int wraps around)
				if (dateInDays - adjust > dateInDays) 
					return (false);
					
				DateDaysToDate (dateInDays - adjust, &next);
				}

			break;



		// Monthly-by-day repeating appointment (ex: the 3rd Friday of every
		// month).
		case repeatMonthlyByDay:
			// Compute the number of month until the appointment repeats again.
			if (searchForward)
				monthsTilNext = ((((date.year - start.year) * monthsInYear) + (date.month - start.month)) + freq - 1) /freq * freq;
			else
				monthsTilNext = (((date.year - start.year) * monthsInYear) + (date.month - start.month)) /freq * freq;

			while (true)
				{
				year = start.year + (start.month - 1 + monthsTilNext) / monthsInYear;
				if (year >= numberOfYears)
					return (false);

				next.year = year;
				next.month = (start.month - 1 + monthsTilNext) % monthsInYear + 1;
	
				dayOfWeek = DayOfWeek (next.month, 1, next.year+firstYear);
				if ((apptRec->repeat->repeatOn % daysInWeek) >= dayOfWeek)
					day = apptRec->repeat->repeatOn - dayOfWeek + 1;
				else
					day = apptRec->repeat->repeatOn + daysInWeek - dayOfWeek + 1;
	
				// If repeat-on day is between the last sunday and the last
				// saturday, make sure we're not passed the end of the month.
				if ( (apptRec->repeat->repeatOn >= domLastSun) &&
					  (day > DaysInMonth (next.month, next.year+firstYear)))
					{
					day -= daysInWeek;
					}
				next.day = day;

				// Its posible that "next date" calculated above is 
				// before the date passed.  If so, move forward
				// by the length of the repeat freguency and perform
				// the calculation again.
				if (searchForward)
					{
					if ( DateToInt(date) > DateToInt (next))
						monthsTilNext += freq;
					else
						break;
					}
				else
					{
					if ( DateToInt(date) < DateToInt (next))
						monthsTilNext -= freq;
					else
						break;
					}
				}
			break;						



		// Monthly-by-date repeating appointment (ex: the 15th of every
		// month).
		case repeatMonthlyByDate:
			// Compute the number of month until the appointment repeats again.
			monthsDiff = ((date.year - start.year) * monthsInYear) + (date.month - start.month);
			if (searchForward)
				{
				monthsTilNext = (monthsDiff + freq - 1) / freq * freq;
				if ((date.day > start.day) && (!(monthsDiff % freq)))
					monthsTilNext += freq;
				}
			else
				{
				monthsTilNext = monthsDiff / freq * freq;
				if ((date.day < start.day) && (!(monthsDiff % freq)))
					monthsTilNext -= freq;
				}
				
			year = start.year + (start.month - 1 + monthsTilNext) / monthsInYear;
			if (year >= numberOfYears)
				return (false);

			next.year = year;
			next.month = (start.month - 1 + monthsTilNext) % monthsInYear + 1;
			next.day = start.day;

			// Make sure we're not passed the last day of the month.
			daysInMonth = DaysInMonth (next.month, next.year+firstYear);
			if (next.day > daysInMonth)
				next.day = daysInMonth;
			break;



		// Yearly repeating appointment.
		case repeatYearly:
			next.day = start.day;
			next.month = start.month;

			if (searchForward)
				{
				year = start.year + ((date.year - start.year + freq - 1) / freq * freq);
				if (	(date.month > start.month) ||
				  		((date.month == start.month) && (date.day > start.day)) )
					year += freq;
				}
			else
				{
				year = start.year + ((date.year - start.year) / freq * freq);
				if (	(date.month < start.month) ||
				  		((date.month == start.month) && (date.day < start.day)) ) 
					year -= freq;
				}
			

			// Specal leap day processing.
			if ( (next.month == february) && (next.day == 29) &&
				  (next.day > DaysInMonth (next.month, year+firstYear)))
				{
				next.day = DaysInMonth (next.month, year+firstYear);
				}				      
			if (year >= numberOfYears)
				return (false);

			next.year = year;	
			break;

		default:
			return (false);
		}
		
	if (searchForward)
		{
		// Is the next occurrence after the end date of the appointment?
		if (DateCompare (next, apptRec->repeat->repeatEndDate) > 0)
			return (false);

		ErrFatalDisplayIf ((DateToInt (next) < DateToInt (*dateP)),
			"Calculation error");
		}
	else
		{
		// Is the next occurrence before the start date of the appointment?
		if (DateCompare (next, apptRec->when->date) < 0)
			return (false);
		}

	*dateP = next;
	return (true);
}

/***********************************************************************
 *
 * FUNCTION:     SetDBBackupBit
 *
 * DESCRIPTION:  This routine sets the backup bit on the given database.
 *					  This is to aid syncs with non Palm software.
 *					  If no DB is given, open the app's default database and set
 *					  the backup bit on it.
 *
 * PARAMETERS:   dbP -	the database to set backup bit,
 *								can be NULL to indicate app's default database
 *
 * RETURNED:     nothing
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			grant	4/1/99	Initial Revision
 *
 ***********************************************************************/
static void SetDBBackupBit(DmOpenRef dbP)
{
	DmOpenRef localDBP;
	LocalID dbID;
	UInt16 cardNo;
	UInt16 attributes;

	// Open database if necessary. If it doesn't exist, simply exit (don't create it).
	if (dbP == NULL)
		{
		localDBP = DmOpenDatabaseByTypeCreator (datebookDBType, sysFileCDatebook, dmModeReadWrite);
		if (localDBP == NULL)  return;
		}
	else
		{
		localDBP = dbP;
		}
	
	// now set the backup bit on localDBP
	DmOpenDatabaseInfo(localDBP, &dbID, NULL, NULL, &cardNo, NULL);
	DmDatabaseInfo(cardNo, dbID, NULL, &attributes, NULL, NULL,
				NULL, NULL, NULL, NULL, NULL, NULL, NULL);
	attributes |= dmHdrAttrBackup;
	DmSetDatabaseInfo(cardNo, dbID, NULL, &attributes, NULL, NULL,
				NULL, NULL, NULL, NULL, NULL, NULL, NULL);
	
	// close database if necessary
   if (dbP == NULL) 
   	{
   	DmCloseDatabase(localDBP);
      }
}



/************************************************************
 *
 *  FUNCTION: ApptAppInfoInit
 *
 *  DESCRIPTION: Create and initialize the app info chunk if missing.
 *
 *  PARAMETERS: database pointer
 *
 *  RETURNS: 0 if successful, errorcode if not
 *
 *  CREATED: 1/25/95 
 *
 *  BY: Roger Flores
 *
 *************************************************************/
static Err	ApptAppInfoInit(DmOpenRef dbP)
{
	UInt16 				cardNo;
	MemHandle 		h;
	LocalID 			dbID;
	LocalID 			appInfoID;
	ApptAppInfoPtr	appInfoP;
	
	if (DmOpenDatabaseInfo(dbP, &dbID, NULL, NULL, &cardNo, NULL))
		return dmErrInvalidParam;
		
	if (DmDatabaseInfo(cardNo, dbID, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &appInfoID, NULL, NULL, NULL))
		return dmErrInvalidParam;
	
	if (appInfoID == 0) 
		{
		h = DmNewHandle(dbP, sizeof(ApptAppInfoType));
		if (! h) return dmErrMemError;

		appInfoID = MemHandleToLocalID (h);
		DmSetDatabaseInfo(cardNo, dbID, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &appInfoID, NULL, NULL, NULL);
		}
		
	// Get pointer to app Info chunk
	appInfoP = MemLocalIDToLockedPtr(appInfoID, cardNo);
	
	// Init it
	DmSet(appInfoP, 0, sizeof(ApptAppInfoType), 0); 

	// Unlock it
	MemPtrUnlock(appInfoP);
	
	return 0;
}



/************************************************************
 *
 *  FUNCTION: ApptUnpack
 *
 *  DESCRIPTION: Fills in the ApptDBRecord structure
 *
 *  PARAMETERS: database record
 *
 *  RETURNS: the record unpacked
 *
 *  CREATED: 1/25/95 
 *
 *  BY: Roger Flores
 *
 *************************************************************/
void ApptUnpack(ApptPackedDBRecordPtr src, ApptDBRecordPtr dest)
{
	ApptDBRecordFlags	flags;
	Char *p;

	
	flags = src->flags;
	p = &src->firstField;

	dest->when = (ApptDateTimeType *) src;
	
	if (flags.alarm) 
		{
		dest->alarm = (AlarmInfoType *) p;
		p += sizeof (AlarmInfoType);
		}
	else
		dest->alarm = NULL;

	
	if (flags.repeat)
		{
		dest->repeat = (RepeatInfoType *) p;
		p += sizeof (RepeatInfoType);
		}
	else
		dest->repeat = NULL;
	
	
	if (flags.exceptions)
		{
		dest->exceptions = (ExceptionsListType *) p;
		p += sizeof (UInt16) + 
			(((ExceptionsListType *) p)->numExceptions * sizeof (DateType));
		}
	else
		dest->exceptions = NULL;
		
	
	if (flags.description)
		{
		dest->description = p;
		p += StrLen(p) + 1;
		}
	else
		dest->description = NULL;
		
	
	if (flags.note)
		{
		dest->note = p;
		}
	else
		dest->note = NULL;
	
}



/***********************************************************************
 *
 * FUNCTION:     DateGetDatabase
 *
 * DESCRIPTION:  Get the application's database.  Open the database if it
 * exists, create it if neccessary.
 *
 * PARAMETERS:   *dbPP - pointer to a database ref (DmOpenRef) to be set
 *					  mode - how to open the database (dmModeReadWrite)
 *
 * RETURNED:     Err - zero if no error, else the error
 *
 * REVISION HISTORY:
 *			Name		Date		Description
 *			----		----		-----------
 *			roger		12/3/97	Initial Revision
 *
 ***********************************************************************/
Err DatebookGetDatabase (DmOpenRef *dbPP, UInt16 mode)
{
	Err error = 0;
	DmOpenRef dbP;
	UInt16 cardNo;
	LocalID dbID;
	
	
	*dbPP = 0;
	dbP = DmOpenDatabaseByTypeCreator(datebookDBType, sysFileCDatebook, mode);
	if (! dbP)
		{
		error = DmCreateDatabase (0, datebookDBName, sysFileCDatebook,
								datebookDBType, false);
		if (error) return error;
		
		dbP = DmOpenDatabaseByTypeCreator(datebookDBType, sysFileCDatebook, mode);
		if (! dbP) return ~0;

		// Set the backup bit.  This is to aid syncs with non Palm software.
		SetDBBackupBit(dbP);
		
		error = ApptAppInfoInit (dbP);
      if (error) 
      	{
			DmOpenDatabaseInfo(dbP, &dbID, NULL, NULL, &cardNo, NULL);
      	DmCloseDatabase(dbP);
      	DmDeleteDatabase(cardNo, dbID);
         return error;
         }
		}
	
	*dbPP = dbP;
	return 0;
}



/***********************************************************************
 *
 * FUNCTION:    DateCompare
 *
 * DESCRIPTION: This routine compares two dates.
 *
 * PARAMETERS:  d1 - a date 
 *              d2 - a date 
 *
 * RETURNED:    if d1 > d2  returns a positive int
 *              if d1 < d2  returns a negative int
 *              if d1 = d2  returns zero
 *
 * NOTE: This routine treats the DateType structure like an unsigned int,
 *       it depends on the fact the the members of the structure are ordered
 *       year, month, day form high bit to low low bit.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	6/12/95		Initial Revision
 *
 ***********************************************************************/
static Int16 DateCompare (DateType d1, DateType d2)
{
	UInt16 int1, int2;
	
	int1 = DateToInt(d1);
	int2 = DateToInt(d2);
	
	if (int1 > int2)
		return (1);
	else if (int1 < int2)
		return (-1);
	return 0;
}



/***********************************************************************
 *
 * FUNCTION:    TimeCompare
 *
 * DESCRIPTION: This routine compares two times.  "No time" is represented
 *              by minus one, and is considered less than all times.
 *
 * PARAMETERS:  nothing
 *
 * RETURNED:    if t1 > t2  returns a positive int
 *              if t1 < t2  returns a negative int
 *              if t1 = t2  returns zero
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	6/12/95		Initial Revision
 *
 ***********************************************************************/
static Int16 TimeCompare (TimeType t1, TimeType t2)
{
	Int16 int1, int2;
	
	int1 = TimeToInt(t1);
	int2 = TimeToInt(t2);
	
	if (int1 > int2)
		return (1);
	else if (int1 < int2)
		return (-1);
	return 0;

}



/************************************************************
 *
 *  FUNCTION: ApptPackedSize
 *
 *  DESCRIPTION: Return the packed size of an ApptDBRecordType 
 *
 *  PARAMETERS: database record
 *
 *  RETURNS: the size in bytes
 *
 *  CREATED: 1/25/95 
 *
 *  BY: Roger Flores
 *
 *************************************************************/
static UInt16 ApptPackedSize (ApptDBRecordPtr r)
{
	UInt16 size;

	
	size = sizeof (ApptDateTimeType) + sizeof (ApptDBRecordFlags);
	
	if (r->alarm != NULL)
		size += sizeof (AlarmInfoType);
	
	if (r->repeat != NULL)
		size += sizeof (RepeatInfoType);
	
	if (r->exceptions != NULL)
		size += sizeof (UInt16) + 
			(r->exceptions->numExceptions * sizeof (DateType));
	
	if (r->description != NULL)
		size += StrLen(r->description) + 1;
	
	if (r->note != NULL)
		size += StrLen(r->note) + 1;
	

	return size;
}



/************************************************************
 *
 *  FUNCTION: ApptPack
 *
 *  DESCRIPTION: Pack an ApptDBRecordType
 *
 *  PARAMETERS: database record
 *
 *  RETURNS: the ApptPackedDBRecord is packed
 *
 *  CREATED: 1/25/95 
 *
 *  BY: Roger Flores
 *
 *************************************************************/
static void ApptPack(ApptDBRecordPtr s, ApptPackedDBRecordPtr d)
{
	ApptDBRecordFlags	flags;
	UInt16 	size;
	UInt32	offset = 0;
	
	
	*(UInt8 *)&flags = 0;			// clear the flags
	
	
	// copy the ApptDateTimeType
	//c = (char *) d;
	offset = 0;
	DmWrite(d, offset, s->when, sizeof(ApptDateTimeType));
	offset += sizeof (ApptDateTimeType) + sizeof (ApptDBRecordFlags);
	

	if (s->alarm != NULL)
		{
		DmWrite(d, offset, s->alarm, sizeof(AlarmInfoType));
		offset += sizeof (AlarmInfoType);
		flags.alarm = 1;
		}
	

	if (s->repeat != NULL)
		{
		DmWrite(d, offset, s->repeat, sizeof(RepeatInfoType));
		offset += sizeof (RepeatInfoType);
		flags.repeat = 1;
		}
	

	if (s->exceptions != NULL)
		{
		size = sizeof (UInt16) + 
			(s->exceptions->numExceptions * sizeof (DateType));
		DmWrite(d, offset, s->exceptions, size);
		offset += size;
		flags.exceptions = 1;
		}
	
	
	if (s->description != NULL)
		{
		size = StrLen(s->description) + 1;
		DmWrite(d, offset, s->description, size);
		offset += size;
		flags.description = 1;
		}
	

	
	if (s->note != NULL)
		{
		size = StrLen(s->note) + 1;
		DmWrite(d, offset, s->note, size);
		offset += size;
		flags.note = 1;
		}
	
	DmWrite(d, sizeof(ApptDateTimeType), &flags, sizeof(flags));
}



/************************************************************
 *
 *  FUNCTION: ApptFindSortPosition
 *
 *  DESCRIPTION: Return where a record is or should be
 *		Useful to find or find where to insert a record.
 *
 *  PARAMETERS: database record
 *
 *  RETURNS: position where a record should be
 *
 *  CREATED: 1/25/95 
 *
 *  BY: Roger Flores
 *
 *************************************************************/
static UInt16 ApptFindSortPosition(DmOpenRef dbP, ApptPackedDBRecordPtr newRecord)
{
	return (DmFindSortPosition (dbP, newRecord, NULL, (DmComparF *)ApptComparePackedRecords, 0));
}



/************************************************************
 *
 *  FUNCTION:    ApptComparePackedRecords
 *
 *  DESCRIPTION: Compare two packed records.
 *
 *  PARAMETERS:  r1    - database record 1
 *				     r2    - database record 2
 *               extra - extra data, not used in the function
 *
 *  RETURNS:    -1 if record one is less
 *		           1 if record two is less
 *
 *  CREATED: 1/14/95 
 *
 *  BY: Roger Flores
 *
 *	COMMENTS:	Compare the two records key by key until
 *	there is a difference.  Return -1 if r1 is less or 1 if r2
 *	is less.  A zero is never returned because if two records
 *	seem identical then their unique IDs are compared!
 *
 *************************************************************/ 
static Int16 ApptComparePackedRecords (ApptPackedDBRecordPtr r1, 
	ApptPackedDBRecordPtr r2, Int16 extra, SortRecordInfoPtr info1, 
	SortRecordInfoPtr info2, MemHandle appInfoH)
{
	Int16 result;

	if ((r1->flags.repeat) || (r2->flags.repeat))
		{
		if ((r1->flags.repeat) && (r2->flags.repeat))
			result = 0;
		else if (r1->flags.repeat)
			result = -1;
		else
			result = 1;
		}

	else
		{
		result = DateCompare (r1->when.date, r2->when.date);
		if (result == 0)
			{
			result = TimeCompare (r1->when.startTime, r2->when.startTime);
			}
		}
	return result;
}



/***********************************************************************
 *
 * FUNCTION:    ApptFindFirst
 *
 * DESCRIPTION: This routine finds the first appointment on the specified
 *              day.
 *
 * PARAMETERS:  dbP    - pointer to the database
 *              date   - date to search for
 *              indexP - pointer to the index of the first record on the 
 *                       specified day (returned value)
 *
 * RETURNED:    true if a record has found
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	6/15/95		Initial Revision
 *
 ***********************************************************************/
Boolean ApptFindFirst (DmOpenRef dbP, DateType date, UInt16* indexP)
{
	Err err;
	Int16 numOfRecords;
	Int16 kmin, probe, i;		// all positions in the database.
	Int16 result = 0;			// result of comparing two records
	UInt16 index;
	MemHandle recordH;
	Boolean found = false;
	ApptPackedDBRecordPtr r;


	kmin = probe = 0;
	numOfRecords = DmNumRecords(dbP);
	
	
	while (numOfRecords > 0)
		{
		i = numOfRecords >> 1;
		probe = kmin + i;
		
		index = probe;
		recordH = DmQueryNextInCategory (dbP, &index, dmAllCategories);
		if (recordH)
			{
			r = (ApptPackedDBRecordPtr) MemHandleLock (recordH);
			if (r->flags.repeat)
				result = 1;
			else
				result = DateCompare (date, r->when.date);
			MemHandleUnlock (recordH);
			}

		// If no handle, assume the record is deleted, deleted records
		// are greater.
		else
			result = -1;
			

		// If the date passed is less than the probe's date, keep searching.
		if (result < 0)
			numOfRecords = i;

		// If the date passed is greater than the probe's date, keep searching.
		else if (result > 0)
			{
			kmin = probe + 1;
			numOfRecords = numOfRecords - i - 1;
			}

		// If the records are equal find the first record on the day.
		else
			{
			found = true;
			*indexP = index;
			while (true)
				{
				err = DmSeekRecordInCategory (dbP, &index, 1, dmSeekBackward, 
					dmAllCategories);
				if (err == dmErrSeekFailed) break;
				
				recordH = DmQueryRecord(dbP, index);
				r = (ApptPackedDBRecordPtr) MemHandleLock (recordH);
				if (r->flags.repeat)
					result = 1;
				else
					result = DateCompare (date, r->when.date);
				MemHandleUnlock (recordH);
				if (result != 0) break;
				*indexP = index;
				}

			break;
			}
		}

	
	// If that were no appointments on the specified day, return the 
	// index of the next appointment (on a future day).
	if (! found)
		{
		if (result < 0) 
			*indexP = probe;
		else if (DmNumRecords(dbP) == 0)
			*indexP = 0;
		else
			*indexP = probe + 1;
		}

	return (found);
}



/************************************************************
 *
 *  FUNCTION: ApptNewRecord
 *
 *  DESCRIPTION: Create a new packed record in sorted position
 *
 *  PARAMETERS: database pointer
 *				database record
 *
 *  RETURNS: ##0 if successful, error code if not
 *
 *  CREATED: 1/25/95 
 *
 *  BY: Roger Flores
 *
 *************************************************************/
Err ApptNewRecord(DmOpenRef dbP, ApptDBRecordPtr r, UInt16 *index)
{
	MemHandle recordH;
	ApptPackedDBRecordPtr recordP;
	UInt16 					newIndex;
	Err err;

	
	// Make a new chunk with the correct size.
	recordH = DmNewHandle (dbP, (UInt32) ApptPackedSize(r));
	if (recordH == NULL)
		return dmErrMemError;

	recordP = MemHandleLock (recordH);
	
	// Copy the data from the unpacked record to the packed one.
	ApptPack (r, recordP);

	newIndex = ApptFindSortPosition(dbP, recordP);

	MemPtrUnlock (recordP);

	// 4) attach in place
	err = DmAttachRecord(dbP, &newIndex, recordH, 0);
	if (err) 
		MemHandleFree(recordH);
	else
		*index = newIndex;
	
	return err;
}



/***********************************************************************
 *
 * FUNCTION:    ApptRepeatsOnDate
 *
 * DESCRIPTION: This routine returns true if a repeating appointment
 *              occurrs on the specified date.
 *
 * PARAMETERS:  apptRec - a pointer to an appointment record
 *              date    - date to check              
 *
 * RETURNED:    true if the appointment occurs on the date specified
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	6/14/95		Initial Revision
 *
 ***********************************************************************/
Boolean ApptRepeatsOnDate (ApptDBRecordPtr apptRec, DateType date)
{
	Int16  i;
	UInt16 freq;
	UInt16 weeksDiff;
	UInt16 dayInMonth;
	UInt16 dayOfWeek;
	UInt16 dayOfMonth;
	UInt16 firstDayOfWeek;
	Int32 dateInDays;
	Int32 startInDays;
	Boolean onDate;
	DatePtr exceptions;
	DateType startDate;

	// Is the date passed before the start date of the appointment?
	if (DateCompare (date, apptRec->when->date) < 0)
		return (false);

	// Is the date passed after the end date of the appointment?
	if (DateCompare (date, apptRec->repeat->repeatEndDate) > 0)
		return (false);
	

	// Get the frequency of occurrecne (ex: every 2nd day, every 3rd month, etc.).  
	freq = apptRec->repeat->repeatFrequency;
	
	// Get the date of the first occurrecne of the appointment.
	startDate = apptRec->when->date;

	switch (apptRec->repeat->repeatType)
		{
		// Daily repeating appointment.
		case repeatDaily:
			dateInDays = DateToDays (date);
			startInDays = DateToDays (startDate);
			onDate = ((dateInDays - startInDays) % freq) == 0;
			break;
			

		// Weekly repeating appointment (ex: every Monday and Friday). 
		// Yes, weekly repeating appointment can occur more then once a
		// week.
		case repeatWeekly:
			// Are we on a day of the week that the appointment repeats on.
			dayOfWeek = DayOfWeek (date.month, date.day, date.year+firstYear);
			onDate = ((1 << dayOfWeek) & apptRec->repeat->repeatOn);
			if (! onDate) break;

			// Are we in a week in which the appointment occurrs, if not 
			// move to that start of the next week in which the appointment
			// does occur.
			dateInDays = DateToDays (date);
			startInDays = DateToDays (startDate);

			firstDayOfWeek = (DayOfWeek (1, 1, firstYear) - 
				apptRec->repeat->repeatStartOfWeek + daysInWeek) % daysInWeek;

			weeksDiff = (((dateInDays + firstDayOfWeek) / daysInWeek) - 
							 ((startInDays + firstDayOfWeek) / daysInWeek)) %freq;
			onDate = (weeksDiff == 0);
			break;


//			// Compute the first occurrence of the appointment that occurs
//			// on the same day of the week as the date passed.
//			startDayOfWeek = DayOfWeek (startDate.month, startDate.day, 
//				startDate.year+firstYear);
//			startInDays = DateToDays (startDate);
//			if (startDayOfWeek < dayOfWeek)
//				startInDays += dayOfWeek - startDayOfWeek;
//			else if (startDayOfWeek > dayOfWeek)
//				startInDays += dayOfWeek+ (daysInWeek *freq) - startDayOfWeek;
//			
//			// Are we in a week in which the appointment repeats.
//			dateInDays = DateToDays (date);
//			onDate = (((dateInDays - startInDays) / daysInWeek) % freq) == 0;
//			break;


		// Monthly-by-day repeating appointment (ex: the 3rd Friday of every
		// month).
		case repeatMonthlyByDay:
			// Are we in a month in which the appointment repeats.
			onDate = ((((date.year - startDate.year) * monthsInYear) + 
						   (date.month - startDate.month)) % freq) == 0;
			if (! onDate) break;

			// Do the days of the month match (ex: 3rd Friday)
			dayOfMonth = DayOfMonth (date.month, date.day, date.year+firstYear);
			onDate = (dayOfMonth == apptRec->repeat->repeatOn);
			if (onDate) break;
			
			// If the appointment repeats on one of the last days of the month,
			// check if the date passed is also one of the last days of the 
			// month.  By last days of the month we mean: last sunday, 
			// last monday, etc.
			if ((apptRec->repeat->repeatOn >= domLastSun) &&
				 (dayOfMonth >= dom4thSun))
				{
				dayOfWeek = DayOfWeek (date.month, date.day, date.year+firstYear);
				dayInMonth = DaysInMonth (date.month, date.year+firstYear);
				onDate = (((date.day + daysInWeek) > dayInMonth) &&
							 (dayOfWeek == (apptRec->repeat->repeatOn % daysInWeek)));
				}
			break;						


		// Monthly-by-date repeating appointment (ex: the 15th of every
		// month).
		case repeatMonthlyByDate:
			// Are we in a month in which the appointment repeats.
			onDate = ((((date.year - startDate.year) * monthsInYear) + 
						   (date.month - startDate.month)) % freq) == 0;
			if (! onDate) break;
			
			// Are we on the same day of the month as the start date.
			onDate = (date.day == startDate.day);
			if (onDate) break;

			// If the staring day of the appointment is greater then the 
			// number of day in the month passed, and the day passed is the 
			// last day of the month, then the appointment repeats on the day.
			dayInMonth = DaysInMonth (date.month, date.year+firstYear);
			onDate = ((startDate.day > dayInMonth) && (date.day == dayInMonth));
			break;


		// Yearly repeating appointment.
		case repeatYearly:
			// Are we in a year in which the appointment repeats.
			onDate = ((date.year - startDate.year) % freq) == 0;
			if (! onDate) break;
			
			// Are we on the month and day that the appointment repeats.
			onDate = (date.month == startDate.month) &&
				      (date.day == startDate.day);
			if (onDate) break;
			
			// Specal leap day processing.
			if ( (startDate.month == february) && 
				  (startDate.day == 29) &&
				  (date.month == february) && 
				  (date.day == DaysInMonth (date.month, date.year+firstYear)))
				{
				onDate = true;
				}				      
			break;

		default:
			return (false);
		}

	// Check for an exception.
	if ((onDate) && (apptRec->exceptions))
		{
		exceptions = &apptRec->exceptions->exception;
		for (i = 0; i < apptRec->exceptions->numExceptions; i++)
			{
			if (DateCompare (date, exceptions[i]) == 0)
				{
				onDate = false;
				break;
				}
			}
		}


	return (onDate);
}

/***********************************************************************
 *
 * FUNCTION:    IsException
 *
 * DESCRIPTION: This routine returns true the date passed is in a 
 *              repeating appointment's exception list.
 *
 * PARAMETERS:  apptRec - a pointer to an appointment record
 *              date    - date to check              
 *
 * RETURNED:    true if the date is an exception date.
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	6/14/95		Initial Revision
 *
 ***********************************************************************/
static Boolean IsException (ApptDBRecordPtr apptRec, DateType date)
{
	int i;
	DatePtr exceptions;

	if (apptRec->exceptions)
		{
		exceptions = &apptRec->exceptions->exception;
		for (i = 0; i < apptRec->exceptions->numExceptions; i++)
			{
			if (DateCompare (date, exceptions[i]) == 0)
			return (true);
			}
		}
	return (false);
}

/***********************************************************************
 *
 * FUNCTION:    	ApptNextRepeat
 *
 * DESCRIPTION:	This routine computes the next occurrence of a 
 *              	repeating appointment.
 *
 * PARAMETERS:		apptRec - a pointer to an appointment record
 *              	dateP   - passed:   date to start from
 *                       	 returned: date of next occurrence   
 *						searchForward	- true if searching for next occurrence
 *										  	- false if searching for most recent       
 *
 * RETURNED:		true if there is an occurrence of the appointment
 *						between the date passed and the appointment's end date
 *						(if searching forward) or start date (if searching
 *						backwards)
 *
 * REVISION HISTORY:
 *			Name	Date		Description
 *			----	----		-----------
 *			art	6/20/95	Initial Revision
 *			gap	9/25/00	Add capability to search backwards for the most
 *								recent occurrence of the event (needed for attention
 *								manager support)
 *
 ***********************************************************************/
Boolean ApptNextRepeat (ApptDBRecordPtr apptRec, DatePtr dateP, Boolean searchForward)
{
	DateType date;
	
	date = *dateP;
	
	while (true)
		{
		// Compute the next time the appointment repeats.
		if (! FindNextRepeat (apptRec, &date, searchForward))
			return (false);

		// Check if the date computed is in the exceptions list.
		if (! IsException (apptRec, date))
			{
			*dateP = date;
			return (true);
			}
			
		DateAdjust (&date, (searchForward) ? 1 : -1);

		}		
}
