/*  
 * DrawEnteredValues
 * -----------------
 *
 * Draw the baby data
 *
 * Parameters: Table, Minimum, Maximum
 *
 * Returns: Nothing
 */
static void DrawEnteredValues(UInt32 *DMax, UInt32 *DMin, UInt32 *l, UInt32 *m, UInt32 *n, UInt32 *k, UInt32 ID_disp, UInt8 Mycolor, DateType date_birth)
{ 
  MemHandle     recordh;
  MeasurDBType  *recordP;
  UInt32        i, prev, prevx, cur, nbrec, recValue, recValue2, recValue3;
  UInt32        totdays, nbdays, posx;

  prev = 0;
  prevx = 0;
  recValue2 = 1;
  totdays = (UInt32) (DateToDays(dateMax) - DateToDays(dateBirth));

  /* Draw in Red */

  if (useColor)
  {
    if (Mycolor == 1)
      WinSetForeColor(119);
    else
      WinSetForeColor(205);
  }

  nbrec = (UInt32) DmNumRecords(MeasurDBRef);
  for (i=0;i < nbrec; i++)
  {
    recordh = DmQueryRecord(MeasurDBRef, (UInt16) i);
    if (recordh)
    {
      recordP = (MeasurDBType *) MemHandleLock(recordh);
      if (recordP->number == ID_disp)
      {
        /* Process only the values before dateMax */
        nbdays = (UInt32) (DateToDays(recordP->date) - DateToDays(date_birth));
        if (nbdays <= totdays)
        {
          if (GraphType == 1)
            recValue = (UInt32) recordP->weight;
          else
          {
            if (GraphType == 2)
              recValue = (UInt32) recordP->height;
            else
            {
              recValue = (UInt32) recordP->weight;
              recValue2 = (UInt32) recordP->height;
            }
          }

          if (recValue > 0 && recValue2 > 0)
          {
            posx = ((nbdays * *k * graphMonth) / totdays) + Base;
            if (GraphType == 3)
            {
              if  (Prefs.Munit == 0  || Prefs.Munit ==  10)
                recValue = recValue * 10000;
              else
                recValue = recValue * 4536;
              if (Prefs.Munit > 9)
                recValue2 = recValue2 * 254 / 100;
              recValue3 = ((recValue / recValue2) * 10 / recValue2);
              cur = *l - ((recValue3 - *DMin) * *n) / *m;
            }
            else
              cur = *l - ((recValue - *DMin) * *n) / *m;
            if (prev > 0)
            {
              if (sonyLibRefNum)
                HRWinDrawLine(sonyLibRefNum, prevx, prev, posx, cur);
              else
                WinDrawLine(prevx, prev, posx, cur);
            }
            prevx = posx;
            prev = cur;
          }
        } 
      }
      MemHandleUnlock(recordh);
    }
  }
}

/*
 * DrawGraphDefaultValues
 * ----------------------
 *
 * Draw the default values
 *
 * Parameters: Table, Minimum, Maximum
 *
 * Returns: Nothing
 */
