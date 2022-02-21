model matching5
  //Model with a large algebraic loop
  constant Integer N=1000;
  Real a[N], x[N];
equation
  a[1] = x[1] - a[N];
  der(x[1]) = 1 - a[1];
  for i in 2:N loop
    a[i] = a[i - 1] - x[i];
    der(x[i]) = a[i];
  end for; 
end matching5;
