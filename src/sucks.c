/*
 * SuckExportMemo
 * ----------------
 *
 * Export the Suckements in the Memo Database
 *
 * Parameters: Nothing
 *
 * Returns: Nothing
 */
void SuckExportMemo(void)
{
  DmOpenRef	MemoDB;
  UInt32	size = 0, linelen;
  MemHandle	h, recordH;
  void		*memorec;
  Char		outline[120];
  Char		sbuffer[10];
  Char		*s;
  DateType	date;
  Char		dateBuffer[dateStringLength];
  Char		*dateStr;
  SuckDBType  *recordP;
  UInt16	index;
  UInt16        rec, recNum;

  MemoDB = DmOpenDatabaseByTypeCreator('DATA', sysFileCMemo, dmModeWrite);
  if (!MemoDB)
    return;

  index = DmNumRecords(MemoDB);
  recordH = DmNewRecord(MemoDB, &index, 4096);
  memorec = MemHandleLock(recordH);

  // Write a header line
  StrCopy(outline, memo1);
  StrCat(outline, WRecord.name);
  StrCat(outline, memo2);
  DateSecondsToDate(TimGetSeconds(), &date);
  DateToAscii(date.month, date.day, date.year + firstYear, dateFormat, dateBuffer);
  dateStr = dateBuffer;
  StrCat(outline, dateStr); 
  StrCat(outline, "\n");
  linelen = StrLen(outline);
  DmWrite(memorec, size, outline, linelen);
  size += linelen;

  StrCopy(outline, "Date;Heure;Sein;Durée;\n");
  linelen = StrLen(outline);
  DmWrite(memorec, size, outline, linelen + 1);
  size += linelen;

  recNum = DmNumRecords(SuckDBRef);

  for (rec = 0; rec < recNum; rec++)
  {
    h = DmQueryRecord(SuckDBRef, rec);
    if (h)
    {
      recordP = (SuckDBType *) MemHandleLock(h);
      if (recordP->number == uniqueID)
      {
          DateToAscii(recordP->date.month, recordP->date.day, recordP->date.year + firstYear, dateFormat, dateBuffer);
          dateStr = dateBuffer;
          StrCopy(outline, dateStr);
          StrCat(outline,";");

          TimeToAscii(recordP->time.hours, recordP->time.minutes, timeFormat, dateBuffer);
          dateStr = dateBuffer;
          StrCat(outline, dateStr);
          StrCat(outline,";");

          if (recordP->breast)
            StrCat(outline, "Gauche");
          else
            StrCat(outline, "Droit");
          StrCat(outline,";");

          s = sbuffer;
          StrPrintF(s, "%d%", recordP->duration);

          StrCat(outline, s);
          StrCat(outline,";\n");

          linelen = StrLen(outline);
          DmWrite(memorec, size, outline, linelen + 1);
          size += linelen; 
      }
      MemHandleUnlock(h);
    }
  }

  MemHandleUnlock(recordH);
  DmResizeRecord(MemoDB, index, size + 1);
  DmReleaseRecord(MemoDB, index, true);

  DmCloseDatabase(MemoDB);

  DisplayError(stringID_transmemo, formID_info);
}

/*
 * SuckCompare
 * ------------
 *
 * Compare two records
 *
 * Parameters: Table, Row, Column, Rectangle
 *
 * Returns: Nothing
 */
Int16 SuckCompare(SuckDBType *r1, SuckDBType *r2, Int16 sortOrder, SortRecordInfoPtr info1, SortRecordInfoPtr info2, MemHandle appInfoH)
{
  Int16 result;

  result = r1->date.year - r2->date.year;
  if (result != 0)
  {
    if (DateToInt(r1->date) == 0xffff)
      return 1;
    if (DateToInt(r2->date) == 0xffff)
      return -1;
    return result;
  }

  result = r1->date.month - r2->date.month;
  if (result != 0)
    return result;

  result = r1->date.day - r2->date.day;
  if (result != 0)
    return result;

  result = r1->time.hours - r2->time.hours;
  if (result != 0)
    return result;

  result = r1->time.minutes - r2->time.minutes;

  return result;
}