static void DrawGraphDefaultValues(UInt32 tabl[18], UInt32 *DMax, UInt32 *DMin, UInt32 *l, UInt32 *m, UInt32 *n, UInt32 *k)
{
  UInt32 i, j;

  /* Draw the graph in Blue */
  if (useColor)
    WinSetForeColor(101);

  i = *l - ((tabl[0] - *DMin) * *n) / *m;
  j = *l - ((tabl[1] - *DMin) * *n) / *m;

  if (sonyLibRefNum)
  {
    HRWinDrawLine(sonyLibRefNum, Base, i, (*k + Base), j);
    i = *l - ((tabl[2] - *DMin) * *n) / *m;
    HRWinDrawLine(sonyLibRefNum, (*k + Base), j, ((*k * 3 / 2) + Base), i);
    j = *l - ((tabl[3] - *DMin) * *n) / *m;
    HRWinDrawLine(sonyLibRefNum, ((*k * 3 / 2) + Base), i, ((*k * 2) + Base), j);
    i = *l - (((tabl[4] - *DMin) * *n) / *m);
    HRWinDrawLine(sonyLibRefNum, ((*k * 2) + Base), j, ((*k * 3) + Base), i);
    if (graphMonth > 3)
    {
      j = *l - (((tabl[5] - *DMin) * *n) / *m);
      HRWinDrawLine(sonyLibRefNum, ((*k * 3) + Base), i, ((*k * 4) + Base), j);
      i = *l - ((tabl[6] - *DMin) * *n) / *m;
      HRWinDrawLine(sonyLibRefNum, ((*k * 4) + Base), j, ((*k * 5) + Base), i);
      j = *l - ((tabl[7] - *DMin) * *n) / *m;
      HRWinDrawLine(sonyLibRefNum, ((*k * 5) + Base), i, ((*k * 6) + Base), j);
      if (graphMonth > 6)
      {
        i = *l - ((tabl[8] - *DMin) * *n) / *m;
        HRWinDrawLine(sonyLibRefNum, ((*k * 6) + Base), j, ((*k * 9) + Base), i);
        j = *l - ((tabl[9] - *DMin) * *n) / *m;
        HRWinDrawLine(sonyLibRefNum, ((*k * 9) + Base), i, ((*k * 12) + Base), j);
        if (graphMonth > 12)
        {
          i = *l - ((tabl[10] - *DMin) * *n) / *m;
          HRWinDrawLine(sonyLibRefNum, ((*k * 12) + Base), j, ((*k * 15) + Base), i);
          j = *l - ((tabl[11] - *DMin) * *n) / *m;
          HRWinDrawLine(sonyLibRefNum, ((*k * 15) + Base), i, ((*k * 18) + Base), j);
          if (graphMonth > 18)
          {
            i = *l - ((tabl[12] - *DMin) * *n) / *m;
            HRWinDrawLine(sonyLibRefNum, ((*k * 18) + Base), j, ((*k * 21) + Base), i);
            j = *l - ((tabl[13] - *DMin) * *n) / *m;
            HRWinDrawLine(sonyLibRefNum, ((*k * 21) + Base), i, ((*k * 24) + Base), j);
            if (graphMonth > 24)
            {
              i = *l - ((tabl[14] - *DMin) * *n) / *m;
              HRWinDrawLine(sonyLibRefNum, ((*k * 24) + Base), j, ((*k * 27) + Base), i);
              j = *l - ((tabl[15] - *DMin) * *n) / *m;
              HRWinDrawLine(sonyLibRefNum, ((*k * 27) + Base), i, ((*k * 30) + Base), j);
              if (graphMonth > 30)
              {
                i = *l - ((tabl[16] - *DMin) * *n) / *m;
                HRWinDrawLine(sonyLibRefNum, ((*k * 30) + Base), j, ((*k * 33) + Base), i);
                j = *l - ((tabl[17] - *DMin) * *n) / *m;
                HRWinDrawLine(sonyLibRefNum, ((*k * 33) + Base), i, ((*k * 36) + Base), j);
              }
            }
          }
        }
      }
    }
  }
  else
  {
    WinDrawLine(Base, i, (*k + Base), j);
    i = *l - ((tabl[2] - *DMin) * *n) / *m;
    WinDrawLine((*k + Base), j, ((*k * 3 / 2) + Base), i);
    j = *l - ((tabl[3] - *DMin) * *n) / *m;
    WinDrawLine(((*k * 3 / 2) + Base), i, ((*k * 2) + Base), j);
    i = *l - (((tabl[4] - *DMin) * *n) / *m);
    WinDrawLine(((*k * 2) + Base), j, ((*k * 3) + Base), i);
    if (graphMonth > 3)
    {
      j = *l - (((tabl[5] - *DMin) * *n) / *m);
      WinDrawLine(((*k * 3) + Base), i, ((*k * 4) + Base), j);
      i = *l - ((tabl[6] - *DMin) * *n) / *m;
      WinDrawLine(((*k * 4) + Base), j, ((*k * 5) + Base), i);
      j = *l - ((tabl[7] - *DMin) * *n) / *m;
      WinDrawLine(((*k * 5) + Base), i, ((*k * 6) + Base), j);
      if (graphMonth > 6)
      {
        i = *l - ((tabl[8] - *DMin) * *n) / *m;
        WinDrawLine(((*k * 6) + Base), j, ((*k * 9) + Base), i);
        j = *l - ((tabl[9] - *DMin) * *n) / *m;
        WinDrawLine(((*k * 9) + Base), i, ((*k * 12) + Base), j);
        if (graphMonth > 12)
        {
          i = *l - ((tabl[10] - *DMin) * *n) / *m;
          WinDrawLine(((*k * 12) + Base), j, ((*k * 15) + Base), i);
          j = *l - ((tabl[11] - *DMin) * *n) / *m;
          WinDrawLine(((*k * 15) + Base), i, ((*k * 18) + Base), j);
          if (graphMonth > 18)
          {
            i = *l - ((tabl[12] - *DMin) * *n) / *m;
            WinDrawLine(((*k * 18) + Base), j, ((*k * 21) + Base), i);
            j = *l - ((tabl[13] - *DMin) * *n) / *m;
            WinDrawLine(((*k * 21) + Base), i, ((*k * 24) + Base), j);
            if (graphMonth > 24)
            {
              i = *l - ((tabl[14] - *DMin) * *n) / *m;
              WinDrawLine(((*k * 24) + Base), j, ((*k * 27) + Base), i);
              j = *l - ((tabl[15] - *DMin) * *n) / *m;
              WinDrawLine(((*k * 27) + Base), i, ((*k * 30) + Base), j);
              if (graphMonth > 30)
              {
                i = *l - ((tabl[16] - *DMin) * *n) / *m;
                WinDrawLine(((*k * 30) + Base), j, ((*k * 33) + Base), i);
                j = *l - ((tabl[17] - *DMin) * *n) / *m;
                WinDrawLine(((*k * 33) + Base), i, ((*k * 36) + Base), j);
              }
            }
          }
        }
      }
    }
  }
}

