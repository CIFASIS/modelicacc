model matching8
  constant Integer N=500;
  Real a[N], x[N], dx[N];
equation
  der(x) = dx;
  for i in 2:N loop
    dx=a[i-1]-a[i];
  end for; 
  for i in 1:N loop
    a[i]=x[i];
  end for;
  dx=1-a[1];
end matching8;