/*
 * SuckDetailFormInit
 * --------------------
 * 
 * Initialize the Suckement detail form
 *
 * Parameters: Nothing
 * 
 * Returns: Nothing
 */
void SuckDetailFormInit(void)
{ 
  ControlType *ctlP, *ctl2P;
  Char *labelP, *label2P;
  FormType *frmP = FrmGetActiveForm();

  ctlP = FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, triggerID_birdate));
  labelP = (Char *)CtlGetLabel(ctlP);
  DateToDOWDMFormat(WTRecord.date.month, WTRecord.date.day, WTRecord.date.year + firstYear, dateFormat, labelP);
  CtlSetLabel(ctlP, labelP);

  ctl2P = FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, triggerID_birtime));
  label2P = (Char *)CtlGetLabel(ctl2P);
  TimeToAscii(WTRecord.time.hours, WTRecord.time.minutes, timeFormat, label2P);
  CtlSetLabel(ctl2P, label2P);

  if (WTRecord.breast)
    CtlSetLabel(Item(listID_breastt), "Gauche");
  else
    CtlSetLabel(Item(listID_breastt), "Droit");

  SetFieldTextFromInt(fieldID_durationm, WTRecord.duration, 1);

  FrmSetFocus(frmP, FrmGetObjectIndex(frmP, fieldID_durationm));
}

/*
 * SuckementsDetailFormHandleEvent
 * ---------------------------------
 *
 * Handle the Suckement detail form
 *
 * Parameters: Event
 *
 * Returns: Boolean
 */
Boolean SuckDetailFormHandleEvent(EventPtr event)
{
  Boolean handled = false;
  FormType *frmP;
  UInt16 zHeight;
  frmP = FrmGetActiveForm();

  switch (event->eType)
  {

  case frmOpenEvent:
    CollapseSetState(frmP, collapseStateUser);
    FrmDrawForm(frmP);
    SuckDetailFormInit();
    handled = true;
    break;

  case keyDownEvent:
    {
      UInt16  fldno;
      FieldType *fldP;
      fldno = FrmGetFocus(frmP);

      switch (FrmGetObjectId(frmP, fldno))
      {
        case fieldID_durationm:
        fldP = Item(fieldID_durationm);
        handled = FldHandleEvent(fldP, event);
        break;
      }

      zHeight = GetIntFromFieldTxt(frmP, fieldID_durationm, 1);
    }
    break;

  case ctlSelectEvent:
    if (event->data.ctlEnter.controlID == triggerID_birdate)
    {
        OutilsSelectDate(&WTRecord.date, event->data.ctlSelect.pControl, stringID_mesdate);
        handled = true;
    }
    if (event->data.ctlEnter.controlID == triggerID_birtime)
    {
        OutilsSelectTime(&WTRecord.time, event->data.ctlSelect.pControl, stringID_birtime);
        handled = true;
    }
    if (event->data.ctlEnter.controlID == listID_breastt)
    {
      ListPtr breastw = FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, listID_breast));
      short choice;

      if (WTRecord.breast)
        LstSetSelection(breastw, 1);
      else
        LstSetSelection(breastw, 0);
      choice = LstPopupList(breastw);
      if (choice == 0)
      {
        WTRecord.breast = true;
        CtlSetLabel(Item(listID_breastt), "Gauche");
      }
      else
      {
        if (choice == 1)
        {
          WTRecord.breast = false;
          CtlSetLabel(Item(listID_breastt), "Droit");
        }
      }
      handled = true;
    }
    if (event->data.ctlEnter.controlID == buttonID_sucdok)
    {
      SuckDBType      *p;
      MemHandle         dataH;
      EventType e;
  
      dataH = DmGetRecord(SuckDBRef, RecordNum);
      p = (SuckDBType *) MemHandleLock(dataH);

      WTRecord.duration = GetIntFromFieldTxt(frmP, fieldID_durationm, 1);

      DmWrite(p, 0, &WTRecord, sizeof(SuckDBType));
      DmReleaseRecord(SuckDBRef, RecordNum, true);
      MemHandleUnlock(dataH);
  
      FrmReturnToForm(0);
      MemSet( &e, sizeof( EventType ), 0 );
      e.eType = winDisplayChangedEvent;
      EvtAddUniqueEventToQueue( &e, 0, true );
      DmQuickSort(SuckDBRef, (DmComparF *) &SuckCompare, 0);
      SuckLoadTable();
      handled = true;
    }
    if (event->data.ctlEnter.controlID == buttonID_sucddel)
    {
      EventType e;

      DmRemoveRecord(SuckDBRef, RecordNum);

      if (TopRecord != 0)
        TopRecord--;

      FrmReturnToForm(0);
      MemSet( &e, sizeof( EventType ), 0 );
      e.eType = winDisplayChangedEvent;
      EvtAddUniqueEventToQueue( &e, 0, true );
      SuckLoadTable();
      handled = true;
    }
    if (event->data.ctlEnter.controlID == buttonID_sucdcan)
    {
      EventType e;

      if (newBaby)
      {
        DmRemoveRecord(SuckDBRef, RecordNum);

        if (TopRecord != 0)
          TopRecord--;
      }

      FrmReturnToForm(0);
      MemSet( &e, sizeof( EventType ), 0 );
      e.eType = winDisplayChangedEvent;
      EvtAddUniqueEventToQueue( &e, 0, true );
      SuckLoadTable();
      handled = true;
    }
    break;

  }
  return(handled);
}

