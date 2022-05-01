For both the two problems:
goldfish/ directory is the kernel files that have been modified or created.

For Page Tracing:
1. page-trace/ is the directory that the three system call sys_start_trace(), sys_stop_trace() and sys_get_trace() modules lie in.
2. page-trace-test/ is the directory of test file "page_trace_test.c" for page trace realization.

For RAS scheduler:
1. ras_page_trace_test/ is the directory of test file "ras_pt_test.c" for RAS scheduler.
2. kernel_log_cmp.txt is the kernel log that records when the test is doing.

For Extended Part:
1. rr_fifo_trace/ is the directory that the sys_rr_fifo_trace() system call module lies in.
2. set_wcounts/ is the directory that the sys_set_wcounts() system call module lies in.
3. ras_trace_bg/ is the directory of test file "ras_trace_bg.c" for background scheduling.
4. cmp/ is the directory of the program that process the kernel log data to help measure efficiency of RR, FIFO and RAS.
5. pic/ is the directory of a matlab program that draw the gantt graph of RR, FIFO and RAS scheduling.
6. processtest.apk is the application program to test background process. It's provided by TA Hang Zeng.

For report:
report.pdf is the report of this Project.
report.md is the source code of the project report.
image/ contains the graphs used in report.

