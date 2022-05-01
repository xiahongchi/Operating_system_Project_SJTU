#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    FILE* fp;
    char* filepath = "./cmp.txt";
    int choice;
    int cycle = 0;
    int running_cycle[9] = {0};
    int marked[9] = {0};
    int start_time[9] = {0};
    int end_time[9] = {0};
    int pid[9] = {0};

    double avt = 0, awt = 0;
    if ((fp = fopen(filepath, "r")) == NULL) {
        printf("invalid filepath.");
        exit(0);
    }
    printf(
        "please input which scheduling algorithm to trace: 1-FIFO, 2-RR, "
        "3-RAS: ");
    scanf("%d", &choice);
    while (true) {
        char buf[500];
        buf[0] = fgetc(fp);
        if (feof(fp))
            break;
        int k = 1;
        while ((buf[k++] = fgetc(fp)) != '\n')
            ;
        buf[k] = '\0';
        switch (choice) {
            case 1:
                if (strncmp("FIFO tracing: ", buf, strlen("FIFO tracing: ")) ==
                    0) {
                    int i;
                    for (i = 0; i < 9; i++) {
                        if (marked[i] && pid[i] == atoi(buf + 18)) {
                            end_time[i] = cycle + 1;
                            running_cycle[i]++;
                            break;
                        }
                    }
                    if (i == 9) {
                        for (i = 0; i < 9; i++) {
                            if (!marked[i]) {
                                pid[i] = atoi(buf + 18);
                                end_time[i] = cycle + 1;
                                running_cycle[i]++;
                                marked[i] = 1;
                                break;
                            }
                        }
                    }
                    printf("%d\t%d\n", cycle++, atoi(buf + 18));
                }
                break;
            case 2:
                if (strncmp("RR tracing: ", buf, strlen("RR tracing: ")) == 0) {
                    char* p = strchr(buf + 16, ',');
                    *p = '\0';
                    int i;
                    for (i = 0; i < 9; i++) {
                        if (marked[i] && pid[i] == atoi(buf + 16)) {
                            end_time[i] = cycle + 1;
                            running_cycle[i]++;
                            break;
                        }
                    }
                    if (i == 9) {
                        for (i = 0; i < 9; i++) {
                            if (!marked[i]) {
                                pid[i] = atoi(buf + 16);
                                end_time[i] = cycle + 1;
                                running_cycle[i]++;
                                marked[i] = 1;
                                break;
                            }
                        }
                    }
                    printf("%d\t%d\n", cycle++, atoi(buf + 16));
                }
                break;
            case 3:
                if (strncmp("RAS Tracing: pid=", buf,
                            strlen("RAS Tracing: pid=")) == 0) {
                    char* p = strchr(buf + 17, ' ');
                    *p = '\0';
                    int i;
                    for (i = 0; i < 9; i++) {
                        if (marked[i] && pid[i] == atoi(buf + 17)) {
                            end_time[i] = cycle + 1;
                            running_cycle[i]++;
                            break;
                        }
                    }
                    if (i == 9) {
                        for (i = 0; i < 9; i++) {
                            if (!marked[i]) {
                                pid[i] = atoi(buf + 17);
                                end_time[i] = cycle + 1;
                                running_cycle[i]++;
                                marked[i] = 1;
                                break;
                            }
                        }
                    }
                    printf("%d\t%d\n", cycle++, atoi(buf + 17));
                }
                break;
            default:
                break;
        }
    }

    for (int i = 0; i < 9; i++) {
        printf("pid=%d, arrival time=%d, terminate time=%d, running time=%d\n",
               pid[i], start_time[i], end_time[i], running_cycle[i]);
        avt += (end_time[i] - start_time[i]);
    }
    for (int i = 0; i < 9; i++) {
        awt += running_cycle[i];
    }
    awt = avt - awt;
    avt /= 9;
    awt /= 9;
    printf("avarage turnaround time=%.2f\navarage waiting time=%.2f\n", avt,
           awt);
    fclose(fp);
    system("pause");
    return 0;
}