/*
 * SuckDrawDate
 * -------------
 *
 * Draw the date in a row
 *
 * Parameters: Table, Row, Column, Rectangle
 *
 * Returns: Nothing
 */
void SuckDrawDate(void *table, Int16 row, Int16 column, RectanglePtr bounds)
{
  DateType date;
  Char dateBuffer[dateStringLength];
  Char* dateStr;
  Int16 drawX, drawY;
  UInt16 dateStrLen;
  FontID curFont;
  FontID oldFont = stdFont;

  // Get the due date to the item being drawn. 
  *((Int16 *) (&date)) = TblGetItemInt(table, row, 0);

  curFont = FntSetFont(oldFont);
  FntSetFont(stdFont);
  DateToAscii(date.month, date.day, date.year + firstYear, dateFormat, dateBuffer);
  dateStr = dateBuffer;

  dateStrLen = StrLen(dateStr);
  drawX = bounds->topLeft.x + bounds->extent.x - FntCharsWidth(dateStr, dateStrLen);
  drawY = bounds->topLeft.y;
  WinDrawChars(dateStr, dateStrLen, drawX, drawY);
  FntSetFont(curFont);
}

/*
 * SuckDrawTime
 * -------------
 *
 * Draw the time in a row
 *
 * Parameters: Table, Row, Column, Rectangle
 *
 * Returns: Nothing
 */
void SuckDrawTime(void *table, Int16 row, Int16 column, RectanglePtr bounds)
{
  TimeType time;
  Char dateBuffer[dateStringLength];
  Char* dateStr;
  Int16 drawX, drawY;
  UInt16 dateStrLen;
  FontID curFont;
  FontID oldFont = stdFont;

  // Get the due date to the item being drawn. 
  *((Int16 *) (&time)) = TblGetItemInt(table, row, 1);

  curFont = FntSetFont(oldFont);
  FntSetFont(stdFont);
  TimeToAscii(time.hours, time.minutes, timeFormat, dateBuffer);
  dateStr = dateBuffer;

  dateStrLen = StrLen(dateStr);
  drawX = bounds->topLeft.x + bounds->extent.x - FntCharsWidth(dateStr, dateStrLen);
  drawY = bounds->topLeft.y;
  WinDrawChars(dateStr, dateStrLen, drawX, drawY);
  FntSetFont(curFont);
}

/*
 * SuckDrawBreast
 * --------------
 *
 * Draw the weight or height in a row
 *
 * Parameters: Table, Row, Column, Rectangle
 *
 * Returns: Nothing
 */
