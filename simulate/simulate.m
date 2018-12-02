% Clean all 
close all;
clear all;
clc;

function retval = normal_delay (r,c)
  disp("===== normal_delay =====" );
  
  if (c < 1000)
        error ("Low sample number, limit is 1k!");
  endif
  retval = stdnormal_rnd(r,c); % (mean = 0, standard deviation = 1)
  figure;
  clf();
  plot(retval);
  title ("normal");        
  grid on;

  figure;
  clf();
  hist(retval,fix([c*0.01]));
  title ("normal histogram");      
  grid on;
  
  disp(mean(retval));
  disp("===== normal_delay end =====");
    
endfunction


function retval = uniform_delay (start,finish,r,c)
  disp("===== uniform_delay =====" );
  
  if (c < 1000)
        error ("Low sample number, limit is 1k!");
  endif
  retval = unifrnd(start,finish,r,c);
  figure;
  clf();
  plot(retval);
  title ("uniform");    
  grid on;

  figure;
  clf();
  hist(retval,fix([c*0.01]));
  title ("uniform histogram");    
  grid on;
  
  disp(mean(retval));
  disp("===== uniform_delay end =====");
    
endfunction



function retval = exponential_delay (lambda,r,c)
  disp("===== exponential_delay =====" );
  
  if (c < 1000)
        error ("Low sample number, limit is 1k!");
  endif
  retval = exprnd(lambda,r,c);
  figure;
  clf();
  plot(retval);
  title ("exponential");  
  grid on;

  figure;
  clf();
  hist(retval,fix([c*0.01]));
  title ("exponential histogram");  
  grid on;
  
  disp(mean(retval));
  disp("===== exponential_delay end =====");
    
endfunction


function retval = poisson_delay (lambda,r,c)
  disp("===== poisson_delay =====" );
  
  if (c < 1000)
        error ("Low sample number, limit is 1k!");
  endif
  retval = poissrnd(lambda,r,c);
  figure;
  clf();
  plot(retval);
  title ("poisson")
  grid on;

  figure;
  clf();
  hist(retval,fix([c*0.01]));
  title ("poisson histogram")
  grid on;
  
  disp(mean(retval));
  disp("===== poisson_delay end =====");
    
endfunction


function retval = pareto_delay (shape,scale,r,c)
  disp("===== pareto_delay =====" );
  
  if (c < 1000)
        error ("Low sample number, limit is 1k!");
  endif
  
  inputArray = power(1 - rand(r,c),-1);
  
  inputArray = power(inputArray,1/scale);
  
  res = inputArray * shape;
  
  figure;
  clf();
  plot(res);
  title ("pareto");
  grid on;

  figure;
  clf();
  hist(res,fix([c*0.01]));
  title ("pareto histogram");
  grid on;
  
  disp(mean(res));
  disp("===== pareto_delay end =====");
    
endfunction

function retval = paretoXm_delay (shape,scale,r,c)
  disp("===== paretoXmin_delay =====" );
  
  if (c < 10)
        error ("Low sample number, limit is 1k!");
  endif
  


  inArr = rand(r,c);
  inArr
  inArr = power(inArr,shape+1);
  inArr = power(inArr, -1);
  f = shape * power(scale,shape);  
  res = f * inArr;
  
  figure;
  clf();
  plot(res);
  title ("pareto Xmin");
  grid on;

  figure;
  clf();
  hist(res,fix([c*0.01]));
  title ("pareto Xmin histogram");
  grid on;
  
  disp(mean(res));
  disp("===== paretoXmin_delay end =====");
    
endfunction


disp("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%" );
disp("===== Simulation Example =====" );
disp("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%" );

Samples = 100000;

%norm = normal_delay     (1,Samples);
%pause(2);
%uniform_delay    (1,10,1,Samples);
%pause(2);
exponential_delay(10000000.0,1,Samples); % λ > 0
%pause(2);
%poisson_delay    (10.0,1,Samples); % λ > 0
%pause(2);
%pareto_delay     (600,1000000,1,Samples); % (shape,scale,r,c)
%pause(2);
%parexm = paretoXm_delay     (6000000,1000000,1,Samples); % (shape,scale,r,c)
pause(2);


disp("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%" );
disp(" == Simulation Example End == " );
disp("%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%" );



