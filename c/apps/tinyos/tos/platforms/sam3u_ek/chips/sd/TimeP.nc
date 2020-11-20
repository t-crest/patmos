/*
 * Copyright (C) 2002     Manuel Novoa III
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the
 *   distribution.
 * - Neither the name of the copyright holders nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
 * WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * port to tos-2.x
 * @author Steve Ayer
 * @date   January, 2010
 */

// includes InfoMem;  let's avoid this...

module TimeP {
  provides {
    interface Init;
    interface Time;
    //    interface ParamView;    save for ip stack port to tos-2.x
  }
  uses {
    interface Timer<TMilli>;
    //    interface NTPClient;  save for ip stack port to tos-2.x
    interface LocalTime64<T32khz>;
  }
}

implementation {

#define HAVE_DST
extern int snprintf(char *str, size_t len, const char *format, ...) __attribute__ ((C));

  struct y_info{
    time_t year_seconds; /* time_t value for beginning of year */
    int8_t wday_offset;   /* tm_day of first day of the year */
    int8_t isleap;
    int16_t dst_first_yday;
    int16_t dst_last_yday;
  };
  time_t g_seconds;
  time_t g_year_seconds;
  time_t g_seconds_from_year;
  uint16_t g_days;
  time_t g_seconds_from_day;

  //#define TZNAME_MAX 7
  //#define LONG_MAX 0x7fffffffL
  time_t g_tick_local_time;
#ifdef CURRENT_TIME  
  time_t g_current_time = CURRENT_TIME;
#else
  time_t g_current_time;
#endif

#ifndef HOST_TIME
#define NUM_YEARS 4
  int16_t g_first_year = 2009;
  struct y_info year_info[NUM_YEARS] = {
    /* unix time start jan 1 1970 */
    // from 2007, dst begins 2nd sunday in march, ends first sunday in november
    { 0x495c4dd0, 3, 0, 67, 305},
    { 0x4B3D8150, 4, 0, 73, 311},
    { 0x4D1EB4D0, 5, 0, 72, 310},
    { 0x4EFFE850, 6, 1, 71, 309}
  };

#else
#define NUM_YEARS 1
  // these will be set by setZoneInfo()
  int16_t g_first_year = 0;
  struct y_info year_info[NUM_YEARS];
#endif

  //  time_t g_local_time; /* from LocalTime.read() */
  uint64_t g_local_time; /* from LocalTime.read(), now 64 bits to handle */

  typedef struct {
    long gmt_offset;
    //    long dst_offset;
    //    short day;					/* for J or normal */
    //    short week;
    //    short month;
    //    short rule_type;			/* J, M, \0 */
    //    char tzname[TZNAME_MAX+1];
  } rule_struct;

  rule_struct _time_tzinfo[1];

  // gmt_offset can stay zero, because any CURRENT_TIME we get is from a local source
  command error_t Init.init() {
    _time_tzinfo[0].gmt_offset = 60L * 0L;

    g_tick_local_time = call LocalTime64.get();
    call Timer.startPeriodic(10*1024L);
    signal Time.tick();
    return SUCCESS;
  }

  command void Time.setCurrentTime(time_t current_time){
    atomic g_current_time = current_time;
    g_local_time = call LocalTime64.get();
  }

  command void Time.setZoneInfo(uint16_t g_year, 
				time_t g_year_time, 
				uint8_t g_zero_day, 
				uint16_t g_dst_fday, 
				uint16_t g_dst_lday){
    g_first_year = g_year;
    
    year_info[0].year_seconds = g_year_time;
    year_info[0].wday_offset = g_zero_day;

    if(!(g_year % 4))
      year_info[0].isleap = 1;
    else
      year_info[0].isleap = 0;

    year_info[0].dst_first_yday = g_dst_fday;
    year_info[0].dst_last_yday = g_dst_lday;
  }

  void dotick(int force) {
    time_t tick = call LocalTime64.get();
    if (force || tick >= (g_tick_local_time + 32768L*10)) {
      signal Time.tick();
      g_tick_local_time = tick;
    }
  }
  event void Timer.fired() {
    dotick(0);
  }

  /*  
   * save this pending tos-2.x port of ip stack
   *
   event void NTPClient.timestampReceived( uint32_t *seconds, uint32_t *fraction ) {
   g_current_time = *seconds;
   g_local_time = call LocalTime64.get();
   dotick(1);
   }
  */

  struct tm __time_tm;

  /* Notes:
   * If time_t is 32 bits, then no overflow is possible.
   * It time_t is > 32 bits, this needs to be adjusted to deal with overflow.
   */

  static const int8_t days_per_month[] = {
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31, /* non-leap */
    29,
  };

#ifdef __UCLIBC_HAS_TM_EXTENSIONS__
  static const char utc_string[] = "UTC";
#endif

  /*
    import time
    for y in range(2004,2008):
       t = time.mktime((y, 1, 1, 0, 0, 0, 0, 0, 0))-5*3600
       print time.gmtime(t)
    
    (2004, 1, 1, 0, 0, 0, 3, 1, 0)
    (2005, 1, 1, 0, 0, 0, 5, 1, 0)
    (2006, 1, 1, 0, 0, 0, 6, 1, 0)
    (2007, 1, 1, 0, 0, 0, 0, 1, 0)
    from below in year_info struct for 2004-7
    { 0x3ff36300, 3, 1 },
    { 0x41d5e800, 5, 0 },
    { 0x43b71b80, 6, 0 },
    { 0x45984f00, 0, 0 }
    
    NTP Timestamp starts Jan 1, 1900 and is 2208988800 jan 1 1970
    '%x' % (int(time.mktime((2004, 1, 1, 0, 0, 0, 3, 1, 0))) -18000 + 2208988800l)
    
    and now, 
    
    (2009, 1, 1, 0, 0, 0, 3, 1, 0)
    (2010, 1, 1, 0, 0, 0, 4, 1, 0)
    (2011, 1, 1, 0, 0, 0, 5, 1, 0)
    (2012, 1, 1, 0, 0, 0, 6, 1, 0)
    
  */

  struct tm *_time_t2tm(const time_t *timer, int localtime, struct tm *result)
  {
    uint16_t seconds16;
    int i;
    time_t seconds = *timer;
    uint16_t days;
    uint16_t hour, min;
    int isleap = 0;

    //if year_info has dst_first_yday and dst_last_yday defined, then HAVE_DST should be defined up top.
#ifdef HAVE_DST
    int isdst = 0;
#endif

    if (localtime) {
      seconds -= _time_tzinfo[0].gmt_offset;
    }

    g_seconds = seconds;
    if (seconds < year_info[0].year_seconds) {
      memset(result, 0, sizeof(struct tm));
      return NULL;
    }
    
    if(NUM_YEARS > 1){
      for (i = 0; i < NUM_YEARS-1; i++) {
	if (seconds < year_info[i+1].year_seconds) {
	  seconds -= year_info[i].year_seconds;
	  result->tm_year = g_first_year + i;
	  isleap = year_info[i].isleap;
	  break;
	}
      }
    }
    else{
      i = 0;
      seconds -= year_info[i].year_seconds;
      result->tm_year = g_first_year + i;
      isleap = year_info[i].isleap;
    }
    g_year_seconds = year_info[i].year_seconds;
    g_seconds_from_year = seconds;
    days = seconds / 86400L;
    g_days = days;
    seconds -= days * 86400L;
    g_seconds_from_day = seconds;
#ifdef HAVE_DST
    if (days >= year_info[i].dst_first_yday && 
	days <= year_info[i].dst_last_yday) {
      isdst = 1;
      seconds += 3600;
      if (seconds < 0) {
	days--;
	seconds += 86400L;
      }
      if (days < 0) {
	result->tm_year--;
	days += 365;
      }
    }
#endif /* HAVE_DST */

    result->tm_yday = days; 
#ifdef HAVE_WDAY
    result->tm_wday = (result->tm_yday + year_info[i+1].wday_offset) % 7;
#endif

    for (i = 0; i < 12; i++) {
      int8_t dpm = days_per_month[i];
      if (i == 1 && isleap) dpm++;
      if (days < dpm)
	break;
      days -= dpm;
    }
    result->tm_mon = i;
    result->tm_mday = 1 + days ;

    hour = seconds / 3600;
    seconds16 = seconds - hour * 3600;
    result->tm_hour = hour;
    min = seconds16 / 60;
    result->tm_sec = seconds16 - min * 60;
    result->tm_min = min;
    return result;
  }

  command error_t Time.gmtime(const time_t *timer, struct tm *ptm)
  {

    _time_t2tm(timer, 0, ptm); /* Can return NULL... */

    return SUCCESS;
  }

  /* Note: timezone locking is done by localtime_r. */

  static int tm_isdst(register const struct tm *__restrict ptm) {
    // no DST in arizona
    return 0;
  }
    
  command error_t Time.localtime(const time_t *timer, struct tm *result)
  {
    _time_t2tm(timer, 1, result);

    return SUCCESS;
  }
  static char *wday_name[] = { "Sun", "Mon", "Tues", "Wed", "Thur", "Fri", "Sat" };
  static char *mon_name[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
  command error_t Time.asctime(const struct tm *tm, char *buf, int buflen)
  {
    char *out = buf;
    char *outmax = buf + buflen;
    out += snprintf(out, outmax - out, "%02d:%02d:%02d",
		    tm->tm_hour, tm->tm_min, tm->tm_sec);
#ifdef HAVE_WDAY
    out += snprintf(out, outmax - out, " %s", wday_name[tm->tm_wday]);
#endif
    out += snprintf(out, outmax - out, " %s %d %d",
		    mon_name[tm->tm_mon], tm->tm_mday, tm->tm_year);
    return SUCCESS;
  }

  command error_t Time.time(time_t *timer)
  {
    // 1/32768 seconds since last NTP response
    uint64_t o = call LocalTime64.get() - g_local_time;

    *timer = g_current_time + (time_t)(o >> 15);
    
    return SUCCESS;
  }

  default event void Time.tick() { }

  /*****************************************
   *  ParamView interface
   * save for ip stack port to tos-2.x   

  const struct Param s_Time[] = {
    { "ntp time", PARAM_TYPE_UINT32, &g_current_time },
    { "local time", PARAM_TYPE_UINT32, &g_local_time },
    { NULL, 0, NULL }
  };

  struct ParamList g_TimeList = { "time", &s_Time[0] };

  command error_t ParamView.init()
  {
    signal ParamView.add( &g_TimeList );
    return SUCCESS;
  }
  *****************************************/

}
