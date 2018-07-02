#include <time.h>
#include <unistd.h>
#include <stdio.h>

int read_load(float *la1, float *la5, float *la15)
{
FILE *f;
int r;

f=fopen("/proc/loadavg", "r");

r=fscanf(f, "%f %f %f", la1, la5, la15);

fclose(f);

return r;
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

f=fopen(log, "w");
if (!f) {
	perror("Failed to open log file for writing");
	return;
}

fprintf(f, "Time,Temp,LoadAvg1,LoadAvg5,LoadAvg15\n");

while (1) {
	float temp,a1,a5,a15;

	t=time(NULL);
	temp=read_temp();
	read_load(&a1, &a5, &a15);

	printf("%ld,%f,%.2f,%.2f,%.2f\n", t, temp, a1, a5, a15);

	int r=fprintf(f, "%ld,%.4f,%.2f,%.2f,%.2f\n", t, temp, a1, a5, a15);
	if (r<0) {
		perror("Failed to write to log file");
		return;
	}
	fflush(f);

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
