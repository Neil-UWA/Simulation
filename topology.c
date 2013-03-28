/**
* @file file.c
* @brief  a program used to generate the topology file 
* @author JINANG LU
* @date 2013-03-22
*/

#include <stdio.h>
#include <stdlib.h>

int main(int argc, const char *argv[])
{
	FILE	*fp	=	NULL;
	FILE	*ap =   NULL;

	char	buffer[10];
	char	topology[] = "SIMULATION";
	char	apFile[] = "ap.map";
	int		x = 0;
	int		y = 0;
	int		count = 0;


	if (fp = fopen(topology, "w")) {
		fprintf(fp, "compile = \"simulation.c mapping.c walking.c accesspoint.c client.c common.c  coverage.c am.c -lm\"\n\n");
		fprintf(fp, "rebootargs = \"uwa1.map\"\n\n");
		fprintf(fp, "drawlinks = true\n");
		fprintf(fp, "mapwidth = 600\nmapheight=300\n");
		fprintf(fp, "icontitle=\"\%\%a\"\n");
		fprintf(fp, "mapscale = 0.3\n");
		fprintf(fp, "mapgrid = 1\n\n");

		if (ap = fopen(apFile, "r")) {
			while (fgets(buffer, sizeof(buffer), ap)) {
				sscanf(buffer, "%d\t%d", &x, &y);
				fprintf(fp, "accesspoint uwa%d {\nx = %d, y = %d\nwlan{}\n}\n", count, x, y);
				count++;
			}
		}
		else {
			printf("reading AP topology file failed\n");
		}

		fprintf(fp, "\n");

		for (int i = count; i < 100; i++) {
			fprintf(fp, "mobile phone%d {wlan{}}\n", i);
		}
	}
	else
		printf("Error happens on openning File %s\n", topology);

	fclose(fp);
	fclose(ap);
	return 0;
}