/*
 * GraphDrawYAxis
 * --------------
 *
 * Draw the default values
 *
 * Parameters: Table, Minimum, Maximum
 *
 * Returns: Nothing
 */
static void GraphDrawYAxis(UInt32 yvalue, UInt32 *l, UInt32 *n, UInt32 *k, UInt32 *indice, UInt32 *m, UInt32 *DMin)
{
  UInt32 Posy = *l - ((yvalue - *DMin) * *n / *m);
  UInt32 Posxf = (*k * graphMonth) + Base;
  UInt16 partEnt, partDec;
  Char s[10];


  /* Draw in grey */

  if (useColor)
    WinSetForeColor(222);

  if (sonyLibRefNum)
  {
    if (useColor)
      HRWinDrawLine(sonyLibRefNum, Base, Posy, Posxf, Posy);

    /* Draw in black */

    if (useColor)
      WinSetForeColor(255);

    HRWinDrawLine(sonyLibRefNum, (Base - 5), Posy, Base, Posy);
  }
  else
  {
    if (useColor)
      WinDrawLine(Base, Posy, Posxf, Posy);

    /* Draw in black */

    if (useColor)
      WinSetForeColor(255);

    WinDrawLine((Base - 5), Posy, Base, Posy);
  }

  partEnt = (UInt16) (yvalue / *indice);
  partDec = (UInt16) (yvalue - (partEnt * *indice));
  if (*indice == 10)
    StrPrintF(s, "%d%c%d", partEnt, decSep, partDec);
  else
      StrPrintF(s, "%d%c%02d", partEnt, decSep, partDec);
  Posy -= 5;

  if (sonyLibRefNum)
    HRWinDrawChars(sonyLibRefNum, s, StrLen(s), 2, Posy);
  else
    if (IsHighDensity)
      WinDrawChars(s, StrLen(s), 2, (Posy - 5));
    else
      WinDrawChars(s, StrLen(s), 2, Posy);
}

/*
 * GraphDrawXAxis
 * --------------
 *
 * Draw the default values
 *
 * Parameters: Table, Minimum, Maximum
 *
 * Returns: Nothing
 */
