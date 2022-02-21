model matching4
  //Model with small algebraic loops and with connections between different indices in one direction
  constant Integer N= 10000;
  Real a[N], b[N], x[N];
equation
  a[1] = 2 * x[1] - b[1];
  a[1] = 2 * b[1] - x[1];
  der(x[1]) = 1 - a[1];
  for i in 2:N loop
    a[i] = 2 * x[i] - b[i];
    a[i] = 2 * b[i] - x[i];
    der(x[i]) = a[i-1] - a[i];
  end for;  
end test4;
