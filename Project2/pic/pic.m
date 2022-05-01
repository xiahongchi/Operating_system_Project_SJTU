file1='fifo.txt';
file2='ras.txt';
file3='rr.txt';
fid1 = fopen(file1,'rt');
fid2 = fopen(file2,'rt');
fid3 = fopen(file3,'rt');
colnum=187; %need to change according to file
a={};
b=zeros(colnum,2);
i=1;
%fid needs to be changed accordingly
while ~feof(fid3)    % while循环表示文件指针没到达末尾，则继续
    % 每次读取一行, str是字符串格式
  a=textscan(fid3,'%s %s');
  b(:,1)=str2num(char(a{1}));
  b(:,2)=str2num(char(a{2}));
  i=i+1;
end
fclose(fid3);
 %b(:,2)= b(:,2)-min( b(:,2));%1->0



axis([0, 190, min(b(:,2)), max(b(:,2))+2]);
set(gca, 'xtick', 0 : 5: 190);
set(gca, 'ytick', min(b(:,2)) : 1 : max(b(:,2))+1);
xlabel('运行时间/tick'), ylabel('进程序号PID');
title('RR调度Gantt图');
Number_securities = colnum;
Number_task = 10;
 
X_start_time = b(:,1)';
X_duration_time = ones(colnum,1);
Y_start_time = b(:,2);
N_job_id = [1];
rec = [0 0 0 0 ];
color=['r','g','b','c','m'];


for i = 1 : Number_securities
   rec(1) = X_start_time(i);
   rec(2) = Y_start_time(i) + 0.7;
   rec(3) = X_duration_time(i);
   rec(4) = 0.6;
  % txt=sprintf('(%d,%d)', N_job_id(i)+1, X_duration_time(i));
   rectangle('Position',rec,'LineWidth',0.5,'LineStyle','none','FaceColor','#0072BD');
   %text(X_start_time(i)+0.2,(Y_start_time(i)+1),txt,'FontWeight','Bold','FontSize',18);
end