static void GraphDrawXAxis(UInt32 posx, UInt32 *posyt, UInt32 *posyb, UInt32 *k)
{
  UInt32 Posx = (*k * posx) + Base;
  UInt32 t;
  Char s[10];
  
  if (IsHighDensity)
  {
    if (posx < 10)
      t = Base - 4;
    else
      t = Base - 8;
  }
  else
  {
    if (posx < 10)
      t = Base - 1;
    else
      t = Base - 5;
  }

  /* Draw in grey */

  if (useColor)
    WinSetForeColor(222);

  if (sonyLibRefNum)
  {
    if (useColor)
      HRWinDrawLine(sonyLibRefNum, Posx, *posyt, Posx, *posyb);
  }
  else
  {
    if (useColor)
      WinDrawLine(Posx, *posyt, Posx, *posyb);
  }

  /* Draw in black */

  if (useColor)
    WinSetForeColor(255);
  StrPrintF(s, "%d", (UInt16) posx);

  if (sonyLibRefNum)
  {
    HRWinDrawLine(sonyLibRefNum, Posx, *posyb, Posx, (*posyb + 5));
    HRWinDrawChars(sonyLibRefNum, s, StrLen(s), ((*k * posx) + t), (*posyb + 5));
  }
  else
  {
    WinDrawLine(Posx, *posyb, Posx, (*posyb + 5));
    WinDrawChars(s, StrLen(s), ((*k * posx) + t), (*posyb + 5));
  }
}

/*
 * DrawDefaultValues
 * -----------------
 *
 * Draw the default values
 *
 * Parameters: Table, Minimum, Maximum
 *
 * Returns: Nothing
 */