void SuckDrawBreast(void *table, Int16 row, Int16 column, RectanglePtr bounds)
{
  UInt16   weight;
  Char sbuffer[10];
  Char *s;
  UInt16 weightStrLen;
  FontID oldFont = stdFont;
  FontID curFont;
  Int16 drawX, drawY;
  
  weight = (UInt16) TblGetItemInt(table, row, column);
  s = sbuffer;
  if (weight == 0)
    StrCopy(s, "Gauche");
  else
    StrCopy(s, "Droit");

  curFont = FntSetFont(oldFont);
  FntSetFont(stdFont);
  weightStrLen = StrLen(s);
  drawX = bounds->topLeft.x;
  drawY = bounds->topLeft.y;
  WinDrawChars(s, weightStrLen, drawX, drawY);
  FntSetFont(curFont);
}

/*
 * SuckDrawSuck
 * ----------------
 *
 * Draw the weight or height in a row
 *
 * Parameters: Table, Row, Column, Rectangle
 *
 * Returns: Nothing
 */
void SuckDrawSuck(void *table, Int16 row, Int16 column, RectanglePtr bounds)
{
  UInt16   weight;
  Char sbuffer[10];
  Char *s;
  UInt16 weightStrLen;
  FontID oldFont = stdFont;
  FontID curFont;
  Int16 drawX, drawY;
  
  weight = (UInt16) TblGetItemInt(table, row, column);
  s = sbuffer;

  StrPrintF(s, "%d", weight);

  curFont = FntSetFont(oldFont);
  FntSetFont(stdFont);
  weightStrLen = StrLen(s);
  drawX = bounds->topLeft.x + bounds->extent.x - FntCharsWidth(s, weightStrLen);
  drawY = bounds->topLeft.y;
  WinDrawChars(s, weightStrLen, drawX, drawY);
  FntSetFont(curFont);
}

/*
 * SuckLoadTable
 * --------------
 *
 * Load the Suckement Table
 *
 * Parameters: Nothing
 *
 * Returns: Nothing
 */
void SuckLoadTable(void)
{
  MemHandle	h;
  SuckDBType	*recordP;
  TablePtr	table;
  Int16		row, rec, breast;
  UInt16	recNum;
  FormPtr	form;

  form = FrmGetActiveForm();
  table = (TablePtr) FrmGetObjectPtr(form, FrmGetObjectIndex(form, tableID_suck));

  row = 0;
  NBRecs = 0;
  recNum = DmNumRecords(SuckDBRef);

  for (rec = 0; rec < recNum; rec++) 
  {
    h = DmQueryRecord(SuckDBRef, (UInt16) rec);
    if (h)
    {
      recordP = (SuckDBType *) MemHandleLock(h);
      if (recordP->number == uniqueID)
      {
        if (NBRecs >= TopRecord && row < NBRows)
        {
          TblSetRowUsable(table, row, true);

          TblSetItemInt(table, row, 0, (*(Int16 *) &recordP->date));
          TblSetItemInt(table, row, 1, (*(Int16 *) &recordP->time));
          if (recordP->breast)
            breast = 0;
          else
            breast = 1;
          TblSetItemInt(table, row, 2, (Int16) breast);
          TblSetItemInt(table, row, 3, (Int16) recordP->duration);

          TblSetRowID(table, row, (UInt16) rec);

          row++;
        }
        dateLast = recordP->date;
        NBRecs++;
      }
      MemHandleUnlock(h);
    }
  }

  // Hide the rows without data
  while (row < TNBRows)
  {
    TblSetRowUsable(table, row, false);
    row++;
  }

  UpdateScrollbar(scrollID_suck);
  TblEraseTable(table);
  TblMarkTableInvalid(table);
  TblDrawTable(table);
}

/*
 * SuckCreate
 * -----------
 *
 * Create a Suckement
 *
 * Parameters: Nothing
 *
 * Returns: Nothing
 */
