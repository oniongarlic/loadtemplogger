#include <time.h>
#include <unistd.h>
#include <stdio.h>

#include <sys/sysinfo.h>

#define FIRMWARE_THROTTLED "/sys/devices/platform/soc/soc:firmware/get_throttled"

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
return read_int(FIRMWARE_THROTTLED);
}

float read_temp()
{
int r,tmp;
float t;
FILE *f;

f=fopen("/sys/devices/virtual/thermal/thermal_zone0/temp", "r");
r=fscanf(f, "%d", &tmp);
if (r==0)
	return -1.0f;
fclose(f);

t=(float)tmp/1000.0f;

return t;
}

int log_temp_and_load(char *log)
{
FILE *f;
time_t t;
int tick=0;
const float lr=1.f/(1 << SI_LOAD_SHIFT);

f=fopen(log, "w");
if (!f) {
	perror("Failed to open log file for writing");
	return 1;
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

	tick++;

	sleep(1);
}

fclose(f);

return 0;
}

int main(int argc, char **argv)
{
if (argc==1)
	return 2;

return log_temp_and_load(argv[1]);
}