static void DrawDefaultValues(FormType *frmP, UInt32 *DMax, UInt32 *DMin, UInt32 *l, UInt32 *m, UInt32 *n, UInt32 *k, UInt32 *j, UInt16 *index, UInt32 *indice)
{

  UInt32  nDMax, nDMin, nDMid;

  /* y axys values */

  nDMax = *DMax / *indice;
  nDMax = nDMax * *indice;

  nDMin = *DMin / *indice;
  nDMin = nDMin * *indice;
  if ((DMin - nDMin) > 0)
    nDMin += *indice;
 
  nDMid = (nDMax + nDMin) / 2;
    
  GraphDrawYAxis(nDMax, l, n, k, indice, m, DMin);
  GraphDrawYAxis(((nDMax + nDMid) / 2), l, n, k, indice, m, DMin);
  GraphDrawYAxis(nDMid, l, n, k, indice, m, DMin);
  GraphDrawYAxis(((nDMid + nDMin) / 2), l, n, k, indice, m, DMin);
  GraphDrawYAxis(nDMin, l, n, k, indice, m, DMin);

  if (useColor)
    WinSetForeColor(255);

  if (sonyLibRefNum)
    HRWinDrawLine(sonyLibRefNum, Base, *l, ((*k * graphMonth) + Base), *l);
  else
    WinDrawLine(Base, *l, ((*k * graphMonth) + Base), *l);

  /* x axys values */

  GraphDrawXAxis(0, j, l, k);
  if (useColor)
    WinSetForeColor(255);

  if (sonyLibRefNum)
    HRWinDrawLine(sonyLibRefNum, Base, *j, Base, *l);
  else
    WinDrawLine(Base, *j, Base, *l);

  switch (graphMonth)
  {
    case 3:
      GraphDrawXAxis(1, j, l, k);
      GraphDrawXAxis(2, j, l, k);
      GraphDrawXAxis(3, j, l, k);
      break;

    case 6:
      GraphDrawXAxis(2, j, l, k);
      GraphDrawXAxis(4, j, l, k);
      GraphDrawXAxis(6, j, l, k);
      break;

    case 9:
      GraphDrawXAxis(3, j, l, k);
      GraphDrawXAxis(6, j, l, k);
      GraphDrawXAxis(9, j, l, k);
      break;

    case 12:
      GraphDrawXAxis(3, j, l, k);
      GraphDrawXAxis(6, j, l, k);
      GraphDrawXAxis(9, j, l, k);
      GraphDrawXAxis(12, j, l, k);
      break;

    case 18:
      GraphDrawXAxis(6, j, l, k);
      GraphDrawXAxis(12, j, l, k);
      GraphDrawXAxis(18, j, l, k);
      break;

    case 24:
      GraphDrawXAxis(6, j, l, k);
      GraphDrawXAxis(12, j, l, k);
      GraphDrawXAxis(18, j, l, k);
      GraphDrawXAxis(24, j, l, k);
      break;

    case 30:
      GraphDrawXAxis(6, j, l, k);
      GraphDrawXAxis(12, j, l, k);
      GraphDrawXAxis(18, j, l, k);
      GraphDrawXAxis(24, j, l, k);
      GraphDrawXAxis(30, j, l, k);
      break;

    case 36:
      GraphDrawXAxis(9, j, l, k);
      GraphDrawXAxis(18, j, l, k);
      GraphDrawXAxis(27, j, l, k);
      GraphDrawXAxis(36, j, l, k);
      break;
  }

  /* Draw the graph itself */

  if (GraphType == 1)
  {
    if (Prefs.Munit == 0 || Prefs.Munit == 10)
    {
      DrawGraphDefaultValues(Kmin, DMax, DMin, l, m, n, k);
      DrawGraphDefaultValues(Kmax, DMax, DMin, l, m, n, k);
    }
    else
    {
      DrawGraphDefaultValues(Lmin, DMax, DMin, l, m, n, k);
      DrawGraphDefaultValues(Lmax, DMax, DMin, l, m, n, k);
    }
  }
  else
  {
    if (GraphType == 2)
    {
      if (Prefs.Munit < 9)
      {
        DrawGraphDefaultValues(Cmin, DMax, DMin, l, m, n, k);
        DrawGraphDefaultValues(Cmax, DMax, DMin, l, m, n, k);
      }
      else
      {
        DrawGraphDefaultValues(Imin, DMax, DMin, l, m, n, k);
        DrawGraphDefaultValues(Imax, DMax, DMin, l, m, n, k);
      }
    }
    else
    {
      if (WRecord.sexe == 'M')
      {
        DrawGraphDefaultValues(BMIminb, DMax, DMin, l, m, n, k);
        DrawGraphDefaultValues(BMImaxb, DMax, DMin, l, m, n, k);
      }
      else
      {
        DrawGraphDefaultValues(BMIming, DMax, DMin, l, m, n, k);
        DrawGraphDefaultValues(BMImaxg, DMax, DMin, l, m, n, k);
      }
    }
  }
  DrawEnteredValues(DMax, DMin, l, m, n, k, uniqueID, 1, dateBirth);
  if (uniqueID2 != 0)
    DrawEnteredValues(DMax, DMin, l, m, n, k, uniqueID2, 2, dateBirth2);
}

/*
 * GraphDrawInit
 * -------------
 *
 * Initialize the drawing parameters
 *
 * Parameters: Nothing
 *
 * Returns: Nothing
 */