void SuckCreate(void)
{
  TablePtr	table;
  UInt16	recNum;
  FormPtr	form;
  MemHandle	h;
  SuckDBType	*recordP;
  DateTimeType	dateTime;

  form = FrmGetActiveForm();
  table = (TablePtr) FrmGetObjectPtr(form, FrmGetObjectIndex(form, tableID_suck));

  TblReleaseFocus(table);

  recNum = 0;
  h = DmNewRecord(SuckDBRef, &recNum, sizeof(SuckDBType));
  recordP = (SuckDBType *) MemHandleLock(h);

  TimSecondsToDateTime(TimGetSeconds(), &dateTime);
  WTRecord.date.year = dateTime.year - firstYear;
  WTRecord.date.month = dateTime.month;
  WTRecord.date.day = dateTime.day;
  WTRecord.time.minutes = dateTime.minute;
  WTRecord.time.hours = dateTime.hour;
  WTRecord.number = uniqueID;
  WTRecord.duration = 0;
  
  DmWrite(recordP, 0, &WTRecord, sizeof(SuckDBType));
  MemHandleUnlock(h);
  DmReleaseRecord(SuckDBRef, recNum, true);
  
  NBRecs++;

  if (NBRecs > NBRows)
    TopRecord = NBRecs - NBRows;

  RecordNum = recNum;
  newBaby = true;
  FrmPopupForm(formID_sucdet);
}

/*
 * SuckFormInit
 * -------------
 *
 * Initialize the Suck Form
 *
 * Parameters: FormType
 *
 * Returns: Nothing
 */
void SuckFormInit(FormType *frmP)
{
  TablePtr  table;
  Int16     row, col;
  RectangleType	r;
  FontID curFont;
  FontID oldFont = stdFont;

  ReadBabyRecord();

  TopRecord = 0;

  table = (TablePtr) FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, tableID_suck));
  TblGetBounds(table, &r);

  TNBRows = TblGetNumberOfRows(table);

  for (row = 0; row < TNBRows; row++)
  {
    TblSetItemStyle(table, row, 0, customTableItem);
    TblSetItemStyle(table, row, 1, customTableItem);
    TblSetItemStyle(table, row, 2, customTableItem);
    TblSetItemStyle(table, row, 3, customTableItem);
  }
  
  curFont = FntSetFont(oldFont);
  FntSetFont(stdFont);
  NBRows = (Int16) (r.extent.y /FntLineHeight());

  for (col = 0; col < 4; col++)
    TblSetColumnUsable(table, col, true);

  TblSetColumnSpacing(table, 1, 3);
  TblSetColumnSpacing(table, 2, 3);
  TblSetColumnSpacing(table, 3, 3);

  TblSetCustomDrawProcedure(table, 0, SuckDrawDate);
  TblSetCustomDrawProcedure(table, 1, SuckDrawTime);
  TblSetCustomDrawProcedure(table, 2, SuckDrawBreast);
  TblSetCustomDrawProcedure(table, 3, SuckDrawSuck);

  SuckLoadTable();

  return;
}

/*
 * SuckFormHandleEvent
 * --------------------
 *  
 * Handle the first Sucks information form
 *  
 * Parameters: Event
 *
 * Returns: Boolean
 */
