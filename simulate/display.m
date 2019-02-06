close all;
clear all;
clf();
source('PackGen_1500_stats_test_1.mat');


b=vector';

index=b([1],1:end);
delay=b([2],1:end);
inter=b([3],1:end);

% Convert to Milis
delay=delay/1000;

% Get Figures
plot(delay);
grid on;
title ("Packet Travel Time");
xlabel ("packet No");
ylabel ("Delay (milis)");
hold on;

figure;
plot(inter);
grid on;
title ("Packet Interarrival Time");
xlabel ("packet No");
ylabel ("Delay (microseconds)");
hold on;

figure;
clf();
plot(index);
grid on;
title  ("Index");
xlabel ("packet No");
ylabel ("Packet index");
hold on;

figure;
clf();
hist(delay,200)
grid on;
title  ("Delay Histogram");
xlabel ("Delay (ms)");
ylabel ("Occurrences");
hold on;

figure;
clf();
hist(inter,200)
grid on;
title  ("Interarrival Delay Histogram");
xlabel ("Delay (usec)");
ylabel ("Occurrences");
hold on;



% Get Mean
mean(delay)

mean(inter)