static void GraphDrawInit(FormType *frmP)
{
  UInt32 j, k, l, m, n, DMax, DMin, indice;
  UInt16 index, month;
  UInt16 gadget = FrmGetObjectIndex(frmP, gadgetID_graph);
  RectangleType rect;

  FrmGetObjectBounds(frmP, gadget, &rect);
  WinEraseRectangle(&rect, 0);

  WinPushDrawState();

  if (IsHighDensity)
      WinSetCoordinateSystem(kCoordinatesDouble);

  dateMax = dateBirth;
  month = dateMax.month + graphMonth;
  if (month > 12)
  {
    dateMax.year++;
    month -= 12;
  } 
  if (month > 12)
  {
    dateMax.year++;
    month -= 12;
  } 
  if (month > 12)
  {
    dateMax.year++;
    month -= 12;
  } 
  dateMax.month = month;

  if (Resolution == 160)
    j = 20;
  else
    j = 40;
 
  switch (graphMonth)
  {
    case 3: index = 4; break;
    case 6: index = 7; break;
    case 12: index = 9; break;
    case 18: index = 11; break;
    case 24: index = 13; break;
    case 30: index = 15; break;
    case 36: index = 17; break;
  }

  if (GraphType == 1)
  {
    if (Prefs.Munit == 0 || Prefs.Munit == 10)
    {
      indice = 100;
      DMin = Kmin[0] / 2;
      DMax = Kmax[index] + (Kmax[index] * 20 / 100);
    }
    else
    {
      indice = 100;
      DMin = Lmin[0] / 2;
      DMax = Lmax[index] + (Lmax[index] * 20 / 100);
    }
    if (DMin > (Wmin / 2))
      DMin = (Wmin / 2);
  }
  else
  {
    if (GraphType == 2)
    {
      if (Prefs.Munit < 9)
      {
        indice = 10;
        DMin = Cmin[0] / 2;
        DMax = Cmax[index] + (Cmax[index] * 20 / 100);
      }
      else
      {
        indice = 10;
        DMin = Imin[0] / 2;
        DMax = Imax[index] + (Imax[index] * 20 / 100);
      }
      if (DMin > (Hmin / 2))
        DMin = (Hmin / 2);
    }
    else
    {
      if (WRecord.sexe == 'M')
      {
        indice = 10;
        DMin = BMIminb[0] / 2;
        DMax = BMImaxb[index] + (BMImaxb[index] * 20 / 100);
      }
      else
      {
        indice = 10;
        DMin = BMIming[0] / 2;
        DMax = BMImaxg[index] + (BMImaxg[index] * 20 / 100);
      }
    }
  }

  m = DMax - DMin;
  if (Resolution == 160)
  {
    l = rect.extent.y;
    n = rect.extent.y - j;
    k = (rect.extent.x - Base) / graphMonth;
    if (graphMonth == 3)
     k -= 1;
  }
  else
  {
    l = (rect.extent.y * 2);
    n = (rect.extent.y * 2) - j;
    k = ((rect.extent.x * 2) - Base - 10) / graphMonth;
  }

  DrawDefaultValues(frmP, &DMax, &DMin, &l, &m, &n, &k, &j, &index, &indice);

  WinPopDrawState();

  if (GraphType == 1)
  {
    if (Prefs.Munit == 0 || Prefs.Munit == 10)
      FrmSetTitle(frmP, titleKg);
    else
      FrmSetTitle(frmP, titleLb);
  }
  else
  {
    if (GraphType == 2)
    {
      if (Prefs.Munit < 9)
        FrmSetTitle(frmP, titleCm);
      else
        FrmSetTitle(frmP, titleIn);
    }
    else
      FrmSetTitle(frmP, titleBm);
  }
}

/*
 * GraphFormInit
 * -------------
 *
 * Initialize the GraphForm
 *
 * Parameters: Nothing
 *
 * Returns: Nothing
 */
static void GraphFormInit(void)
{
  FormType *frmP;
  Err     err;
  UInt32  attr;

  frmP = FrmGetActiveForm();

  IsHighDensity = false;

  if (sonyLibRefNum)
  {
    Base = 40;
    Resolx = Resoly = Resolution = 320;
    /* The depth and color flag are already set */
    err = HRWinScreenMode(sonyLibRefNum, winScreenModeSet, &Resolx, &Resoly, &Depth, &useColor);
  }
  else
  {
    Resolx = Resoly = Resolution = 160;
    Base = 30;
    WinScreenMode(winScreenModeGetSupportsColor, NULL, NULL, NULL, &useColor);

    if (IsOS5)
    {
      err = FtrGet(sysFtrCreator, sysFtrNumWinVersion, &attr);
      if (attr >= 4)
      {
        Base = 50;
        Resolx = Resoly = Resolution = 320;
        IsHighDensity = true;
      } 

      WinScreenGetAttribute(winScreenDepth, &attr);

      Depth = attr;
      if (attr < 8)
        useColor = false;
    }
    else
      WinScreenMode(winScreenModeGetSupportedDepths, NULL, NULL, &Depth, NULL);

    err = WinScreenMode(winScreenModeSet, &Resolx, &Resoly, &Depth, &useColor);
  }

  FrmDrawForm(frmP);

  GraphDrawInit(frmP);
}

/*
 * GraphFormHandleEvent
 * --------------------
 *  
 * Handle the graphical screen
 *  
 * Parameters: Event
 *
 * Returns: Boolean
 */
