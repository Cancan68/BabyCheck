/*
 * ModuloNumb
 * ----------
 *
 * Add the first digit to the second digit until the total is < 10
 *
 * Parameters: Integer
 *
 * Returns: Integer
 */
UInt16 ModuloNumb(UInt16 number)
{
  UInt16 part_beg, part_end;

  if (number > 10 || number == 10)
  {
       part_beg = number / 10;
       part_end = number % 10;
       
       number = part_beg + part_end;
       number = ModuloNumb(number);
  }
  return number;
}

/*
 * ModuloSign
 * ----------
 *
 * Subtract 360 to an integer until it become <= 360
 *
 * Parameters: Integer
 *
 * Returns: Integer
 */
UInt16 ModuloSign(UInt16 number)
{
  if (number > 360)
  {
       number -= 360;
       number = ModuloSign(number);
  }
  return number;
}

/*
 * FindZodiacal
 * ------------
 *
 * Search the zodiacal sign corresponding to the birth date
 *
 * Parameters: Date
 *
 * Returns: Integer
 */
UInt16 FindZodiacal(DateType date)
{
  if (date.month ==  3)
  {
    if (date.day < 21)
      return 11;
    else
      return 0;
  }
  if  (date.month == 4)
  {
    if (date.day < 20)
      return 0;
    else
      return 1;
  }
  if  (date.month == 5)
  {
    if (date.day < 21)
      return 1;
    else
      return 2;
  }
  if  (date.month == 6)
  {
    if (date.day < 21)
      return 2;
    else
      return 3;
  }
  if  (date.month == 7)
  {
    if (date.day < 23)
      return 3;
    else
      return 4;
  }
  if  (date.month == 8)
  {
    if (date.day < 23)
      return 4;
    else
      return 5;
  }
  if  (date.month == 9)
  {
    if (date.day < 23)
      return 5;
    else
      return 6;
  }
  if  (date.month == 10)
  {
    if (date.day < 23)
      return 6;
    else
      return 7;
  }
  if  (date.month == 11)
  {
    if (date.day < 22)
      return 7;
    else
      return 8;
  }
  if  (date.month == 12)
  {
    if (date.day < 22)
      return 8;
    else
      return 9;
  }
  if  (date.month == 1)
  {
    if (date.day < 20)
      return 9;
    else
      return 10;
  }
  if  (date.month == 2)
  {
    if (date.day < 18)
      return 10;
    else
      return 11;
  }
  return 0;
}

/*
 * Calculate
 * ---------
 *
 * Calculate the lucky number and the lucky period for a given firstname.
 * Display the result in three fields (number, zodiacal sign, period)
 *
 * Parameters: String
 *
 * Returns: Nothing
 */
void Calculate(Char *firstname, DateType date)
{
  Char *f_name;
  UInt16 len, numb_i, sign_i;

  f_name = firstname;
  len = StrLen(f_name);

  if (!len || !f_name)
  {
    SetFieldTextFromStr(fieldID_number, " ");
    SetFieldTextFromStr(fieldID_sign, " ");
    SetFieldTextFromStr(fieldID_period, " ");
    SetFieldTextFromStr(fieldID_signp, " ");
    return;
  }
  
  numb_i = 0;
  sign_i = 0;

  while (*f_name)
  {
    switch (*f_name)
    {
      case 'a':
      case 'A':
      case 'à':
      case 'â':
        numb_i += 1;
        sign_i += 1;
        break;
      case 'b':
      case 'B':
        numb_i += 2;
        sign_i += 2;
        break;
      case 'c':
      case 'C':
      case 'ç':
        numb_i += 3;
        sign_i += 20;
        break;
      case 'd':
      case 'D':
        numb_i += 4;
        sign_i += 4;
        break;
      case 'e':
      case 'E':
      case 'é':
      case 'è':
      case 'ê':
        numb_i += 5;
        sign_i += 5;
        break;
      case 'f':
      case 'F':
        numb_i += 6;
        sign_i += 80;
        break;
      case 'g':
      case 'G':
        numb_i += 7;
        sign_i += 3;
        break;
      case 'h':
      case 'H':
        numb_i += 8;
        sign_i += 8;
          break;
      case 'i':
      case 'I':
      case 'î':
        numb_i += 9;
        sign_i += 10;
        break;
      case 'j':
      case 'J':
        numb_i += 1;
        sign_i += 10;
        break;
      case 'k':
      case 'K':
        numb_i += 2;
        sign_i += 20;
        break;
      case 'l':
      case 'L':
        numb_i += 3;
        sign_i += 30;
        break;
      case 'm':
      case 'M':
        numb_i += 4;
        sign_i += 40;
        break;
      case 'n':
      case 'N':
        numb_i += 5;
        sign_i += 50;
        break;
      case 'o':
      case 'O':
      case 'ô':
      case 'ò':
        numb_i += 6;
        sign_i += 70;
        break;
      case 'p':
      case 'P':
        numb_i += 7;
        sign_i += 80;
        break;
      case 'q':
      case 'Q':
        numb_i += 8;
        sign_i += 100;
        break;
      case 'r':
      case 'R':
        numb_i += 9;
        sign_i += 200;
        break;
      case 's':
      case 'S':
        numb_i += 1;
        sign_i += 300;
        break;
      case 't':
      case 'T':
        numb_i += 2;
        sign_i += 400;
        break;
      case 'u':
      case 'U':
      case 'û':
      case 'ù':
        numb_i += 3;
        sign_i += 6;
        break;
      case 'v':
      case 'V':
        numb_i += 4;
        sign_i += 6;
        break;
      case 'w':
      case 'W':
        numb_i += 5;
        sign_i += 6;
        break;
      case 'x':
      case 'X':
        numb_i += 6;
        sign_i += 60;
        break;
      case 'y':
      case 'Y':
        numb_i += 7;
        sign_i += 10;
        break;
      case 'z':
      case 'Z':
        numb_i += 8;
        sign_i += 7;
        break;
    }
    f_name++;
  }

  numb_i = ModuloNumb(numb_i);
  sign_i = ModuloSign(sign_i);
  if (sign_i > 30)
      sign_i = ((sign_i - 1) / 30);
  else
      sign_i = 0;
 
  SetFieldTextFromInt(fieldID_number, numb_i, 1);

  SetFieldTextFromStr(fieldID_period, ZodiacalPeriod[sign_i]);

  sign_i = FindZodiacal(date);
  SetFieldTextFromStr(fieldID_sign, ZodiacalSign[sign_i]);
  SetFieldTextFromStr(fieldID_signp, ZodiacalPeriod[sign_i]);
}

/*
 * SymbolFormHandleEvent
 * ---------------------
 *
 * Hanlde the symbol form
 *
 * Parameters: Event
 *
 * Returns: Boolean
 */
Boolean SymbolFormHandleEvent(EventPtr event)
{
  Boolean handled = false;
  
  switch (event->eType)
  {
  case frmOpenEvent:
    CollapseSetState(FrmGetActiveForm(), collapseStateUser);
    FrmDrawForm(FrmGetActiveForm());
    ReadBabyRecord();
    ReadBabyMeasur();
    Calculate(WRecord.name, WMRecord.date);
    break;

  case ctlSelectEvent:
    if (event->data.ctlEnter.controlID == buttonID_symok)
    {
        EventType e;
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
