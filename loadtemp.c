#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <sys/sysinfo.h>

#define HWMON_TEMP	"/sys/class/hwmon/hwmon0/temp1_input"
#define HWMON_THROTTLED	"/sys/class/hwmon/hwmon1/in0_lcrit_alarm"

static void fprint_srt_time(FILE *stream, float seconds)
{
int hours=0,minutes=0;
int whole=floor(seconds);
int fraction=(seconds-whole)*1000.0;
int isec=(int)seconds;

if (seconds>60) {
 hours=(int)floor(seconds/3600.0);
 minutes=(seconds/60.0)-hours;
}
isec=isec % 60;

fprintf(stream, "%02d:%02d:%02d,%03d", hours, minutes, isec, fraction);
}

int read_int(const char *file)
{
FILE *f;
int r,tmp;

f=fopen(file, "r");
r=fscanf(f, "%d", &tmp);
if (r==0)
	return -1;
fclose(f);

return tmp;
}

#if 0
int read_load(float *la1, float *la5, float *la15)
{
FILE *f;
int r;

f=fopen("/proc/loadavg", "r");

r=fscanf(f, "%f %f %f", la1, la5, la15);

fclose(f);

return r;
}
#endif

int read_throttled()
{
return read_int(HWMON_THROTTLED);
}

float read_temp()
{
int r,tmp;
float t;
FILE *f;

f=fopen(HWMON_TEMP, "r");
r=fscanf(f, "%d", &tmp);
if (r==0)
	return -1.0f;
fclose(f);

t=(float)tmp/1000.0f;

return t;
}

int log_temp_and_load(const char *log, const char *str)
{
FILE *f, *s=NULL;
time_t t;
int tick=0;
const float lr=1.f/(1 << SI_LOAD_SHIFT);

f=fopen(log, "w");
if (!f) {
	perror("Failed to open log file for writing");
	return 1;
}

if (str) {
	s=fopen(str, "w");
	if (!s) {
		perror("Failed to open str file for writing");
		return 1;
	}
}

fprintf(f, "Tick,Time,Temp,LoadAvg1,LoadAvg5,LoadAvg15,Throttled,MemFree,MemUsed,SwapUsed\n");

while (1) {
	int th,sr;
	float temp;
	struct sysinfo info;
	unsigned long mf,mu,su;

	t=time(NULL);
	temp=read_temp();

	// XXX: Check the bits
	th=read_throttled();

	sr=sysinfo(&info);
	if (sr!=0) {
		perror("sysinfo");
		return 1;
	}

	mf=info.freeram*info.mem_unit;
	mu=(info.totalram-info.freeram)*info.mem_unit;
	su=(info.totalswap-info.freeswap)*info.mem_unit;

	printf("%d,%ld,%f,%.2f,%.2f,%.2f,%d,%lu,%lu,%lu\n",
		tick, t,
		temp,
		info.loads[0]*lr, info.loads[1]*lr, info.loads[2]*lr,
		th>0 ? 1 : 0,
		mf, mu,	su);

	int r=fprintf(f, "%d,%ld,%.4f,%.2f,%.2f,%.2f,%d,%lu,%lu,%lu\n",
		tick, t, temp,
		info.loads[0]*lr, info.loads[1]*lr, info.loads[2]*lr,
		th>0 ? 1 : 0,
		mf, mu,	su);

	if (r<0) {
		perror("Failed to write to log file");
		return 1;
	}
	fflush(f);

	if (s) {
		fprintf(s, "%d\n", tick);
		fprint_srt_time(s, (float)tick);
		fprintf(s, " --> ");
		fprint_srt_time(s, (float)tick+1.0);
		fprintf(s, "\n%.2fC (%.2f, %.2f, %.2f) T:%d\nFree: %lu, Used: %lu, Swap: %lu\n\n",
		temp, info.loads[0]*lr, info.loads[1]*lr, info.loads[2]*lr,
		th>0 ? 1 : 0, mf/1024, mu/1024, su/1024);

		fflush(s);
	}

	tick++;

	sleep(1);
}

fclose(f);
if (s)
	fclose(s);

return 0;
}

int main(int argc, char **argv)
{
if (argc==1)
	return 2;

return log_temp_and_load(argv[1], argc>2 ? argv[2] : NULL);
}