static Boolean GraphFormHandleEvent(EventPtr event)
{
  FormType *frmP = FrmGetActiveForm();
  Boolean handled = false;

  switch (event->eType)
  {

  case frmOpenEvent:
  {
    ListType *monthw = Item(listID_month);
    ControlType *ctl = Item(listID_montht);
    ControlType *ctl2 = Item(listID_baby2t);
    Char *months = (Char *) CtlGetLabel(ctl);
    Char *babys = (Char *) CtlGetLabel(ctl2);
    short choice;
    Boolean FitInWidth;
    Char    horzEllipsis;
    UInt16  width;
    UInt16  len;
    FontID  curFont;
    Char szTitle[30];
    Int16 offx, offy;

    listbabycomp = Item(listID_baby2);
    LstSetListChoices(listbabycomp, NULL, (Int16) num_baby);
    LstSetHeight(listbabycomp, (Int16) num_baby);
    LstSetDrawFunction(listbabycomp, BabyDrawList);

    choice = 0;
    switch (graphMonth)
    {
      case 3: choice = 0; break;
      case 6: choice = 1; break;
      case 12: choice = 2; break;
      case 18: choice = 3; break;
      case 24: choice = 4; break;
      case 30: choice = 5; break;
      case 36: choice = 6; break;
      default: choice = 0; break;
    }
    LstSetSelection(monthw, choice);
    StrCopy(months, LstGetSelectionText(monthw, choice));
    CtlSetLabel(ctl, months);
    LstSetSelection(listbabycomp, 0);
    StrCopy(szTitle, WRecord.name);
    for (len = 0; len < StrLen(szTitle); len++)
    {
      if (szTitle[len] == '\n')
        szTitle[len] = ' ';
    }

    len = StrLen(szTitle);
    width = 70;
    curFont = FntSetFont(stdFont );
    FntCharsInWidth(szTitle, &width, &len, &FitInWidth );
    FntSetFont(curFont);
    if(!FitInWidth)
    {
      ChrHorizEllipsis( &horzEllipsis );
      szTitle[len-1] = horzEllipsis;
      szTitle[len] = '\0';
    }
    StrCopy(babys, szTitle);
    CtlSetLabel(ctl2, babys);

    CollapseSetState(frmP, collapseStateUser);

  if (romVersion >= 0x04000000)
  {
    if (CollapseResizeForm(frmP, false, &offx, &offy))
    {
      CollapseMoveResizeFormObject(frmP, gadgetID_graph, 0, 0, offx, offy);
      CollapseMoveResizeFormObject(frmP, buttonID_graphok, 0, offy, 0, 0);
      CollapseMoveResizeFormObject(frmP, listID_montht, offx, offy, 0, 0);
      CollapseMoveResizeFormObject(frmP, listID_month, offx, offy, 0, 0);
      CollapseMoveResizeFormObject(frmP, listID_baby2t, offx, offy, 0, 0);
      CollapseMoveResizeFormObject(frmP, listID_baby2, offx, offy, 0, 0);
    }
  }


    GraphFormInit();
    handled = true;
    break;
  }

  case winDisplayChangedEvent:
  {
    Int16 offx, offy;

    frmP = FrmGetActiveForm();

  if (romVersion >= 0x04000000)
  {
    if (CollapseResizeForm(frmP, false, &offx, &offy))
    {
      CollapseMoveResizeFormObject(frmP, gadgetID_graph, 0, 0, offx, offy);
      CollapseMoveResizeFormObject(frmP, buttonID_graphok, 0, offy, 0, 0);
      CollapseMoveResizeFormObject(frmP, listID_montht, offx, offy, 0, 0);
      CollapseMoveResizeFormObject(frmP, listID_month, offx, offy, 0, 0);
      CollapseMoveResizeFormObject(frmP, listID_baby2t, offx, offy, 0, 0);
      CollapseMoveResizeFormObject(frmP, listID_baby2, offx, offy, 0, 0);
    }
  }

    FrmDrawForm(frmP);
    GraphDrawInit(FrmGetActiveForm());
    handled = true;
    break;
  }


  case frmUpdateEvent:
    FrmDrawForm(FrmGetActiveForm());
    GraphDrawInit(FrmGetActiveForm());
    handled = true;
    break;
 
  case frmCloseEvent:
  {
    EventType e;
    FrmEraseForm(FrmGetActiveForm());
    FrmDeleteForm(FrmGetActiveForm());
    MemSet( &e, sizeof( EventType ), 0 );
    e.eType = winDisplayChangedEvent;
    EvtAddUniqueEventToQueue( &e, 0, true );
    handled = true;
    break;
  }

  case ctlSelectEvent:
  {

    switch (event->data.ctlEnter.controlID)
    {
      case buttonID_graphok:
      {
        EventType e;
        FrmReturnToForm(0);
        MemSet( &e, sizeof( EventType ), 0 );
        e.eType = winDisplayChangedEvent;
        EvtAddUniqueEventToQueue( &e, 0, true );
        handled = true;
        break;
      }
 
      case listID_montht:
      {
        ListPtr monthw = Item(listID_month);
        ControlType *ctl = Item(listID_montht);
        Char *s = (Char *) CtlGetLabel(ctl);
        short choice;

        choice = 0;
        switch (graphMonth)
        { 
          case 3: choice = 0; break;
          case 6: choice = 1; break;
          case 12: choice = 2; break;
          case 18: choice = 3; break;
          case 24: choice = 4; break;
          case 30: choice = 5; break;
          case 36: choice = 6; break;
          default: choice = 0; break;
        }
        LstSetSelection(monthw, choice);
        choice = LstPopupList(monthw);
        if (choice >= 0)
        {
          switch (choice)
          {
            case 6: graphMonth = 36; break;
            case 5: graphMonth = 30; break;
            case 4: graphMonth = 24; break;
            case 3: graphMonth = 18; break;
            case 2: graphMonth = 12; break;
            case 1: graphMonth = 6; break;
            case 0: graphMonth = 3; break;
          }
          StrCopy(s, LstGetSelectionText(monthw, choice));
          CtlSetLabel(ctl, s);
          GraphDrawInit(FrmGetActiveForm());
        }
        handled = true;
        break;
      } 

      case listID_baby2t:
      {
        short choice;
        MemHandle recordH;
        BabyDBType *recordP;
        UInt32 tmp_unique;
  Boolean FitInWidth;
  Char            horzEllipsis;
  UInt16  width;
  UInt16  len;
  FontID  curFont;
  Char szTitle[30];

        ControlType *ctl = Item(listID_baby2t);
        Char *s = (Char *) CtlGetLabel(ctl);

        choice = 0;
        LstSetSelection(listbabycomp, choice);
        choice = LstPopupList(listbabycomp);

        if (choice >= 0)
        {
          recordH = DmQueryRecord(BabyDBRef, choice); 
          recordP = (BabyDBType *) MemHandleLock(recordH);
    StrCopy(szTitle, recordP->name);
    for (len = 0; len < StrLen(szTitle); len++)
    {
      if (szTitle[len] == '\n')
        szTitle[len] = ' ';
    }

    len = StrLen(szTitle);
    width = 70;
    curFont = FntSetFont(stdFont );
    FntCharsInWidth(szTitle, &width, &len, &FitInWidth );
    FntSetFont(curFont);
    if(!FitInWidth)
    {
      ChrHorizEllipsis( &horzEllipsis );
      szTitle[len-1] = horzEllipsis;
      szTitle[len] = '\0';
    }
          StrCopy(s, szTitle);
          tmp_unique = uniqueID;
          CtlSetLabel(ctl, s);
          DmRecordInfo(BabyDBRef, choice, NULL, &uniqueID2, NULL);
          uniqueID = uniqueID2;
          ReadBabyMeasur();
          dateBirth2 = WMRecord.date;
          uniqueID = tmp_unique;
          if (uniqueID2 == uniqueID) 
              uniqueID2 = 0;
          MemHandleUnlock(recordH);
          GraphDrawInit(FrmGetActiveForm());
        }
        handled = true;
        break;
      } 
    }
    break;
    }

  }
  return(handled);
}
