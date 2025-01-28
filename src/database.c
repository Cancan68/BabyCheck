#ifndef database_H
#define database_H

#include <PalmOS.h>
#include "babycheck.h"

/*
 * ReadBabyRecord
 * --------------
 * 
 * Read a Baby record and fill the WRecord temp area with it
 * 
 * Parameters: Nothing
 * 
 * Returns: Nothing
 */
void ReadBabyRecord(void)
{ 
  UInt16 index;
  MemHandle recordH;
  BabyDBType *recordP;
  
  index = LstGetSelection(listbaby);
  
  recordH = DmQueryRecord(BabyDBRef, index);
  if (recordH)
  {
    recordP = (BabyDBType *) MemHandleLock(recordH);

    StrCopy(WRecord.name, recordP->name);
    WRecord.time = recordP->time;
    WRecord.sexe = recordP->sexe;
    DmRecordInfo(BabyDBRef, index, NULL, &uniqueID, NULL);
    MemHandleUnlock(recordH);
  }
}

/*
 * ReadBabyMeasur
 * --------------
 *
 * Read a Baby measurement record
 *
 * Parameters: Nothing
 *
 * Returns: Nothing
 */
void ReadBabyMeasur(void)
{
  MemHandle	recordH;
  MeasurDBType	*recordP;
  UInt16 	i;
  Boolean	trouve = false;

  RecordNum = 0;

  for (i = 0; i < DmNumRecords(MeasurDBRef); i++)
  {
    recordH = DmQueryRecord(MeasurDBRef, i);
    if (recordH)
    {
      recordP = (MeasurDBType *) MemHandleLock(recordH);
      if (recordP->number == uniqueID)
      {
        if (trouve)
        {
          if (recordP->date.year <= WMRecord.date.year)
          {
            if (recordP->date.year < WMRecord.date.year)
            {
              RecordNum = i;
            }
            else
            {
              if (recordP->date.month <= WMRecord.date.month)
              {
                if (recordP->date.month < WMRecord.date.month)
                {
                  RecordNum = i;
                }
                else
                {
                  if (recordP->date.day < WMRecord.date.day)
                  {
                    RecordNum = i;
                  }
                }
              }
            }
          }
        }
        else
          RecordNum = i;
                                    
        if (!trouve || (RecordNum == i && trouve))
        {
          WMRecord.date = recordP->date;
          WMRecord.weight = recordP->weight;
          WMRecord.height = recordP->height;
          WMRecord.number = uniqueID;
          trouve = true;
        }
      }
      MemHandleUnlock(recordH);
    }
  }

  return;
}

#endif
