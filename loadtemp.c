#include <time.h>
#include <unistd.h>
#include <stdio.h>

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

int read_load(float *la1, float *la5, float *la15)
{
FILE *f;
int r;

f=fopen("/proc/loadavg", "r");

r=fscanf(f, "%f %f %f", la1, la5, la15);

fclose(f);

return r;
}

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

void log_temp_and_load(char *log)
{
FILE *f;
time_t t;
int tick=0;

f=fopen(log, "w");
if (!f) {
	perror("Failed to open log file for writing");
	return;
}

fprintf(f, "Tick,Time,Temp,LoadAvg1,LoadAvg5,LoadAvg15,Throttled\n");

while (1) {
	int th;
	float temp,a1,a5,a15;

	t=time(NULL);
	temp=read_temp();
	read_load(&a1, &a5, &a15);

	// XXX: Check the bits
	th=read_throttled();

	printf("%d,%ld,%f,%.2f,%.2f,%.2f,%d\n", tick, t, temp, a1, a5, a15, th>0 ? 1 : 0);

	int r=fprintf(f, "%d,%ld,%.4f,%.2f,%.2f,%.2f,%d\n", tick, t, temp, a1, a5, a15, th>0 ? 1 : 0);
	if (r<0) {
		perror("Failed to write to log file");
		return;
	}
	fflush(f);

	tick++;

	sleep(1);
}

fclose(f);
}

int main(int argc, char **argv)
{
if (argc==1)
	return 1;

log_temp_and_load(argv[1]);

return 0;
}
