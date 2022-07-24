
void getTimeLocal()
{
  time_t tT = now();
  // local time
  time_t tTlocal = CE.toLocal(tT);
  w = weekday(tTlocal);
  d = day(tTlocal);
  mo = month(tTlocal);
  ye = year(tTlocal);
  h = hour(tTlocal);
  m = minute(tTlocal);
  s = second(tTlocal);
  //localMillisAtUpdate = millis();
  //client.stop();
}

// =======================================================================
