/*
 * ShotsSaveRecord
 * ---------------
 *
 * Save the Shots values
 *
 * Parameters: Nothing
 *
 * Returns: Nothing
 */
static void ShotsSaveRecord(void)
{ 
  MemHandle     h;
  WordsDBType   *recordP;
  UInt16	rec, attr;

  /* Get the value from the checkboxes */

  for (rec = 0; rec < 20; rec++)
  {
    if (CtlGetValue(Item(rec + 2200)))
      ShotsTable[rec] = '1';
    else
      ShotsTable[rec] = '0';
  }

  ShotsTable[29]='\0';

  h = DmGetRecord(WordsDBRef, RecordNum);
  recordP = MemHandleLock(h);

  DmStrCopy(recordP, OffsetOf(WordsDBType, word), ShotsTable);
  MemHandleUnlock(h);
  DmRecordInfo(WordsDBRef, RecordNum, &attr, NULL, NULL);
  attr |= dmRecAttrDirty;
  DmSetRecordInfo(WordsDBRef, RecordNum, &attr, NULL);

  DmReleaseRecord(WordsDBRef, RecordNum, true);

  return;
}

/*
 * ShotsFormInit
 * -------------
 *
 * Initialize the Shots Form
 *
 * Parameters: FormType
 *
 * Returns: Nothing
 */
static void ShotsFormInit(FormType *frmP)
{
  MemHandle     h;
  WordsDBType   *recordP;
  UInt16        attr, recNum, rec;
  DateTimeType  dateTime;
  Boolean	Create = true;

  ReadBabyRecord();

  recNum = DmNumRecords(WordsDBRef);

  for (rec = 0; (rec < recNum  && Create); rec++)
  {
    h = DmQueryRecord(WordsDBRef, rec);
    if (h)
    {
      recordP = (WordsDBType *) MemHandleLock(h);
      if (recordP->number == uniqueID && recordP->code == 'S')
      {
        RecordNum = rec;
        Create = false;
        StrCopy(ShotsTable, &recordP->word);
      }
      MemHandleUnlock(h);
    }
  }
  
  if (Create)
  {
    for (rec = 0; rec < 30; rec++)
    {
      ShotsTable[rec] = '0';
    }
    ShotsTable[29]='\0';
   
    recNum = 0; 
    h = DmNewRecord(WordsDBRef, &recNum, sizeof(WordsDBType) + 30);
    recordP = MemHandleLock(h);

    TimSecondsToDateTime(TimGetSeconds(), &dateTime);
    WWRecord.date.year = dateTime.year - firstYear;
    WWRecord.date.month = dateTime.month;
    WWRecord.date.day = dateTime.day;
    WWRecord.number = uniqueID;
    WWRecord.code = 'S';
    
    DmWrite(recordP, 0, &WWRecord.number, sizeof(WWRecord.number));
    DmWrite(recordP, OffsetOf(WordsDBType, date), &WWRecord.date, sizeof(DateType));
    DmWrite(recordP, OffsetOf(WordsDBType, code), &WWRecord.code, sizeof(Char));
    DmStrCopy(recordP, OffsetOf(WordsDBType, word), ShotsTable);
    MemHandleUnlock(h);
    DmRecordInfo(WordsDBRef, recNum, &attr, NULL, NULL);
    attr |= dmRecAttrDirty;
    DmSetRecordInfo(WordsDBRef, recNum, &attr, NULL);

    DmReleaseRecord(WordsDBRef, recNum, true);
    RecordNum = recNum;
  }

  /* Initialize the checkboxes */

  for (rec = 0; rec < 20; rec++)
  {
    if (ShotsTable[rec] == '0')
      CtlSetValue(Item(rec + 2200), false);
    else
      CtlSetValue(Item(rec + 2200), true);
  }

  return;
}

/*
 * ShotsFormHandleEvent
 * --------------------
 *  
 * Handle the shots information form
 *  
 * Parameters: Event
 *
 * Returns: Boolean
 */
static Boolean ShotsFormHandleEvent(EventPtr event)
{
  Boolean handled = false;
  FormType *frmP;

  switch (event->eType)
  {

  case frmOpenEvent:
    frmP = FrmGetActiveForm();
    CollapseSetState(frmP, collapseStateUser);
    FrmDrawForm(frmP);
    ShotsFormInit(frmP);
    handled = true;
    break;
 
  case ctlSelectEvent:
    if (event->data.ctlEnter.controlID == buttonID_shotscancel)
    {
        EventType e;

      FrmReturnToForm(0);
        MemSet( &e, sizeof( EventType ), 0 );
        e.eType = winDisplayChangedEvent;
        EvtAddUniqueEventToQueue( &e, 0, true );
      handled = true;
    }
    if (event->data.ctlEnter.controlID == buttonID_shotsok)
    {

        EventType e;
      ShotsSaveRecord();
      FrmReturnToForm(0);
        MemSet( &e, sizeof( EventType ), 0 );
        e.eType = winDisplayChangedEvent;
        EvtAddUniqueEventToQueue( &e, 0, true );
      handled = true;
    }
     break;
  }
  return(handled);
}