Boolean SuckFormHandleEvent(EventPtr event)
{
  Boolean handled = false;
  FormType *frmP;

  switch (event->eType)
  {

  case frmOpenEvent:
  {
    Int16 offx, offy;
    TablePtr	table;
    RectangleType	r;
    FontID curFont;
    FontID oldFont = stdFont;

    frmP = FrmGetActiveForm();

    CollapseSetState(frmP, collapseStateUser);  

  if (romVersion >= 0x04000000)
  {
    if (CollapseResizeForm(frmP, false, &offx, &offy))
    {
      if (offy > 0)
        offy += 2;
      if (offy < 0)
        offy -= 2;
      CollapseMoveResizeFormObject(frmP, tableID_suck, 0, 0, 0, offy);
      CollapseMoveResizeFormObject(frmP, buttonID_sucback, 0, offy, 0, 0);
      CollapseMoveResizeFormObject(frmP, buttonID_sucnew, 0, offy, 0, 0);
      CollapseMoveResizeFormObject(frmP, buttonID_sucmem, 0, offy, 0, 0);
      CollapseMoveResizeFormObject(frmP, scrollID_suck, 0, 0, 0, offy);
    }
}
    table = (TablePtr) FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, tableID_suck));
    TblGetBounds(table, &r);

    curFont = FntSetFont(oldFont);
    FntSetFont(stdFont);
    NBRows = (Int16) (r.extent.y /FntLineHeight());

    FrmDrawForm(frmP);
    SuckFormInit(frmP);
    handled = true;
    break;
  }

  case winDisplayChangedEvent:
  {
    Int16 offx, offy;
    FormType *frmP;
    TablePtr	table;
    RectangleType	r;
    FontID curFont;
    FontID oldFont = stdFont;

    frmP = FrmGetActiveForm();

  if (romVersion >= 0x04000000)
  {
    if (CollapseResizeForm(frmP, false, &offx, &offy))
    {
      if (offy > 0)
        offy += 2;
      if (offy < 0)
        offy -= 2;
      CollapseMoveResizeFormObject(frmP, tableID_suck, 0, 0, 0, offy);
      CollapseMoveResizeFormObject(frmP, buttonID_sucback, 0, offy, 0, 0);
      CollapseMoveResizeFormObject(frmP, buttonID_sucnew, 0, offy, 0, 0);
      CollapseMoveResizeFormObject(frmP, buttonID_sucmem, 0, offy, 0, 0);
      CollapseMoveResizeFormObject(frmP, scrollID_suck, 0, 0, 0, offy);
    }
}
    table = (TablePtr) FrmGetObjectPtr(frmP, FrmGetObjectIndex(frmP, tableID_suck));
    TblGetBounds(table, &r);

    curFont = FntSetFont(oldFont);
    FntSetFont(stdFont);
    NBRows = (Int16) (r.extent.y /FntLineHeight());

    SuckLoadTable();
    FrmDrawForm(frmP); 
    handled = true;
    break;
  }

  case sclExitEvent:
    TopRecord = event->data.sclExit.newValue;
    SuckLoadTable();
    handled = true;
    break;

  case keyDownEvent:
    switch (event->data.keyDown.chr)
    {
      case pageUpChr:
        if (TopRecord < NBRows)
          TopRecord = 0;
        else
          TopRecord -= NBRows;
        SuckLoadTable();
        handled = true;
        break;

      case pageDownChr:
        TopRecord += NBRows;
        if ((TopRecord + NBRows) > NBRecs)
          TopRecord = NBRecs - NBRows;
        SuckLoadTable();
        handled = true;
        break;
    }
    break;

  case ctlSelectEvent:
    if (event->data.ctlEnter.controlID == buttonID_sucback)
    {
      EventType e;
      FrmReturnToForm(0);
      MemSet( &e, sizeof( EventType ), 0 );
      e.eType = winDisplayChangedEvent;
      EvtAddUniqueEventToQueue( &e, 0, true );
      handled = true;
    }
    if (event->data.ctlEnter.controlID == buttonID_sucnew)
    {
      SuckCreate();
      handled = true;
    }
    if (event->data.ctlEnter.controlID == buttonID_sucmem)
    {
      EventType e;
      SuckExportMemo();
      MemSet( &e, sizeof( EventType ), 0 );
      e.eType = winDisplayChangedEvent;
      EvtAddUniqueEventToQueue( &e, 0, true );
      handled = true;
    }
    break;

  case tblSelectEvent:
    // handle successful tap on a cell
    // for a checkbox or popup, tblExitEvent will be called instead of 
    // tblSelectEvent if the user cancels the control
    break;
      
  case tblEnterEvent:
  {
    Int16  row = event->data.tblEnter.row;
        
    TablePtr		table = event->data.tblEnter.pTable;
    SuckDBType	*p;
    MemHandle		dataH;

    RecordNum = (UInt16) TblGetRowID(table, row);
    dataH = DmQueryRecord(SuckDBRef, RecordNum);
    p = (SuckDBType *) MemHandleLock(dataH);

    WTRecord.number = p->number;
    WTRecord.time = p->time;
    WTRecord.date = p->date;
    WTRecord.duration = p->duration;
    WTRecord.breast = p->breast;

    MemHandleUnlock(dataH);

    newBaby = false;
    FrmPopupForm(formID_sucdet);
    handled = true;

    break;
  }
  }
  return(handled);
}